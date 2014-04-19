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

#define DEBUG

using std::cout;

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
int numTypes = 0;
bool regenerate = false;

extern int yyparse(void);
extern void loadFile(char *filename);
extern Deque<FileSource *> source; 

char *inputFileName = NULL;

extern Ident lastPosVar, allPosVar;

void PrintUsage()
{
  cout << "Usage: dip [options] <filename>\n\n"
    << "Options:\n"
    << " -t, --time 		Print elapsed time\n"
    << " -d, --dump-all		Dump AST, symboltable, and code DAG\n"
    << " -q, --quite		Quiet, don't print progress\n"
    << " --reorder-bdd		Disable BDD index reordering [no, random, heuristic]\n"
    << "Example: ./dip -t -d --reorder-bdd=random foo.mona\n\n";
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
	cout << "[*] Variables reordered by heuristic approach" << std::endl;
	varMap.initializeFromLists(bound, free);
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
	cout << "[*] Variables reorder randomly" << std::endl;
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
    cout << "Time: ";
    timer_parsing.print();
  }

  delete untypedAST;

  if (options.dump) {
	symbolTable.dump();
	// Dump AST for main formula, verify formulas, and assertion
	cout << "Main formula:\n";
	(ast->formula)->dump();
  }

  // Flattening of the formula
  ast->formula = (ASTForm*) (ast->formula)->toSecondOrder();
  if(options.dump) {
    cout << "\nFlattening of the formula:\n";
    (ast->formula)->dump();
  }

  // Transform AST to existentional Prenex Normal Form
  ast->formula = (ASTForm*) (ast->formula)->toExistentionalPNF();

  if(options.dump) {
    cout << "\nAfter transformation:\n";
    (ast->formula)->dump();

    Deque<ASTForm *>::iterator vf;
    Deque<char *>::iterator vt;
    for (vf = ast->verifyformlist.begin(), vt = ast->verifytitlelist.begin();
	 vf != ast->verifyformlist.end(); vf++, vt++) {
      cout << "\n\nFormula " << *vt << ":\n";
      (*vf)->dump();
    }
    cout << "\n\nAssertions:\n";
    (ast->assertion)->dump();
    cout << "\n";

    if (lastPosVar != -1)
      cout << "\nLastPos variable: " 
	   << symbolTable.lookupSymbol(lastPosVar) << "\n";
    if (allPosVar != -1)
      cout << "\nAllPos variable: " 
	   << symbolTable.lookupSymbol(allPosVar) << "\n";
    
    // Dump ASTs for predicates and macros
    PredLibEntry *pred = predicateLib.first();
    while (pred != NULL) {
      if (pred->isMacro)
	cout << "\nMacro '";
      else
	cout << "\nPredicate '";
      cout << symbolTable.lookupSymbol(pred->name) 
	   << "':\n";
      (pred->ast)->dump();
      cout << "\n";
      pred = predicateLib.next();
    }

    // Dump restrictions
    if (symbolTable.defaultRestriction1) {
      cout << "\nDefault first-order restriction (" 
	   << symbolTable.lookupSymbol(symbolTable.defaultIdent1) << "):\n";
      symbolTable.defaultRestriction1->dump();
      cout << "\n";
    }
    if (symbolTable.defaultRestriction2) {
      cout << "\nDefault second-order restriction (" 
	   << symbolTable.lookupSymbol(symbolTable.defaultIdent2) << "):\n";
      symbolTable.defaultRestriction2->dump();
      cout << "\n";
    }

    Ident id;
    for (id = 0; id < (Ident) symbolTable.noIdents; id++) {
      Ident t;
      ASTForm *f = symbolTable.getRestriction(id, &t);
      if (f) {
	cout << "\nRestriction for #" << id << " (" 
	     << symbolTable.lookupSymbol(id) << "):";
	if (t != -1)
	  cout << " default\n";
	else {
	  cout << "\n";
	  f->dump();
	  cout << "\n";
	}
      }
    }
  }
  
  ///////// Conversion to Tree Automata ////////

  // Table or BDD tracks are reordered
  reorder(options.reorder, ast->formula);
#ifdef DEBUG
  varMap.dumpMap();
#endif

  IdentList freeVars, bound;
  (ast->formula)->freeVars(&freeVars, &bound);

  // First formula in AST representation is split into matrix and prefix part.
  ASTForm *matrix, *prefix;
  splitMatrixAndPrefix(ast, matrix, prefix);

  // Transform prefix to set of sets of second-order variables
#ifdef DEBUG
  cout << "Converting prefix to list of lists: \n";
  prefix->dump();
#endif

  PrefixListType plist = convertPrefixFormulaToList(prefix);
  PrefixListType nplist(plist);
  closePrefix(plist, &freeVars, (prefix->kind == aNot));
  closePrefix(nplist, &freeVars, (prefix->kind != aNot));

#ifdef DEBUG
  cout << "\n";
  for(PrefixListType::iterator it = plist.begin(); it != plist.end(); ++it) {
	  cout << "[";
	  for(VariableSet::iterator jt = it->begin(); jt != it->end(); ++jt) {
		  cout << *jt << " ";
	  }
	  cout << "], ";
  }
  cout << "\n";
  cout << "\n";
  for(PrefixListType::iterator it = nplist.begin(); it != nplist.end(); ++it) {
	  cout << "[";
	  for(VariableSet::iterator jt = it->begin(); jt != it->end(); ++jt) {
		  cout << *jt << " ";
	  }
	  cout << "], ";
  }
  cout << "\n";
#endif

  Automaton formulaAutomaton;
  // WS1S formula is transformed to unary NTA
  if(options.mode != TREE) {
	  matrix->toUnaryAutomaton(formulaAutomaton, false);
  // WS2S formula is transformed to binary NTA
  } else {
	  matrix->toBinaryAutomaton(formulaAutomaton, false);
  }

  VATA::Serialization::AbstrSerializer* serializer =
		  new VATA::Serialization::TimbukSerializer();
  std::cout << formulaAutomaton.DumpToString(*serializer);

  if (options.dump) {
	  symbolTable.dump();
  }

  ///////// DECISION PROCEDURE /////////////////////////////////////////////
  int decided;
  TSatExample example;
  TUnSatExample counterExample;
  try {
	  // Deciding WS1S formula
	  if(options.mode != TREE) {
		  decided = decideWS1S(formulaAutomaton, example, counterExample, plist, nplist);
	  // Deciding WS2S formula
	  } else {
		  decided = decideWS2S(formulaAutomaton, example, counterExample);
	  }

	  // Outing the results of decision procedure
	  cout << "Formula is ";
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

  delete ast;
    
  if (options.statistics)
    print_statistics();

  if (options.time) {
    timer_total.stop();
    cout << "\nTotal time:     ";
    timer_total.print();
    print_timing();
  }
  else if (options.printProgress) { 
    timer_total.stop();
    cout << "\nTotal time: ";
    timer_total.print();
  }
}
