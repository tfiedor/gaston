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
    // TODO: implement
    return form;
}

/**
 * Restricts the first order quantifier to second order one
 *
 * @param[in] form:     traversed All1 node
 */
AST* SecondOrderRestricter::visit(ASTForm_All1* form) {
    // TODO: implement
    return form;
}