//
// Created by Raph on 18/01/2016.
//

#include "ZeroOrderRemover.h"
#include "../../Frontend/ast.h"

AST* ZeroOrderRemover::visit(ASTForm_All0 *form) {
    // Fixme: form is lost;
    return new ASTForm_All2(nullptr, form->vl, form->f, Pos());
}

AST* ZeroOrderRemover::visit(ASTForm_Ex0 *form) {
    // Fixme: form is lost
    return new ASTForm_Ex2(nullptr, form->vl, form->f, Pos());
}

AST* ZeroOrderRemover::visit(ASTForm_Var0 *form) {
    // Fixme: form is lost
    return new ASTForm_Sub(new ASTTerm2_Var2(form->GetVar(), Pos()), new ASTTerm2_Empty(Pos()), Pos());
}
