/*****************************************************************************
 *  dWiNA - Deciding WSkS using non-deterministic automata
 *
 *  Copyright (c) 2015  Tomas Fiedor <xfiedo01@stud.fit.vutbr.cz>
 *
 *  Description:
 *    Visitor for restricting the syntax of the WSkS logic.
 *
 *****************************************************************************/
#include "SyntaxRestricter.h"

/**
 * Transforms implication to restricted syntax as follows:
 *  A => B = not A or B
 *
 *  @param[in] form     traversed Impl node
 */
AST* SyntaxRestricter::visit(ASTForm_Impl* form) {
    // TODO: implement
    return form;
}

/**
 * Transforms biimplication to two implications:
 *  A <=> B = A => B and B => A
 *
 * @param[in] form      traversed Biimpl node
 */
AST* SyntaxRestricter::visit(ASTForm_Biimpl* form) {
    // TODO: implement
    return form;
}