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
    assert(form->f->kind != aImpl);
    assert(form->f->kind != aBiimpl);

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
    } else {
        result = form;
    }
    return result;
}