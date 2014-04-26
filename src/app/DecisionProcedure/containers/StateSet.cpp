#include "StateSet.hh"

/* Public Methods */

/**
 * Prints Set of StateType states
 */
void LeafStateSet::dump() {
	std::cout << this->state;
}

StateType LeafStateSet::getState() {
	return this->state;
}

/**
 * Prints Set of StateType states
 */
void MacroStateSet::dump() {
	std::cout << "{";

	unsigned int numberOfStates = this->macroStates.size();

	for (TStateSet* macroState : this->macroStates) {
		--numberOfStates;
		macroState->dump();
		if (numberOfStates != 0) {
			std::cout << ", ";
		}
	}
	std::cout << "}";
}

StateSetList MacroStateSet::getMacroStates() {
	return this->macroStates;
}

StateSetList MacroStateSet::getMacroStates() const {
	return this->macroStates;
}
