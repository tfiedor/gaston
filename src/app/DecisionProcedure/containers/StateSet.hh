#ifndef __STATE_SET__
#define __STATE_SET__

#include <deque>
#include <iostream>
#include <sstream>
#include <string>

class TStateSet;

// < Typedefs >
typedef size_t StateType;
typedef std::deque<TStateSet*> StateSetList;

/**
 * Base class for StateSet class, either MacroStateSet or LeafStateSet
 */
class TStateSet {
private:
public:
	// < Public Methods >
	virtual void dump() {};
	virtual std::string ToString() {};
	friend inline std::ostream& operator<<(std::ostream& os, const TStateSet& tss) {os << "something";};
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

	void dump();
	std::string ToString();
	bool isSink() {return this->stateIsSink; }

	StateType getState();
	StateType getState() const {return this->getState();}

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
	void dump();
	std::string ToString();
	StateSetList getMacroStates();
	StateSetList getMacroStates() const;
	void addState(TStateSet* state);

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
