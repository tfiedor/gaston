/*****************************************************************************
 *  dWiNA - Deciding WSkS using non-deterministic automata
 *
 *  Copyright (c) 2015  Tomas Fiedor <ifiedortom1@fit.vutbr.cz>
 *
 *  Description:
 *    Visitor for flattening of the formula
 *
 *****************************************************************************/
#include "Flattener.h"

/**
 * Flattens the non-one pluses to cascade of +1
 *
 * @param[in] term      traversed Plus term
 */
AST* Flattener::visit(ASTTerm1_Plus* term) {
    // TODO: implement
    return term;
}

/**
 * Flattens the x = y to simplify the output
 *
 * @param[in] form:     traversed Equal1 node
 */
AST* Flattener::visit(ASTForm_Equal1* form) {
    // TODO: implement
    return form;
}

/**
 * Flattens the X = Y to simplify the formulae
 *
 * @param[in] form      traversed Equal2 node
 */
AST* Flattener::visit(ASTForm_Equal2* form) {
    // TODO: implement
    return form;
}

/**
 * Flattens the x != y to simplify the formulae
 *
 * @param[in] form      traversed NotEqual1 node
 */
AST* Flattener::visit(ASTForm_NotEqual1* form) {
    // TODO: implement
    return form;
}

/**
 * Flattens the X != Y to simplify the formulae
 *
 * @param[in] form      traversed NotEqual2 node
 */
AST* Flattener::visit(ASTForm_NotEqual2* form) {
    // TODO: implement
    return form;
}

/**
 * Flattens the x < y to simplify the formula
 *
 * @param[in] form:     traversed Less node
 */
AST* Flattener::visit(ASTForm_Less* form) {
    // TODO: implement
    return form;
}

/**
 * Flattens the x <= y to simplify the formula
 *
 * @param[in] form:    traversed LessEq node
 */
AST* Flattener::visit(ASTForm_LessEq* form) {
    // TODO: implement
    return form;
}

/**
 * Flattens the x in X to simplify the formula
 *
 * @param[in] form:    traversed In node
 */
AST* Flattener::visit(ASTForm_In* form) {
    // TODO: implement
    return form;
}

/**
 * Flattens the x notin X to simplify the formula
 *
 * @param[in] form:     traversed Notin node
 */
AST* Flattener::visit(ASTForm_Notin* form) {
    // TODO: implement
    return form;
}

/**
 * Flattens the X sub Y  to simplify the formula
 *
 * @param[in] form:     traversed Sub node
 */
AST* Flattener::visit(ASTForm_Sub* form) {
    // TODO: implement
    return form;
}

/**
 * Flattens the Ex1 phi to simplify the formula
 *
 * @param[in] form:     traversed Ex1 node
 */
AST* Flattener::visit(ASTForm_Ex1* form) {
    // TODO: implement
    return form;
}

/**
 * Flattens the All1 phi to simplify the formula
 *
 * @param[in] form:     traversed All1 node
 */
AST* Flattener::visit(ASTForm_All1* form) {
    // TODO: implement
    return form;
}