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
#include "../mtbdd/ondriks_mtbdd.hh"
#include <vata/bdd_bu_tree_aut.hh>
#include <vata/parsing/timbuk_parser.hh>
#include <vata/serialization/timbuk_serializer.hh>
#include <vata/util/binary_relation.hh>

enum AutType {INTERSECTION, UNION, PROJECTION, BASE, COMPLEMENT};
enum AutSubType {FINAL, NONFINAL};
enum AutBaseType {BT_SUB, BT_IN, BT_EQF, BT_EQS, BT_LSS, BT_LEQ, BT_FO, BT_T, BT_F};

/**
 * Base class for symbolic automata
 */
class SymbolicAutomaton {
// TODO: ADD POINTER TO FORMULA
public:
    // < Used Typedefs >
    // TODO: Change to something more efficient
    using StateSet_ptr           = std::shared_ptr<MacroStateSet>;
    using StateSet               = MacroStateSet*;
    using Symbol                 = char;
    using FixPoint_MTBDD         = VATA::MTBDDPkg::OndriksMTBDD<std::pair<MacroStateSet*, bool> >;
    using ISect_Type             = FixPoint_MTBDD*;
    using LeafAutomaton_Type     = VATA::BDDBottomUpTreeAut;
    using StateToStateTranslator = VATA::AutBase::StateToStateTranslWeak;
    using StateToStateMap        = std::unordered_map<StateType, StateType>;

    static StateType stateCnt;

    // < Public Constructors >
    SymbolicAutomaton() {}
protected:
    // < Private Members >
    StateSet_ptr _initialStates;
    StateSet_ptr _finalStates;

    virtual void _InitializeAutomaton() = 0;
    virtual void _InitializeInitialStates() = 0;
    virtual void _InitializeFinalStates() = 0;

// < Public API >
public:
    virtual StateSet GetInitialStates();
    virtual StateSet GetFinalStates();
    virtual StateSet Pre(Symbol*, StateSet) = 0;
    virtual ISect_Type IntersectNonEmpty(Symbol*, StateSet) = 0;

    virtual void dump() = 0;
};

/**
 *
 */
class BinaryOpAutomaton : public SymbolicAutomaton {
protected:
    std::shared_ptr<SymbolicAutomaton> lhs_aut;
    std::shared_ptr<SymbolicAutomaton> rhs_aut;
    virtual void _InitializeAutomaton() { this->_InitializeInitialStates(); this->_InitializeFinalStates(); }
    virtual void _InitializeInitialStates();
    virtual void _InitializeFinalStates();

public:
    virtual StateSet Pre(Symbol*, StateSet);
    virtual ISect_Type IntersectNonEmpty(Symbol*, StateSet);
    BinaryOpAutomaton(SymbolicAutomaton* lhs, SymbolicAutomaton* rhs) : lhs_aut(lhs), rhs_aut(rhs) { this->_InitializeAutomaton(); }
    BinaryOpAutomaton() {}
    virtual void dump();
};

/**
 *
 */
class IntersectionAutomaton : public BinaryOpAutomaton {
public:
    IntersectionAutomaton() {}
    IntersectionAutomaton(SymbolicAutomaton* lhs, SymbolicAutomaton* rhs) : BinaryOpAutomaton(lhs, rhs) { this->_InitializeAutomaton(); }
};

/**
 *
 */
class UnionAutomaton : public BinaryOpAutomaton {
public:
    UnionAutomaton() {}
    UnionAutomaton(SymbolicAutomaton* lhs, SymbolicAutomaton* rhs) : BinaryOpAutomaton(lhs, rhs) { this->_InitializeAutomaton(); }
};

/**
 *
 */
class ComplementAutomaton : public SymbolicAutomaton {
protected:
    std::shared_ptr<SymbolicAutomaton> _aut;
    virtual void _InitializeAutomaton() { this->_InitializeInitialStates(); this->_InitializeFinalStates(); }
    virtual void _InitializeInitialStates();
    virtual void _InitializeFinalStates();

public:
    ComplementAutomaton() {}
    ComplementAutomaton(SymbolicAutomaton *aut) : _aut(aut) { this->_InitializeAutomaton(); }

    virtual StateSet Pre(Symbol*, StateSet);
    virtual ISect_Type IntersectNonEmpty(Symbol*, StateSet);
    virtual void dump();
};

/**
 *
 */
class ProjectionAutomaton : public SymbolicAutomaton {
protected:
    std::shared_ptr<SymbolicAutomaton> _aut;

    virtual void _InitializeAutomaton() { this->_InitializeInitialStates(); this->_InitializeFinalStates(); }
    virtual void _InitializeInitialStates();
    virtual void _InitializeFinalStates();

public:
    ProjectionAutomaton(SymbolicAutomaton* aut) : _aut(aut) { this->_InitializeAutomaton(); }

    virtual StateSet Pre(Symbol*, StateSet);
    virtual ISect_Type IntersectNonEmpty(Symbol*, StateSet);
    virtual void dump();
};

/**
 *
 */
class BaseAutomaton : public SymbolicAutomaton {
protected:
    virtual void _InitializeAutomaton();
    virtual void _InitializeInitialStates();
    virtual void _InitializeFinalStates();

    std::shared_ptr<LeafAutomaton_Type> _base_automaton;
    void _RenameStates();

public:
    BaseAutomaton(LeafAutomaton_Type* aut) : _base_automaton(aut) { this->_InitializeAutomaton(); }
    virtual ISect_Type IntersectNonEmpty(Symbol*, StateSet);
    virtual StateSet Pre(Symbol*, StateSet);
    virtual void baseAutDump();
};

class SubAutomaton : public BaseAutomaton {
public:
    SubAutomaton(LeafAutomaton_Type* aut) : BaseAutomaton(aut) {}
    virtual void dump() { std::cout << "Sub\n"; this->baseAutDump(); }
};

class TrueAutomaton : public BaseAutomaton {
public:
    TrueAutomaton(LeafAutomaton_Type* aut) : BaseAutomaton(aut) {}
    virtual void dump() { std::cout << "True\n"; this->baseAutDump(); }
};

class FalseAutomaton : public BaseAutomaton {
public:
    FalseAutomaton(LeafAutomaton_Type* aut) : BaseAutomaton(aut) {}
    virtual void dump() { std::cout << "False\n"; this->baseAutDump(); }
};


class InAutomaton : public BaseAutomaton {
public:
    InAutomaton(LeafAutomaton_Type* aut) : BaseAutomaton(aut) {}
    virtual void dump() { std::cout << "True\n"; this->baseAutDump(); }
};

class FirstOrderAutomaton : public BaseAutomaton {
public:
    FirstOrderAutomaton(LeafAutomaton_Type* aut) : BaseAutomaton(aut) {}
    virtual void dump() { std::cout << "FirstOrder\n"; this->baseAutDump(); }
};

class EqualFirstAutomaton : public BaseAutomaton {
public:
    EqualFirstAutomaton(LeafAutomaton_Type* aut) : BaseAutomaton(aut) {}
    virtual void dump() { std::cout << "Equal1\n"; this->baseAutDump(); }
};

class EqualSecondAutomaton : public BaseAutomaton {
public:
    EqualSecondAutomaton(LeafAutomaton_Type* aut) : BaseAutomaton(aut) {}
    virtual void dump() { std::cout << "Equal2\n"; this->baseAutDump(); }
};

class LessAutomaton : public BaseAutomaton {
public:
    LessAutomaton(LeafAutomaton_Type* aut) : BaseAutomaton(aut) {}
    virtual void dump() { std::cout << "Less\n"; this->baseAutDump(); }
};

class LessEqAutomaton : public BaseAutomaton {
public:
    LessEqAutomaton(LeafAutomaton_Type* aut) : BaseAutomaton(aut) {}
    virtual void dump() { std::cout << "LessEq\n"; this->baseAutDump(); }
};

#endif //WSKS_SYMBOLICAUTOMATA_H
