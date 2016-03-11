/*****************************************************************************
 *  gaston - We pay homage to Gaston, an Africa-born brown fur seal who
 *    escaped the Prague Zoo during the floods in 2002 and made a heroic
 *    journey for freedom of over 300km all the way to Dresden. There he
 *    was caught and subsequently died due to exhaustion and infection.
 *    Rest In Piece, brave soldier.
 *
 *  Copyright (c) 2015  Tomas Fiedor <ifiedortom@fit.vutbr.cz>
 *      Notable mentions: Ondrej Lengal <ondra.lengal@gmail.com>
 *
 *  Description:
 *      Header file for all decision procedure methods. Globally used headers,
 *      function defines, etc.
 *****************************************************************************/
#ifndef __DEC_PROC__H__
#define __DEC_PROC__H__

// VATA headers
#include <vata/bdd_bu_tree_aut.hh>
#include <vata/parsing/timbuk_parser.hh>
#include <vata/serialization/timbuk_serializer.hh>
#include <vata/util/binary_relation.hh>
#include <vata/util/aut_description.hh>
#include <vata/util/convert.hh>

// MONA headers
#include "../Frontend/ast.h"
#include "../Frontend/ident.h"
#include "../Frontend/env.h"

#include <deque>
#include <memory>
#include <unordered_map>
#include <utility>

#include "mtbdd/ondriks_mtbdd.hh"
#include "containers/VarToTrackMap.hh"
#include "containers/StateSet.hh"
#include "containers/Cache.hh"
#include "automata.hh"
#include "mtbdd_factors.hh"
#include "environment.hh"

extern VarToTrackMap varMap;
extern Options options;

// < Module Typedefs >
using VariableSet = std::vector<unsigned int>;
using PrefixListType = std::deque<VariableSet>;

// < FOR SYMBOLIC METHOD >
// TODO: Move to environment.h?
using StateType = size_t;
using StateTuple = std::vector<StateType>;
using BaseAut_States = VATA::Util::OrdVector<StateType>;
using BaseAut_MTBDD = VATA::MTBDDPkg::OndriksMTBDD<BaseAut_States>;
// Compiler complaining fucker
using Fixpoint_Leaf = MacroStateSet*;
using Fixpoint_LeafType = std::pair<Fixpoint_Leaf, bool>;
using FixPoint_MTBDD = VATA::MTBDDPkg::OndriksMTBDD<Fixpoint_LeafType>;
using FixPoint_MTBDD_T = VATA::MTBDDPkg::OndriksMTBDD<MacroStateSet*>;
using FixPoint_MTBDD_B = VATA::MTBDDPkg::OndriksMTBDD<bool>;
using StateSet = MacroStateSet*;
using Automaton = VATA::BDDBottomUpTreeAut;
using StateHT = std::unordered_set<StateType>;
using MTBDDLeafStateSet = VATA::Util::OrdVector<StateType>;
using      TransMTBDD = VATA::MTBDDPkg::OndriksMTBDD<MTBDDLeafStateSet>;
using MacroTransMTBDD = VATA::MTBDDPkg::OndriksMTBDD<MacroStateSet*>;
using BaseFinalStatesType = StateHT;
using FinalStateType = MacroStateSet*;
using StateSetType = StateHT;

// < Module Functions >
int decideWS1S(Automaton & aut, PrefixListType formulaPrefixSet, PrefixListType negFormulaPrefixSet);
int decideWS2S(Automaton & aut);
bool existsSatisfyingExample(Automaton & aut, MacroStateSet* initialState, PrefixListType formulaPrefixSet);
bool existsUnsatisfyingExample(Automaton & aut, MacroStateSet* initialState, PrefixListType negFormulaPrefixSet);
PrefixListType convertPrefixFormulaToList(ASTForm* formula);
void closePrefix(PrefixListType & prefix, IdentList* freeVars, bool negationIsTopmonst);
BaseFinalStatesType getBaseFinalStates(Automaton & aut);
TransMTBDD* getMTBDDForStateTuple(Automaton & aut, const StateTuple & states);
void getInitialStatesOfAutomaton(Automaton & aut, MTBDDLeafStateSet &);
MacroStateSet* constructInitialState(Automaton &  aut, unsigned numberOfDeterminizations);
bool StateIsFinal(Automaton & aut, TStateSet* state, unsigned level, PrefixListType & prefix);
MacroStateSet* GetZeroPost(Automaton & aut, TStateSet*& state, unsigned level, PrefixListType & prefix);
MacroStateSet* GetZeroMacroPost(Automaton & aut, TStateSet*& state, unsigned level, PrefixListType & prefix);
int getProjectionVariable(unsigned level, PrefixListType & prefix);
MacroTransMTBDD GetMTBDDForPost(Automaton & aut, TStateSet* state, unsigned level, PrefixListType & prefix);
bool isNotEnqueued(StateSetList & queue, TStateSet*& state, unsigned level);

// < Backward decision procedure functions >
int decideWS1S_backwards(Automaton &aut, PrefixListType formulaPrefixSet, PrefixListType negFormulaPrefixSet, bool formulaIsGround, bool topmostIsNegation);
bool testValidity(Automaton &aut, PrefixListType prefix, bool topmostIsNegation);
MacroStateSet* computeFinalStates(Automaton &aut, PrefixListType prefix, unsigned int detNo);
bool initialStateIsInFinalStates(MacroStateSet *initial, MacroStateSet *finalStates, unsigned int level);

// < Symbolic decision procedure functions >
ASTForm* _unfoldCore(ASTForm* form, IdentList* fParams, ASTList* rParams);

#endif
