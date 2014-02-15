#define _LANGUAGE_C_PLUS_PLUS

#include <iostream>
#include <new>
#include <sys/time.h>
#include <sys/resource.h>
#include <signal.h>

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

using std::cout;

Options options;
MonaUntypedAST *untypedAST;
SymbolTable symbolTable(1019);
PredicateLib predicateLib;
Offsets offsets;
CodeTable *codeTable;
Guide guide;
AutLib lib;
int numTypes = 0;
bool regenerate = false;

extern int yyparse(void);
extern void loadFile(char *filename);
extern Deque<FileSource *> source; 

char *inputFileName = NULL;

extern Ident lastPosVar, allPosVar;

bool 
ParseArguments(int argc, char *argv[])
{
  options.printProgress = true;
  options.analysis = true;
  options.optimize = 1;
  options.reorder = false; //true;

  switch (argc) {
  case 1:
      return false;

  case 2:
    if (argv[1][0] == '-')
      return false;

  default:
    for (int i = 1; i < argc - 1; i++) {
      
      if (argv[i][0] != '-')
	return false;

      if (strcmp(argv[i], "-demo") == 0)
	options.demo = true;
      else
	switch (argv[i][1]) {
	case 'o':
	  if (sscanf(argv[i]+2, "%u", &options.optimize) != 1)
	    return false;
	  break;
	case 'x':
	  if (argv[i][2] == 'w') {
	    options.printProgress = false;
	    options.externalWhole = true;
	    options.whole = true;
	    options.analysis = false;
	  }
	  else
	    return false;
	  break;
	case 'g':
	  options.printProgress = false;
	  switch (argv[i][2]) {
	  case 'w':
	    options.graphvizDFA = true;
	    options.whole = true;
	    options.analysis = false;
	    break;
	  case 'd':
	    options.graphvizDAG = true;
	    options.analysis = false;
	    break;
	  case 's':
	    options.graphvizSatisfyingEx = true;
	    options.analysis = true;
	    break;
	  case 'c':
	    options.graphvizCounterEx = true;
	    options.analysis = true;
	    break;
	  default:
	    return false;
	  }
	  break;
	default:
	  switch (argv[i][1]) {
	  case 'w':
	    options.whole = true;
	    break;
	  case 'n':
	    options.analysis = false;
	    break;
	  case 'd':
	    options.dump = true;
	    break;
	  case 't':
	    options.time = true;
	    break;
	  case 's':
	    options.statistics = true;
	    break;
	  case 'q':
	    options.printProgress = false;
	    break;
	  case 'c':
	    options.analysis = true;
	    break;
	  case 'e':
	    options.separateCompilation = true;
	    break;
	  case 'i':
	    options.intermediate = true;
	    options.statistics = true;
	    break;
	  case 'f':
	    options.treemodeOutput = true;
	    options.analysis = true;
	    break;
	  case 'h':
	    options.inheritedAcceptance = true;
	    break;
	  case 'u':
	    options.unrestrict = true;
	    break;
	  case 'm':
	    options.alternativeM2LStr = true;
	    break;
	  case 'r':
	    options.reorder = false;
	    break;
	  case 'p':
	    break; // ignore for compatibility
	  default:
	    return false;
	  }
	  if (argv[i][2] != 0)
	    return false;
	  break;
	}
    }
  } 
  
  inputFileName = argv[argc-1];
  return true;
}

int 
main(int argc, char *argv[])
{
  if (!ParseArguments(argc, argv)) {
    exit(-1);
  }

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

  if (true) {
    symbolTable.dump();
    // Dump AST for main formula, verify formulas, and assertion
    cout << "Main formula:\n";
    (ast->formula)->dump();

    // Transform AST to existentional Prenex Normal Form
    ast->formula = (ASTForm*) (ast->formula)->toExistentionalPNF();
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
  
  if (options.mode == TREE && (options.dump || options.whole) && 
      !options.externalWhole)
    printGuide();

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
