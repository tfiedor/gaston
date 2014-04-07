#ifndef __DEC_PROC__H__
#define __DEC_PROC__H__

// VATA headers
#include <vata/bdd_bu_tree_aut.hh>
#include <vata/parsing/timbuk_parser.hh>
#include <vata/serialization/timbuk_serializer.hh>
#include <vata/util/binary_relation.hh>

// MONA headers
#include "../Frontend/ast.h"

// < Module Typedefs >
typedef bool TSatExample;
typedef bool TUnSatExample;

using Automaton = VATA::BDDBottomUpTreeAut;

// < Module Functions >
int decideWS1S(Automaton aut, TSatExample & example, TUnSatExample & counterExample);
int decideWS2S(Automaton aut, TSatExample & example, TUnSatExample & counterExample);
TSatExample findSatisfyingExample();
TUnSatExample findUnsatisfyingExample();
bool existsSatisfyingExample();
bool existsUnsatisfyingExample();

#endif
