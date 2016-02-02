/*****************************************************************************
 *  gaston - We pay homage to Gaston, an Africa-born brown fur seal who
 *    escaped the Prague Zoo during the floods in 2002 and made a heroic
 *    journey for freedom of over 300km all the way to Dresden. There he
 *    was caught and subsequently died due to exhaustion and infection.
 *    Rest In Piece, brave soldier.
 *
 *  Copyright (c) 2015  Tomas Fiedor <ifiedortom@fit.vutbr.cz>
 *      Notable mentions:   Ondrej Lengal <ondra.lengal@gmail.com>
 *                              (author of VATA)
 *                          Petr Janku <ijanku@fit.vutbr.cz>
 *                              (MTBDD and automata optimizations)
 *
 *  Description:
 *      The main file of the Gaston tool
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
		<< "     --test          Test specified problem [val, sat, unsat]\n"
		<< "     --walk-aut      Does the experiment generating the special dot graph\n"
		<< " -e, --expand-tagged Expand automata with given tag on first line of formula\n"
		<< " -q, --quiet		 Quiet, don't print progress\n"
		<< " -oX                 Optimization level [1 = safe optimizations [default], 2 = heuristic]\n"
		<< "Example: ./gaston -t -d foo.mona\n\n";
}

/**
 * Parses input arguments into options
 *
 * @param argc: number of arguments
 * @param argv: list of arguments
 * @return: false if arguments are wrong
 */
bool ParseArguments(int argc, char *argv[]) {
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
			else if(strcmp(argv[i], "--test=val") == 0) {
				options.test = TestType::VALIDITY;
			} else if(strcmp(argv[i], "--test=sat") == 0) {
				options.test = TestType::SATISFIABILITY;
			} else if(strcmp(argv[i], "--test=unsat") == 0) {
				options.test = TestType::UNSATISFIABILITY;
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
		G_DEBUG_FORMULA_AFTER_PHASE("Predicate Unfolding");
		(ast->formula)->dump();
	}

    // Additional filter phases
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
	timer_formula.stop();

	if (options.dump) {
		G_DEBUG_FORMULA_AFTER_PHASE("All phases");
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
	}

	cout << "[*] Elapsed time: ";
	timer_formula.print();
	cout << "\n";

	///////// Conversion to Tree Automata ////////

	// Table or BDD tracks are reordered
	initializeVarMap(ast->formula);
#if (DEBUG_VARIABLE_SETS == true)
	varMap.dumpMap();
	std::cout << "\n";
	std::cout << "Free Vars:\n";
	freeVars.dump();
	std::cout << "\nBound:\n";
	bound.dump();
	std::cout << "\n";
#endif

	if(options.monaWalk) {
		std::string monaDot(inputFileName);
		monaDot += "-walk.dot";
		MonaAutomataDotWalker monaWalker(monaDot);
		(ast->formula)->accept(monaWalker);
		return 0;
	}

	SymbolicAutomaton* symAutomaton;
	timer_automaton.start();

    std::cout << "[*] Constructing 'Symbolic' Automaton using gaston\n";
    symAutomaton = (ast->formula)->toSymbolicAutomaton(false);
    if(options.printProgress)
        symAutomaton->DumpAutomaton();
    std::cout << "\n";

	timer_automaton.stop();
	if (options.dump) {
		std::cout << "\n[*] Formula translation to Automaton [DONE]\n";
		cout << "[*] Elapsed time: ";
		timer_automaton.print();
		cout << "\n";
	}

	///////// DECISION PROCEDURE /////////////////////////////////////////////
	int decided;
	try {
	// Deciding WS1S formula
		timer_deciding.start();
        if (options.mode != TREE) {
            decided = ws1s_symbolic_decision_procedure(symAutomaton);
            delete symAutomaton;
        } else {
            throw NotImplementedException();
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