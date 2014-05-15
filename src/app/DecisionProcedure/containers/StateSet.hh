#ifndef __STATE_SET__
#define __STATE_SET__

#include <vector>
#include <iostream>
#include <sstream>
#include <string>
#include <algorithm>

#include <boost/dynamic_bitset.hpp>

class TStateSet;
class MacroStateSet;
class LeafStateSet;

#define USE_DYNAMIC_BITSETS

// < Typedefs >
typedef size_t StateType;
typedef std::vector<TStateSet*> StateSetList;
typedef std::vector<TStateSet*>::iterator StateSetIterator;
using TLeafMask = boost::dynamic_bitset<>;

// < Enums >
enum {SET, STATE, MACROSTATE};

/**
 * Base class for StateSet class, either MacroStateSet or LeafStateSet
 */
class TStateSet {
private:
public:
	// < Static Members >
	static unsigned int stateNo;

	// < Public Members >
	unsigned int type;
	TLeafMask leaves;


	// < Public Methods >
	virtual void dump() {}
	virtual bool DoCompare(TStateSet*) {return false;};
	virtual bool CanBePruned(TStateSet*, unsigned) {return false;};
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
	LeafStateSet (StateType q) : state(q), stateIsSink(false) {type = STATE;}
	LeafStateSet () : state(-1), stateIsSink(true) {type = STATE;}

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
		if (lhs->type == MACROSTATE) {
			return false;
		} else {
			// TODO: THIS MAY BE SUICIDAL!!!!
			//LeafStateSet *lhss = reinterpret_cast<LeafStateSet*>(lhs);
			LeafStateSet* lhss = (LeafStateSet*)(lhs);
			return this->state == lhss->getState();
		}
	}

	bool CanBePruned(TStateSet* lhs, unsigned pruneUpTo) {
		if(lhs->type == MACROSTATE) {
			return false;
		} else {
			LeafStateSet* lhss = (LeafStateSet*)(lhs);
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
	MacroStateSet () { type = MACROSTATE;}
	MacroStateSet (StateSetList Q) : macroStates(Q) { type = MACROSTATE;}
	MacroStateSet (StateSetList Q, TLeafMask l) : macroStates(Q) { type = MACROSTATE; leaves = l;}
	MacroStateSet (StateSetList Q, StateType q): macroStates(Q) {
		type = MACROSTATE;
		//leaves.set(q, true);
	}

	~MacroStateSet () {
		for(StateSetIterator it = this->macroStates.begin(); it != this->macroStates.end(); ++it) {
			delete (*it);
		}
	}

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
		if (lhs->type == STATE) {
			return false;
		} else {
			// test if they are same
			// Optimization: sets of level 1 are compared by bitwise operations
			if(this->leaves.any() && lhs->leaves.any()) {
				unsigned size = TStateSet::stateNo;
				for(unsigned i = 0; i < size; ++i) {
					if(this->leaves[i] ^ lhs->leaves[i]) {
						return false;
					}
				}
				return true;
			}

			// TODO: THIS MAY BE SUICIDAL!!!!
			//MacroStateSet* lhss = reinterpret_cast<MacroStateSet*>(lhs);
			MacroStateSet* lhss = (MacroStateSet*)(lhs);
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
	 * @return True, if lhs is bigger and thus can be pruned
	 */
	bool CanBePruned(TStateSet* lhs, unsigned pruneUpTo) {
		if(pruneUpTo == -1) {
			return this->DoCompare(lhs);
		} else {
			if(lhs->type == STATE) {
				return false;
			// test if they are in relation, otherwise they can be pruned
			} else {
				if(pruneUpTo == 0) {
					if(this->leaves.any() && lhs->leaves.any()) {
						bool can = lhs->leaves.is_subset_of(this->leaves);
						if(can) {
							//std::cout << "Can be pruned lul\n";
						}
						return can;
					}
				}

				MacroStateSet* lhss = (MacroStateSet*)(lhs);
				StateSetList lhsStates = lhss->getMacroStates();

				for (auto state: lhsStates) {
					auto matching_iter = std::find_if(this->macroStates.begin(), this->macroStates.end(),
							[state, pruneUpTo](TStateSet* s) {
								return state->CanBePruned(s, pruneUpTo -1);
							});
					if (matching_iter == this->macroStates.end()) {
						return false;
					}
				}

				//std::cout << "Can be pruned lol\n";
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
