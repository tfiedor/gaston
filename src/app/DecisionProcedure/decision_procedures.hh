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

#include <deque>
#include <memory>

#include "mtbdd/ondriks_mtbdd.hh"
#include "containers/VarToTrackMap.hh"
#include "containers/StateSet.hh"
#include "automata.hh"
#include "mtbdd_factors.hh"

extern VarToTrackMap varMap;

// < Module Typedefs >
typedef bool TSatExample;
typedef bool TUnSatExample;
typedef std::deque<unsigned int> VariableSet;
typedef std::deque<VariableSet> PrefixListType;

using Automaton = VATA::BDDBottomUpTreeAut;
using StateType = size_t;
using StateHT = std::unordered_set<StateType>;
using StateTuple = std::vector<StateType>;
using MTBDDLeafStateSet = VATA::Util::OrdVector<StateType>;
using TransMTBDD = VATA::MTBDDPkg::OndriksMTBDD<MTBDDLeafStateSet>;
using MacroTransMTBDD = VATA::MTBDDPkg::OndriksMTBDD<MacroStateSet*>;
typedef StateHT FinalStatesType;
typedef StateHT StateSetType;

// < Module Functions >
int decideWS1S(Automaton & aut, TSatExample & example, TUnSatExample & counterExample, PrefixListType formulaPrefixSet, PrefixListType negFormulaPrefixSet);
int decideWS2S(Automaton & aut, TSatExample & example, TUnSatExample & counterExample);
TSatExample findSatisfyingExample();
TUnSatExample findUnsatisfyingExample();
bool existsSatisfyingExample(Automaton & aut, MacroStateSet* initialState, PrefixListType formulaPrefixSet);
bool existsUnsatisfyingExample(Automaton & aut, MacroStateSet* initialState, PrefixListType negFormulaPrefixSet);
PrefixListType convertPrefixFormulaToList(ASTForm* formula);
void closePrefix(PrefixListType & prefix, IdentList* freeVars, bool negationIsTopmonst);
FinalStatesType computeFinalStates(Automaton & aut);
TransMTBDD* getMTBDDForStateTuple(Automaton & aut, const StateTuple & states);
void getInitialStatesOfAutomaton(Automaton & aut, MTBDDLeafStateSet &);
MacroStateSet* constructInitialState(Automaton &  aut, unsigned numberOfDeterminizations);
bool StateIsFinal(Automaton & aut, TStateSet* state, unsigned level, PrefixListType & prefix);
TStateSet* GetZeroPost(Automaton & aut, TStateSet* state, unsigned level, PrefixListType & prefix);
MacroTransMTBDD* GetMTBDDForPost(Automaton & aut, TStateSet* state, unsigned level, PrefixListType & prefix);

#endif
