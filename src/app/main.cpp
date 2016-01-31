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
#include <fstream>
#include <new>
#include <sys/time.h>
#include <sys/resource.h>
#include <signal.h>
#include <list>
#include <map>

// < VATA Headers >
#include <vata/bdd_bu_tree_aut.hh>
#include <vata/parsing/timbuk_parser.hh>
#include <vata/serialization/timbuk_serializer.hh>
#include <vata/util/binary_relation.hh>

// < MONA Frontend Headers >
#include "Frontend/bdd.h"
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
#include "DecisionProcedure/containers/SymbolicAutomata.h"
#include "DecisionProcedure/containers/Term.h"
#include "DecisionProcedure/visitors/AntiPrenexer.h"
#include "DecisionProcedure/visitors/BooleanUnfolder.h"
#include "DecisionProcedure/visitors/UniversalQuantifierRemover.h"
#include "DecisionProcedure/visitors/SyntaxRestricter.h"
#include "DecisionProcedure/visitors/SecondOrderRestricter.h"
#include "DecisionProcedure/visitors/PrenexNormalFormTransformer.h"
#include "DecisionProcedure/visitors/Flattener.h"
#include "DecisionProcedure/visitors/PredicateUnfolder.h"
#include "DecisionProcedure/visitors/NegationUnfolder.h"
#include "DecisionProcedure/visitors/Reorderer.h"
#include "DecisionProcedure/visitors/BinaryReorderer.h"
#include "DecisionProcedure/visitors/QuantificationMerger.h"
#include "DecisionProcedure/visitors/DotWalker.h"
#include "DecisionProcedure/visitors/MonaAutomataDotWalker.h"
#include "DecisionProcedure/visitors/BaseAutomataMerger.h"
#include "DecisionProcedure/visitors/ExistentialPrenexer.h"
#include "DecisionProcedure/visitors/ZeroOrderRemover.h"
#include "DecisionProcedure/visitors/Tagger.h"
#include "DecisionProcedure/visitors/FixpointDetagger.h"

// < Typedefs and usings >
using std::cout;
using std::cerr;
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
int numTypes = 0;
bool regenerate = false;
Timer timer_conversion, timer_mona, timer_base;

extern int yyparse(void);
extern void loadFile(char *filename);
extern void (*mona_callback)();
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
		<< " -ga, --print-aut	 Print automaton in graphviz\n"
		<< "     --no-automaton  Don't dump Automaton\n"
		<< "     --use-mona-dfa  Uses MONA for building base automaton\n"
		<< "     --no-expnf      Implies --use-mona-dfa, does not convert formula to exPNF\n"
		<< "     --test          Test specified problem [val, sat, unsat]\n"
		<< "     --walk-aut      Does the experiment generating the special dot graph\n"
		<< " -e, --expand-tagged Expand automata with given tag on first line of formula\n"
		<< " -q, --quiet		 Quiet, don't print progress\n"
		<< " -oX                 Optimization level [1 = safe optimizations [default], 2 = heuristic]\n"
		<< " --method            Use either symbolic (novel), forward (EEICT'14) or backward method (TACAS'15) for deciding WSkS [symbolic, backward, forward]\n"
		<< "Example: ./gaston -t -d foo.mona\n\n";
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
			else if(strcmp(argv[i], "-ga") == 0 || strcmp(argv[i], "--print-aut") == 0)
				options.graphvizDAG = true;
			else if(strcmp(argv[i], "--walk-aut") == 0)
				options.monaWalk = true;
			else if(strcmp(argv[i], "--expand-tagged") == 0)
				options.expandTagged = true;
			else if(strcmp(argv[i], "--no-automaton") == 0)
				options.dontDumpAutomaton = true;
			else if(strcmp(argv[i], "--use-mona-dfa") == 0) {
				options.useMonaDFA = true;
				options.construction = AutomataConstruction::DETERMINISTIC_AUT;
			} else if(strcmp(argv[i], "--test=val") == 0) {
				options.test = TestType::VALIDITY;
			} else if(strcmp(argv[i], "--test=sat") == 0) {
				options.test = TestType::SATISFIABILITY;
			} else if(strcmp(argv[i], "--test=unsat") == 0) {
				options.test = TestType::UNSATISFIABILITY;
			} else if(strcmp(argv[i], "--method=forward") == 0) {
				options.method = Method::FORWARD;
			} else if(strcmp(argv[i], "--method=backward") == 0) {
				options.method = Method::BACKWARD;
			} else if(strcmp(argv[i], "--method=symbolic") == 0) {
				options.method = Method::SYMBOLIC;
				options.construction = AutomataConstruction::SYMBOLIC_AUT;
				options.noExpnf = true;
			} else if(strcmp(argv[i], "--to-expnf") == 0) {
				options.noExpnf = false;
			} else if(strcmp(argv[i], "--no-expnf") == 0) {
				options.noExpnf = true;
				options.useMonaDFA = true;
			} else {
				switch (argv[i][1]) {
					case 'e':
						options.expandTagged = true;
						break;
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
 * Implementation is not sure at the moment, but should reorder the symbol
 * table or BDD track, so it is optimized for using of projection during
 * the decision procedure process. It should consider the structure of prefix
 * of given formula, so BDD used in transitions of automata can be better
 * reordered
 */
void initializeVarMap(ASTForm* formula) {
	IdentList free, bound;
	formula->freeVars(&free, &bound);

	IdentList *vars = ident_union(&free, &bound);
	if (vars != 0) {
		varMap.initializeFromList(vars);
	}
	delete vars;

}

void bdd_callback() {
	throw MonaFailureException();
}

void readTags(char* filename, std::string& output) {
	std::ifstream infile(filename);
	std::getline(infile, output);
	infile.close();
}

void parseTags(std::string&s, std::list<size_t>& tags) {
	std::string delimiter = ";";
	std::string beginDelimiter = ":";
	// Fixme: better things

	size_t pos = 0;

	pos = s.find(beginDelimiter);
	if(pos == std::string::npos) {

		return;
	}
	s = s.substr(pos+1);
	std::string token;
	while ((pos = s.find(delimiter)) != std::string::npos) {
		token = s.substr(0, pos);
		tags.insert(tags.end(), std::stoi(token));
		s.erase(0, pos + delimiter.length());
	}
	tags.insert(tags.end(), std::stoi(s));
}

int main(int argc, char *argv[]) {
	/* Parse initial arguments */
	if (!ParseArguments(argc, argv)) {
		PrintUsage();
		exit(-1);
	}

	if(options.monaWalk) {
		mona_callback = bdd_callback;
	}

	/* Initialization of timer used for statistics */
	initTimer();
	Timer timer_total, timer_formula, timer_automaton, timer_deciding;
	timer_total.start();

	///////// PARSING ////////////////////////////////////////////////////////
	Timer timer_parsing;
	timer_parsing.start();

	std::list<size_t> tags;
	std::string stringTag("");
	readTags(inputFileName, stringTag);
	parseTags(stringTag, tags);

	loadFile(inputFileName);
	yyparse();
	MonaAST *ast = untypedAST->typeCheck();
	lastPosVar = ast->lastPosVar;
	allPosVar = ast->allPosVar;

	timer_parsing.stop();

	// Prints progress if dumping is set
	if (options.printProgress) {
		G_DEBUG_FORMULA_AFTER_PHASE("loading");
		cout << "[*] Elapsed time: ";
		timer_parsing.print();
	}

	// Clean up untypedAST
	delete untypedAST;

	// Remove Trues and Falses from the formulae
	BooleanUnfolder bu_visitor;
	ast->formula = static_cast<ASTForm *>((ast->formula)->accept(bu_visitor));

	if (options.dump) {
		// Dump AST for main formula, verify formulas, and assertion
		G_DEBUG_FORMULA_AFTER_PHASE("boolean unfolding");
		(ast->formula)->dump();
	}

	timer_formula.start();

	// First close the formula if we are testing something specific
	IdentList freeVars, bound;
	(ast->formula)->freeVars(&freeVars, &bound);
	bool formulaIsGround = freeVars.empty();
	if(!formulaIsGround) {
		switch(options.test) {
			case TestType::VALIDITY:
				// Fixme: this is incorrect, we need to tell that the variable is in first order
				ast->formula = new ASTForm_All1(nullptr, &freeVars, ast->formula, Pos());
				break;
			case TestType::SATISFIABILITY:
				ast->formula = new ASTForm_Ex1(nullptr, &freeVars, ast->formula, Pos());
				break;
			case TestType::UNSATISFIABILITY:
				ast->formula = new ASTForm_Ex1(nullptr, &freeVars, new ASTForm_Not(ast->formula, Pos()), Pos());
				break;
			default:
				assert(false && "Cannot handle the unground formulae right now.");
				break;
		}
	}

	// Flattening of the formula
	PredicateUnfolder predicateUnfolder;
	ast->formula = static_cast<ASTForm *>((ast->formula)->accept(predicateUnfolder));

	if (options.dump) {
		G_DEBUG_FORMULA_AFTER_PHASE("predicate unfolding");
		(ast->formula)->dump();
	}

	if (options.noExpnf == false) {
		// Transform AST to existentional Prenex Normal Form
		cerr << "[!] Called [deprecated] parameter '--no-expnf'";
		return 0;
	} else {


		#define CALL_FILTER(filter) \
            filter filter##_visitor;    \
            ast->formula = static_cast<ASTForm *>(ast->formula->accept(filter##_visitor));    \
            if(options.dump) {    \
                G_DEBUG_FORMULA_AFTER_PHASE(#filter);    \
                (ast->formula)->dump();    std::cout << "\n";    \
            } \
            if(options.graphvizDAG) {\
                std::string filter##_dotName(""); \
				filter##_dotName += #filter; \
                filter##_dotName += ".dot"; \
                DotWalker filter##_dw_visitor(filter##_dotName); \
                (ast->formula)->accept(filter##_dw_visitor); \
            }
		FILTER_LIST(CALL_FILTER)
		#undef CALL_FILTER

		Tagger tagger(tags);
		(ast->formula)->accept(tagger);

        if(options.graphvizDAG) {
			std::string dotFileName(inputFileName);
			dotFileName += ".dot";
			DotWalker dw_visitor(dotFileName);
			(ast->formula)->accept(dw_visitor);
		}
	}

	timer_formula.stop();

	if (options.dump) {
		G_DEBUG_FORMULA_AFTER_PHASE("ex-PNF conversion");
		(ast->formula)->dump();

		// dumping symbol table
		#if (DUMP_NO_SYMBOL_TABLE == false)
			cout << "\n\n[*] Created symbol table:";
			symbolTable.dump();
			cout << "\n";
		#endif

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

	cout << "[*] Input file transformed into formula in Existential Prenex Normal Form\n";
	cout << "[*] Elapsed time: ";
	timer_formula.print();
	cout << "\n";

	///////// Conversion to Tree Automata ////////

	// Table or BDD tracks are reordered
	initializeVarMap(ast->formula);
#if (DEBUG_VARIABLE_SETS == true)
	varMap.dumpMap();
	std::cout << "\n";
#endif

#if (DEBUG_VARIABLE_SETS == true)
	std::cout << "Free Vars:\n";
	freeVars.dump();
	std::cout << "\nBound:\n";
	bound.dump();
	std::cout << "\n";
#endif

	// Definitions for compatibility between the methods
	PrefixListType plist;
	PrefixListType nplist;
	ASTForm *matrix = nullptr, *prefix = nullptr;
	bool topmostIsNegation;
	if (options.method == Method::FORWARD || options.method == Method::BACKWARD) {
		// First formula in AST representation is split into matrix and prefix part.
		// Only for FORWARD and BACKWARD method
		splitMatrixAndPrefix(ast, matrix, prefix);
		topmostIsNegation = (prefix->kind == aNot);
		if (options.noExpnf == false) {
			matrix = matrix->restrictFormula();
		}

		if (options.dump) {
			std::cout << "[*] Dumping restricted matrix\n";
			matrix->dump();
			std::cout << "\n";
		}

		// Transform prefix to set of sets of second-order variables
		// TODO: This might be wrong
		plist = convertPrefixFormulaToList(prefix);
		nplist = plist;

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
		if (freeVars.size() != 0) {
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
	}

	if(options.monaWalk) {
		std::string monaDot(inputFileName);
		monaDot += "-walk.dot";
		MonaAutomataDotWalker monaWalker(monaDot);
		(ast->formula)->accept(monaWalker);
		return 0;
	}

	Automaton vataAutomaton;
	SymbolicAutomaton* symAutomaton;
	timer_automaton.start();
	if(options.construction != AutomataConstruction::SYMBOLIC_AUT) {
		// Use mona for building automaton instead of VATA
		// -> this may fail on insufficient memory
		if (options.construction == AutomataConstruction::DETERMINISTIC_AUT) {
			std::cout << "[*] Constructing 'Deterministic' Automaton using MONA\n";
			constructAutomatonByMona(matrix, vataAutomaton);
			std::cout << "[*] Converted 'Deterministic' Automaton to 'NonDeterministic' Automaton\n";
			// Build automaton by ourselves, may build huge automata
		} else {
			// WS1S formula is transformed to unary NTA
			if (options.mode != TREE) {
				std::cout << "[*] Constructing 'NonDeterministic' Unary Automaton using VATA\n";
				matrix->toUnaryAutomaton(vataAutomaton, false);
				// WS2S formula is transformed to binary NTA
			} else {
				std::cout << "[*] Constructing 'NonDeterministic' Binary Automaton using VATA\n";
				matrix->toBinaryAutomaton(vataAutomaton, false);
			}
		}
	} else {
		std::cout << "[*] Constructing 'Symbolic' Automaton using gaston\n";
		symAutomaton = (ast->formula)->toSymbolicAutomaton(false);
		if(options.printProgress)
			symAutomaton->DumpAutomaton();
		std::cout << "\n";
	}

	timer_automaton.stop();
	if (options.dump) {
		std::cout << "\n[*] Formula translation to Automaton [DONE]\n";
		cout << "[*] Elapsed time: ";
		timer_automaton.print();
		cout << "\n";
	}

	if(options.construction != SYMBOLIC_AUT) {
		std::cout << "[*] Reindexing states in VATA Automaton\n";
		// reindex the states, for space optimizations for bitsets
		StateHT reachable;
		vataAutomaton = vataAutomaton.RemoveUnreachableStates(&reachable);

		StateType stateCnt = 0;
		StateToStateMap translMap;
		StateToStateTranslator stateTransl(translMap,
										   [&stateCnt](const StateType &) { return stateCnt++; });

		vataAutomaton = vataAutomaton.ReindexStates(stateTransl);
		TStateSet::stateNo = reachable.size();

		if (options.dump) {
			std::cout << "[*] Number of states in resulting automaton: " << TStateSet::stateNo << "\n";
		}

		// Dump automaton
		if (options.dump && !options.dontDumpAutomaton) {
			VATA::Serialization::AbstrSerializer *serializer = new VATA::Serialization::TimbukSerializer();
			std::cout << vataAutomaton.DumpToString(*serializer, "symbolic") << "\n";
			delete serializer;
		}

		#if (DEBUG_BDDS == true)
			StateHT allStates;
			auto vataAut = vataAutomaton.RemoveUnreachableStates(&allStates);
			vataAutomaton = vataAut.RemoveUselessStates();
			TransMTBDD * tbdd = getMTBDDForStateTuple(vataAutomaton, Automaton::StateTuple({}));
			std::cout << "Leaf : bdd\n";
			//std::cout << TransMTBDD::DumpToDot({tbdd}) << "\n\n";
			// Dump bdds
			for (auto state : allStates) {
				TransMTBDD* bdd = getMTBDDForStateTuple(vataAutomaton, Automaton::StateTuple({state}));
				std::cout << state << " : bdd\n";
        		//std::cout << TransMTBDD::DumpToDot({bdd}) << "\n\n";
			}
		#endif
	}
	///////// DECISION PROCEDURE /////////////////////////////////////////////
	int decided;
	try {
	// Deciding WS1S formula
		timer_deciding.start();
		if(options.method == Method::FORWARD) {
			if(options.mode != TREE) {
				decided = decideWS1S(vataAutomaton, plist, nplist);
			} else {
				throw NotImplementedException();
			}
		} else if(options.method == Method::BACKWARD) {
			if(options.mode != TREE) {
				decided = decideWS1S_backwards(vataAutomaton, plist, nplist, formulaIsGround, topmostIsNegation);
			} else {
				throw NotImplementedException();
			}
		// Deciding WS2S formula
		} else if(options.method == Method::SYMBOLIC) {
			if (options.mode != TREE) {
				decided = ws1s_symbolic_decision_procedure(symAutomaton);
				delete symAutomaton;
			} else {
				throw NotImplementedException();
			}
		} else {
			std::cout << "[!] Unsupported mode for deciding\n";
		}
		timer_deciding.stop();

		// Outing the results of decision procedure
		cout << "[!] Formula is ";
		switch(decided) {
			case SATISFIABLE:
				cout << "\033[1;34m'SATISFIABLE'\033[0m";
				break;
			case UNSATISFIABLE:
				cout << "\033[1;31m'UNSATISFIABLE'\033[0m";
				break;
			case VALID:
				cout << "\033[1;32m'VALID'\033[0m";
				break;
			default:
				cout << "undecided due to an error.\n";
				break;
		}

		cout << "\n[*] Decision procedure: ";
		timer_deciding.print();
		cout << "[*] DFA creation:       ";
		timer_mona.print();
		cout << "[*] MONA <-> VATA:      ";
		timer_conversion.print();
		cout << "[*] Bases creation:     ";
		timer_automaton.print();
		// Something that was used is not supported by dWiNA
	} catch (NotImplementedException& e) {
		std::cerr << e.what() << std::endl;
	}

	// Prints timing
	if (options.time || options.printProgress) {
		timer_total.stop();
		cout << "\n[*] Total elapsed time:     ";
		timer_total.print();
	}

	// Clean up
	delete ast;
	Deque<FileSource *>::iterator i;
	for (i = source.begin(); i != source.end(); i++)
		delete *i;

	PredLibEntry *pred = predicateLib.first();
	while(pred != nullptr) {
		delete pred->ast;
		pred = predicateLib.next();
	}

	return 0;
}
