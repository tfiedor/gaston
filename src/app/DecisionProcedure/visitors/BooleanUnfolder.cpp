/*****************************************************************************
 *  dWiNA - Deciding WSkS using non-deterministic automata
 *
 *  Copyright (c) 2015  Tomas Fiedor <xfiedo01@stud.fit.vutbr.cz>
 *
 *  Description:
 *    Visitor for unfolding the true and false from the formula
 *
 *****************************************************************************/

#include "BooleanUnfolder.h"

/**
 * Transforms the formula according to the following laws:
 *  True and phi = phi
 *  False and phi = False
 *
 * @param[in] form:     traversed And node
 */
AST* BooleanUnfolder::visit(ASTForm_And* form) {
    // True and phi = phi
    if(form->f1->kind == aTrue) {
        return form->f2;
    } else if(form->f2->kind == aTrue) {
        return form->f1;
    // False and phi = False
    } else if(form->f1->kind == aFalse) {
        return form->f1;
    } else if(form->f2->kind == aFalse) {
        return form->f2;
    // Else do nothing
    } else {
        return form;
    }
}

/**
 * Transforms the formula according to the following laws:
 *  True or phi = True
 *  False or phi = phi
 *
 *  @param[in] form:    traversed Or node
 */
AST* BooleanUnfolder::visit(ASTForm_Or* form) {
    // True or phi = True
    if(form->f1->kind == aTrue) {
        return form->f1;
    } else if(form->f2->kind == aTrue) {
        return form->f2;
    // False or phi = phi
    } else if(form->f1->kind == aFalse) {
        return form->f2;
    } else if(form->f2->kind == aFalse) {
        return form->f1;
    // Else do nothing
    } else {
        return form;
    }
}

/**
 * Transforms the formula according to the following laws:
 *  Not True = False
 *  Not False = True
 *
 *  @param[in] form:    traversed Not node
 */
AST* BooleanUnfolder::visit(ASTForm_Not* form) {
    // Not True = False
    if(form->f->kind == aTrue) {
        return new ASTForm_False(form->f->pos);
    // Not False = True
    } else if(form->f->kind == aFalse) {
        return new ASTForm_True(form->f->pos);
    // Else do nothing
    } else {
        return form;
    }
}

/**
 * Transforms the formula according to the following laws:
 *  False => phi = True
 *  True => phi = phi
 *  phi => True = True
 *  phi => False = not phi
 *
 *  @param[in] form:    traversed Impl node
 */
AST* BooleanUnfolder::visit(ASTForm_Impl* form) {
    // False => phi = True
    if (form->f1->kind == aFalse) {
        return new ASTForm_True(form->pos);
    // True => phi = phi
    } else if(form->f1->kind == aTrue) {
        return form->f2;
    // phi => True = True
    } else if(form->f2->kind == aTrue) {
        return form->f2;
    // phi => False = not phi
    } else if(form->f2->kind == aFalse) {
        return new ASTForm_Not(form->f1, form->pos);
    // Else do nothing
    } else {
        return form;
    }
}

/**
 * Transforms the formula according to the following laws:
 *  True <=> phi = phi
 *  False <=> phi = not phi
 *
 *  @param[in] form:    traversed Biimpl node
 */
AST* BooleanUnfolder::visit(ASTForm_Biimpl* form) {
    // True <=> phi = phi
    if(form->f1->kind == aTrue) {
        return form->f2;
    } else if(form->f2->kind == aTrue) {
        return form->f1;
    // False <=> phi = not phi
    } else if(form->f1->kind == aFalse) {
        return new ASTForm_Not(form->f2, form->pos);
    } else if(form->f2->kind == aFalse) {
        return new ASTForm_Not(form->f1, form->pos);
    // Else do nothing
    } else {
        return form;
    }
}