#ifndef __MTBDD_FACTORS_H__
#define __MTBDD_FACTORS_H__

#include "mtbdd/apply1func.hh"
#include "mtbdd/apply2func.hh"
#include "mtbdd/void_apply1func.hh"
#include "containers/StateSet.hh"
#include "decision_procedures.hh"

using MTBDDLeafStateSet = VATA::Util::OrdVector<StateType>;

/**
 * Family of MTBDD manipulation functors
 */

GCC_DIAG_OFF(effc++)
class StateDeterminizatorFunctor : public VATA::MTBDDPkg::Apply1Functor<StateDeterminizatorFunctor, MTBDDLeafStateSet, MacroStateSet*> {
GCC_DIAG_ON(effc++)
public:
	// < Public Methods >
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
			states.push_back(new LeafStateSet());
		}

		return new MacroStateSet(states, mask);
	}
};

GCC_DIAG_OFF(effc++)
class MacroStateDeterminizatorFunctor : public VATA::MTBDDPkg::Apply1Functor<MacroStateDeterminizatorFunctor, MacroStateSet*, MacroStateSet*> {
GCC_DIAG_ON(effc++)
public:
	// < Public Methods >
	inline MacroStateSet* ApplyOperation(MacroStateSet* lhs) {
		StateSetList states;
		states.push_back(lhs);

		return new MacroStateSet(states);
	}
};

GCC_DIAG_OFF(effc++)
class MacroUnionFunctor : public VATA::MTBDDPkg::Apply2Functor<MacroUnionFunctor, MacroStateSet*, MacroStateSet*, MacroStateSet*> {
GCC_DIAG_ON(effc++)
public:
	// < Public Methods >
	inline MacroStateSet* ApplyOperation(MacroStateSet* lhs, MacroStateSet* rhs) {
		StateSetList lhsStates = lhs->getMacroStates();
		StateSetList rhsStates = rhs->getMacroStates();

		for (auto state : rhsStates) {
			// TODO: this should be special function
			auto matching_iter = std::find_if(lhsStates.begin(), lhsStates.end(),
					[state](TStateSet* s) {
#ifdef PRUNE_BY_RELATION
						return state->CanBePruned(s);
#else
						return s->DoCompare(state);
#endif
					});
			if (matching_iter == lhsStates.end()) {
				lhsStates.push_back(state);
			}
		}
		if(lhs->leaves.any() && rhs->leaves.any()) {
			return new MacroStateSet(lhsStates, lhs->leaves | rhs->leaves);
		} else {
			return new MacroStateSet(lhsStates);
		}

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
	inline void ApplyOperation(MacroStateSet* lhs) {
		collected.push_back(lhs);
	}
};

GCC_DIAG_OFF(effc++)
class StateCollectorFunctor : public VATA::MTBDDPkg::VoidApply1Functor<StateCollectorFunctor, MTBDDLeafStateSet> {
GCC_DIAG_ON(effc++)
private:
	MTBDDLeafStateSet & collected;

public:
	// < Public Constructors >
	StateCollectorFunctor(MTBDDLeafStateSet & l) : collected(l) {}

	// <Public Methods >
	inline void ApplyOperation(MTBDDLeafStateSet lhs) {
		collected = collected.Union(lhs);
	}
};

GCC_DIAG_OFF(effc++)
class AdditionApplyFunctor : public VATA::MTBDDPkg::Apply2Functor<AdditionApplyFunctor, MTBDDLeafStateSet, MTBDDLeafStateSet, MTBDDLeafStateSet> {
GCC_DIAG_ON(effc++)

public:

	inline MTBDDLeafStateSet ApplyOperation(const MTBDDLeafStateSet& lhs, const MTBDDLeafStateSet& rhs)
	{
		return lhs.Union(rhs);
	}
};

#endif
