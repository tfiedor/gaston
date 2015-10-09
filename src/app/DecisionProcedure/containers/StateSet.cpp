/*****************************************************************************
 *  dWiNA - Deciding WSkS using non-deterministic automata
 *
 *  Copyright (c) 2014  Tomas Fiedor <xfiedo01@stud.fit.vutbr.cz>
 *
 *  Description:
 *    Implementation of MacroStates - StateSet
 *
 *****************************************************************************/

#include "StateSet.hh"

unsigned int TStateSet::lastId = 0;

/* Public Methods */

/**
 * Prints Set of StateType states
 */
void LeafStateSet::dump() {
	std::cout << this->state;
}

void LeafStateSet::closed_dump(unsigned int level) {
	std::cout << this->state;
}

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
/*StateType LeafStateSet::getState() {
	return this->state;
}*/

/**
 * Return state asociated with macro-state which is nothing
 * Note: I like to do dirty things to my code
 *
 * @return: -1
 */
/*StateType MacroStateSet::getState() {
	return -1;
}*/

/**
 * Conversion to string representation
 *
 * @return: string representation of macro state
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
 * Prints representation according to the upward and downward closures
 */
void MacroStateSet::closed_dump(unsigned int level) {
	if(level % 2 == 0) {
		std::cout << "v{ ";
	} else {
		std::cout << "^"  << "{ ";
	}

	unsigned int numberOfStates = this->macroStates.size();

	for (TStateSet* macroState : this->macroStates) {
		--numberOfStates;
		macroState->closed_dump(level - 1);
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
 * TODO: Should add antichain pruning
 * @param state: state to be added
 */
void MacroStateSet::addState(TStateSet* state) {
	if(!(state->type == STATE && state->stateIsSink)) {
		auto matching_iter = std::find_if(this->macroStates.begin(), this->macroStates.end(),
			[state](TStateSet* s) {
				return s->DoCompare(state);
			});
		this->macroStates.push_back(state);
	} else {
		std::cerr << "Warning: Trying to add sink state to MacroState\n";
	}
}

/**
 * Does union of two macrostates
 *
 * @param[in] state: Macro state set we are adding (unioning with)
 */
void MacroStateSet::unionMacroSets(MacroStateSet* state) {

}

bool MacroStateSet::Intersects(MacroStateSet *states) {
	for(auto state : states->getMacroStates()) {
		auto matching_iter = std::find_if(this->macroStates.begin(), this->macroStates.end(),
			[state](TStateSet* s) {
				return s->DoCompare(state);
			});
		if(matching_iter != this->macroStates.end()) {
			return true;
		}
	}
	return false;
}

/**
 * Counts number of states in state space
 */
unsigned int MacroStateSet::measureStateSpace() {
	unsigned int count = 1;
	for(auto state : this->macroStates) {
		count += state->measureStateSpace();
	}
	return count;
}

unsigned int TStateSet::stateNo = 0;
