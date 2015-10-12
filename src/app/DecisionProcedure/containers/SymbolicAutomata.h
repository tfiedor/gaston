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

#include "../mtbdd/apply1func.hh"
#include "../mtbdd/apply2func.hh"
#include "../mtbdd/void_apply1func.hh"
#include "../mtbdd/ondriks_mtbdd.hh"
#include "../utils/Symbol.h"
#include "../../Frontend/ident.h"
#include "../../Frontend/ast.h"
#include "StateSet.hh"
#include <vector>
#include <vata/bdd_bu_tree_aut.hh>
#include <vata/parsing/timbuk_parser.hh>
#include <vata/serialization/timbuk_serializer.hh>
#include <vata/util/binary_relation.hh>

class Term;

enum AutType {INTERSECTION, UNION, PROJECTION, BASE, COMPLEMENT};
enum AutSubType {FINAL, NONFINAL};
enum AutBaseType {BT_SUB, BT_IN, BT_EQF, BT_EQS, BT_LSS, BT_LEQ, BT_FO, BT_T, BT_F};

using BaseAut_States = VATA::Util::OrdVector<StateType>;


/**
 * Base class for symbolic automata
 */
class SymbolicAutomaton {
public:
    // < Used Typedefs >
    // TODO: Change to something more efficient
    using Term_ptr               = std::shared_ptr<Term>;
    using Formula_ptr            = ASTForm*;
    using StateSet_ptr           = std::shared_ptr<MacroStateSet>;
    using StateSet               = Term_ptr;
    using Symbol                 = ZeroSymbol;;
    using ISect_Type             = std::pair<Term_ptr, bool>;
    using LeafAutomaton_Type     = VATA::BDDBottomUpTreeAut;
    using StateToStateTranslator = VATA::AutBase::StateToStateTranslWeak;
    using StateToStateMap        = std::unordered_map<StateType, StateType>;
    using WorkListTerm           = Term;
    using WorkListTerm_raw       = Term*;
    using WorkListTerm_ptr       = Term_ptr;
    using WorkListSet            = std::vector<std::shared_ptr<WorkListTerm>>;

    static StateType stateCnt;

protected:
    // < Private Members >
    Formula_ptr _form;
    Term_ptr _initialStates;
    Term_ptr _finalStates;

    virtual void _InitializeAutomaton() = 0;
    virtual void _InitializeInitialStates() = 0;
    virtual void _InitializeFinalStates() = 0;
    virtual ISect_Type _IntersectNonEmptyCore(Symbol*, StateSet) = 0;

// < Public API >
public:
    // < Public Constructors >
    SymbolicAutomaton(Formula_ptr form) : _form(form) {}

    virtual StateSet GetInitialStates();
    virtual StateSet GetFinalStates();
    virtual StateSet Pre(Symbol*, StateSet) = 0;
    ISect_Type IntersectNonEmpty(Symbol*, StateSet);

    virtual void dump() = 0;
};

/**
 *
 */
class BinaryOpAutomaton : public SymbolicAutomaton {
protected:
    std::shared_ptr<SymbolicAutomaton> lhs_aut;
    std::shared_ptr<SymbolicAutomaton> rhs_aut;
    bool (*_eval_result)(bool, bool);
    bool (*_eval_early)(bool);
    virtual void _InitializeAutomaton() { this->_InitializeInitialStates(); this->_InitializeFinalStates(); }
    virtual void _InitializeInitialStates();
    virtual void _InitializeFinalStates();
    virtual ISect_Type _IntersectNonEmptyCore(Symbol*, StateSet);

public:
    virtual StateSet Pre(Symbol*, StateSet);
    BinaryOpAutomaton(SymbolicAutomaton* lhs,
                      SymbolicAutomaton* rhs,
                      Formula_ptr form)
            : SymbolicAutomaton(form), lhs_aut(lhs), rhs_aut(rhs) { this->_InitializeAutomaton(); }
    virtual void dump();
};

/**
 *
 */
class IntersectionAutomaton : public BinaryOpAutomaton {
public:
    IntersectionAutomaton(SymbolicAutomaton* lhs,
                          SymbolicAutomaton* rhs,
                          Formula_ptr form)
            : BinaryOpAutomaton(lhs, rhs, form) {
        this->_InitializeAutomaton();
        this->_eval_result = [](bool a, bool b) {return a && b; };
        this->_eval_early = [](bool a) {return !a;};
    }
};

/**
 *
 */
class UnionAutomaton : public BinaryOpAutomaton {
public:
    UnionAutomaton(SymbolicAutomaton* lhs,
                   SymbolicAutomaton* rhs,
                   Formula_ptr form)
            : BinaryOpAutomaton(lhs, rhs, form) {
        this->_InitializeAutomaton();
        this->_eval_result = [](bool a, bool b) {return a || b; };
        this->_eval_early = [](bool a) { return a;};
    }
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
    virtual ISect_Type _IntersectNonEmptyCore(Symbol*, StateSet);

public:
    ComplementAutomaton(SymbolicAutomaton *aut, Formula_ptr form) : SymbolicAutomaton(form), _aut(aut) { this->_InitializeAutomaton(); }

    virtual StateSet Pre(Symbol*, StateSet);
    virtual void dump();
};

/**
 *
 */
class ProjectionAutomaton : public SymbolicAutomaton {
protected:
    std::shared_ptr<SymbolicAutomaton> _aut;
    IdentList* _projected_vars;


    virtual void _InitializeAutomaton() {
        this->_InitializeInitialStates();
        this->_InitializeFinalStates();
        this->_projected_vars = static_cast<ASTForm_uvf*>(this->_form)->vl;
    }
    virtual void _InitializeInitialStates();
    virtual void _InitializeFinalStates();
    virtual ISect_Type _IntersectNonEmptyCore(Symbol*, StateSet);

public:
    ProjectionAutomaton(SymbolicAutomaton* aut, Formula_ptr form) : SymbolicAutomaton(form), _aut(aut) { this->_InitializeAutomaton(); }

    virtual StateSet Pre(Symbol*, StateSet);
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
    virtual ISect_Type _IntersectNonEmptyCore(Symbol*, StateSet);

    std::shared_ptr<LeafAutomaton_Type> _base_automaton;
    void _RenameStates();

public:
    BaseAutomaton(LeafAutomaton_Type* aut, Formula_ptr form) : SymbolicAutomaton(form), _base_automaton(aut) { this->_InitializeAutomaton(); }
    virtual StateSet Pre(Symbol*, StateSet);
    virtual void baseAutDump();
};

class SubAutomaton : public BaseAutomaton {
public:
    SubAutomaton(LeafAutomaton_Type* aut, Formula_ptr form) : BaseAutomaton(aut, form) { }
    virtual void dump() { std::cout << "Sub\n"; this->baseAutDump();}
};

class TrueAutomaton : public BaseAutomaton {
public:
    TrueAutomaton(LeafAutomaton_Type* aut, Formula_ptr form) : BaseAutomaton(aut, form) {}
    virtual void dump() { std::cout << "True\n"; this->baseAutDump(); }
};

class FalseAutomaton : public BaseAutomaton {
public:
    FalseAutomaton(LeafAutomaton_Type* aut, Formula_ptr form) : BaseAutomaton(aut, form) {}
    virtual void dump() { std::cout << "False\n"; this->baseAutDump(); }
};

class InAutomaton : public BaseAutomaton {
public:
    InAutomaton(LeafAutomaton_Type* aut, Formula_ptr form) : BaseAutomaton(aut, form) {}
    virtual void dump() { std::cout << "True\n"; this->baseAutDump(); }
};

class FirstOrderAutomaton : public BaseAutomaton {
public:
    FirstOrderAutomaton(LeafAutomaton_Type* aut, Formula_ptr form) : BaseAutomaton(aut, form) {}
    virtual void dump() { std::cout << "FirstOrder\n"; this->baseAutDump(); }
};

class EqualFirstAutomaton : public BaseAutomaton {
public:
    EqualFirstAutomaton(LeafAutomaton_Type* aut, Formula_ptr form) : BaseAutomaton(aut, form) {}
    virtual void dump() { std::cout << "Equal1\n"; this->baseAutDump(); }
};

class EqualSecondAutomaton : public BaseAutomaton {
public:
    EqualSecondAutomaton(LeafAutomaton_Type* aut, Formula_ptr form) : BaseAutomaton(aut, form) {}
    virtual void dump() { std::cout << "Equal2\n"; this->baseAutDump(); }
};

class LessAutomaton : public BaseAutomaton {
public:
    LessAutomaton(LeafAutomaton_Type* aut, Formula_ptr form) : BaseAutomaton(aut, form) {}
    virtual void dump() { std::cout << "Less\n"; this->baseAutDump(); }
};

class LessEqAutomaton : public BaseAutomaton {
public:
    LessEqAutomaton(LeafAutomaton_Type* aut, Formula_ptr form) : BaseAutomaton(aut, form) {}
    virtual void dump() { std::cout << "LessEq\n"; this->baseAutDump(); }
};

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< FUNCTORS >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

GCC_DIAG_OFF(effc++)
class BaseCollectorFunctor : public VATA::MTBDDPkg::VoidApply1Functor<BaseCollectorFunctor, BaseAut_States> {
    GCC_DIAG_ON(effc++)
private:
    BaseAut_States& collected;

public:
    // < Public Constructors >
    BaseCollectorFunctor(BaseAut_States& l) : collected(l) {}

    // < Public Methods >
    /**
     * @param lhs: operand of apply
     */
    inline void ApplyOperation(BaseAut_States lhs) {
        collected.insert(lhs);
    }
};

GCC_DIAG_OFF(effc++)
class MaskerFunctor : public VATA::MTBDDPkg::Apply2Functor<MaskerFunctor, BaseAut_States, BaseAut_States, BaseAut_States> {
    GCC_DIAG_ON(effc++)
public:
    // < Public Methods >
    /**
     * @param lhs: left operand
     * @param rhs: right operand
     * @return union of leaf operands
     */
    inline BaseAut_States ApplyOperation(BaseAut_States lhs, BaseAut_States rhs) {
        if(rhs.size() == 0) {
            return rhs;
        } else {
            return lhs;
        }
    }
};


#endif //WSKS_SYMBOLICAUTOMATA_H
