#ifndef __DEC_PROC__H__
#define __DEC_PROC__H__

// VATA headers
#include <vata/bdd_bu_tree_aut.hh>
#include <vata/parsing/timbuk_parser.hh>
#include <vata/serialization/timbuk_serializer.hh>
#include <vata/util/binary_relation.hh>
#include <vata/util/aut_description.hh>

// MONA headers
#include "../Frontend/ast.h"

#include <deque>

#include "containers/VarToTrackMap.hh"

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
typedef StateHT FinalStatesType;
typedef StateHT StateSetType;

// < Module Functions >
int decideWS1S(Automaton aut, TSatExample & example, TUnSatExample & counterExample);
int decideWS2S(Automaton aut, TSatExample & example, TUnSatExample & counterExample);
TSatExample findSatisfyingExample();
TUnSatExample findUnsatisfyingExample();
bool existsSatisfyingExample(FinalStatesType fm);
bool existsUnsatisfyingExample(FinalStatesType fm);
PrefixListType convertPrefixFormulaToList(ASTForm* formula);
FinalStatesType computeFinalStates(Automaton aut);

#endif
