/*****************************************************************************
 *  dWiNA - Deciding WSkS using non-deterministic automata
 *
 *  Copyright (c) 2014  Tomas Fiedor <xfiedo01@stud.fit.vutbr.cz>
 *
 *  Description:
 *    WSkS Decision Procedure
 *
 *****************************************************************************/

#include <cstdio>
#include "environment.hh"
#include "decision_procedures.hh"

// Global Variables

#ifdef USE_STATECACHE
extern MultiLevelMCache<bool> StateCache;
#endif
#ifdef USE_BDDCACHE
extern MultiLevelMCache<MacroTransMTBDD> BDDCache;
#endif

/**
 * Computes the final states from automaton
 *
 * @param aut: Automaton for matrix
 * @return: final states of automaton corresponding to final formula
 */
FinalStatesType computeFinalStates(Automaton & aut) {
	return aut.GetFinalStates();
}

/**
 * Test, whether the state is already enqueued
 *
 * @param queue: queue of states
 * @param state: looked up state
 * @return: true if state is in queue
 */
bool isNotEnqueued(StateSetList & queue, TStateSet*& state, unsigned level) {
	// tries to find matching state in list/queue/wtv
	// if not found .end() is returned

	// pruning of states
	auto matching_iter = std::find_if(queue.begin(), queue.end(),
			[state, level](TStateSet* s) {
#ifdef PRUNE_BY_RELATION
				return state->CanBePruned(s, level);
#else
				return s->DoCompare(state);
#endif
			});

	return matching_iter == queue.end();
}

bool isNotEnqueued(StateSetList & queue, MacroStateSet*& state, unsigned level) {
	// tries to find matching state in list/queue/wtv
	// if not found .end() is returned

	auto matching_iter = std::find_if(queue.begin(), queue.end(),
			[state, level](TStateSet* s) {
#ifdef PRUNE_BY_RELATION
				return state->CanBePruned(s, level);
#else
				return s->DoCompare(state);
#endif
			});

	return matching_iter == queue.end();
}

/**
 * Checks whether there exists a satisfying example for formula
 *
 * @return: true if there exists a sat example
 */
bool existsSatisfyingExample(Automaton & aut, MacroStateSet* initialState, PrefixListType formulaPrefixSet) {
	unsigned int determinizationNo = formulaPrefixSet.size();
	bool stateIsFinal;

	StateSetList worklist;
	StateSetList processed;
	worklist.push_back(initialState);

	// while we have states to handle
	while(worklist.size() != 0) {
		TStateSet* q = worklist.back();
		worklist.pop_back();
		processed.push_back(q);

		// if some state is final then, there exists a satisfying example
		if(StateIsFinal(aut, q, determinizationNo, formulaPrefixSet)) {
			return true;
		} else {
			// construct the post of the state
			const MacroTransMTBDD & postMTBDD = GetMTBDDForPost(aut, q, determinizationNo, formulaPrefixSet);

			// collect reachable state to one macro state which is the successor of this procedure
			StateSetList reachable;
			MacroStateCollectorFunctor msc(reachable);
			msc(postMTBDD);

			// push all successors to workilst
			for (auto it = reachable.begin(); it != reachable.end(); ++it) {
				if (isNotEnqueued(processed, *it, determinizationNo)) {
					worklist.push_back(*it);
				}
			}
		}
	}

	// didn't find a accepting state, ending
	return false;
}

/**
 * Checks whether there exists an unsatisfying example for formula
 *
 * @return: true if there exists an unsat example
 */
bool existsUnsatisfyingExample(Automaton & aut, MacroStateSet* initialState, PrefixListType negFormulaPrefixSet) {
	// Yes this is confusing :) should be renamed
	return existsSatisfyingExample(aut, initialState, negFormulaPrefixSet);
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
 * @param formulaPrefixSet: set of second-order variables corresponding to
 * 		the prefix of the closed formula phi
 * @param negFormulaPrefixSet: set of second-order variables corresponding to
 * 		the prefix of the closed negation of formula phi
 * @return: Decision procedure results
 */
int decideWS1S(Automaton & aut, PrefixListType formulaPrefixSet, PrefixListType negFormulaPrefixSet) {
	// Number of determinizations
	unsigned formulaDeterminizations = formulaPrefixSet.size();
	unsigned negFormulaDeterminizations = negFormulaPrefixSet.size();
	unsigned cacheSize = (formulaDeterminizations >= negFormulaDeterminizations) ? formulaDeterminizations : negFormulaDeterminizations;

#ifdef USE_STATECACHE
	StateCache.extend(cacheSize);
#endif
#ifdef USE_BDDCACHE
	BDDCache.extend(cacheSize);
#endif

	if(options.dump) {
		std::cout << "[*] Commencing decision procedure for WS1S\n";
	}

	// Construct initial state of final automaton
	MacroStateSet* initialState = constructInitialState(aut, formulaDeterminizations);
	MacroStateSet* negInitialState = constructInitialState(aut, negFormulaDeterminizations);

	// Compute the final states
	StateHT allStates;
	aut.RemoveUnreachableStates(&allStates);

	// checks if there exists a satisfying example in formula
	bool hasExample = existsSatisfyingExample(aut, initialState, formulaPrefixSet);
	if(options.dump) {
		if(hasExample) {
			std::cout << "[!] Found Satisfying example in formula\n";
		} else {
			std::cout << "[-] Satisfying example not found in formula\n";

			delete initialState;
			delete negInitialState;

			return UNSATISFIABLE;
		}
	}

	// checks if there exists a unsatisfying example in formula
	bool hasCounterExample = existsUnsatisfyingExample(aut, negInitialState, negFormulaPrefixSet);
	if(options.dump) {
		if(hasCounterExample) {
			std::cout << "[!] Found Unsatisfying example in formula\n";
		} else {
			std::cout << "[-] Unsatisfying example not found in formula\n";
		}
	}

	int answer;
	// No satisfiable solution was found
	if(!hasExample) {
		answer = UNSATISFIABLE;
	// There exists a satisfiable solution and does not exist an unsatisfiable solution
	} else if (!hasCounterExample) {
		answer = VALID;
	// else there only exists a satisfiable solution
	} else if (hasExample) {
		answer = SATISFIABLE;
	// THIS SHOULD NOT HAPPEN
	} else {
		answer = -1;
	}

	delete initialState;
	delete negInitialState;

	return answer;
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
		StateSetList worklist;
		StateSetList processed;
		MacroStateSet* macroState = reinterpret_cast<MacroStateSet*>(state);

		// Look into Cache
		bool isFinal;
#ifdef USE_STATECACHE
		if(StateCache.retrieveFromCache(macroState, isFinal, level)) {
			return isFinal;
		}
#endif

		// enqueue initial states
		StateSetList states = macroState->getMacroStates();
		for (auto state : states) {
			worklist.push_back(state);
		}

		while (worklist.size() != 0) {
			TStateSet* q = worklist.back();
			worklist.pop_back();
			processed.push_back(q);

			if (StateIsFinal(aut, q, level - 1, prefix)) {
#ifdef USE_STATECACHE
				StateCache.storeIn(macroState, false, level);
#endif
				return false;
			} else {
				// Enqueue all its successors
				MacroStateSet* zeroSuccessor = GetZeroPost(aut, q, level-1, prefix);
				if ((level - 1) == 0) {
					StateSetList s = zeroSuccessor->getMacroStates();
					for(auto it = s.begin(); it != s.end(); ++it) {
						if(isNotEnqueued(processed, *it, level-1)) {
							StateType leafState = reinterpret_cast<LeafStateSet*>(*it)->getState();
							worklist.push_back(*it);
						}
					}
				} else {
					if (isNotEnqueued(processed, zeroSuccessor, level-1)) {

						worklist.push_back(zeroSuccessor);
					}
				}
			}
		}

#ifdef USE_STATECACHE
		StateCache.storeIn(macroState, true, level);
#endif
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
				set.push_back(varMap[value]);
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
	unsigned int prefixSize = prefix.size();

	// phi = neg exists X ...
	if (negationIsTopmost) {
		VariableSet s;
		prefix.push_back(s);
	}
	// we will add new level of quantification
	VariableSet set;
	quantifiedSize = freeVars->size();
	for (unsigned i = 0; i < quantifiedSize; ++i) {
		value = freeVars->get(i);
		//set.push_back(varMap[value]);
	}
	prefix.push_back(set);
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
MacroStateSet* GetZeroPost(Automaton & aut, TStateSet*& state, unsigned level, PrefixListType & prefix) {
	const MacroTransMTBDD & transPost = GetMTBDDForPost(aut, state, level, prefix);
	MacroStateSet *postStates = transPost.GetValue(constructUniversalTrack());

	return postStates;
}

/**
 * Returns variable we are projecting on level
 *
 * @param level: level of projection
 * @param prefix: prefix of formula
 * @return: number of projected variable
 */
int getProjectionVariable(unsigned level, PrefixListType & prefix) {
	int index = level;
	if (prefix[index].size() == 0) {
		return 0;
	} else {
		return prefix[index][0]+1;
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
		if (projecting > 0) {
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

		// Look into cache
#ifdef USE_BDDCACHE
		if(BDDCache.inCache(mState, level)) {
			return BDDCache.lookUp(mState, level);
		}
#endif

		StateSetList states = mState->getMacroStates();
		// get post for all states under lower level
		TStateSet* front = states.back();
		states.pop_back();
		MacroStateDeterminizatorFunctor msdf;
		MacroUnionFunctor muf;

		// get first and determinize it
		const MacroTransMTBDD & frontPost = GetMTBDDForPost(aut, front, level-1, prefix);
		int projecting = getProjectionVariable(level-1, prefix);

		MacroTransMTBDD detResultMtbdd = (level == 1) ? frontPost : (msdf(frontPost)).Project(
				[&frontPost, projecting](size_t var) {return var < projecting;}, muf);
		// do the union of posts represented as mtbdd

		while(!states.empty()) {
			front = states.back();
			states.pop_back();
			const MacroTransMTBDD & nextPost = GetMTBDDForPost(aut, front, level-1, prefix);
			detResultMtbdd = muf(detResultMtbdd, (level == 1) ? nextPost : (msdf(nextPost)).Project(
					[&nextPost, projecting](size_t var) {return var < projecting;}, muf));
		}

		// cache the results
#ifdef USE_BDDCACHE
		BDDCache.storeIn(mState, detResultMtbdd, level);
#endif

		// do projection and return;
		return detResultMtbdd;
	}
}

/**
 * Similar decision procedure, we'll not be solving WS2S at the moment
 *
 * @return: Decision procedure results
 */
int decideWS2S(Automaton & aut) {
	throw NotImplementedException();
}
