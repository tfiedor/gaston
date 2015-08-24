/*****************************************************************************
 *  dWiNA - Deciding WSkS using non-deterministic automata
 *
 *  Copyright (c) 2015  Tomas Fiedor <ifiedortom1@fit.vutbr.cz>
 *
 *  Description:
 *    Visitor for transforming the formula to Prenex Normal Form, i.e. in
 *    the form of:
 *      ex Xn: ... not ex X1: phi
 *
 *****************************************************************************/

#include "PrenexNormalFormTransformer.h"

/**
 * Pushes quantifiers to the root of the formulae, creating a prefix
 *
 * @param[in] form:     traversed binary connective node
 */
AST* PrenexNormalFormTransformer::visit(ASTForm_ff* form) {
    // TODO: implement
    return form;
}

/**
 * Negates the quantifier during the pushing
 *
 * @param[in] form:     traversed Not node
 */
AST* PrenexNormalFormTransformer::visit(ASTForm_Not* form) {
    // TODO: implement
    return form;
}