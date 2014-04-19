#include <cstdio>
#include <deque>
#include "environment.hh"
#include "decision_procedures.hh"

/**
 * Computes the final states from automaton
 *
 * @param aut: Automaton for matrix
 * @return: final states of automaton corresponding to final formula
 *
 * TODO: StateHT for now, this is not how it should work :)
 */
FinalStatesType computeFinalStates(Automaton aut) {
	return aut.GetFinalStates();
}

/**
 * Checks whether there exists a satisfying example for formula
 *
 * @return: true if there exists a sat example
 */
bool existsSatisfyingExample(FinalStatesType fm) {
	return fm.size() != 0;
}

/**
 * Checks whether there exists an unsatisfying example for formula
 *
 * @return: true if there exists an unsat example
 */
bool existsUnsatisfyingExample(FinalStatesType fm, StateHT qm) {
	return fm.size() != qm.size();
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
 * @param formulaPrefixSet: set of second-order variables corresponding to
 * 		the prefix of the closed formula phi
 * @param negFormulaPrefixSet: set of second-order variables corresponding to
 * 		the prefix of the closed negation of formula phi
 * @return: Decision procedure results
 */
int decideWS1S(Automaton aut, TSatExample & example, TUnSatExample & counterExample, PrefixListType formulaPrefixSet, PrefixListType negFormulaPrefixSet) {
	std::cout << "Deciding WS1S formula transformed to automaton" << std::endl;

	// Getting initial states
	const MTBDDLeafStateSet & matrixInitialStates = getInitialStatesOfAutomaton(aut);
	std::cout << "Initial states of original automaton corresponding to the matrix of formula are ";
	std::cout << VATA::Util::Convert::ToString(matrixInitialStates) << "\n";

	// Compute the final states
	StateHT allStates;
	aut.RemoveUnreachableStates(&allStates);

	FinalStatesType fm;
	fm = computeFinalStates(aut);

	bool hasExample = existsSatisfyingExample(fm);
	bool hasCounterExample = existsUnsatisfyingExample(fm, allStates);

	// No satisfiable solution was found
	if(!hasExample) {
		//counterExample = findUnsatisfyingExample();
		return UNSATISFIABLE;
	// There exists a satisfiable solution and does not exist an unsatisfiable solution
	} else if (hasExample && !hasCounterExample) {
		//example = findSatisfyingExample();
		return VALID;
	// else there only exists a satisfiable solution
	} else if (hasExample) {
		//example = findSatisfyingExample();
		//counterExample = findUnsatisfyingExample();
		return SATISFIABLE;
	// THIS SHOULD NOT HAPPEN
	} else {
		return -1;
	}
}

/**
 * Implementation of workset-based algorithm for deciding whether the given
 * macro-state is final or not. Macro-state is final if all its substates are
 * non-final
 *
 * @param state: macro state we are checking
 * @param level: level of projection
 * @return True if the macro-state is final
 */
bool StateIsFinal(MacroState state, unsigned level) {
	// return whether the state is final in automaton
	if (level == 0) {

	// level > 0
	} else {
		std::deque<MacroState> worklist;
		// TODO: fill the worklist with states of macro-state

		while (worklist.size() != 0) {
			MacroState q;// = worklist.pop_front();
			if (StateIsFinal(q, level - 1)) {
				return false;
			} else {
				// TODO: enque the successors
			}
		}

		return true;
	}

}

/**
 * Takes formula, the prefix, and converts it to the set of sets of second
 * order variables, according to the variable map;
 *
 * @param formula: formula corresponding to the prefix
 * @return: list of lists of second-order variables
 */
PrefixListType convertPrefixFormulaToList(ASTForm* formula) {
	PrefixListType list;
	VariableSet set;
	unsigned int quantifiedSize;
	unsigned int value;
	bool isFirstNeg = true;

	// empty prefix is just one empty list
	if (formula->kind == aTrue) {
		list.push_front(set);
		return list;
	}

	ASTForm* iterator = formula;
	// while we are not at the end of the prefix
	while (iterator->kind != aTrue) {
		// Add to set
		if (iterator->kind == aEx2) {
			ASTForm_Ex2* exf = (ASTForm_Ex2*) iterator;

			quantifiedSize = (exf->vl)->size();
			for (unsigned i = 0; i < quantifiedSize; ++i) {
				value = (exf->vl)->pop_front();
				set.push_front(varMap[value]);
			}
			iterator = exf->f;
		// Create new set
		} else if (iterator->kind == aNot) {
			if (!isFirstNeg) {
				list.push_front(set);
				set.clear();
			} else {
				isFirstNeg = false;
			}

			ASTForm_Not* notf = (ASTForm_Not*) iterator;
			iterator = notf->f;
		// Fail, should not happen
		} else {
			assert(false);
		}
	}

	if (set.size() != 0) {
		list.push_front(set);
	}

	return list;
}

/**
 * Does the closure of the formula, by adding the variables to the list of
 * prefix sets
 *
 * @param prefix: list of second-order variables corresponding to the prefix
 * @param freeVars: list of free variables in formula
 * @param negationIsTopMost: whether the prefix had negation on left or no
 */
void closePrefix(PrefixListType & prefix, IdentList* freeVars, bool negationIsTopmost) {
	unsigned int quantifiedSize;
	unsigned value;

	// phi = neg exists X ...
	if (negationIsTopmost) {
		// we will add new level of quantification
		VariableSet set;
		quantifiedSize = freeVars->size();
		for (unsigned i = 0; i < quantifiedSize; ++i) {
			value = freeVars->get(i);
			set.push_front(varMap[value]);
		}
		prefix.push_front(set);
	// phi = exists X ...
	} else {
		// adding to existing level of quantification
		quantifiedSize = freeVars->size();
		for (unsigned i = 0; i < quantifiedSize; ++i) {
			value = freeVars->get(i);
			prefix[0].push_front(varMap[value]);
		}
	}
}

/**
 * Gets MTBDD representation of transition relation from state tuple
 *
 * @param aut: NTA
 * @param states: states for which we want transition relation
 * @return: MTBDD corresponding to the relation
 * TODO: NOT WORKING AS IT SHOULD - DELETE MAYBE?
 */
inline void getMTBDDForStateTuple(const TransMTBDD* & bdd, Automaton aut, const StateTuple & states) {
	std::cout << "in getMTBDDForStateTuple\n";
	uintptr_t bddAsInt = aut.GetTransMTBDDForTuple(states);
	bdd = reinterpret_cast<const TransMTBDD*&> (bddAsInt);
	std::cout << VATA::Util::Convert::ToString(bdd->GetValue(constructUniversalTrack())) << "\n";
}

/**
 * Returns set of initial states of automaton
 *
 * @param aut: automaton
 * @return: set of initial states
 */
const MTBDDLeafStateSet & getInitialStatesOfAutomaton(Automaton aut) {
	const TransMTBDD* bdd;
	uintptr_t bddAsInt = aut.GetTransMTBDDForTuple(Automaton::StateTuple());
	bdd = reinterpret_cast<const TransMTBDD*> (bddAsInt);
	return (bdd->GetValue(constructUniversalTrack()));
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
