#ifndef __STATE_SET__
#define __STATE_SET__

#include <deque>
#include <iostream>

class TStateSet;

// < Typedefs >
typedef size_t StateType;
typedef std::deque<TStateSet*> StateSetList;

class TStateSet {
private:
public:
	// < Public Methods >
	virtual void dump() {};
};

class LeafStateSet : public TStateSet {
private:
	// < Private Members >
	StateType state;

public:
	// < Public Methods >
	LeafStateSet (StateType q) : state(q) {}

	void dump();

	StateType getState();
};

class MacroStateSet : public TStateSet {
private:
	// < Private Members >
	StateSetList macroStates;

public:
	// < Public Methods >
	MacroStateSet (StateSetList Q) : macroStates(Q) {}
	void dump();
	StateSetList getMacroStates();
	StateSetList getMacroStates() const;
};

#endif
