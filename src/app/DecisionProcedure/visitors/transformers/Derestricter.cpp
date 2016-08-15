//
// Created by Raph on 15/08/2016.
//

#include "Derestricter.h"
#include "../../../Frontend/ast.h"
#include "../../../Frontend/symboltable.h"

extern SymbolTable symbolTable;

AST* Derestricter::visit(ASTForm_ff* form) {
    // I guess having the FirstOrder() is not fault
    return form;
}

AST* Derestricter::visit(ASTForm_Ex2* form) {
    if(symbolTable.lookupType(*(form->vl->begin())) == MonaTypeTag::Varname1) {
        return new ASTForm_Ex1(form->ul, form->vl, form->f, form->pos);
    } else {
        return form;
    }
}