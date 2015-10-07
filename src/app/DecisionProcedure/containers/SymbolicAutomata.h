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
#include <vata/bdd_bu_tree_aut.hh>
#include <vata/parsing/timbuk_parser.hh>
#include <vata/serialization/timbuk_serializer.hh>
#include <vata/util/binary_relation.hh>

enum AutType {INTERSECTION, UNION, PROJECTION, BASE, COMPLEMENT};
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
    using LeafAutomaton_Type = VATA::BDDBottomUpTreeAut;
    using StateToStateTranslator = VATA::AutBase::StateToStateTranslWeak;
    using StateToStateMap        = std::unordered_map<StateType, StateType>;

    static StateType stateCnt;
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
    IntersectionAutomaton(SymbolicAutomaton* lhs, SymbolicAutomaton* rhs) : lhs_aut(lhs), rhs_aut(rhs) {}
    IntersectionAutomaton(std::shared_ptr<SymbolicAutomaton> lhs, std::shared_ptr<SymbolicAutomaton> rhs) : lhs_aut(lhs), rhs_aut(rhs) {}
    virtual void dump();
};

class BaseAutomaton : public SymbolicAutomaton {
protected:
    std::shared_ptr<LeafAutomaton_Type> _base_automaton;

public:
    BaseAutomaton(LeafAutomaton_Type* aut) : _base_automaton(aut) {}
    virtual ISect_Type IntersectNonEmpty(Symbol&, StateSet&);
    virtual StateSet Pre(Symbol&, StateSet&);
};

class SubAutomaton : public BaseAutomaton {
protected:
    virtual void _InitializeInitialStates();
    virtual void _InitializeFinalStates();

public:
    SubAutomaton(LeafAutomaton_Type* aut) : BaseAutomaton(aut) {}
    virtual void dump();
};

class TrueAutomaton : public BaseAutomaton {
protected:
    virtual void _InitializeInitialStates();
    virtual void _InitializeFinalStates();

public:
    TrueAutomaton(LeafAutomaton_Type* aut) : BaseAutomaton(aut) {}
    virtual void dump() { std::cout << "True"; }
};

class FalseAutomaton : public BaseAutomaton {
protected:
    virtual void _InitializeInitialStates();
    virtual void _InitializeFinalStates();

public:
    FalseAutomaton(LeafAutomaton_Type* aut) : BaseAutomaton(aut) {}
    virtual void dump() { std::cout << "False"; }
};


class InAutomaton : public BaseAutomaton {
protected:
    virtual void _InitializeInitialStates();
    virtual void _InitializeFinalStates();

public:
    InAutomaton(LeafAutomaton_Type* aut) : BaseAutomaton(aut) {}
    virtual void dump() { std::cout << "True"; }
};

class FirstOrderAutomaton : public BaseAutomaton {
protected:
    virtual void _InitializeInitialStates();
    virtual void _InitializeFinalStates();

public:
    FirstOrderAutomaton(LeafAutomaton_Type* aut) : BaseAutomaton(aut) {}
    virtual void dump() { std::cout << "FirstOrder"; }
};

class EqualFirstAutomaton : public BaseAutomaton {
protected:
    virtual void _InitializeInitialStates();
    virtual void _InitializeFinalStates();

public:
    EqualFirstAutomaton(LeafAutomaton_Type* aut) : BaseAutomaton(aut) {}
    virtual void dump() { std::cout << "Equal1"; }
};

class EqualSecondAutomaton : public BaseAutomaton {
protected:
    virtual void _InitializeInitialStates();
    virtual void _InitializeFinalStates();

public:
    EqualSecondAutomaton(LeafAutomaton_Type* aut) : BaseAutomaton(aut) {}
    virtual void dump() { std::cout << "Equal2"; }
};

class LessAutomaton : public BaseAutomaton {
protected:
    virtual void _InitializeInitialStates();
    virtual void _InitializeFinalStates();

public:
    LessAutomaton(LeafAutomaton_Type* aut) : BaseAutomaton(aut) {}
    virtual void dump() { std::cout << "Less"; }
};

class LessEqAutomaton : public BaseAutomaton {
protected:
    virtual void _InitializeInitialStates();
    virtual void _InitializeFinalStates();

public:
    LessEqAutomaton(LeafAutomaton_Type* aut) : BaseAutomaton(aut) {}
    virtual void dump() { std::cout << "LessEq"; }
};

#endif //WSKS_SYMBOLICAUTOMATA_H
