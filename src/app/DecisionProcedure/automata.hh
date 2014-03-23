#ifndef __AUTOMATA__H__
#define __AUTOMATA__H__

// VATA headers
#include <vata/bdd_bu_tree_aut.hh>
#include <vata/parsing/timbuk_parser.hh>
#include <vata/serialization/timbuk_serializer.hh>
#include <vata/util/binary_relation.hh>

#include "../Frontend/symboltable.h"
#include "containers/VarToTrackMap.hh"

extern VarToTrackMap varMap;
extern SymbolTable symbolTable;

using Automaton = VATA::BDDBottomUpTreeAut;

char charToAsgn(char c);
void addTransition(Automaton& aut, Automaton::StateTuple q, int x, int y, char* track, int qf);
void addTransition(Automaton& aut, Automaton::StateTuple q, int x, char track, int qf);
void addUniversalTransition(Automaton& automaton, Automaton::StateTuple from, Automaton::StateType to);

Automaton::SymbolType constructUniversalTrack();

/**
 * Sets state as final in automaton, according to the whether we are
 * complementing the automaton or not
 *
 * @param[in] automaton: automaton, where we are setting states
 * @param[in] complement: whether we are constructing complement automaton
 * @param[in] state: which state we are setting as final
 */
inline void setFinalState(Automaton &automaton, bool complement, unsigned int state) {
	if(!complement) {
		automaton.SetStateFinal(state);
	}
}

/**
 * Sets state as non-final in automaton, according to the whether we are
 * complementing the automaton or not
 *
 * @param[in] automaton: automaton, where we are setting states
 * @param[in] complement: whether we are constructing complement automaton
 * @param[in] state: which state we are setting as non final
 */
inline void setNonFinalState(Automaton &automaton, bool complement, unsigned int state) {
	if(complement) {
		automaton.SetStateFinal(state);
	}
}

#endif
