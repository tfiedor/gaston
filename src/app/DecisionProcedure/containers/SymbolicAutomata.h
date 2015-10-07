/*****************************************************************************
 *  gaston - no real logic behind the name, we simply liked the poor seal gaston. R.I.P. brave soldier.
 *
 *  Copyright (c) 2015  Tomas Fiedor <ifiedortom@fit.vutbr.cz>
 *      Notable mentions: Ondrej Lengal <ondra.lengal@gmail.com>
 *          			  Overeating Panda <if-his-simulation-reduction-works>
 *
 *****************************************************************************/

#ifndef WSKS_SYMBOLICAUTOMATA_H
#define WSKS_SYMBOLICAUTOMATA_H

#include <vector>

#include "StateSet.hh"

enum AutType {INTERSECTION, UNION, PROJECT, BASE, COMPLEMENT};
enum AutSubType {FINAL, NONFINAL};

/**
 * Base class for symbolic automata
 */
class SymbolicAutomaton {
public:
    // < Used Typedefs >
    // TODO: Change to something more efficient
    using StateSet = std::shared_ptr<TStateSet>;
    using Symbol = char;
    using ISect_Type = bool;
protected:
    // < Private Members >
    StateSet _initialStates;
    StateSet _finalStates;

    virtual void _InitializeInitialStates() = 0;
    virtual void _InitializeFinalStates() = 0;

// < Public API >
public:
    virtual StateSet GetInitialStates();
    virtual StateSet GetFinalStates();
    virtual StateSet Pre(Symbol&, StateSet&) = 0;
    virtual ISect_Type IntersectNonEmpty(Symbol&, StateSet&) = 0; // TODO: Should return MTBDD somehow
    //virtual bool IntersectNonEmpty(Symbol&, StateSet&, StateSet&) = 0; // TODO: do we need this?

    virtual void dump() = 0;
};

// TODO: There should be difference between Final and Nonfinal automaton, according to the structure and things
class IntersectionAutomaton : public SymbolicAutomaton {
private:
    std::shared_ptr<SymbolicAutomaton> lhs_aut;
    std::shared_ptr<SymbolicAutomaton> rhs_aut;

protected:
    virtual void _InitializeInitialStates();
    virtual void _InitializeFinalStates();

public:
    virtual StateSet Pre(Symbol&, StateSet&);
    virtual ISect_Type IntersectNonEmpty(Symbol&, StateSet&);
    //IntersectionAutomaton(SymbolicAutomaton& lhs, SymbolicAutomaton& rhs) : lhs_aut(lhs), rhs_aut(rhs) {}
    virtual void dump();
};

class BaseAutomaton : public SymbolicAutomaton {
public:
    virtual ISect_Type IntersectNonEmpty(Symbol&, StateSet&);
};

class SubAutomaton : public BaseAutomaton {
protected:
    virtual void _InitializeInitialStates();
    virtual void _InitializeFinalStates();

public:
    virtual StateSet Pre(Symbol&, StateSet&);
    virtual void dump();
};

#endif //WSKS_SYMBOLICAUTOMATA_H
