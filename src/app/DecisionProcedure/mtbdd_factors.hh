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

		for (auto state : lhs) {
			states.push_back(new LeafStateSet(state));
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

#endif
