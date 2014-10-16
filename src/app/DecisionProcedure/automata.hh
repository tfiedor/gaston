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

// VATA headers
#include <vata/bdd_bu_tree_aut.hh>
#include <vata/parsing/timbuk_parser.hh>
#include <vata/serialization/timbuk_serializer.hh>
#include <vata/util/binary_relation.hh>

#include "../Frontend/symboltable.h"
#include "../Frontend/st_dfa.h"
#include "../Frontend/env.h"
#include "containers/VarToTrackMap.hh"
#include "environment.hh"

extern VarToTrackMap varMap;
extern SymbolTable symbolTable;
extern Options options;

using Automaton = VATA::BDDBottomUpTreeAut;

char charToAsgn(char c);
void addTransition(Automaton& aut, unsigned int q, int x, int y, char* track, int qf);
void addTransition(Automaton& aut, unsigned int q, int x, char track, int qf);
void addTransition(Automaton& aut, unsigned int q, char* transition, unsigned qf);
void addTransition(Automaton& aut, unsigned int q, Automaton::SymbolType transition, unsigned qf);
void addUniversalTransition(Automaton& automaton, unsigned int from, unsigned int to);
void convertMonaToVataAutomaton(Automaton& v_aut, DFA* m_aut, IdentList* vars, int varNum, unsigned* offsets);

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
	// @see setInitialState for comment
	if(!complement) {
		if(options.method == FORWARD) {
			automaton.SetStateFinal(state);
		} else if(options.method == BACKWARD) {
			automaton.AddTransition(Automaton::StateTuple({}), constructUniversalTrack(), state);
		} else {
			std::cerr << "Method not implemented yet!";
			throw NotImplementedException();
		}
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
		if(options.method == FORWARD) {
			automaton.SetStateFinal(state);
		} else if(options.method == BACKWARD) {
			automaton.AddTransition(Automaton::StateTuple({}), constructUniversalTrack(), state);
		} else {
			std::cerr << "Method not implemented yet!";
			throw NotImplementedException();
		}
	}
}

/**
 * Sets state as initial, that means, that from state {} there exists a
 * transition over tracks X
 *
 * @param[in] automaton: automaton, where we are setting states
 * @param[in] state: which state should be initial
 */
inline void setInitialState(Automaton &automaton, unsigned int state) {
	// In forward construction, we are working with posts, so every initial
	// state has a transition over tracks X from initial tuple {}
	if(options.method == FORWARD){
		automaton.AddTransition(Automaton::StateTuple({}), constructUniversalTrack(), state);
	// In backward construction, we want to work with pres, so every initial
	// state is a final one here
	} else if(options.method == BACKWARD) {
		automaton.SetStateFinal(state);
	} else {
		std::cerr << "Method not implemented yet!";
		throw NotImplementedException();
	}
}

#endif
