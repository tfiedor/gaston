#ifndef __MTBDD_FACTORS_H__
#define __MTBDD_FACTORS_H__

#include "mtbdd/apply1func.hh"

/**
 * Family of MTBDD manipulation functors
 */

/**
 * Class for doing the IthProjection of the MTBDD
 */
GCC_DIAG_OFF(effc++)
class IthProjectionFunctor : VATA::MTBDDPkg::Apply1Functor<
	IthProjectionFunctor, StateSet, StateSet> {
GCC_DIAG_ON(effc++)
	public:
	// < Public Methods >
	StateSet ApplyOperation(const StateSet & lhs);
};

/**
 * Class for iterating over the MTBDD and minimizing it
 */
GCC_DIAG_OFF(effc++)
class MinimiseSetFunctor : VATA::MTBDDPkg::Apply1Functor<
	MinimiseSetFunctor, StateSet, StateSet> {
GCC_DIAG_ON(effc++)
	public:
	// < Public Methods >
	StateSet ApplyOperation(const StateSet & lhs);
}

/**
 * Class for doing the union of two MTBDD with minimisation
 */
GCC_DIAG_OFF(effc++)
class MinimiseUnionFunctor : VATA::MTBDDPkg::Apply2Functor<
	MinimiseUnionFunctor, StateSet, StateSet, StateSet> {
GCC_DIAG_ON(effc++)
	public:
	// < Public Methods >
	StateSet ApplyOperation(const StateSet & lhs, const StateSet & rhs);
}

/**
 * Collects newly created states to workset
 */
GCC_DIAG_OFF(effc++)
class WorksetCollectorFunctor : VATA::MTBDDPkg::VoidApply1Functor<
	WorksetCollectorFunctor, StateSet> {
GCC_DIAG_ON(effc++)
	public:
	// < Public Methods >
	void ApplyOperation(const StateSet & lhs);
}

#endif
