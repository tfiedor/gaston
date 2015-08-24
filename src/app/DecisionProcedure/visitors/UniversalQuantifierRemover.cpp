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

/**
 * Removes the universal quantifier as follows:
 *  All0 x. phi -> not Ex0 x. not phi
 *
 * @param[in] form      traversed All node
 */
AST* UniversalQuantifierRemover::visit(ASTForm_All0* form) {
    // TODO: implement
    return form;
}

/**
 * Removes the universal quantifier as follows:
 *  All1 x. phi -> not Ex2 x. not phi
 *
 * @param[in] form      traversed All node
 */
AST* UniversalQuantifierRemover::visit(ASTForm_All1* form) {
    // TODO: implement
    return form;
}

/**
 * Removes the universal quantifier as follows:
 *  All2 x. phi -> not Ex2 x. not phi
 *
 * @param[in] form      traversed All node
 */
AST* UniversalQuantifierRemover::visit(ASTForm_All2* form) {
    // TODO: implement
    return form;
}