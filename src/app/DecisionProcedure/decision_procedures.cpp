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
	std::deque<TStateSet*> processed;
	worklist.push_front(initialState);

	while(worklist.size() != 0) {
		TStateSet* q = worklist.front();
		worklist.pop_front();
		processed.push_back(q);

		if(!StateIsFinal(aut, q, determinizationNo, formulaPrefixSet)) {
			std::cout << "\nYes, there exist!\n";
			return true;
		// TODO: here should antichains take place
		} else {
			// construct the post of the state
			const MacroTransMTBDD & postMTBDD = GetMTBDDForPost(aut, q, determinizationNo, formulaPrefixSet);

			// collect reachable state to one macro state which is the successor of this procedure
			StateSetList reachable;
			MacroStateCollectorFunctor msc(reachable);
			msc(postMTBDD);
			assert(reachable.size() == 1);

			TStateSet *newPost = reachable[0];

			auto matching_iter = std::find_if(processed.begin(), processed.end(),
					[newPost](TStateSet* s) {
						return s->DoCompare(newPost);
					});
			if (matching_iter == processed.end()) {
				worklist.push_back(newPost);
			}
		}
	}
	std::cout << "\nNo, there is not T_T!\n";
	return false;
}

/**
 * Checks whether there exists an unsatisfying example for formula
 *
 * @return: true if there exists an unsat example
 */
bool existsUnsatisfyingExample(Automaton & aut, MacroStateSet* initialState, PrefixListType negFormulaPrefixSet) {
	std::cout << "Does UNSAT example exist?\n";
	// Yes this is confusing :) should be renamed
	return existsSatisfyingExample(aut, initialState, negFormulaPrefixSet);
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
bool StateIsFinal(Automaton & aut, TStateSet* state, unsigned level, PrefixListType & prefix) {
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
			if (StateIsFinal(aut, q, level - 1, prefix)) {
				state->dump();
				std::cout << " is NONFINAL\n";
				return false;
			} else {
				// TODO: THIS IS NOT DONE!!!!
				// Enqueue all its successors
				TStateSet* zeroSuccessor = GetZeroPost(aut, q, level, prefix);
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
			isFirstNeg = false;
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
	return reinterpret_cast<TransMTBDD*>(bddAsInt);
}

/**
 * Returns set of initial states of automaton
 *
 * @param aut: automaton
 * @return: set of initial states
 */
void getInitialStatesOfAutomaton(Automaton & aut, MTBDDLeafStateSet & initialStates) {
	TransMTBDD* bdd = getMTBDDForStateTuple(aut, Automaton::StateTuple());

	// TODO: solve sink state
	StateCollectorFunctor scf(initialStates);
	scf(*bdd);
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
	MTBDDLeafStateSet matrixInitialStates;
	getInitialStatesOfAutomaton(aut, matrixInitialStates);
	std::cout << "Initial states of original automaton corresponding to the matrix of formula are ";
	std::cout << VATA::Util::Convert::ToString(matrixInitialStates) << "\n";

	// first construct the set of leaf states
	StateSetList states;
	for (auto state : matrixInitialStates) {
		states.push_back(new LeafStateSet(state));
	}
	// TODO: FOR NOW ////
	//states.push_back(new LeafStateSet(1));
	///////////////////

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

/**
 * Constructs a post through zero tracks from @p state of @p level with respect
 * to @p prefix. First computes the post of the macro-state and then proceeds
 * with getting the 0 tracks successors and collecting the reachable states
 *
 * @param aut: base automaton
 * @param state: initial state we are getting zero post for
 * @param level: level of macro inception
 * @param prefix: list of variables for projection
 * @return: zero post of initial @p state
 */
TStateSet* GetZeroPost(Automaton & aut, TStateSet* state, unsigned level, PrefixListType & prefix) {
	/*MacroStateSet *macroState = reinterpret_cast<MacroStateSet*>(state);
	// get post for all states under lower level
	// do the union of these posts
	for (auto state: states) {
		MacroStateSet* transMtbdd = GetMTBDDForPost(aut, state, level - 1, prefix);
	}
	// do the prefix thingie for 00000XXXX
	// collect reachable states
	StateSetList reachableStates;
	return new MacroStateSet(reachableStates);*/
	return nullptr;
}

int getProjectionVariable(unsigned level, PrefixListType & prefix) {
	if (prefix.size() == level) {
		return 0;
	} else {
		int index = prefix.size() - 1 - level;
		return prefix[index].size();
	}
}

/**
 * Generates post of @p state, by constructing posts of lesser level and
 * doing the union of these states with projection over the prefix
 *
 * @param aut: base automaton
 * @param state: initial state we are generating post for
 * @param level: level of inception
 * @param prefix: list of variables for projection
 * @return MTBDD representing the post of the state @p state
 */
MacroTransMTBDD GetMTBDDForPost(Automaton & aut, TStateSet* state, unsigned level, PrefixListType & prefix) {
	// Convert MTBDD from VATA to MacroStateRepresentation
	if (level == 0) {
		// Is Leaf State set
		LeafStateSet* lState = reinterpret_cast<LeafStateSet*>(state);
		StateType stateValue = lState->getState();
		TransMTBDD *stateTransition = getMTBDDForStateTuple(aut, Automaton::StateTuple({stateValue}));

		int projecting = getProjectionVariable(level, prefix);
		StateDeterminizatorFunctor sdf;
		if (projecting != 0) {
			AdditionApplyFunctor adder;
			TransMTBDD projected = stateTransition->Project(
				[stateTransition, projecting](size_t var) {return var < projecting;}, adder);
			return sdf(projected);
		} else {
		// Convert to TStateSet representation
			return sdf(*stateTransition);
		}
	} else {
		MacroStateSet* mState = reinterpret_cast<MacroStateSet*>(state);
		StateSetList states = mState->getMacroStates();
		// get post for all states under lower level
		TStateSet* front = states.front();
		states.pop_front();
		MacroStateDeterminizatorFunctor msdf;
		MacroUnionFunctor muf;

		// get first and determinize it
		const MacroTransMTBDD & frontPost = GetMTBDDForPost(aut, front, level-1, prefix);
		//std::cout << "----------------------<" << level << ">----------------------\n";
		// TODO: This may not be correct, not sure atm
		int projecting = getProjectionVariable(level-1, prefix);

		//std::cout << "Projecting on level " << level << " by " << projecting << "\n\n";
		MacroTransMTBDD detResultMtbdd = (level == 1) ? frontPost : (msdf(frontPost)).Project(
				[&frontPost, projecting](size_t var) {return var <= projecting;}, muf);
		//std::cout << "BDD after projection: " << MacroTransMTBDD::DumpToDot({&detResultMtbdd});
		//std::cout << "\n\n";
		// do the union of posts represented as mtbdd
		while(!states.empty()) {
			front = states.front();
			states.pop_front();
			const MacroTransMTBDD & nextPost = GetMTBDDForPost(aut, front, level-1, prefix);
			//std::cout << "BDD before projection: " << MacroTransMTBDD::DumpToDot({&nextPost});
			//std::cout << "BDD after projection: " << MacroTransMTBDD::DumpToDot({&detResultMtbdd});
			//std::cout << "\n\n";
			//MacroTransMTBDD tempDet = msdf(nextPost);
			detResultMtbdd = muf(detResultMtbdd, (level == 1) ? nextPost : (msdf(nextPost)).Project(
					[&nextPost, projecting](size_t var) {return var <= projecting;}, muf));
		}

		//std::cout << "Returning from level " << level << "\n";
		//std::cout << "BDD: " << MacroTransMTBDD::DumpToDot({&detResultMtbdd});
		//std::cout << "\n\n";
		//std::cout << "---------------------->" << level << "<----------------------\n";

		// do projection and return;
		return detResultMtbdd;
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
int decideWS2S(Automaton & aut, TSatExample & example, TUnSatExample & counterExample) {
	std::cout << "Deciding WS2S formula transformed to automaton" << std::endl;
	throw NotImplementedException();
}
