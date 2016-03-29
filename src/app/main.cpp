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
#include "DecisionProcedure/checkers/SymbolicChecker.h"

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
char *inputFileName = NULL;
Timer timer_conversion, timer_mona, timer_base, timer_automaton;
Timer timer_gaston;

//extern void (*mona_callback)();

extern Ident lastPosVar, allPosVar;

void bdd_callback() {
    throw MonaFailureException();
}

/**
 * Prints usage of the program to the standard output.
 */
void PrintUsage() {
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
			else if(strcmp(argv[i], "--walk-aut") == 0) {
                options.monaWalk = true;
                //mona_callback = bdd_callback;
            } else if(strcmp(argv[i], "--expand-tagged") == 0)
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

int main(int argc, char *argv[]) {
	/* Parse initial arguments */
	if (!ParseArguments(argc, argv)) {
		PrintUsage();
		exit(-1);
	}
	initTimer();

	timer_gaston.start();
    Checker *checker = new SymbolicChecker();
    checker->LoadFormulaFromFile();
    checker->CloseUngroundFormula();
    checker->PreprocessFormula();

    if(options.monaWalk) {
        checker->CreateAutomataSizeEstimations();
        delete checker;
        return 0;
    } else {
        checker->ConstructAutomaton();
        checker->Decide();
    }
	timer_gaston.stop();
	std::cout << "\n[*] Total elapsed time: ";
	timer_gaston.print();

    delete checker;

	return 0;
}