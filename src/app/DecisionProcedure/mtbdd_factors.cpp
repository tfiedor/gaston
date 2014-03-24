#include "mtbdd_factors.hh"

/**
 * Implementation of ith-projection over MTBDD
 *
 * @param lhs: left side of functor
 * @return: set after ith-projection
 */
StateSet IthProjectionFunctor::ApplyOperation(const StateSet & lhs) {
	return lhs;
}

/**
 * Implementation of minisation of MTBDD represented transition function
 *
 * @param lhs: left side of functor
 * @return: minimised set
 */
StateSet MinimiseSetFunctor::ApplyOperation(const StateSet & lhs) {
	return lhs;
}

/**
 * Implementation of union and minimisation of two MTBDD
 *
 * @param lhs: left side of functor
 * @param rhs: right side of functor
 * @return: minimised union of lhs and rhs;
 */
StateSet MinimiseUnionFunctor::ApplyOperation(const StateSet & lhs, const StateSet & rhs) {
	return lhs;
}

/**
 * Collects new states to functor
 *
 * @param lhs: left side of functor
 */
void WorksetCollectorFunctor::ApplyOperation(const StateSet & lhs) {

}
