//
// Created by Raph on 07/10/2015.
//

#include "../Frontend/ast.h"
#include "../DecisionProcedure/containers/SymbolicAutomata.h"
#include <memory>

// TODO: FUCKING PLUS IS MISSING!!!!

template<class TemplatedAutomaton>
SymbolicAutomaton* baseToSymbolicAutomaton(ASTForm* form, bool doComplement) {
    Automaton aut;
    form->toUnaryAutomaton(aut, doComplement);
    TemplatedAutomaton* tmp = new TemplatedAutomaton(new Automaton(aut), form);
    return tmp;
}

SymbolicAutomaton* ASTForm_True::toSymbolicAutomaton(bool doComplement) {
    return baseToSymbolicAutomaton<TrueAutomaton>(this, doComplement);
}

SymbolicAutomaton* ASTForm_False::toSymbolicAutomaton(bool doComplement) {
    return baseToSymbolicAutomaton<FalseAutomaton>(this, doComplement);
}

SymbolicAutomaton* ASTForm_In::toSymbolicAutomaton(bool doComplement) {
    return baseToSymbolicAutomaton<InAutomaton>(this, doComplement);
}

SymbolicAutomaton* ASTForm_FirstOrder::toSymbolicAutomaton(bool doComplement) {
    return baseToSymbolicAutomaton<FirstOrderAutomaton>(this, doComplement);
}

SymbolicAutomaton* ASTForm_Equal1::toSymbolicAutomaton(bool doComplement) {
    return baseToSymbolicAutomaton<EqualFirstAutomaton>(this, doComplement);
}

SymbolicAutomaton* ASTForm_Equal2::toSymbolicAutomaton(bool doComplement) {
    return baseToSymbolicAutomaton<EqualSecondAutomaton>(this, doComplement);
}

SymbolicAutomaton* ASTForm_Less::toSymbolicAutomaton(bool doComplement) {
    return baseToSymbolicAutomaton<LessAutomaton>(this, doComplement);
}

SymbolicAutomaton* ASTForm_LessEq::toSymbolicAutomaton(bool doComplement) {
    return baseToSymbolicAutomaton<LessEqAutomaton>(this, doComplement);
}

SymbolicAutomaton* ASTForm_Sub::toSymbolicAutomaton(bool doComplement) {
    return baseToSymbolicAutomaton<SubAutomaton>(this, doComplement);
}

/**
 * Returns IntersectionAutomaton consisting of converted left and right automaton
 *
 * @param doComplement: true if we are making complementon
 */
SymbolicAutomaton* ASTForm_And::toSymbolicAutomaton(bool doComplement) {
    // TODO: doComplement should be better handled
    SymbolicAutomaton* lhs_aut;
    lhs_aut = this->f1->toSymbolicAutomaton(doComplement);
    SymbolicAutomaton* rhs_aut;
    rhs_aut = this->f2->toSymbolicAutomaton(doComplement);
    return new IntersectionAutomaton(lhs_aut, rhs_aut, this);
}

SymbolicAutomaton* ASTForm_Or::toSymbolicAutomaton(bool doComplement) {
    // TODO: doComplement should be better handled
    SymbolicAutomaton* lhs_aut;
    lhs_aut = this->f1->toSymbolicAutomaton(doComplement);
    SymbolicAutomaton* rhs_aut;
    rhs_aut = this->f2->toSymbolicAutomaton(doComplement);
    return new UnionAutomaton(lhs_aut, rhs_aut, this);
}

SymbolicAutomaton* ASTForm_Not::toSymbolicAutomaton(bool doComplement) {
    SymbolicAutomaton* aut;
    aut = this->f->toSymbolicAutomaton(!doComplement);
    return new ComplementAutomaton(aut, this);
}

SymbolicAutomaton* ASTForm_Ex2::toSymbolicAutomaton(bool doComplement) {
    SymbolicAutomaton* aut;
    aut = this->f->toSymbolicAutomaton(doComplement);
    return new ProjectionAutomaton(aut, this);
}