/*****************************************************************************
 *  dWiNA - Deciding WSkS using non-deterministic automata
 *
 *  Copyright (c) 2015  Tomas Fiedor <ifiedortom1@fit.vutbr.cz>
 *
 *  Description:
 *    Visitor for doing the anti-prenexing. This means instead of pushing
 *    the quantifiers higher to root, we push them deeper towards leaves.
 *    We do this if we see that some variable is not bound in the formula.
 *    Thus we can push the quantifier to the lhs or rhs.
 *
 *****************************************************************************/

#include "AntiPrenexer.h"

/**
 * @param[in] form:     traversed Ex0 node
 */
AST* AntiPrenexer::visit(ASTForm_Ex0* form) {
    // TODO: implement
    return form;
}

/**
 * @param[in] form:     traversed Ex1 node
 */
AST* AntiPrenexer::visit(ASTForm_Ex1* form) {
    // TODO: implement;
    return form;
}

/**
 * @param[in] form:     traversed Ex2 node
 */
AST* AntiPrenexer::visit(ASTForm_Ex2* form) {
    // TODO: implement
    return form;
}

/**
 * @param[in] form:     traversed All0 node
 */
AST* AntiPrenexer::visit(ASTForm_All0* form) {
    // TODO: implement
    return form;
}

/**
 * @param[in] form:     traversed All1 node
 */
AST* AntiPrenexer::visit(ASTForm_All1* form) {
    // TODO: implement
    return form;
}

/**
 * @param[in] form:     traversed All2 node
 */
AST* AntiPrenexer::visit(ASTForm_All2* form) {
    // TODO: implement
    return form;
}