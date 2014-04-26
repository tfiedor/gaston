#include "mtbdd_factors.hh"

/**
 * Does determinization of MTBDD converting from VATA representation to macro
 * state representation
 *
 * @param lhs: operand
 * @return: determinized MTBDD
 */
/*inline TStateSet* StateDeterminizatorFunctor::ApplyOperation(const MTBDDLeafStateSet & lhs) {
	StateSetList states;

	for (auto state : lhs) {
		states.push_back(new LeafStateSet(state));
	}

	return new MacroStateSet(states);
}*/

/**
 * Does determinization of i-1 level
 *
 * @param lhs: operand
 * @return: determinized MTBDD
 */
/*TStateSet* MacroStateDeterminizatorFunctor::ApplyOperation(TStateSet* lhs) {
	StateSetList states;

	for (auto state : lhs) {
		states.push_back(state);
	}

	return new MacroStateSet(states);
}*/

