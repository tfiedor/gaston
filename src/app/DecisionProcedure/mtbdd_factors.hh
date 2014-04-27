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
class StateDeterminizatorFunctor : public VATA::MTBDDPkg::Apply1Functor<StateDeterminizatorFunctor, MTBDDLeafStateSet, TStateSet*> {
GCC_DIAG_ON(effc++)
public:
	// < Public Methods >
	inline TStateSet* ApplyOperation(const MTBDDLeafStateSet & lhs) {
		StateSetList states;

		if (lhs.size() != 0) {
			for (auto state : lhs) {
				states.push_back(new LeafStateSet(state));
			}
		} else {
			states.push_back(new LeafStateSet());
		}

		return new MacroStateSet(states);
	}
};

GCC_DIAG_OFF(effc++)
class MacroStateDeterminizatorFunctor : public VATA::MTBDDPkg::Apply1Functor<MacroStateDeterminizatorFunctor, TStateSet*, TStateSet*> {
GCC_DIAG_ON(effc++)
public:
	// < Public Methods >
	inline TStateSet* ApplyOperation(TStateSet* lhs) {
		StateSetList states;
		MacroStateSet* mlhs = reinterpret_cast<MacroStateSet*>(lhs);
		states.push_back(mlhs);

		return new MacroStateSet(states);
	}
};

GCC_DIAG_OFF(effc++)
class MacroUnionFunctor : public VATA::MTBDDPkg::Apply2Functor<MacroUnionFunctor, TStateSet*, TStateSet*, TStateSet*> {
GCC_DIAG_ON(effc++)
public:
	// < Public Methods >
	inline TStateSet* ApplyOperation(TStateSet* lhs, TStateSet* rhs) {
		MacroStateSet* mlhs = reinterpret_cast<MacroStateSet*>(lhs);
		MacroStateSet* mrhs = reinterpret_cast<MacroStateSet*>(rhs);
		StateSetList lhsStates = mlhs->getMacroStates();
		StateSetList rhsStates = mrhs->getMacroStates();
		for (auto state : rhsStates) {
			lhsStates.push_back(state);
		}

		return new MacroStateSet(lhsStates);
	}
};

#endif
