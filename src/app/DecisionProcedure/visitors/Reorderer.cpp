/*****************************************************************************
 *  dWiNA - Deciding WSkS using non-deterministic automata
 *
 *  Copyright (c) 2015  Tomas Fiedor <ifiedortom1@fit.vutbr.cz>
 *
 *  Description:
 *    Visitor for doing the reordering of the formulae. This could be
 *    optimization for our future method, so simpler formulae are traversed
 *    first achieving better performance
 *
 *****************************************************************************/

#include "Reorderer.h"

/**
 * @param[in] form:     traversed binary logical connective
 */
AST* Reorderer::visit(ASTForm_ff* form) {
    assert(form->kind != aImpl);
    assert(form->kind != aBiimpl);
    assert(form != nullptr);

    //TODO: implement
    return form;
}

/**
 * @param[in] form:     traversed And node
 */
AST* Reorderer::visit(ASTForm_And* form) {
    assert(form != nullptr);
    return static_cast<ASTForm_ff*>(form)->accept(*this);
}

/**
 * @param[in] form:     traversed Or node
 */
AST* Reorderer::visit(ASTForm_Or* form) {
    assert(form != nullptr);
    return static_cast<ASTForm_ff*>(form)->accept(*this);
}

/**
 * @param[in] form:     traversed Impl node
 */
AST* Reorderer::visit(ASTForm_Impl* form) {
    assert(false && "Implication should not be in reordered formula");
    return form;
}

/**
 * @param[in] form:     traversed Biimpl node
 */
AST* Reorderer::visit(ASTForm_Biimpl* form) {
    assert(false && "Biimplication should not be in reordered formula");
    return form;
}