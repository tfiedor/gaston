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
    assert(form != nullptr);

    // not f1
    ASTForm_Not* notF = new ASTForm_Not(form->f1, form->f1->pos);
    // not f1 or f2
    ASTForm_Or* notForFF = new ASTForm_Or(notF, form->f2, form->pos);
    return notForFF;
}

/**
 * Transforms biimplication to two implications:
 *  A <=> B = A => B and B => A
 *
 * @param[in] form      traversed Biimpl node
 */
AST* SyntaxRestricter::visit(ASTForm_Biimpl* form) {
    assert(form != nullptr);

    ASTForm_Not* notF = new ASTForm_Not(form->f1, form->f1->pos);
    ASTForm_Not* notFF = new ASTForm_Not(form->f2, form->f2->pos);

    ASTForm* ff1 = form->f1->clone();
    ASTForm* ff2 = form->f2->clone();

    ASTForm* impl1 = new ASTForm_Or(notF, ff2, form->pos);
    ASTForm* impl2 = new ASTForm_Or(ff1, notFF, form->pos);

    ASTForm_And* biimpl = new ASTForm_And(impl1, impl2, form->pos);

    return biimpl;
}