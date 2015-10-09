/*****************************************************************************
 *  dWiNA - Deciding WSkS using non-deterministic automata
 *
 *  Copyright (c) 2014  Tomas Fiedor <xfiedo01@stud.fit.vutbr.cz>
 *
 *  Description:
 *    MTBDD functors
 *
 *****************************************************************************/

#ifndef __MTBDD_FACTORS_H__
#define __MTBDD_FACTORS_H__

#include "mtbdd/apply1func.hh"
#include "mtbdd/apply2func.hh"
#include "mtbdd/void_apply1func.hh"
#include "containers/StateSet.hh"
#include "decision_procedures.hh"
#include <boost/range/join.hpp>

using MTBDDLeafStateSet = VATA::Util::OrdVector<StateType>;
using BaseAut_States = VATA::Util::OrdVector<StateType>;

/**
 * Family of MTBDD manipulation functors
 */

/**
 * StateDeterminizator makes out of leafs macro-states
 */
GCC_DIAG_OFF(effc++)
class StateDeterminizatorFunctor : public VATA::MTBDDPkg::Apply1Functor<StateDeterminizatorFunctor, MTBDDLeafStateSet, MacroStateSet*> {
GCC_DIAG_ON(effc++)
public:
	// < Public Methods >
	/**
	 * @param lhs: operand of determinization
	 * @return determinized macro-state
	 */
	inline MacroStateSet* ApplyOperation(const MTBDDLeafStateSet & lhs) {
		StateSetList states;
		TLeafMask mask;

		if (lhs.size() != 0) {
			for (auto state : lhs) {
				states.push_back(new LeafStateSet(state));
				mask.resize(TStateSet::stateNo, false);
				mask.set(state, true);
			}
		} else {
			//states.push_back(new LeafStateSet());
		}

		return new MacroStateSet(states, mask);
	}
};

/**
 * MacroStateDeterminizatorFunctor creates MacroStates out of leaf states
 */
GCC_DIAG_OFF(effc++)
class MacroStateDeterminizatorFunctor : public VATA::MTBDDPkg::Apply1Functor<MacroStateDeterminizatorFunctor, MacroStateSet*, MacroStateSet*> {
GCC_DIAG_ON(effc++)
public:
	// < Public Methods >
	/**
	 * @param lhs: operand - macro-state state of level i
	 * @return: macro state of level i+1
	 */
	inline MacroStateSet* ApplyOperation(MacroStateSet* lhs) {
		StateSetList states;
		states.push_back(lhs);

		return new MacroStateSet(states);
	}
};

/**
 * MacroUnionFunctor does the union of two automata, during the union states inside
 * are pruned according to the defined simulation relation to yield smaller leaves
 * in process
 */
GCC_DIAG_OFF(effc++)
class MacroUnionFunctor : public VATA::MTBDDPkg::Apply2Functor<MacroUnionFunctor, MacroStateSet*, MacroStateSet*, MacroStateSet*> {
GCC_DIAG_ON(effc++)
public:
	// < Public Methods >
	/**
	 * @param lhs: left operand
	 * @param rhs: right operand
	 * @return union of leaf operands
	 */
	inline MacroStateSet* ApplyOperation(MacroStateSet* lhs, MacroStateSet* rhs) {
		StateSetList lhsStates = lhs->getMacroStates();
		StateSetList rhsStates = rhs->getMacroStates();

		for (auto state : rhsStates) {
			// compare with other states if we can prune
			auto matching_iter = std::find_if(lhsStates.begin(), lhsStates.end(),
					[state](TStateSet* s) {
#if (PRUNE_BY_RELATION == true)
						// [TODO] Not sure if OK
						return state->CanBePruned(s, 0);
#else
						return s->DoCompare(state);
#endif
					});
			// otherwise push to the set
			if (matching_iter == lhsStates.end()) {
				lhsStates.push_back(state);
			}
		}

		// constructs the leaves bit set, if any are set, i.e. bitsets are
		// supporteted at that level
		if(lhs->leaves.any() && rhs->leaves.any()) {
			return new MacroStateSet(lhsStates, lhs->leaves | rhs->leaves);
		} else {
			return new MacroStateSet(lhsStates);
		}

	}
};


/**
 * MacroUnionFunctor does the union of two automata, during the union states inside
 * are pruned according to the defined simulation relation to yield smaller leaves
 * in process
 */
GCC_DIAG_OFF(effc++)
class MacroPrunedUnionFunctor : public VATA::MTBDDPkg::Apply2Functor<MacroPrunedUnionFunctor, MacroStateSet*, MacroStateSet*, MacroStateSet*> {
GCC_DIAG_ON(effc++)
private:

public:
	unsigned int level;

	// < Public Constructors>
	MacroPrunedUnionFunctor(unsigned int l) : level(l) {}

	// < Public Methods >
	/**
	 * @param lhs: left operand
	 * @param rhs: right operand
	 * @return union of leaf operands
	 */
	inline MacroStateSet* ApplyOperation(MacroStateSet* lhs, MacroStateSet* rhs) {
		StateSetList lhsStates = lhs->getMacroStates();
		StateSetList rhsStates = rhs->getMacroStates();
		StateSetList unionStates;

		// union of upward closed things
		auto lbegin = lhsStates.begin();
		auto rbegin = rhsStates.begin();
		auto lend = lhsStates.end();
		auto rend = rhsStates.end();
		if (level % 2 == 0) {
			for (auto it = lbegin; it != lend; ++it) {
				auto matching_iter = std::find_if(lbegin, lend,
						[it, this](TStateSet* s) {
							return (s != *it) && (*it)->isSubsumed(s, this->level);
						});
				if(matching_iter != lend) {
					continue;
				} else {
					matching_iter = std::find_if(rbegin, rend,
						[it, this](TStateSet* s) {
							return (s != *it) && (*it)->isSubsumed(s, this->level);
						});
					if(matching_iter == rend) {
						unionStates.push_back(*it);
					}
				}
			}
			for (auto it = rbegin; it != rend; ++it) {
				auto matching_iter = std::find_if(lbegin, lend,
						[it, this](TStateSet* s) {
							return (s != *it) && (*it)->isSubsumed(s, this->level);
						});
				if(matching_iter != lend) {
					continue;
				} else {
					matching_iter = std::find_if(rbegin, rend,
						[it, this](TStateSet* s) {
							return (s != *it) && (*it)->isSubsumed(s, this->level);
						});
					if(matching_iter == rend) {
						unionStates.push_back(*it);
					}
				}
			}
		} else {
			for (auto it = lbegin; it != lend; ++it) {
				auto matching_iter = std::find_if(lbegin, lend,
						[it, this](TStateSet* s) {
							return (s != *it) && s->isSubsumed((*it), this->level);
						});
				if(matching_iter == lend) {
					continue;
				} else {
					matching_iter = std::find_if(rbegin, rend,
						[it, this](TStateSet* s) {
							return (s != *it) && s->isSubsumed((*it), this->level);
						});
					if(matching_iter == rend) {
						unionStates.push_back(*it);
					}
				}
			}

			for (auto it = rbegin; it != rend; ++it) {
				auto matching_iter = std::find_if(lbegin, lend,
						[it, this](TStateSet* s) {
							return (s != *it) && s->isSubsumed((*it), this->level);
						});
				if(matching_iter != lend) {
					continue;
				} else {
					matching_iter = std::find_if(rbegin, rend,
						[it, this](TStateSet* s) {
							return (s != *it) && s->isSubsumed((*it), this->level);
						});
					if(matching_iter == rend) {
						unionStates.push_back(*it);
					}
				}
			}
		}

		// constructs the leaves bit set, if any are set, i.e. bitsets are
		// supporteted at that level

		return new MacroStateSet(unionStates);
	}
};

GCC_DIAG_OFF(effc++)
class MacroStateCollectorFunctor : public VATA::MTBDDPkg::VoidApply1Functor<MacroStateCollectorFunctor, MacroStateSet*> {
GCC_DIAG_ON(effc++)
private:
	StateSetList & collected;

public:
	// < Public Constructors >
	MacroStateCollectorFunctor(StateSetList & l) : collected(l) {}

	// < Public Methods >
	/**
	 * @param lhs: operand of apply
	 */
	inline void ApplyOperation(MacroStateSet* lhs) {
		collected.push_back(lhs);
	}
};

/**
 * StateCollectorFunctor takes a MTBDD and collects all states in leaves
 */
GCC_DIAG_OFF(effc++)
class StateCollectorFunctor : public VATA::MTBDDPkg::VoidApply1Functor<StateCollectorFunctor, BaseAut_States> {
GCC_DIAG_ON(effc++)
private:
	BaseAut_States & collected;

public:
	// < Public Constructors >
	StateCollectorFunctor(BaseAut_States & l) : collected(l) {}

	// <Public Methods >
	/**
	 * @param lhs: operand of apply
	 */
	inline void ApplyOperation(BaseAut_States lhs) {
		collected = collected.Union(lhs);
	}
};

/**
 * AdditionApplyFunctor takes two MTBDD and does the union of the leaf sets
 * used for projection.
 */
GCC_DIAG_OFF(effc++)
class AdditionApplyFunctor : public VATA::MTBDDPkg::Apply2Functor<AdditionApplyFunctor, MTBDDLeafStateSet, MTBDDLeafStateSet, MTBDDLeafStateSet> {
GCC_DIAG_ON(effc++)

public:
	/**
	 * @param lhs: left operand
	 * @param rhs: right operand
	 * @return: union of leafs
	 */
	inline MTBDDLeafStateSet ApplyOperation(const MTBDDLeafStateSet& lhs, const MTBDDLeafStateSet& rhs)
	{
		return lhs.Union(rhs);
	}
};

#endif
