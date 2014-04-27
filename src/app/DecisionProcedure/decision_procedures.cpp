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
FinalStatesType computeFinalStates(Automaton & aut) {
	return aut.GetFinalStates();
}

/**
 * Checks whether there exists a satisfying example for formula
 *
 * TODO: final projection
 * @return: true if there exists a sat example
 */
bool existsSatisfyingExample(Automaton & aut, MacroStateSet* initialState, PrefixListType formulaPrefixSet) {
	std::cout << "Does SAT example exist?\n";
	unsigned int determinizationNo = formulaPrefixSet.size();
	bool stateIsFinal;

	std::deque<TStateSet*> worklist;
	worklist.push_front(initialState);

	while(worklist.size() != 0) {
		TStateSet* q = worklist.front();
		worklist.pop_front();
		if(StateIsFinal(aut, q, determinizationNo)) {
			return true;
		} else {
			TStateSet* zeroSucc = GetZeroPost(aut, q, determinizationNo);
		}
	}

	return false;
}

/**
 * Checks whether there exists an unsatisfying example for formula
 *
 * @return: true if there exists an unsat example
 */
bool existsUnsatisfyingExample(Automaton & aut, MacroStateSet* initialState, PrefixListType negFormulaPrefixSet) {
	std::cout << "Does UNSAT example exist?\n";
	unsigned int determinizationNo = negFormulaPrefixSet.size();

	bool initialStateIsFinal = StateIsFinal(aut, initialState, determinizationNo);
	return initialStateIsFinal;
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
int decideWS1S(Automaton & aut, TSatExample & example, TUnSatExample & counterExample, PrefixListType formulaPrefixSet, PrefixListType negFormulaPrefixSet) {
	std::cout << "Deciding WS1S formula transformed to automaton" << std::endl;

	// Construct initial state of final automaton
	MacroStateSet* initialState = constructInitialState(aut, formulaPrefixSet.size());
	initialState->dump();
	MacroStateSet* negInitialState = constructInitialState(aut, negFormulaPrefixSet.size());

	// Compute the final states
	StateHT allStates;
	aut.RemoveUnreachableStates(&allStates);

	FinalStatesType fm;
	fm = computeFinalStates(aut);

	bool hasExample = existsSatisfyingExample(aut, initialState, formulaPrefixSet);
	bool hasCounterExample = existsUnsatisfyingExample(aut, negInitialState, negFormulaPrefixSet);

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
 * @param aut: automaton // FOR NOW! may be replaced by cache
 * @param state: macro state we are checking
 * @param level: level of projection
 * @return True if the macro-state is final
 */
bool StateIsFinal(Automaton & aut, TStateSet* state, unsigned level) {
	// return whether the state is final in automaton
	if (level == 0) {
		LeafStateSet* leaf = reinterpret_cast<LeafStateSet*>(state);
		StateType q = leaf->getState();
		return aut.IsStateFinal(q);
	// level > 0
	} else {
		std::deque<TStateSet*> worklist;
		MacroStateSet* macroState = reinterpret_cast<MacroStateSet*>(state);
		StateSetList states = macroState->getMacroStates();
		for (auto state : states) {
			worklist.push_back(state);
		}

		while (worklist.size() != 0) {
			TStateSet* q = worklist.front();
			worklist.pop_front();
			if (StateIsFinal(aut, q, level - 1)) {
				state->dump();
				std::cout << " is NONFINAL\n";
				return false;
			} else {
				// Enqueue all its successors
				TStateSet* zeroSuccessor = GetZeroPost(aut, q, level);
				if (zeroSuccessor != nullptr)
					worklist.push_front(zeroSuccessor);
			}
		}

		state->dump();
		std::cout << " is FINAL\n";
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
 */
TransMTBDD* getMTBDDForStateTuple(Automaton & aut, const StateTuple & states) {
	uintptr_t bddAsInt = aut.GetTransMTBDDForTuple(states);
	std::cout << bddAsInt << "\n";
	return reinterpret_cast<TransMTBDD*>(bddAsInt);
}

/**
 * Returns set of initial states of automaton
 *
 * @param aut: automaton
 * @return: set of initial states
 */
const MTBDDLeafStateSet & getInitialStatesOfAutomaton(Automaton & aut) {
	TransMTBDD* bdd = getMTBDDForStateTuple(aut, Automaton::StateTuple());

	// SOME TEEEEEEEESTS /////////////////////
	std::cout << "\nSome teeests\n\n";
	StateDeterminizatorFunctor sdFunctor;
	MacroTransMTBDD mbdd = sdFunctor(*bdd);
	MacroTransMTBDD *mbdd_ptr = &mbdd;
	TStateSet* states = mbdd_ptr->GetValue(constructUniversalTrack());
	//states->dump();
	std::cout << MacroTransMTBDD::DumpToDot({&mbdd}) << "\n\n";
	std::cout << (*reinterpret_cast<MacroStateSet*>(states));

	std::cout << "\nMore teeests\n\n";
	MacroStateDeterminizatorFunctor msdFunctor;
	MacroTransMTBDD mbdd2 = msdFunctor(mbdd);
	TStateSet* dStates = mbdd2.GetValue(constructUniversalTrack());
	//dStates->dump();
	//std::cout << MacroTransMTBDD::DumpToDot({&mbdd2}) << "\n\n";

	MacroUnionFunctor muFunctor;
	MacroTransMTBDD mbdd3 = muFunctor(mbdd2, mbdd2);
	TStateSet* muStates = mbdd3.GetValue(constructUniversalTrack());
	//muStates->dump();
	std::cout << "\n\n";

	///////////////////////////////////////////
	return (bdd->GetValue(constructUniversalTrack()));
}

/**
 * Constructs new initial state for the final automaton, according to the
 * number of determinizations that we are going to need.
 *
 * @param aut: matrix automaton
 * @param numberOfDeterminizations: how many levels we will need
 * @return initial state of the final automaton
 */
MacroStateSet* constructInitialState(Automaton & aut, unsigned numberOfDeterminizations) {
	// Getting initial states
	const MTBDDLeafStateSet & matrixInitialStates = getInitialStatesOfAutomaton(aut);
	std::cout << "Initial states of original automaton corresponding to the matrix of formula are ";
	std::cout << VATA::Util::Convert::ToString(matrixInitialStates) << "\n";

	// first construct the set of leaf states
	StateSetList states;
	for (auto state : matrixInitialStates) {
		states.push_back(new LeafStateSet(state));
	}

	// now add some levels
	MacroStateSet* ithState;
	while (numberOfDeterminizations != 0){
		ithState = new MacroStateSet(states);
		states.clear();
		states.push_back(ithState);
		--numberOfDeterminizations;
	}

	return ithState;
}

TStateSet* GetZeroPost(Automaton & aut, TStateSet* state, unsigned level) {
	// get post for all states under lower level
	// do the union of these posts
	// do the prefix thingie for 00000XXXX
	// collect reachable states
	return nullptr;
}

MacroTransMTBDD* GetMTBDDForPost(Automaton & aut, TStateSet* state, unsigned level) {
	// Convert MTBDD from VATA to MacroStateRepresentation
	if (level == 0) {

	} else {
	// get post for all states under lower level
	// do the union of posts represented as mtbdd
	// do projection
	// return it
	}
	return nullptr;
}

/**
 * Similar decision procedure, we'll not be solving WS2S at the moment
 * since it's probably far too hard than expected
 *
 * @param example: satisfiable example for WS1S formula
 * @param counterExample: unsatisfiable counter-example for formula
 * @return: Decision procedure results
 */
int decideWS2S(Automaton & aut, TSatExample & example, TUnSatExample & counterExample) {
	std::cout << "Deciding WS2S formula transformed to automaton" << std::endl;
	throw NotImplementedException();
}
