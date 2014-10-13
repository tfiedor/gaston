/*****************************************************************************
 *  dWiNA - Deciding WSkS using non-deterministic automata
 *
 *  Copyright (c) 2014  Tomas Fiedor <xfiedo01@stud.fit.vutbr.cz>
 *
 *  Description:
 *    Implementation of generic cache
 *
 *****************************************************************************/

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
	StateType state;
	unsigned int type;
	TLeafMask leaves;
	bool stateIsSink;

	unsigned int id;

	static unsigned int lastId;

	// < Public Methods >
	virtual void dump() {}
	virtual void closed_dump(unsigned int level) {}
	virtual bool DoCompare(TStateSet*) {return false;}
	virtual bool CanBePruned(TStateSet*, unsigned) {return false;}
	virtual bool isEmpty() {}
	virtual std::string ToString() {}
	virtual unsigned int measureStateSpace() {}
	friend inline std::ostream& operator<<(std::ostream& os, const TStateSet& tss) {os << "";}

	virtual bool isSubsumed(TStateSet*, unsigned int level) { return false;};
};

/**
 * Representation of LeafStateSet, i.e set of plain StateType states
 *
 * Example of LeafStateSet: {1, 2, 3}
 */
class LeafStateSet : public TStateSet {
private:
	// < Private Members >
	bool stateIsSink;

public:
	// < Public Methods >
	LeafStateSet (StateType q) : stateIsSink(false) {type = STATE; id = 0; state = q;}
	LeafStateSet () : stateIsSink(true) {type = STATE; id = 0; state = -1;}

	void dump();
	void closed_dump(unsigned int level);
	std::string ToString();
	bool isSink() {return this->stateIsSink; }
	bool isEmpty() { return this->stateIsSink; }
	unsigned int measureStateSpace() {return 0;}

	~LeafStateSet() {leaves.reset();}

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
			//LeafStateSet* lhss = (LeafStateSet*)(lhs);
			return this->state == lhs->state;
		}
	}

	/**
	 * Implementation of simulation pruning bounded up to @p pruneUpTo
	 *
	 * @param lhs: state we are comparing with
	 * @param pruneUpTo: how much we prune
	 * @return true if can be pruned
	 */
	bool CanBePruned(TStateSet* lhs, unsigned pruneUpTo) {
		if(lhs->type == MACROSTATE) {
			return false;
		} else {
			LeafStateSet* lhss = (LeafStateSet*)(lhs);
			return this->state == lhss->state;
		}
	}

	/**
	 * Implementation of state subsumption
	 */
	bool isSubsumed(TStateSet* lhs, unsigned int level) {
		if(lhs->type == MACROSTATE) {
			return false;
		} else {
			//LeafStateSet* lhss = (LeafStateSet*)(lhs);
			return this->state == lhs->state;
		}
	}

	/**
	  * Overloading of the << operator for Macrostate Class
	  *
	  * @param os: output stream
	  * @param mss: macro state to be outputed
	  * @return: output stream
	  */
	friend inline std::ostream& operator<<(std::ostream& os, const LeafStateSet& mss) {
		std::ostringstream ss;
		ss << mss.state;
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
	MacroStateSet (StateSetList Q) : macroStates(Q) { type = MACROSTATE; id = 0;}
	MacroStateSet (StateSetList Q, TLeafMask l): macroStates(Q) { type = MACROSTATE; leaves = l;  id = 0;}
	MacroStateSet (StateSetList Q, StateType q): macroStates(Q) {
		type = MACROSTATE;
	    id = 0;
		//leaves.set(q, true);
	}

	~MacroStateSet () {
		for(StateSetIterator it = this->macroStates.begin(); it != this->macroStates.end(); ++it) {
			delete (*it);
		}
		macroStates.clear();
		leaves.reset();
	}

	void dump();
	void closed_dump(unsigned int level);
	std::string ToString();
	StateSetList getMacroStates();
	StateSetList getMacroStates() const;
	void addState(TStateSet* state);
	void unionMacroSets(MacroStateSet* state);
	bool isEmpty() { return this->macroStates.size() == 0; }
	unsigned int measureStateSpace();

	MacroStateSet(const MacroStateSet &mSet) {
		this->leaves = mSet.leaves;
		StateSetList states = mSet.getMacroStates();
		for (auto state : states) {
			this->macroStates.push_back(state);
		}
	}

	MacroStateSet& operator=(const MacroStateSet &mSet) {
		this->leaves = mSet.leaves;
		StateSetList states = mSet.getMacroStates();
		for (auto state : states) {
			this->macroStates.push_back(state);
		}

		return *this;
	}

	/**
	 * Overloaded comparison operator
	 *
	 * @param lhs: left operator
	 * @return: true if they are the same
	 */
	bool DoCompare(TStateSet* lhs) {
		if (lhs->type == STATE) {
			return false;
		} else if(this->id != 0 & lhs->id != 0) {
			return this->id == lhs->id;
		} else {
			// test if they are same
			// Optimization: sets of level 1 are compared by bitwise operations
#ifdef USE_BINARY
			if(this->leaves.any() && lhs->leaves.any()) {
				unsigned size = TStateSet::stateNo;
				for(unsigned i = 0; i < size; ++i) {
					if(this->leaves[i] ^ lhs->leaves[i]) {
						return false;
					}
				}
				return true;
			}
#endif

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
	 * Implementation of simulation pruning bounded up to @p pruneUpTo
	 *
	 * @param lhs: state we are comparing with
	 * @param pruneUpTo: how much we prune
	 * @return true if can be pruned
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
						return lhs->leaves.is_subset_of(this->leaves);
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

				return true;
			}
		}
	}

	/**
	 * Implementation of subsumption
	 * TODO: Add logic with IDs
	 *
	 * @param lhs: other operand
	 * @param level: level of sumbsumption
	 * @return: true if subsumption holds
	 */
    bool isSubsumed(TStateSet* lhs, unsigned int level) {
    	if(lhs->type == STATE) {
    		return false;
    	} else {
    		if(level == 1) {
				if(this->leaves.any() && lhs->leaves.any()) {
					unsigned size = TStateSet::stateNo;
					for(unsigned i = 0; i < size; ++i) {
						if(this->leaves[i] ^ lhs->leaves[i]) {
							return false;
						}
					}
					return true;
				}
    		}

    		MacroStateSet *lhss = (MacroStateSet*) (lhs);
    		StateSetList lhsStates = lhss->getMacroStates();

    		// downward closed, which contains upward closed, we prune smaller
    		if(level % 2 == 0) {
    			// forall x in X: exists y in Y: x is subsumed by y
    			for (auto state : this->macroStates) {
    				auto matching_iter = std::find_if(lhsStates.begin(), lhsStates.end(),
    						[state, level](TStateSet *s) {
    							return state->isSubsumed(s, level-1);
    						});
    				if (matching_iter == lhsStates.end()) {
    					return false;
    				}
    			}

    			return true;
    		// upward closed, which contains downward closed things, we prune bigger
    		} else {
    			// forall y in Y: exists x in X: x is subsumed by y
    			for (auto state : lhsStates) {
    				auto matching_iter = std::find_if(this->macroStates.begin(), this->macroStates.end(),
    						[state, level](TStateSet *s) {
    							return s->isSubsumed(state, level-1);
    						});
    				if (matching_iter == this->macroStates.end()) {
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
