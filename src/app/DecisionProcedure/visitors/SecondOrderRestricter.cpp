/*****************************************************************************
 *  dWiNA - Deciding WSkS using non-deterministic automata
 *
 *  Copyright (c) 2015  Tomas Fiedor <ifiedortom1@fit.vutbr.cz>
 *
 *  Description:
 *    Visitor for restricting the orders of the formulae to second order only
 *
 *****************************************************************************/

#include "SecondOrderRestricter.h"

/**
 * Restricts the first order quantifer to second order one
 *
 * @param[in] form:     traversed Ex1 node
 */
AST* SecondOrderRestricter::visit(ASTForm_Ex1* form) {
    ASTForm_Ex2 *ex = new ASTForm_Ex2(form->ul, form->vl, form->f, form->pos);
    return ex;
}

/**
 * Restricts the first order quantifier to second order one
 *
 * @param[in] form:     traversed All1 node
 */
AST* SecondOrderRestricter::visit(ASTForm_All1* form) {
    ASTForm_All2 *all = new ASTForm_All2(form->ul, form->vl, form->f, form->pos);
    return all;
}

/**
 * Restricts the first order quantifier to second order one
 */
AST* SecondOrderRestricter::visit(ASTForm_Equal1* form) {
    ASTTerm *lhs = form->t1, *rhs = form->t2;

    if(lhs->kind == aVar1) {
        lhs = new ASTTerm2_Var2(static_cast<ASTTerm1_Var1*>(lhs)->getVar(), lhs->pos);
    }
    if(rhs->kind == aVar2) {
        rhs = new ASTTerm2_Var2(static_cast<ASTTerm1_Var1*>(rhs)->getVar(), rhs->pos);
    }

    return new ASTForm_Equal2(static_cast<ASTTerm2*>(lhs), static_cast<ASTTerm2*>(rhs), form->pos);
}