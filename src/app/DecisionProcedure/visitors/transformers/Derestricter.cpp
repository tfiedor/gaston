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
        return form->f2->accept(*this);
    } else if(form->f2->kind == aFirstOrder) {
        return form->f1->accept(*this);
    } else {
        form->f1 = static_cast<ASTForm*>(form->f1->accept(*this));
        form->f2 = static_cast<ASTForm*>(form->f2->accept(*this));
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

AST* Derestricter::visit(ASTForm_Not* form) {
    form->f = static_cast<ASTForm*>(form->f->accept(*this));
    return form;
}

AST* Derestricter::visit(ASTForm_Ex2* form) {
    // Fixme: this is wrong, maybe it is source of problems?
    IdentList firstOrder, secondOrder, zeroOrder;
    for(auto it = form->vl->begin(); it != form->vl->end(); ++it) {
        switch(symbolTable.lookupType(*it)) {
            case MonaTypeTag::Varname0:
                this->_convertedZeroOrderVars.insert(*it);
                zeroOrder.push_back(*it);
                break;
            case MonaTypeTag::Varname1:
                firstOrder.push_back(*it);
                break;
            case MonaTypeTag::Varname2:
                secondOrder.push_back(*it);
                break;
            default:
                assert(false && "Some unsupported MonaTypeTag in Derestricter");
        }
    }

    ASTForm* result = static_cast<ASTForm*>(form->f->accept(*this));
    if(zeroOrder.size() > 0) {
        result = new ASTForm_Ex0(new IdentList(zeroOrder), result, result->pos);
    }
    if(firstOrder.size() > 0) {
        result = new ASTForm_Ex1(nullptr, new IdentList(firstOrder), result, result->pos);
    }
    if(secondOrder.size() > 0) {
        result = new ASTForm_Ex2(nullptr, new IdentList(secondOrder), result, result->pos);
    }

    return result;
}

AST* Derestricter::visit(ASTForm_Sub* form) {
    if(form->T1->kind == aVar2 && form->T2->kind == aEmpty) {
        ASTTerm2_Var2* var = static_cast<ASTTerm2_Var2*>(form->T1);
        auto found = this->_convertedZeroOrderVars.find(var->n);
        if(found != this->_convertedZeroOrderVars.end()) {
            // Fixme: Leak
            return new ASTForm_Var0(var->n, form->pos);
        }
    }
    return form;
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

AST* Defirstorderer::visit(ASTForm_FirstOrder* form) {
    return new ASTForm_True(Pos());
}
