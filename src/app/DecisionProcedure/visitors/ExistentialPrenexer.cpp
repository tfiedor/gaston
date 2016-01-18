//
// Created by Raph on 17/01/2016.
//

#include "ExistentialPrenexer.h"
#include "../../Frontend/ast.h"
#include "../../Frontend/ast_visitor.h"

template<class BinopClass>
AST* ExistentialPrenexer::moveExistUp(AST* form) {
    BinopClass *bForm = reinterpret_cast<BinopClass*>(form);
    if(bForm->f1->kind == ASTKind::aEx1 || bForm->f2->kind == ASTKind::aEx2) {
        ASTForm_uvf* exForm = reinterpret_cast<ASTForm_uvf*>(bForm->f1);
        bForm->f1 = exForm->f;
        exForm->f = bForm;
        return exForm;
    } else if(bForm->f2->kind == ASTKind::aEx1 || bForm->f2->kind == ASTKind::aEx2) {
        ASTForm_uvf* exForm = reinterpret_cast<ASTForm_uvf*>(bForm->f2);
        bForm->f2 = exForm->f;
        exForm->f = bForm;
        return exForm;
    } else {
        return form;
    }
}

AST* ExistentialPrenexer::visit(ASTForm_And* form) {
    return reinterpret_cast<ASTForm*>((moveExistUp<ASTForm_And>(form)))->accept(*this);
}

AST* ExistentialPrenexer::visit(ASTForm_Or* form) {
    return reinterpret_cast<ASTForm*>((moveExistUp<ASTForm_Or>(form)))->accept(*this);
}