//
// Created by Raph on 15/08/2016.
//

#include "Derestricter.h"
#include "../../../Frontend/ast.h"
#include "../../../Frontend/symboltable.h"

extern SymbolTable symbolTable;

template<class BinaryForm>
AST* Derestricter::_visitBinaryForm(BinaryForm* form) {
    return form;
    if(form->f1->kind == aFirstOrder) {
        return form->f2;
    } else if(form->f2->kind == aFirstOrder) {
        return form->f1;
    } else {
        return form;
    }
}

AST* Derestricter::visit(ASTForm_And* form) {
    return this->_visitBinaryForm<ASTForm_And>(form);
}

AST* Derestricter::visit(ASTForm_Biimpl* form) {
    return this->_visitBinaryForm<ASTForm_Biimpl>(form);
}

AST* Derestricter::visit(ASTForm_Impl* form) {
    return this->_visitBinaryForm<ASTForm_Impl>(form);
}

AST* Derestricter::visit(ASTForm_Or* form) {
    return this->_visitBinaryForm<ASTForm_Or>(form);
}

AST* Derestricter::visit(ASTForm_Ex2* form) {
    if(symbolTable.lookupType(*(form->vl->begin())) == MonaTypeTag::Varname1) {
        return new ASTForm_Ex1(form->ul, form->vl, form->f, form->pos);
    } else {
        return form;
    }
}