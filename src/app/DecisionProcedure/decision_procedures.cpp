#include <cstdio>
#include "environment.hh"
#include "decision_procedures.hh"

/**
 * Checks whether there exists a satisfying example for formula
 *
 * @return: true if there exists a sat example
 */
bool existsSatisfyingExample() {
	return false;
}

/**
 * Checks whether there exists an unsatisfying example for formula
 *
 * @return: true if there exists an unsat example
 */
bool existsUnsatisfyingExample() {
	return false;
}

/**
 * Tries to construct a satisfiable example from automaton
 *
 * @return: satisfying example for formula
 */
TSatExample findSatisfyingExample() {
	return 0;
}

/**
 * Tries to construct an unsatisfiable example for automaton
 *
 * @return: unsatisfiable example for formula
 */
TUnSatExample findUnsatisfyingExample() {
	return 1;
}

/**
 * Performs a decision procedure of automaton corresponding to the formula phi
 * This takes several steps, as first we compute the final states of the
 * corresponding subset construction automaton and then try to find some
 * example and counterexample for the automaton/formula
 *
 * It holds, that formula is unsatisfiable if there does not exist such
 * example, valid if there does not exists a unsatisfiable counterexample
 * and else it is satisfiable.
 *
 * @param example: satisfiable example for WS1S formula
 * @param counterExample: unsatisfiable counter-example for formula
 * @return: Decision procedure results
 */
int decideWS1S(Automaton aut, TSatExample & example, TUnSatExample & counterExample) {
	std::cout << "Deciding WS1S formula transformed to automaton" << std::endl;

	bool hasExample = existsSatisfyingExample();
	bool hasCounterExample = existsUnsatisfyingExample();

	// No satisfiable solution was found
	if(!hasExample) {
		counterExample = findUnsatisfyingExample();
		return UNSATISFIABLE;
	// There exists a satisfiable solution and does not exist an unsatisfiable solution
	} else if (hasExample && !hasCounterExample) {
		example = findSatisfyingExample();
		return VALID;
	// else there only exists a satisfiable solution
	} else if (hasExample) {
		example = findSatisfyingExample();
		counterExample = findUnsatisfyingExample();
		return SATISFIABLE;
	// THIS SHOULD NOT HAPPEN
	} else {
		return -1;
	}
}

/**
 * Similar decision procedure, we'll not be solving WS2S at the moment
 * since it's probably far too hard than expected
 *
 * @param example: satisfiable example for WS1S formula
 * @param counterExample: unsatisfiable counter-example for formula
 * @return: Decision procedure results
 */
int decideWS2S(Automaton aut, TSatExample & example, TUnSatExample & counterExample) {
	std::cout << "Deciding WS2S formula transformed to automaton" << std::endl;
	throw NotImplementedException();
}
