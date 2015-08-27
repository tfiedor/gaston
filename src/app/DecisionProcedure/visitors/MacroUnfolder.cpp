/*****************************************************************************
 *  dWiNA - Deciding WSkS using non-deterministic automata
 *
 *  Copyright (c) 2015  Tomas Fiedor <ifiedortom1@fit.vutbr.cz>
 *
 *  Description:
 *    Visitor for unfolding Calls, i.e. replaces the call by formula and extract
 *    the information.
 *
 *    TODO: think if we can omit this in our method
 *
 *****************************************************************************/

#include "MacroUnfolder.h"

/**
 * @param[in] form:     traversed Var0 node
 */
AST* MacroUnfolder::visit(ASTForm_Var0* form) {
    assert(form != nullptr);
    // TODO: implement
    return form;
}

/**
 * @param[in] form:     traversed Var1 node
 */
AST* MacroUnfolder::visit(ASTTerm1_Var1* term) {
    assert(form != nullptr);
    // TODO: implement
    return form;
}

/**
 * @param[in] form:     traversed Var2 node
 */
AST* MacroUnfolder::visit(ASTTerm2_Var2* term) {
    assert(form != nullptr);
    // TODO: implement
    return form;
}

/**
 * @param[in] form:     traversed zero order quantifier node
 */
AST* MacroUnfolder::visit(ASTForm_vf* form) {
    assert(form != nullptr);
    // TODO: implement
    return form;
}

/**
 * @param[in] form:     traversed quantifier node
 */
AST* MacroUnfolder::visit(ASTForm_uvf* form) {
    assert(form != nullptr);
    // TODO: implement
    return form;
}

/**
 * @param[in] form:     traversed Ex0 node
 */
AST* MacroUnfolder::visit(ASTForm_Ex0* form) {
    assert(form != nullptr);
    return static_cast<ASTForm_vf*>(form)->accept(*this);
}

/**
 * @param[in] form:     traversed Ex1 node
 */
AST* MacroUnfolder::visit(ASTForm_Ex1* form) {
    assert(form != nullptr);
    return static_cast<ASTForm_uvf*>(form)->accept(*this);
}

/**
 * @param[in] form:     traversed Ex2 node
 */
AST* MacroUnfolder::visit(ASTForm_Ex2* form) {
    assert(form != nullptr);
    return static_cast<ASTForm_uvf*>(form)->accept(*this);
}

/**
 * @param[in] form:     traversed All0 form
 */
AST* MacroUnfolder::visit(ASTForm_All0* form) {
    assert(form != nullptr);
    return static_cast<ASTForm_vf*>(form)->accept(*this);
}

/**
 * @param[in] form:     traversed All1 form
 */
AST* MacroUnfolder::visit(ASTForm_All1* form) {
    assert(form != nullptr);
    return static_cast<ASTForm_uvf*>(form)->accept(*this);
}

/**
 * @param[in] form:     traversed All2 form
 */
AST* MacroUnfolder::visit(ASTForm_All2* form) {
    assert(form != nullptr);
    return static_cast<ASTForm_uvf*>(form)->accept(*this);
}