/*****************************************************************************
 *  dWiNA - Deciding WSkS using non-deterministic automata
 *
 *  Copyright (c) 2014  Tomas Fiedor <xfiedo01@stud.fit.vutbr.cz>
 *
 *  Description:
 *    Implementation of common Automata related operations
 *
 *****************************************************************************/

#ifndef __AUTOMATA__H__
#define __AUTOMATA__H__

#include "../Frontend/symboltable.h"
#include "../Frontend/st_dfa.h"
#include "../Frontend/env.h"
#include "../Frontend/ast.h"
#include "../Frontend/offsets.h"
#include "../Frontend/timer.h"
#include "containers/VarToTrackMap.hh"
#include "environment.hh"

#include <cstring>
#include <list>
#include <algorithm>

#include "automata.hh"
#include "environment.hh"

#include "visitors/transformers/Derestricter.h"
#include "visitors/transformers/ShuffleVisitor.h"

using std::cout;

extern Offsets offsets;
extern CodeTable *codeTable;
extern Timer timer_conversion, timer_mona;
extern VarToTrackMap varMap;
extern SymbolTable symbolTable;
extern Options options;

using Automaton = VATA::BDDBottomUpTreeAut;

char charToAsgn(char c);
void toMonaAutomaton(ASTForm *form, DFA*& dfa, bool);

#endif
