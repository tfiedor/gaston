#ifndef __DEC_PROC__H__
#define __DEC_PROC__H__

// VATA headers
#include <vata/bdd_bu_tree_aut.hh>
#include <vata/parsing/timbuk_parser.hh>
#include <vata/serialization/timbuk_serializer.hh>
#include <vata/util/binary_relation.hh>

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

// < Module Functions >
int decideWS1S(Automaton aut, TSatExample & example, TUnSatExample & counterExample);
int decideWS2S(Automaton aut, TSatExample & example, TUnSatExample & counterExample);
TSatExample findSatisfyingExample();
TUnSatExample findUnsatisfyingExample();
bool existsSatisfyingExample();
bool existsUnsatisfyingExample();
PrefixListType convertPrefixFormulaToList(ASTForm* formula);

#endif
