//
// Created by Raph on 07/10/2015.
//

#include "../Frontend/ast.h"
#include "../Frontend/timer.h"
#include "../DecisionProcedure/containers/SymbolicAutomata.h"
#include "../DecisionProcedure/environment.hh"
#include "../DecisionProcedure/automata.hh"
#include "../DecisionProcedure/visitors/NegationUnfolder.h"
#include <memory>

extern Timer timer_base;

SymbolicAutomaton* ASTForm::toSymbolicAutomaton(bool doComplement) {
    // If the sfa was already initialized (it is thus shared somehow,
    // we return the pointer, otherwise we first create the symbolic
    // automaton and return the thing.
    if(this->sfa == nullptr) {
        this->sfa = this->_toSymbolicAutomatonCore(doComplement);
    }
    return this->sfa;
}

template<class TemplatedAutomaton>
SymbolicAutomaton* baseToSymbolicAutomaton(ASTForm* form, bool doComplement) {
    Automaton aut;
    #if (AUT_CONSTRUCT_BY_MONA == true)
    constructAutomatonByMona(form, aut);
    #else
    NegationUnfolder nu_visitor;
    ASTForm* nonNegatedAutomaton = reinterpret_cast<ASTForm*>(form->accept(nu_visitor));
    nonNegatedAutomaton->toUnaryAutomaton(aut, doComplement);
    #endif

    #if (OPT_REDUCE_AUT_LAST == true)
    auto aut1 = aut.RemoveUnreachableStates();
    aut = aut1.RemoveUselessStates();
    #endif

    // Fixme: this is shit, come on!
    return new TemplatedAutomaton(new Automaton(std::move(aut)), form);
}

SymbolicAutomaton* ASTForm_True::_toSymbolicAutomatonCore(bool doComplement) {
    return baseToSymbolicAutomaton<TrueAutomaton>(this, doComplement);
}

SymbolicAutomaton* ASTForm_False::_toSymbolicAutomatonCore(bool doComplement) {
    return baseToSymbolicAutomaton<FalseAutomaton>(this, doComplement);
}

SymbolicAutomaton* ASTForm_In::_toSymbolicAutomatonCore(bool doComplement) {
    return baseToSymbolicAutomaton<InAutomaton>(this, doComplement);
}

SymbolicAutomaton* ASTForm_FirstOrder::_toSymbolicAutomatonCore(bool doComplement) {
    return baseToSymbolicAutomaton<FirstOrderAutomaton>(this, doComplement);
}

SymbolicAutomaton* ASTForm_Equal1::_toSymbolicAutomatonCore(bool doComplement) {
    return baseToSymbolicAutomaton<EqualFirstAutomaton>(this, doComplement);
}

SymbolicAutomaton* ASTForm_Equal2::_toSymbolicAutomatonCore(bool doComplement) {
    return baseToSymbolicAutomaton<EqualSecondAutomaton>(this, doComplement);
}

SymbolicAutomaton* ASTForm_Less::_toSymbolicAutomatonCore(bool doComplement) {
    return baseToSymbolicAutomaton<LessAutomaton>(this, doComplement);
}

SymbolicAutomaton* ASTForm_LessEq::_toSymbolicAutomatonCore(bool doComplement) {
    return baseToSymbolicAutomaton<LessEqAutomaton>(this, doComplement);
}

SymbolicAutomaton* ASTForm_Sub::_toSymbolicAutomatonCore(bool doComplement) {
    return baseToSymbolicAutomaton<SubAutomaton>(this, doComplement);
}

/**
 * Returns IntersectionAutomaton consisting of converted left and right automaton
 *
 * @param doComplement: true if we are making complementon
 */
SymbolicAutomaton* ASTForm_And::_toSymbolicAutomatonCore(bool doComplement) {
    SymbolicAutomaton* lhs_aut;
#if (OPT_CREATE_QF_AUTOMATON == true)
    // TODO: WE ARE MISSING COMPLEMENTATION
    IdentList free, bound;
    this->f1->freeVars(&free, &bound);
    if(bound.empty()) {
        lhs_aut = baseToSymbolicAutomaton<GenericBaseAutomaton>(this->f1, false);
    } else {
        lhs_aut = this->f1->toSymbolicAutomaton(doComplement);
    }
#else
    lhs_aut = this->f1->toSymbolicAutomaton(doComplement);
#endif
    SymbolicAutomaton* rhs_aut;
#if (OPT_CREATE_QF_AUTOMATON == true)
    // TODO: WE ARE MISSING COMPLEMENTATION
    free.reset();
    bound.reset();
    this->f2->freeVars(&free, &bound);
    if(bound.empty()) {
        rhs_aut = baseToSymbolicAutomaton<GenericBaseAutomaton>(this->f2, false);
    } else {
        rhs_aut = this->f2->toSymbolicAutomaton(doComplement);
    }
#else
    rhs_aut = this->f2->toSymbolicAutomaton(doComplement);
#endif
    return new IntersectionAutomaton(lhs_aut, rhs_aut, this);
}

SymbolicAutomaton* ASTForm_Or::_toSymbolicAutomatonCore(bool doComplement) {
    SymbolicAutomaton* lhs_aut;
#if (OPT_CREATE_QF_AUTOMATON == true)
    // TODO: WE ARE MISSING COMPLEMENTATION
    IdentList free, bound;
    this->f1->freeVars(&free, &bound);
    if(bound.empty()) {
        lhs_aut = baseToSymbolicAutomaton<GenericBaseAutomaton>(this->f1, false);
    } else {
        lhs_aut = this->f1->toSymbolicAutomaton(doComplement);
    }
#else
    lhs_aut = this->f1->toSymbolicAutomaton(doComplement);
#endif
    SymbolicAutomaton* rhs_aut;
#if (OPT_CREATE_QF_AUTOMATON == true)
    // TODO: WE ARE MISSING COMPLEMENTATION
    free.reset();
    bound.reset();
    this->f2->freeVars(&free, &bound);
    if(bound.empty()) {
        rhs_aut = baseToSymbolicAutomaton<GenericBaseAutomaton>(this->f2, false);
    } else {
        rhs_aut = this->f2->toSymbolicAutomaton(doComplement);
    }
#else
    rhs_aut = this->f2->toSymbolicAutomaton(doComplement);
#endif
    return new UnionAutomaton(lhs_aut, rhs_aut, this);
}

bool is_base_automaton(ASTForm* f) {
    return f->kind != aOr &&
           f->kind != aAnd &&
           f->kind != aEx2;
}

SymbolicAutomaton* ASTForm_Not::_toSymbolicAutomatonCore(bool doComplement) {
    #if (OPT_CREATE_QF_AUTOMATON == true)
        // TODO: WE ARE MISSING COMPLEMENTATION
        IdentList free, bound;
        this->f->freeVars(&free, &bound);
        if(bound.empty()) {
            SymbolicAutomaton* baseAut = baseToSymbolicAutomaton<GenericBaseAutomaton>(this->f, false);
            return new ComplementAutomaton(baseAut, this);
        }
    #endif

    #if (OPT_DRAW_NEGATION_IN_BASE == true)
    if(is_base_automaton(this->f)) {
        return baseToSymbolicAutomaton<GenericBaseAutomaton>(this, !doComplement);
    }
    #endif
    SymbolicAutomaton* aut;
    aut = this->f->toSymbolicAutomaton(doComplement);
    return new ComplementAutomaton(aut, this);
}

SymbolicAutomaton* ASTForm_Ex2::_toSymbolicAutomatonCore(bool doComplement) {
    #if (OPT_CREATE_QF_AUTOMATON == true)
        // TODO: WE ARE MISSING COMPLEMENTATION
        IdentList free, bound;
        this->f->freeVars(&free, &bound);
        if(bound.empty()) {
            SymbolicAutomaton* baseAut = baseToSymbolicAutomaton<GenericBaseAutomaton>(this->f, false);
            return new ProjectionAutomaton(baseAut, this);
        }
    #endif

    SymbolicAutomaton* aut;
    aut = this->f->toSymbolicAutomaton(doComplement);
    return new ProjectionAutomaton(aut, this);
}