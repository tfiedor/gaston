#define _LANGUAGE_C_PLUS_PLUS

#include <iostream>
#include <new>
#include <sys/time.h>
#include <sys/resource.h>
#include <signal.h>
#include <map>

// VATA headers
#include <vata/bdd_bu_tree_aut.hh>
#include <vata/parsing/timbuk_parser.hh>
#include <vata/serialization/timbuk_serializer.hh>
#include <vata/util/binary_relation.hh>

// MONA Frontend headers
#include "Frontend/env.h"
#include "Frontend/untyped.h"
#include "Frontend/predlib.h"
#include "Frontend/ast.h"
#include "Frontend/code.h"
#include "Frontend/st_dfa.h"
#include "Frontend/st_gta.h"
#include "Frontend/timer.h"
#include "Frontend/lib.h"
#include "Frontend/printline.h"
#include "Frontend/config.h"
#include "Frontend/offsets.h"
#include "Frontend/ident.h"

// DecProc headers
#include <DecisionProcedure/environment.hh>
#include <DecisionProcedure/decision_procedures.hh>
#include <DecisionProcedure/containers/VarToTrackMap.hh>
#include "DecisionProcedure/containers/Cache.hh"

#define DEBUG

using std::cout;

using StateToStateTranslator = VATA::AutBase::StateToStateTranslWeak;
using StateToStateMap         = std::unordered_map<StateType, StateType>;
using Automaton = VATA::BDDBottomUpTreeAut;

typedef unsigned int uint;

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

#ifdef USE_STATECACHE
MultiLevelMCache<bool> StateCache;
#endif
#ifdef USE_BDDCACHE
MultiLevelMCache<MacroTransMTBDD> BDDCache;
#endif

extern int yyparse(void);
extern void loadFile(char *filename);
extern Deque<FileSource *> source; 

char *inputFileName = NULL;

extern Ident lastPosVar, allPosVar;

void PrintUsage()
{
  cout << "Usage: dWiNA [options] <filename>\n\n"
    << "Options:\n"
    << " -t, --time 		Print elapsed time\n"
    << " -d, --dump-all		Dump AST, symboltable, and code DAG\n"
    << " -q, --quiet		Quiet, don't print progress\n"
    << " --reorder-bdd		Disable BDD index reordering [no, random, heuristic]\n"
    << "Example: ./dWiNA -t -d --reorder-bdd=random foo.mona\n\n";
}

bool 
ParseArguments(int argc, char *argv[])
{
  options.printProgress = true;
  options.analysis = true;
  options.optimize = 1;
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
      else {
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

	while(true) {
		if(formIter->kind == aEx2) {
			previous = formIter;
			formIter = ((ASTForm_Ex2*) formIter)->f;
		} else if (formIter->kind == aNot && (((ASTForm_Not*)formIter)->f)->kind == aEx2) {
			previous = formIter;
			formIter = ((ASTForm_Not*) formIter)->f;
		} else {
			if (previous != 0) {
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
 */
void randomReorder(IdentList *free, IdentList *bound) {
	if(options.dump) {
		cout << "[*] Variables reorder randomly" << std::endl;
	}
	IdentList *vars = ident_union(free, bound);
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

int 
main(int argc, char *argv[])
{
  /* Parse initial arguments */
  if (!ParseArguments(argc, argv)) {
	  PrintUsage();
    exit(-1);
  }

  /* Initialization of timer used for statistics */
  initTimer();
  Timer timer_total;
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

  // Flattening of the formula
  ast->formula = (ASTForm*) (ast->formula)->toSecondOrder();
  if(options.dump) {
    cout << "\n\n[*] Flattened formula:\n";
    (ast->formula)->dump();
  }

  // Transform AST to existentional Prenex Normal Form
  ast->formula = (ASTForm*) (ast->formula)->toExistentionalPNF();

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
  }
  
  ///////// Conversion to Tree Automata ////////

  // Table or BDD tracks are reordered
  reorder(options.reorder, ast->formula);

  IdentList freeVars, bound;
  (ast->formula)->freeVars(&freeVars, &bound);

  // First formula in AST representation is split into matrix and prefix part.
  ASTForm *matrix, *prefix;
  splitMatrixAndPrefix(ast, matrix, prefix);

  // Transform prefix to set of sets of second-order variables
  PrefixListType plist = convertPrefixFormulaToList(prefix);
  PrefixListType nplist(plist);
  closePrefix(plist, &freeVars, (prefix->kind == aNot));
  closePrefix(nplist, &freeVars, (prefix->kind != aNot));

  Automaton formulaAutomaton;
  // WS1S formula is transformed to unary NTA
  if(options.mode != TREE) {
	  matrix->toUnaryAutomaton(formulaAutomaton, false);
  // WS2S formula is transformed to binary NTA
  } else {
	  matrix->toBinaryAutomaton(formulaAutomaton, false);
  }

  if(options.dump) {
	  std::cout << "[*] Formula transformed into non-determinsitic tree automaton\n";
  }

  StateHT reachable;
  formulaAutomaton = formulaAutomaton.RemoveUnreachableStates(&reachable);

  // reindex the states, for space optimizations for bitsets
  StateType stateCnt = 0;
  StateToStateMap translMap;
  StateToStateTranslator stateTransl(translMap,
	[&stateCnt](const StateType&){return stateCnt++;});
  TStateSet::stateNo = reachable.size();

  formulaAutomaton = formulaAutomaton.ReindexStates(stateTransl);

  if(options.dump) {
	  VATA::Serialization::AbstrSerializer* serializer =
			  new VATA::Serialization::TimbukSerializer();
	  std::cout << formulaAutomaton.DumpToString(*serializer) << "\n";
	  delete serializer;
  }

  ///////// DECISION PROCEDURE /////////////////////////////////////////////
  int decided;
  try {
	  // Deciding WS1S formula
	  if(options.mode != TREE) {
		  decided = decideWS1S(formulaAutomaton, plist, nplist);
	  // Deciding WS2S formula
	  } else {
		  decided = decideWS2S(formulaAutomaton);
	  }

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
		  cout << "undecidable due to unforeseen error.\n";
		  break;
	  }
  } catch (NotImplementedException& e) {
	  std::cerr << e.what() << std::endl;
  }

  ///////// CLEAN UP ///////////////////////////////////////////////////////

  delete ast->assertion;
  delete matrix;
  delete prefix;

  /*predicateLib.cleanUp();
  symbolTable.cleanUp();

  //StateCache.clear();
  //BDDCache.clear();

  Deque<FileSource *>::iterator i;
  for (i = source.begin(); i != source.end(); i++)
    delete *i;*/
    
  if (options.statistics)
    print_statistics();

  if (options.time) {
    timer_total.stop();
    cout << "\n[*] Total elapsed time:     ";
    timer_total.print();
    print_timing();
  }
  else if (options.printProgress) { 
    timer_total.stop();
    cout << "\n[*] Total elapsed time: ";
    timer_total.print();
  }

  return 0;
}
