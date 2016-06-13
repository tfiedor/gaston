/*****************************************************************************
 *  dWiNA - Deciding WSkS using non-deterministic automata
 *
 *  Copyright (c) 2015  Tomas Fiedor <ifiedortom@fit.vutbr.cz>
 *
 *  Description:
 *    Traverses the tree and unfolds negations, i.e. tries to fold them as
 *    deeply as possible to terms of logic. This way the formula is as simple
 *    as possible.
 *
 *****************************************************************************/
#include "NegationUnfolder.h"
#include "../../../Frontend/ast.h"

AST* NegationUnfolder::visit(ASTForm_Not* form) {
    ASTForm* result;
    // not not phi = phi
    if(form->f->kind == aNot) {
        result = static_cast<ASTForm_Not*>(form->f)->f;

        (form->f)->detach();
        delete form;
    // not (A or B) = not A and not B
    // not (A and B) = not A or not B
    } else if(form->f->kind == aOr || form->f->kind == aAnd) {
        ASTForm_Not* lhs, *rhs;
        ASTForm_ff* child = static_cast<ASTForm_ff*>(form->f);
        lhs = new ASTForm_Not(child->f1, child->pos);
        rhs = new ASTForm_Not(child->f2, child->pos);

        if(form->f->kind == aOr) {
            result = new ASTForm_And(static_cast<ASTForm*>(lhs->accept(*this)), static_cast<ASTForm*>(rhs->accept(*this)), form->pos);
        // kind == aAnd
        } else {
            assert(form->f->kind == aAnd);
            result = new ASTForm_Or(static_cast<ASTForm*>(lhs->accept(*this)), static_cast<ASTForm*>(rhs->accept(*this)), form->pos);
        }

        (form->f)->detach();
        delete form;
    // not (A <=> B) = not A <=> B
    } else if(form->f->kind == aBiimpl) {
        ASTForm_Not* lhs;
        ASTForm_ff* child = static_cast<ASTForm_ff*>(form->f);
        lhs = new ASTForm_Not(child->f1, child->pos);
        child->f2 = static_cast<ASTForm*>(child->f2->accept(*this));
        child->f1 = static_cast<ASTForm*>(lhs->accept(*this));

        result = child;
        form->detach();
        delete form;
    // not (A => B) = A and not B
    } else if(form->f->kind == aImpl) {
        ASTForm_Not *rhs;
        ASTForm_ff *child = static_cast<ASTForm_ff *>(form->f);
        rhs = new ASTForm_Not(child->f2, child->pos);
        result = new ASTForm_And(static_cast<ASTForm *>(child->f1->accept(*this)),
                                 static_cast<ASTForm *>(rhs->accept(*this)), form->pos);

        (form->f)->detach();
        delete form;
    } else if(form->f->kind == aNotin) {
        ASTForm_Notin *notin = static_cast<ASTForm_Notin*>(form->f);
        result = new ASTForm_In(notin->t1, notin->T2, notin->pos);
        (form->f)->detach();
        delete form;
    } else if(form->f->kind == aIn) {
        ASTForm_In *in = static_cast<ASTForm_In*>(form->f);
        result = new ASTForm_Notin(in->t1, in->T2, in->pos);
        (form->f)->detach();
        delete form;
    // form
    } else {
        result = form;
    }
    return result;
}