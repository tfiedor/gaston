/*****************************************************************************
 *  gaston - no real logic behind the name, we simply liked the poor seal gaston. R.I.P. brave soldier.
 *
 *  Copyright (c) 2015  Tomas Fiedor <ifiedortom@fit.vutbr.cz>
 *      Notable mentions: Ondrej Lengal <ondra.lengal@gmail.com>
 *          			  Overeating Panda <if-his-simulation-reduction-works>
 *
 *****************************************************************************/

#define _LANGUAGE_C_PLUS_PLUS

// < System Headers >
#include <iostream>
#include <new>
#include <sys/time.h>
#include <sys/resource.h>
#include <signal.h>
#include <map>

// < VATA Headers >
#include <vata/bdd_bu_tree_aut.hh>
#include <vata/parsing/timbuk_parser.hh>
#include <vata/serialization/timbuk_serializer.hh>
#include <vata/util/binary_relation.hh>

// < MONA Frontend Headers >
#include "Frontend/env.h"
#include "Frontend/untyped.h"
#include "Frontend/predlib.h"
#include "Frontend/ast.h"
#include "Frontend/ast_visitor.h"
#include "Frontend/code.h"
#include "Frontend/st_dfa.h"
#include "Frontend/st_gta.h"
#include "Frontend/timer.h"
#include "Frontend/lib.h"
#include "Frontend/printline.h"
#include "Frontend/config.h"
#include "Frontend/offsets.h"
#include "Frontend/ident.h"

// < dWiNA Headers >
#include <DecisionProcedure/environment.hh>
#include <DecisionProcedure/decision_procedures.hh>
#include <DecisionProcedure/containers/VarToTrackMap.hh>
#include "DecisionProcedure/containers/Cache.hh"
#include "DecisionProcedure/visitors/BooleanUnfolder.h"
#include "DecisionProcedure/visitors/UniversalQuantifierRemover.h"
#include "DecisionProcedure/visitors/SecondOrderRestricter.h"

// < Typedefs and usings >
using std::cout;
using StateToStateTranslator = VATA::AutBase::StateToStateTranslWeak;
using StateToStateMap        = std::unordered_map<StateType, StateType>;
using Automaton 			 = VATA::BDDBottomUpTreeAut;

typedef unsigned int uint;

// < Global variables >
Options options;
MonaUntypedAST *untypedAST;
SymbolTable symbolTable(1019);
PredicateLib predicateLib;
Offsets offsets;
CodeTable *codeTable;
Guide guide;
AutLib lib;
VarToTrackMap varMap;
IdentList inFirstOrder;
int numTypes = 0;
bool regenerate = false;

#if (USE_STATECACHE == true)
MultiLevelMCache<bool> StateCache;
#endif
#if (USE_BDDCACHE == true)
MultiLevelMCache<MacroTransMTBDD> BDDCache;
#endif

extern int yyparse(void);
extern void loadFile(char *filename);
extern Deque<FileSource *> source; 

char *inputFileName = NULL;

extern Ident lastPosVar, allPosVar;

/**
 * Prints usage of the program to the standard output.
 */
void PrintUsage()
{
	cout << "Usage: gaston [options] <filename>\n\n"
		<< "Options:\n"
		<< " -t, --time 		 Print elapsed time\n"
		<< " -d, --dump-all		 Dump AST, symboltable, and code DAG\n"
		<< "     --no-automaton  	 Don't dump Automaton\n"
		<< "     --use-mona-dfa  	 Uses MONA for building base automaton\n"
		<< "     --no-expnf      	 Implies --use-mona-dfa, does not convert formula to exPNF\n"
		<< " -q, --quiet		 Quiet, don't print progress\n"
		<< " -oX                 	 Optimization level [1 = safe optimizations [default], 2 = heuristic]\n"
		<< " --method            	 Use either symbolic (novel), forward (EEICT'14) or backward method (TACAS'15) for deciding WSkS [symbolic, backward, forward]\n"
		//<< " --reorder-bdd		 Disable BDD index reordering [no, random, heuristic]\n\n"
		<< "Example: ./gaston -t -d --reorder-bdd=random foo.mona\n\n";
}

/**
 * Parses input arguments into options
 *
 * @param argc: number of arguments
 * @param argv: list of arguments
 * @return: false if arguments are wrong
 */
bool ParseArguments(int argc, char *argv[])
{
	options.printProgress = true;
	options.analysis = true;
	options.optimize = 1;
	options.dontDumpAutomaton = false;
	options.reorder = HEURISTIC; //true;

	switch (argc) {
	// missing file with formula
	case 1:
		return false;

	// missing file with formula, some option stated
	case 2:
		if (argv[1][0] == '-')
			return false;

	default:
		for (int i = 1; i < argc - 1; i++) {
			if (argv[i][0] != '-')
				return false;
			if (strcmp(argv[i], "--dump-all") == 0)
				options.dump = true;
			else if(strcmp(argv[i], "--time") == 0)
				options.time = true;
			else if(strcmp(argv[i], "--quiet") == 0)
				options.printProgress = false;
			else if(strcmp(argv[i], "--reorder-bdd=no") == 0)
				options.reorder = NO;
			else if(strcmp(argv[i], "--reorder-bdd=random") == 0)
				options.reorder = RANDOM;
			else if(strcmp(argv[i], "--reorder-bdd=heuristic") == 0)
				options.reorder = HEURISTIC;
			else if(strcmp(argv[i], "--no-automaton") == 0)
				options.dontDumpAutomaton = true;
			else if(strcmp(argv[i], "--use-mona-dfa") == 0)
				options.useMonaDFA = true;
			else if(strcmp(argv[i], "--method=forward") == 0)
				options.method = Method::FORWARD;
			else if(strcmp(argv[i], "--method=backward") == 0)
				options.method = Method::BACKWARD;
			else if(strcmp(argv[i], "--method=symbolic") == 0)
				options.method = Method::SYMBOLIC;
			else if(strcmp(argv[i], "--no-expnf") == 0) {
				options.noExpnf = true;
				options.useMonaDFA = true;
			} else {
				switch (argv[i][1]) {
					case 'd':
						options.dump = true;
						break;
					case 't':
						options.time = true;
						break;
					case 'q':
						options.printProgress = false;
						break;
					case 'o':
						options.optimize = argv[i][2] - '0';
						break;
					default:
						return false;
				}
			}
		}
	}

	inputFileName = argv[argc-1];
	return true;
}

/**
 * Splits input formula into prefix and matrix, i.e. chain of quantifiers
 * followed by quantifier free formula. Matrix is used for conversion to
 * automaton and prefix is used for on-the-fly construction.
 *
 * @param formula: Input WSkS formula, that will be split
 * @param matrix: output part of the formula - matrix, i.e. quantifier free
 * 		formula
 * @param prefix: output part of the formula - prefix, i.e. chain of quantifiers
 */
void splitMatrixAndPrefix(MonaAST* formula, ASTForm* &matrix, ASTForm* &prefix) {
	ASTForm* formIter = formula->formula;
	ASTForm* previous = 0;

	matrix = formula->formula;
	prefix = formula->formula;

	/**
	 * While we didn't encounter an atomic formula we iterate through quantifiers
	 * and negations and construct the prefix
	 */
	while(true) {
		if(formIter->kind == aEx2) {
			previous = formIter;
			formIter = ((ASTForm_Ex2*) formIter)->f;
		} else if (formIter->kind == aNot && (((ASTForm_Not*)formIter)->f)->kind == aEx2) {
			previous = formIter;
			formIter = ((ASTForm_Not*) formIter)->f;
		} else {
			if (previous != 0) {
				// use the True as the last formula
				if (previous->kind == aEx2) {
					ASTForm_Ex2* q = (ASTForm_Ex2*) previous;
					q->f = new ASTForm_True(q->pos);
				} else {
					ASTForm_Not* q = (ASTForm_Not*) previous;
					q->f = new ASTForm_True(q->pos);
				}
			} else {
				prefix = new ASTForm_True(Pos());
			}
			matrix = formIter;
			break;
		}
	}
}

/**
 * No reordering
 *
 * @param free: list of free variables
 * @param bound: list of bounded variables
 */
void noReorder(IdentList *free, IdentList *bound) {
	IdentList *vars = ident_union(free, bound);
	if (vars != 0) {
		varMap.initializeFromList(vars);
	}
}

/**
 * Does reordering of variables so in output BDD tracks it is easier to remove
 * the track and reorder the BDD afterwards. This is done by the prefix of
 * given formula
 *
 * @param free: list of free variables
 * @param bound: list of bounded variables
 */
void heuristicReorder(IdentList *free, IdentList *bound) {
	if(options.dump) {
		cout << "[*] Variables reordered by heuristic approach" << std::endl;
	}
	varMap.initializeFromLists(free, bound);
}

/**
 * Randomly shuffles list of vars
 *
 * @param[in] list: list to be shuffled
 * @return: shuffled list
 */
IdentList* shuffle(IdentList* list) {
	// TODO: not implemented yet
	return list;
}

/**
 * Does random reordering of variables
 *
 * @param free: list of free variables
 * @param bound: list of bounded variables
 */
void randomReorder(IdentList *free, IdentList *bound) {
	if(options.dump) {
		cout << "[*] Variables reorder randomly" << std::endl;
	}
	IdentList *vars = ident_union(free, bound);
	// TODO: Not implemented, not needed at all
	if (vars != 0) {
		vars = shuffle(vars);
		varMap.initializeFromList(vars);
	}
}

/**
 * Implementation is not sure at the moment, but should reorder the symbol
 * table or BDD track, so it is optimized for using of projection during
 * the decision procedure process. It should consider the structure of prefix
 * of given formula, so BDD used in transitions of automata can be better
 * reordered
 *
 * Several heuristics will be tried out:
 *  1) Random reorder
 *  2) No reorder
 *  3) Prefix-reorder
 */
void reorder(ReorderMode mode, ASTForm* formula) {
	IdentList free, bound;
	formula->freeVars(&free, &bound);

	if (mode == NO)
		noReorder(&free, &bound);
	else if (mode == HEURISTIC) {
		heuristicReorder(&free, &bound);
	} else if (mode == RANDOM) {
		randomReorder(&free, &bound);
	}
}

int main(int argc, char *argv[])
{
	/* Parse initial arguments */
	if (!ParseArguments(argc, argv)) {
		PrintUsage();
		exit(-1);
	}

	/* Initialization of timer used for statistics */
	initTimer();
	Timer timer_total, timer_formula, timer_automaton, timer_deciding;
	timer_total.start();

	///////// PARSING ////////////////////////////////////////////////////////
	Timer timer_parsing;
	timer_parsing.start();

	loadFile(inputFileName);
	yyparse();
	MonaAST *ast = untypedAST->typeCheck();
	lastPosVar = ast->lastPosVar;
	allPosVar = ast->allPosVar;

	timer_parsing.stop();

	// Prints progress if dumping is set
	if (options.printProgress) {
		cout << "[*] Parsing input formula " << inputFileName << "\n";
		cout << "[*] Elapsed time: ";
		timer_parsing.print();
	}

	delete untypedAST;

	if (options.dump) {
	// Dump AST for main formula, verify formulas, and assertion
		cout << "[*] Main formula:\n";
		(ast->formula)->dump();
	}

	// INSERT FUN HERE
	cout << "\n[*] Main formula:\n";
	(ast->formula)->dump();
	BooleanUnfolder bu_visitor;
	ast->formula = static_cast<ASTForm*>((ast->formula)->accept(bu_visitor));
	cout << "\n[*] Main formula after Boolean unfolding:\n";
	(ast->formula)->dump();
	cout << "\n";

	timer_formula.start();
	if(options.noExpnf == false) {
	// Flattening of the formula
	try {
		ast->formula = (ASTForm*) (ast->formula)->toSecondOrder();
	} catch (NotImplementedException e) {
		cout << "[!] Formula is 'UNSUPPORTED'\n";
		return 0;
	}
		if(options.dump) {
			cout << "\n\n[*] Flattened formula:\n";
			(ast->formula)->dump();
		}

		// Transform AST to existentional Prenex Normal Form
		ast->formula = (ASTForm*) (ast->formula)->toExistentionalPNF();
	} else {
		if(ast->formula->kind == aAnd) {
			ASTForm_And* andFormula = (ASTForm_And*) ast->formula;
			if(andFormula->f1->kind == aTrue) {
				ast->formula = static_cast<ASTForm*>(andFormula->f2);
			} else if(andFormula->f2->kind == aTrue) {
			  ast->formula = static_cast<ASTForm*>(andFormula->f1);
			}
		}

		UniversalQuantifierRemover uqr_visitor;
		SecondOrderRestricter sor_visitor;
		ast->formula = static_cast<ASTForm*>(ast->formula->accept(uqr_visitor));
		ast->formula = static_cast<ASTForm*>(ast->formula->accept(sor_visitor));
	}
	timer_formula.stop();

	if(options.dump) {
		cout << "\n\n[*] Formula in exPNF:\n";
		(ast->formula)->dump();

		// dumping symbol table
		cout << "\n\n[*] Created symbol table:";
		symbolTable.dump();
		cout << "\n";

		// Dump ASTs for predicates and macros
		PredLibEntry *pred = predicateLib.first();
		while (pred != NULL) {
			if (pred->isMacro)
				cout << "\n[*] Dumping Macro '";
			else
				cout << "\n[*] Dumping Predicate '";
			cout << symbolTable.lookupSymbol(pred->name)
			<< "':\n";
			(pred->ast)->dump();
			cout << "\n";
			pred = predicateLib.next();
		}

		cout << "[*] Input file transformed into formula in Existential Prenex Normal Form\n";
		cout << "[*] Elapsed time: ";
		timer_formula.print();
		cout << "\n";
	}

	///////// Conversion to Tree Automata ////////

	// Table or BDD tracks are reordered
	reorder(options.reorder, ast->formula);
#if (DEBUG_VARIABLE_SETS == true)
	varMap.dumpMap();
	std::cout << "\n";
#endif

	IdentList freeVars, bound;
	(ast->formula)->freeVars(&freeVars, &bound);

	bool formulaIsGround = freeVars.empty();

#if (DEBUG_VARIABLE_SETS == true)
	std::cout << "Free Vars:\n";
	freeVars.dump();
	std::cout << "\nBound:\n";
	bound.dump();
	std::cout << "\n";
#endif

	// First formula in AST representation is split into matrix and prefix part.
	ASTForm *matrix, *prefix;
	splitMatrixAndPrefix(ast, matrix, prefix);
	bool topmostIsNegation = (prefix->kind == aNot);
	if(options.noExpnf == false) {
		matrix = matrix->restrictFormula();
	}

	if(options.dump) {
		std::cout << "[*] Dumping restricted matrix\n";
		matrix->dump();
		std::cout << "\n";
	}

	// Transform prefix to set of sets of second-order variables
	PrefixListType plist = convertPrefixFormulaToList(prefix);
	PrefixListType nplist(plist);

#if (DEBUG_FORMULA_PREFIX == true)
	std::cout << "[?] Prefixes before closing\n";
	for(auto it = plist.begin(); it != plist.end(); ++it) {
		std::cout << "[";
		for(auto itt = (*it).begin(); itt != (*it).end(); ++itt) {
			std::cout << (*itt) << ", ";
		}
		std::cout << "] ";
	}
	std::cout << "\n";
	for(auto it = nplist.begin(); it != nplist.end(); ++it) {
		std::cout << "[";
		for(auto itt = (*it).begin(); itt != (*it).end(); ++itt) {
			std::cout << (*itt) << ", ";
		}
		std::cout << "] ";
	}
	std::cout << "\n";
#endif

	// If formula is not ground, we close it
	if(freeVars.size() != 0) {
		closePrefix(plist, &freeVars, topmostIsNegation);
		closePrefix(nplist, &freeVars, (prefix->kind != aNot));
		topmostIsNegation = false;
	}

#if (DEBUG_FORMULA_PREFIX == true)
	std::cout << "[?] Prefixes after closing\n";
	for(auto it = plist.begin(); it != plist.end(); ++it) {
		std::cout << "[";
		for(auto itt = (*it).begin(); itt != (*it).end(); ++itt) {
			std::cout << (*itt) << ", ";
		}
		std::cout << "] ";
	}
	std::cout << "\n";
	for(auto it = nplist.begin(); it != nplist.end(); ++it) {
		std::cout << "[";
		for(auto itt = (*it).begin(); itt != (*it).end(); ++itt) {
			std::cout << (*itt) << ", ";
		}
		std::cout << "] ";
	}
	std::cout << "\n";
#endif

	Automaton formulaAutomaton;
	timer_automaton.start();
	// Use mona for building automaton instead of VATA
	// -> this may fail on insufficient memory
	if(options.useMonaDFA) {
		std::cout << "[*] Using MONA DFA (with minimizations) to build base automaton\n";

		// First code is generated, should be in DAG
		codeTable = new CodeTable;
		VarCode formulaCode = matrix->makeCode();

		DFA *dfa = 0;

		// Initialization
		bdd_init();
		codeTable->init_print_progress();

		dfa = formulaCode.DFATranslate();
		formulaCode.remove();

		// unrestrict automata
		DFA *temp = dfaCopy(dfa);
		dfaUnrestrict(temp);
		dfa = dfaMinimize(temp);
		dfaFree(temp);

		// some freaking crappy initializations
		IdentList::iterator id;
		int ix = 0;
		int numVars = varMap.TrackLength();
		char **vnames = new char*[numVars];
		unsigned *offs = new unsigned[numVars];

		IdentList free, bounded;
		matrix->freeVars(&free, &bounded);
		IdentList* vars = ident_union(&free, &bounded);

		// iterate through all variables
		for (id = vars->begin();id != vars->end(); id++, ix++) {
			vnames[ix] = symbolTable.lookupSymbol(*id);
			offs[ix] = offsets.off(*id);
		}

		convertMonaToVataAutomaton(formulaAutomaton, dfa, vars, numVars, offs);
		// Build automaton by ourselves, may build huge automata
	} else {
		// WS1S formula is transformed to unary NTA
		if(options.mode != TREE) {
			matrix->toUnaryAutomaton(formulaAutomaton, false);
		// WS2S formula is transformed to binary NTA
		} else {
			matrix->toBinaryAutomaton(formulaAutomaton, false);
		}
	}
	timer_automaton.stop();

	if(options.dump) {
		std::cout << "[*] Formula transformed into non-deterministic tree automaton\n";
		cout << "[*] Elapsed time: ";
		timer_automaton.print();
		cout << "\n";
	}

	// reindex the states, for space optimizations for bitsets
	StateHT reachable;
	formulaAutomaton = formulaAutomaton.RemoveUnreachableStates(&reachable);

	StateType stateCnt = 0;
	StateToStateMap translMap;
	StateToStateTranslator stateTransl(translMap,
									   [&stateCnt](const StateType&){return stateCnt++;});

	formulaAutomaton = formulaAutomaton.ReindexStates(stateTransl);
	TStateSet::stateNo = reachable.size();

	if(options.dump) {
		std::cout<< "[*] Number of states in resulting automaton: " << TStateSet::stateNo << "\n";
	}

	// Dump automaton
	if(options.dump && !options.dontDumpAutomaton) {
		VATA::Serialization::AbstrSerializer* serializer = new VATA::Serialization::TimbukSerializer();
		std::cerr << formulaAutomaton.DumpToString(*serializer, "symbolic") << "\n";
		//std::cout << formulaAutomaton.DumpToDot() << "\n";
		delete serializer;
	}

#if (DEBUG_BDDS == true)
	StateHT allStates;
	formulaAutomaton.RemoveUnreachableStates(&allStates);
	TransMTBDD * tbdd = getMTBDDForStateTuple(formulaAutomaton, Automaton::StateTuple({}));
	std::cout << "Leaf : bdd\n";
	std::cout << TransMTBDD::DumpToDot({tbdd}) << "\n\n";
	// Dump bdds
	for (auto state : allStates) {
		TransMTBDD* bdd = getMTBDDForStateTuple(formulaAutomaton, Automaton::StateTuple({state}));
		std::cout << state << " : bdd\n";
		std::cout << TransMTBDD::DumpToDot({bdd}) << "\n\n";
	}
#endif

	///////// DECISION PROCEDURE /////////////////////////////////////////////
	int decided;
	try {
	// Deciding WS1S formula
		timer_deciding.start();
		try {
			if(options.mode != TREE) {
				// TODO: This should be encapsulated in some Checker Class, no time now though
				if(options.method == FORWARD) {
					decided = decideWS1S(formulaAutomaton, plist, nplist);
				} else {
					decided = decideWS1S_backwards(formulaAutomaton, plist, nplist, formulaIsGround, topmostIsNegation);
				}
			// Deciding WS2S formula
			} else {
				decided = decideWS2S(formulaAutomaton);
			}
		} catch (std::bad_alloc) {
			std::cout << "[!] Insufficient memory for deciding\n";
			decided = -1;
		}
		timer_deciding.stop();

		// Outing the results of decision procedure
		cout << "[!] Formula is ";
		switch(decided) {
			case SATISFIABLE:
				cout << "'SATISFIABLE'\n";
				break;
			case UNSATISFIABLE:
				cout << "'UNSATISFIABLE'\n";
				break;
			case VALID:
				cout << "'VALID'\n";
				break;
			default:
				cout << "undecided due to an error.\n";
				break;
		}
		cout << "[*] Decision procedure elapsed time: ";
		timer_deciding.print();
		cout << "\n";
		// Something that was used is not supported by dWiNA
	} catch (NotImplementedException& e) {
		std::cerr << e.what() << std::endl;
	}

	if(options.dump) {
		std::cout << "[*] State cache statistics:\n";
		StateCache.dumpStats();
	}

	// Prints timing
	if (options.time) {
		timer_total.stop();
		cout << "\n[*] Total elapsed time:     ";
		timer_total.print();
	} else if (options.printProgress) {
		timer_total.stop();
		cout << "\n[*] Total elapsed time: ";
		timer_total.print();
	}

	return 0;
}
