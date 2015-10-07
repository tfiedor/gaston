//
// Created by Raph on 07/10/2015.
//

#include "../Frontend/ast.h"
#include "../DecisionProcedure/containers/SymbolicAutomata.h"
#include <memory>

template<class TemplatedAutomaton>
std::shared_ptr<SymbolicAutomaton> baseToSymbolicAutomaton(ASTForm* form, bool doComplement) {
    Automaton aut;
    form->toUnaryAutomaton(aut, doComplement);
    return std::make_shared<TemplatedAutomaton>(new Automaton(aut));
}

std::shared_ptr<SymbolicAutomaton> ASTForm_True::toSymbolicAutomaton(bool doComplement) {
    return nullptr;
}

std::shared_ptr<SymbolicAutomaton> ASTForm_False::toSymbolicAutomaton(bool doComplement) {
    return nullptr;
}

std::shared_ptr<SymbolicAutomaton> ASTForm_In::toSymbolicAutomaton(bool doComplement) {
    return nullptr;
}

std::shared_ptr<SymbolicAutomaton> ASTForm_FirstOrder::toSymbolicAutomaton(bool doComplement) {
    return nullptr;
}

std::shared_ptr<SymbolicAutomaton> ASTForm_Equal1::toSymbolicAutomaton(bool doComplement) {
    return nullptr;
}

std::shared_ptr<SymbolicAutomaton> ASTForm_Equal2::toSymbolicAutomaton(bool doComplement) {
    return nullptr;
}

std::shared_ptr<SymbolicAutomaton> ASTForm_Less::toSymbolicAutomaton(bool doComplement) {
    return nullptr;
}

std::shared_ptr<SymbolicAutomaton> ASTForm_LessEq::toSymbolicAutomaton(bool doComplement) {
    return nullptr;
}

std::shared_ptr<SymbolicAutomaton> ASTForm_Sub::toSymbolicAutomaton(bool doComplement) {
    return baseToSymbolicAutomaton<SubAutomaton>(this, doComplement);
}

/**
 * Returns IntersectionAutomaton consisting of converted left and right automaton
 *
 * @param doComplement: true if we are making complementon
 */
std::shared_ptr<SymbolicAutomaton> ASTForm_And::toSymbolicAutomaton(bool doComplement) {
    // TODO: doComplement should be better handled
    std::shared_ptr<SymbolicAutomaton> lhs_aut;
    lhs_aut = this->f1->toSymbolicAutomaton(doComplement);
    std::shared_ptr<SymbolicAutomaton> rhs_aut;
    rhs_aut = this->f2->toSymbolicAutomaton(doComplement);
    return std::make_shared<IntersectionAutomaton>(lhs_aut, rhs_aut);
}

std::shared_ptr<SymbolicAutomaton> ASTForm_Or::toSymbolicAutomaton(bool doComplement) {
    return nullptr;
}

std::shared_ptr<SymbolicAutomaton> ASTForm_Not::toSymbolicAutomaton(bool doComplement) {
    return nullptr;
}