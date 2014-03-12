#include "automata.hh"

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
void addTransition(Automaton& aut, Automaton::StateTuple q, int x, int y, char* track, int qf) {
	// TODO: add assert to tracklen
	Automaton::SymbolType bddTrack = constructUniversalTrack();
	bddTrack.SetIthVariableValue(x, track[0]);
	bddTrack.SetIthVariableValue(y, track[1]);
	aut.AddTransition(q, bddTrack, qf);
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
void addTransition(Automaton& aut, Automaton::StateTuple q, int x, char track, int qf) {
	Automaton::SymbolType bddTrack = constructUniversalTrack();
	bddTrack.SetIthVariableValue(x, track);
	aut.AddTransition(q, bddTrack, qf);
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
		Automaton::StateTuple from,
		Automaton::StateType to) {
	automaton.AddTransition(from, constructUniversalTrack(), to);
}


/**
 * Constructs universal track X^k according to the number of variables used
 * in formula, i.e. in symbol table
 *
 * @return: universal track for transition
 */
Automaton::SymbolType constructUniversalTrack() {
	unsigned int trackLen = symbolTable.noIdents;
	Automaton::SymbolType transitionTrack;
	transitionTrack.AddVariablesUpTo(trackLen);
	return transitionTrack;
}
