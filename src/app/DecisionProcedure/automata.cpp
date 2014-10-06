/*****************************************************************************
 *  dWiNA - Deciding WSkS using non-deterministic automata
 *
 *  Copyright (c) 2014  Tomas Fiedor <xfiedo01@stud.fit.vutbr.cz>
 *
 *  Description:
 *    Implementation of common Automata related operations
 *
 *****************************************************************************/

#include "automata.hh"

#include <cstdio>
#include <cstdlib>

/**
 * Converts character to specification of VATA
 *
 * @param[in] c: classical character from {0, 1, X}
 * @return: character from enum {ONE, ZERO, DONT_CARE} from VATA spec.
 */

char charToAsgn(char c) {
	switch(c) {
	case '0':
		return 0x01;
		break;
	case '1':
		return 0x02;
		break;
	case 'X':
		return 0x03;
		break;
	default:
		assert(false);
	}
}

/**
 * Constructs following transition for automaton
 *   q -(transition)-> qf
 *
 *   @param[in] aut: automaton, where we are adding track
 *   @param[in] q: initial state
 *   @param[in] transition: track of state
 *   @param[in] qf: post state
 */
void addTransition(Automaton& aut, unsigned int q, Automaton::SymbolType transition, unsigned int qf) {
	if(options.method == FORWARD) {
		aut.AddTransition(Automaton::StateTuple({q}), transition, qf);
	} else if (options.method == BACKWARD) {
		aut.AddTransition(Automaton::StateTuple({qf}), transition, q);
	} else {
		std::cerr << "Method not implemented\n";
		throw NotImplementedException();
	}
}

/**
 * Constructs following transition for automaton
 *   q -(transition)-> qf
 *
 *   @param[in] aut: automaton, where we are adding track
 *   @param[in] q: initial state
 *   @param[in] transition: track of state
 *   @param[in] qf: post state
 *
 *   TODO: Is this needed?
 */
void addTransition(Automaton& aut, unsigned int q, char* transition, unsigned int qf) {
	addTransition(aut, q, Automaton::SymbolType(transition), qf);
}

/**
 * Constructs following transition for automaton:
 *   q -(xx'XY'xx)-> qf
 *
 * @param aut: automaton, where we are added track of len 2
 * @param q: state tuple from which transition occurs
 * @param x: position in bdd track for first character of given transition string
 * @param y: position in bdd track for second character of given transition string
 * @param track: transition string
 * @param qf: state where we head
 */
void addTransition(Automaton& aut, unsigned int q, int x, int y, char* track, int qf) {
	// TODO: add assert to tracklen
	Automaton::SymbolType bddTrack = constructUniversalTrack();
	bddTrack.SetIthVariableValue(varMap[x], charToAsgn(track[0]));
	bddTrack.SetIthVariableValue(varMap[y], charToAsgn(track[1]));
	addTransition(aut, q, bddTrack, qf);
}

/**
 * Constructs following transition for automaton:
 *   q -(xx'X'xx)-> qf
 *
 * @param aut: automaton, where we are added track of len 2
 * @param q: state tuple from which transition occurs
 * @param x: position in bdd track for first character of given transition string
 * @param track: transition string
 * @param qf: state where we head
 */
void addTransition(Automaton& aut, unsigned int q, int x, char track, int qf) {
	Automaton::SymbolType bddTrack = constructUniversalTrack();
	bddTrack.SetIthVariableValue(varMap[x], charToAsgn(track));
	addTransition(aut, q, bddTrack, qf);
}

/**
 * Adds universal transition (X^k) to automaton leading from state from to to
 *
 * @param automaton: automaton, where we are adding universal transition
 * @param from: state tuple of left hand side of transition
 * @param to: state tuple of right hand side of transition
 */
void addUniversalTransition(
		Automaton& automaton,
		unsigned int from,
		unsigned int to) {
	addTransition(automaton, from, constructUniversalTrack(), to);
}

/**
 * Constructs universal track X^k according to the number of variables used
 * in formula, i.e. in symbol table
 *
 * @return: universal track for transition
 */
Automaton::SymbolType constructUniversalTrack() {
	unsigned int trackLen = varMap.TrackLength();
	Automaton::SymbolType transitionTrack;
	transitionTrack.AddVariablesUpTo(trackLen);
	return transitionTrack;
}
