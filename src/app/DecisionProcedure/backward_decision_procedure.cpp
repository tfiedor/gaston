/*****************************************************************************
 *  dWiNA - Deciding WSkS using non-deterministic automata
 *
 *  Copyright (c) 2014  Tomas Fiedor <ifiedortom@fit.vutbr.cz>
 *
 *  Description:
 *    WSkS Decision Procedure
 *
 *****************************************************************************/

#include <boost/dynamic_bitset.hpp>

#include "environment.hh"
#include "decision_procedures.hh"

//#define DEBUG_BDP
//#define DEBUG_PREFIX
#define PRUNE_BY_SUBSUMPTION

// Global Variables

#ifdef USE_BDDCACHE
extern MultiLevelMCache<MacroTransMTBDD> BDDCache;
#endif

/**
 * The core of the algorithm, the very special and superb and awesome and
 * completely mind-blowing principle of Ondra and Lukas.
 * Wow. So easy. Much power. Many admirations.
 *
 * Basically we screw the upward/downward closed structure of the generators
 * and work only with generators and computes predecessors through zero tracks.
 * Since the automaton is reversed we basically work with posts, most of which
 * is implemented in forward procedure (yay!).
 *
 * TODO: Pruning by simulation relation
 *
 * @param[in] aut: base automaton
 * @param[in] prefix: prefix, list of second-order variables
 * @param[in] detNo: number of determizations needed
 * @return: MacroState representing all final states
 */
MacroStateSet* computeFinalStates(Automaton &aut, PrefixListType prefix, unsigned int detNo) {
	StateSetList worklist;
	StateSetList processed;
	StateSetList states;
	boost::dynamic_bitset<> leafQueue;
	leafQueue.resize(TStateSet::stateNo+1);

	if (detNo == 0) {
		// Since we are working with pre, final states are actual initial
		MTBDDLeafStateSet matrixInitialStates;
		getInitialStatesOfAutomaton(aut, matrixInitialStates);

		for (auto state : matrixInitialStates) {
			LeafStateSet *newLeaf = new LeafStateSet(state);
			leafQueue.set(newLeaf->state+1);
			worklist.push_back(newLeaf);
			states.push_back(newLeaf);
		}
	} else {
		MacroStateSet *finalStatesBelow = computeFinalStates(aut, prefix, detNo-1);
#ifdef DEBUG_BDP
		std::cout << "[computeFinalStates] Dumping final states from level " << detNo - 1 << "\n";
		finalStatesBelow->dump();
		std::cout << "\n";
#endif
		worklist.push_back(finalStatesBelow);
		states.push_back(finalStatesBelow);
	}

	unsigned int i = 0;
	while(worklist.size() != 0) {
		TStateSet* q = worklist.back();
		worklist.pop_back();
		processed.push_back(q);

#ifdef DEBUG_BDP
		std::cout << "[computeFinalStates] Dumping actual working state, iteration " << i++ << "\n";
		q->dump();
		std::cout << "\n\n";
#endif

		TStateSet* predecessors = GetZeroMacroPost(aut, q, detNo, prefix);
#ifdef DEBUG_BDP
		std::cout << "[computeFinalStates] Dumping predecessor of current working state: \n";
		predecessors->dump();
		std::cout << "\n";
#endif

		for(auto state : ((MacroStateSet*)predecessors)->getMacroStates()) {
#ifdef PRUNE_BY_SUBSUMPTION
			if (detNo == 0) {
				unsigned int pos = state->state+1;
				if(!leafQueue.test(pos)) {
					worklist.push_back(state);
					states.push_back(state);
					leafQueue.set(pos, true);
				}
			// pruning upward closed things
			} else if(detNo % 2 == 0) {
				auto matching_iter = std::find_if(processed.begin(), processed.end(),
						[state, detNo](TStateSet* s) {
							return state->isSubsumed(s, detNo);
						});
				if(matching_iter == processed.end()) {
					worklist.push_back(state);
					states.push_back(state);
				} else {
#ifdef DEBUG_BDP
					std::cout << "[isSubsumed] Pruning upward closed state\n";
					state->dump();
				    std::cout << "\n";
#endif
				}
			// pruning downward closed things
			} else {
				auto matching_iter = std::find_if(processed.begin(), processed.end(),
						[state, detNo](TStateSet* s) {
							return s->isSubsumed(state, detNo);
						});
				if(matching_iter == processed.end()) {
					worklist.push_back(state);
					states.push_back(state);
				} else {
#ifdef DEBUG_BDP

					std::cout << "[isSubsumed] Pruning downward closed state\n";
					state->dump();
					MacroStateSet* z = new MacroStateSet(states);
					std::cout << "\n";
					z->dump();
					//delete state;
				    std::cout << "\n";
#endif
				}
			}
#else
			if(isNotEnqueued(processed, state, detNo)) {
				worklist.push_back(state);
				states.push_back(state);
			}
			std::cout << "\n";
#endif
		}

	}

#ifdef PRUNE_BY_SUBSUMPTION
	StateSetList pruned;
	MacroStateSet* z;
	if(detNo == 0) {
		z = new MacroStateSet(states);
	} else {
		//std::cout << "States no = " << states.size() << "\n";
		if(detNo % 2 == 0) {
			while(!states.empty()) {
				TStateSet* front = states.back();
				states.pop_back();
				auto matching_iter = std::find_if(states.begin(), states.end(),
						[front, detNo](TStateSet* s) {
							return s->isSubsumed(front, detNo);
						});
				if(matching_iter == states.end()) {
					pruned.push_back(front);
					//std::cout << "Fuck you dimwit\n";
				} else {
					/*std::cout << "[isSubsumed] Pruning state at last\n";
					front->dump();
					std::cout << "\n";*/
				}
			}
		} else {
			while(!states.empty()) {
				TStateSet* front = states.back();
				states.pop_back();
				auto matching_iter = std::find_if(states.begin(), states.end(),
						[front, detNo](TStateSet* s) {
							return front->isSubsumed(s, detNo);
						});
				if(matching_iter == states.end()) {
					pruned.push_back(front);
					//std::cout << "Fuck you dimwit\n";
				} else {
					/*std::cout << "[isSubsumed] Pruning state at last\n";
					front->dump();
					std::cout << "\n";*/
				}
			}
		}
		z = new MacroStateSet(pruned);
	}

#else
	MacroStateSet* z = new MacroStateSet(states);
#endif

#ifdef DEBUG_BDP
	std::cout << "[computeFinalStates] Returning Z:";
	z->dump();
	std::cout << "\n";
	std::cout << "[-----------------------------------------------------------------]\n";
#endif

	return z;
}

/**
 * Tests if initial state is in final states or not.
 *
 * Current implementation should work like this: There are two kinds of terms
 * at trees: Downward-closed Sets (DcS) and Upward Closed mit dem Krügel Operator Sets (UcS)
 * for DcS the looked-up initial state should be contain in the DcS, ergo. it is OR node,
 * and for UcS it should be completely contained, so it is AND node.
 *
 * TODO: MacroStateSet partitioning
 *
 * @param[in] initial: initial state of automaton
 * @param[in] finalStates: set of final states
 * @return: True if initial is in finalStates
 */
bool initialStateIsInFinalStates(MacroStateSet *initial, MacroStateSet *finalStates, unsigned int level) {
	// This probably will be more problematic than we think
	if(level == 1) {
		// TODO: This may need some optimizations
		for(auto state : ((MacroStateSet *) finalStates)->getMacroStates()) {
			bool isCovered = false;
			for(auto istate : ((MacroStateSet *) initial)->getMacroStates()) {
				for(auto lstate : ((MacroStateSet *) state)->getMacroStates()) {
					if(istate->DoCompare(lstate)) {
						isCovered = true;
						break;
					}
				}
				if(isCovered) {
					break;
				}
			}
			if(!isCovered) {
				//std::cout << "return false, something not covered;\n";
				return false;
			}
		}
		//std::cout << "return true;\n";
		return true;
	} else {
		// is singleton, so we get the first
		MacroStateSet* newInitialStateSet = (MacroStateSet*) (initial->getMacroStates())[0];
		if(level % 2 == 0) {
			// Downward closed
			StateSetList members = finalStates->getMacroStates();
			for(auto state : members) {
				if(initialStateIsInFinalStates(newInitialStateSet, (MacroStateSet*) state, level - 1)) {
					return true;
				}
			}
			return false;
		// level % 2 == 1
		} else {
			// Upward closed
			StateSetList members = finalStates->getMacroStates();
			for (auto state : members) {
				if(!initialStateIsInFinalStates(newInitialStateSet, (MacroStateSet*) state, level - 1)) {
					return false;
				}
			}
			return true;
		}
	}
}

/**
 * Constructs a set of final state of automaton and then tests if the initial
 * state is subset of the final states;
 *
 * @param[in] aut: base automaton
 * @param[in] prefix: list of second-order variables for projection
 */
bool testValidity(Automaton &aut, PrefixListType prefix, bool topmostIsNegation) {
	unsigned int determinizationNumber = prefix.size();
#ifdef DEBUG_PREFIX
	for(auto it = prefix.begin(); it != prefix.end(); ++it) {
		std::cout << "[";
		for(auto itt = (*it).begin(); itt != (*it).end(); ++itt) {
			std::cout << (*itt) << ", ";
		}
		std::cout << "] ";
	}
	std::cout << "\n";
#endif

	MacroStateSet* initialState = constructInitialState(aut, determinizationNumber);
#ifdef DEBUG_BDP
	std::cout << "[testValidity] Dumping initial state:\n";
	initialState->dump();
	std::cout << "\n";
#endif

	// compute the final set of states
	StateSetList states;
	MacroStateSet* predFinalStates = computeFinalStates(aut, prefix, determinizationNumber-1);
	states.push_back(predFinalStates);
	MacroStateSet* finalStates = new MacroStateSet(states);
	std::cout << "[*] Size of the searched space: " << finalStates->measureStateSpace() << "\n";

#ifdef DEBUG_BDP
	std::cout << "[testValidity] Dumping computed final states:\n";
	finalStates->closed_dump(determinizationNumber);
	std::cout << "\n";
#endif

	// if initial state is in final states then validity holds
	bool result;
	if(determinizationNumber % 2 == 0) {
		result = initialStateIsInFinalStates(initialState, finalStates, determinizationNumber);
	} else {
		result = !initialStateIsInFinalStates(initialState, finalStates, determinizationNumber);
	}

	if(topmostIsNegation) {
		return result;
	} else {
		return !result;
	}
}

/**
 * Implementation of backward decision procedurefor WSkS. We try to compute
 * final sets from backwards and then test if initial state is in the set of
 * final sets to decide the formula
 *
 * @param[in] aut: base automaton (quantifier free)
 * @param[in] formulaPrefixSet: set of second-order variables corresponding to
 * 		the prefix of the closed formula phi
 * @param[in] negFormulaPrefixSet: set of second-order variables corresponding to
 * 		the prefix of the closed negation of formula phi
 * @param[in] formulaIsGround: true if formula is ground, i.e. there are no free vars
 *      Note, that for ground formula, there exists only two answers, as formula
 *      can either be valid or unsatisfiable.
 * @return: Decision procedure results
 */
int decideWS1S_backwards(Automaton &aut, PrefixListType formulaPrefixSet, PrefixListType negFormulaPrefixSet, bool formulaIsGround, bool topmostIsNegation) {
	if(options.dump) {
		std::cout << "[*] Commencing backward decision procedure for WS1S\n";
	}

//#ifdef DEBUG_DP
	if(formulaIsGround) {
		std::cout << "[*] Formula is ground\n";
	} else {
		std::cout << "[*] Formula is not ground\n";
	}
//#endif

	// If formula is ground, then we only test validity/unsat and not satisfiablity
	if(formulaIsGround) {
		bool formulaIsValid = testValidity(aut, formulaPrefixSet, topmostIsNegation);
		if(formulaIsValid) {
			return VALID;
		} else {
			return UNSATISFIABLE;
		}
	// If formula is unground and closed formula is valid, then formula may still
	// be invalid, so we have to test validity of closure of negation of formula.
	// If negation of formula after closure is the negation valid, then we can
	// say, that there exists a counterexample and hence is the formula sat.
	} else {
		bool formulaIsValid = testValidity(aut, formulaPrefixSet, topmostIsNegation);
		// formula is UNSAT
		if(!formulaIsValid) {
			return UNSATISFIABLE;
		} else {
			bool formulaIsValid = !testValidity(aut, negFormulaPrefixSet, topmostIsNegation);
			if(formulaIsValid) {
				return VALID;
			} else {
				return SATISFIABLE;
			}
		}
	}
}
