/*****************************************************************************
 *  dWiNA - Deciding WSkS using non-deterministic automata
 *
 *  Copyright (c) 2015  Tomas Fiedor <xfiedo01@stud.fit.vutbr.cz>
 *
 *  Description:
 *    Visitor for removing the universal quantifier
 *
 *****************************************************************************/
#include "UniversalQuantifierRemover.h"
#include "../../../Frontend/ast.h"

/**
 *  not AllX. phi -> ExX. not phi
 *
 *  @param[in]  form  form to be negated
 */
template<class Universal, class Existential>
AST* UniversalQuantifierRemover::_negateQuantifier(ASTForm_Not* form) {
    Universal* universal = static_cast<Universal*>(form->f);
    ASTForm_Not* negPhi = new ASTForm_Not(universal->f, universal->f->pos);
    return new Existential(universal->ul, universal->vl, negPhi, form->pos);
}

template<>
AST* UniversalQuantifierRemover::_negateQuantifier<ASTForm_All0, ASTForm_Ex0>(ASTForm_Not* form) {
    ASTForm_All0* universal = static_cast<ASTForm_All0*>(form->f);
    ASTForm_Not* negPhi = new ASTForm_Not(universal->f, universal->f->pos);
    return new ASTForm_Ex0(universal->vl, negPhi, form->pos);
}

/**
 * This is to not recieve the bloody double negation
 *
 * @param[in]  form  traversed node
 */
AST* UniversalQuantifierRemover::visit(ASTForm_Not *form) {
    switch(form->f->kind) {
        case aAll0:
            return this->_negateQuantifier<ASTForm_All0, ASTForm_Ex0>(form);
        case aAll1:
            return this->_negateQuantifier<ASTForm_All1, ASTForm_Ex1>(form);
        case aAll2:
            return this->_negateQuantifier<ASTForm_All2, ASTForm_Ex2>(form);
        default:
            return form;
    }
}

/**
 * Removes the universal quantifier as follows:
 *  All0 x. phi -> not Ex0 x. not phi
 *
 * @param[in] form      traversed All node
 */
AST* UniversalQuantifierRemover::visit(ASTForm_All0* form) {
    ASTForm_Not* negPhi = new ASTForm_Not(form->f, form->f->pos);
    ASTForm_Ex0* exNegPhi = new ASTForm_Ex0(form->vl, negPhi, form->pos);
    ASTForm_Not* negExNegPhi = new ASTForm_Not(exNegPhi, form->pos);
    return negExNegPhi;
}

/**
 * Removes the universal quantifier as follows:
 *  All1 x. phi -> not Ex2 x. not phi
 *
 * @param[in] form      traversed All node
 */
AST* UniversalQuantifierRemover::visit(ASTForm_All1* form) {
    ASTForm_Not* negPhi = new ASTForm_Not(form->f, form->f->pos);
    ASTForm_Ex1* exNegPhi = new ASTForm_Ex1(form->ul, form->vl, negPhi, form->pos);

    // Delete the all
    form->detach();
    delete form;

    return new ASTForm_Not(exNegPhi, Pos());
}

/**
 * Removes the universal quantifier as follows:
 *  All2 x. phi -> not Ex2 x. not phi
 *
 * @param[in] form      traversed All node
 */
AST* UniversalQuantifierRemover::visit(ASTForm_All2* form) {
    ASTForm_Not* negPhi = new ASTForm_Not(form->f, form->f->pos);
    ASTForm_Ex2* exNegPhi = new ASTForm_Ex2(form->ul, form->vl, negPhi, form->pos);

    // Delete the all2
    form->detach();
    delete form;

    return new ASTForm_Not(exNegPhi, form->pos);
}