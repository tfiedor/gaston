//
// Created by Raph on 15/08/2016.
//

#include "Derestricter.h"
#include "../../../Frontend/ast.h"
#include "../../../Frontend/symboltable.h"

extern SymbolTable symbolTable;

template<class BinaryForm>
AST* Derestricter::_visitBinaryForm(BinaryForm* form) {
    // Fixme: Unground formulae should be made an exception
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

template<class BinaryForm>
AST* Defirstorderer::_visitBinaryForm(BinaryForm* form) {
    if(form->f1->kind == aFirstOrder) {
        return (form->f2)->accept(*this);
    } else if(form->f2->kind == aFirstOrder) {
        return (form->f1)->accept(*this);
    } else {
        form->f1 = static_cast<ASTForm*>(form->f1->accept(*this));
        form->f2 = static_cast<ASTForm*>(form->f2->accept(*this));
        return form;
    }
}

AST* Defirstorderer::visit(ASTForm_And* form) {
    return this->_visitBinaryForm<ASTForm_And>(form);
}

AST* Defirstorderer::visit(ASTForm_Biimpl* form) {
    return this->_visitBinaryForm<ASTForm_Biimpl>(form);
}

AST* Defirstorderer::visit(ASTForm_Impl* form) {
    return this->_visitBinaryForm<ASTForm_Impl>(form);
}

AST* Defirstorderer::visit(ASTForm_Or* form) {
    return this->_visitBinaryForm<ASTForm_Or>(form);
}