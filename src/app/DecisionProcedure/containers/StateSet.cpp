#include "StateSet.hh"

/* Public Methods */

/**
 * Prints Set of StateType states
 */
void LeafStateSet::dump() {
	std::cout << this->state;
}

/*std::ostream& operator<<(std::ostream& os, const LeafStateSet& mss) {
	std::ostringstream ss;
	ss << mss.state;
	os << ss.str();
}*/

/**
 * Overloaded function for conversion to string
 *
 * @return: string representation of leaf, i.e 2 f.e.
 */
std::string LeafStateSet::ToString() {
	std::ostringstream ss;
	ss << this->state;
	return ss.str();
}

/**
 * Returns state asociated with leaf
 *
 * @return: state of the leaf
 */
StateType LeafStateSet::getState() {
	return this->state;
}

/**
 * Conversion to string representation
 *
 * @return: string representation of macro state
 *
 * TODO: may not be needed
 */
std::string MacroStateSet::ToString() {
	std::ostringstream ss;
	ss << "{";
	StateSetList states = this->macroStates;
	unsigned int numberOfStates = this->macroStates.size();
	for (auto state : states) {
		--numberOfStates;
		ss << state->ToString();
		if (numberOfStates != 0) {
			ss << ", ";
		}
	}
	ss << "}";
	return ss.str();
}

/**
 * Prints Set of StateType states
 */
void MacroStateSet::dump() {
	std::cout << "{";

	unsigned int numberOfStates = this->macroStates.size();

	// Each macro state is dumped
	for (TStateSet* macroState : this->macroStates) {
		--numberOfStates;
		macroState->dump();
		if (numberOfStates != 0) {
			std::cout << ", ";
		}
	}
	std::cout << "}";
}

/**
 * Retrieves list of states contained in macro state
 *
 * @return: list of state sets
 */
StateSetList MacroStateSet::getMacroStates() {
	return this->macroStates;
}

/**
 * Const variation of getMacroStates() function
 *
 * @return: list of state sets
 */
StateSetList MacroStateSet::getMacroStates() const {
	return this->macroStates;
}

/**
 * Adds state to the macro set
 *
 * @param state: state to be added
 */
void MacroStateSet::addState(TStateSet* state) {
	this->macroStates.push_back(state);
}
