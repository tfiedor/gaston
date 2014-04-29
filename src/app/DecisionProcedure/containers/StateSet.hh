#ifndef __STATE_SET__
#define __STATE_SET__

#include <deque>
#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>

class TStateSet;
class MacroStateSet;
class LeafStateSet;

// < Typedefs >
typedef size_t StateType;
typedef std::deque<TStateSet*> StateSetList;

// < Enums >
enum {SET, STATE, MACROSTATE};

/**
 * Base class for StateSet class, either MacroStateSet or LeafStateSet
 */
class TStateSet {
private:
public:
	// < Public Methods >
	virtual int getType() {return SET;}
	virtual void dump() {}
	virtual bool DoCompare(TStateSet*) {return false;};
	virtual std::string ToString() {}
	friend inline std::ostream& operator<<(std::ostream& os, const TStateSet& tss) {os << "";}
};

/**
 * Representation of LeafStateSet, i.e set of plain StateType states
 *
 * Example of LeafStateSet: {1, 2, 3}
 */
class LeafStateSet : public TStateSet {
private:
	// < Private Members >
	StateType state;
	bool stateIsSink;

public:
	// < Public Methods >
	LeafStateSet (StateType q) : state(q), stateIsSink(false) {}
	LeafStateSet () : state(-1), stateIsSink(true) {}

	int getType() {return STATE;}
	void dump();
	std::string ToString();
	bool isSink() {return this->stateIsSink; }

	StateType getState();
	StateType getState() const {return this->getState();}

	/**
	 * Overloaded comparison operator
	 *
	 * @param lhs: left operator
	 * @return: true if they are the same
	 */
	bool DoCompare(TStateSet* lhs) {
		int lhsType = lhs->getType();
		if (lhsType == MACROSTATE) {
			return false;
		} else {
			LeafStateSet *lhss = dynamic_cast<LeafStateSet*>(lhs);
			return this->state == lhss->getState();
		}
	}

	/**
	  * Overloading of the << operator for Macrostate Class
	  *
	  * @param os: output stream
	  * @param mss: macro state to be outputted
	  * @return: output stream
	  */
	friend inline std::ostream& operator<<(std::ostream& os, const LeafStateSet& mss) {
		std::ostringstream ss;
		ss << mss.getState();
		os << ss.str();
	}
};

/**
 * MacroStateSet is representation of StateSet consisting of other state sets
 *
 * Example of MacroStateSet: {{{0, 1}, {2}},{{3}}}
 */
class MacroStateSet : public TStateSet {
private:
	// < Private Members >
	StateSetList macroStates;

public:
	// < Public Methods >
	MacroStateSet (StateSetList Q) : macroStates(Q) {}

	int getType() {return MACROSTATE;}
	void dump();
	std::string ToString();
	StateSetList getMacroStates();
	StateSetList getMacroStates() const;
	void addState(TStateSet* state);

	/**
	 * Overloaded comparison operator
	 *
	 * @param lhs: left operator
	 * @return: true if they are the same
	 */
	bool DoCompare(TStateSet* lhs) {
		int lhsType = lhs->getType();

		if (lhsType == STATE) {
			return false;
		} else {
			MacroStateSet* lhss = dynamic_cast<MacroStateSet*>(lhs);
			StateSetList lhsStates = lhss->getMacroStates();

			if(lhsStates.size() != this->macroStates.size()) {
				return false;
			} else {
				// iterate through all -.-
				for (auto state : this->macroStates) {
					auto matching_iter = std::find_if(lhsStates.begin(), lhsStates.end(),
							[state](TStateSet* s) {
								return s->DoCompare(state);
							});
					if (matching_iter == lhsStates.end()) {
						return false;
					}
				}

				return true;
			}
		}
	}

	/**
	 * Overloading of the << operator for Macrostate Class
	 *
	 * @param os: output stream
	 * @param mss: macro state to be outputted
	 * @return: output stream
	 */
	friend inline std::ostream& operator<<(std::ostream& os, const MacroStateSet& mss){
		os << "{";

		unsigned int numberOfStates = mss.macroStates.size();

		for (TStateSet* macroState : mss.macroStates) {
			--numberOfStates;
			os << macroState->ToString();
			if (numberOfStates != 0) {
				os << ", ";
			}
		}
		os << "}";
	}
};

#endif
