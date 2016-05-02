//
// Created by Raph on 28/04/2016.
//

#include "ContinuationSwitcher.h"
#include "../../../Frontend/symboltable.h"

extern SymbolTable symbolTable;

template<class Binop>
void ContinuationSwitcher::_switch(Binop* form) {
    if(!form->f1->is_restriction) {
        ASTForm *f = form->f1;
        form->f1 = form->f2;
        form->f2 = f;
    }
}

void ContinuationSwitcher::visit(ASTForm_And *form) {
    if(form->under_complement) {
        if(!form->f1->epsilon_in && form->f2->epsilon_in) {
            // Switch
            this->_switch<ASTForm_And>(form);
        }
        form->epsilon_in = form->f1->epsilon_in || form->f2->epsilon_in;
    } else {
        if(form->f1->epsilon_in && !form->f2->epsilon_in) {
            // Switch
            this->_switch<ASTForm_And>(form);
        }
        form->epsilon_in = form->f2->epsilon_in && form->f2->epsilon_in;
    }
}

void ContinuationSwitcher::visit(ASTForm_Or *form) {
    if(form->under_complement) {
        if(form->f1->epsilon_in && !form->f2->epsilon_in) {
            // Switch
            this->_switch<ASTForm_Or>(form);
        }
        form->epsilon_in = form->f1->epsilon_in && form->f2->epsilon_in;
    } else {
        if(!form->f1->epsilon_in && form->f2->epsilon_in) {
            // Switch
            this->_switch<ASTForm_Or>(form);
        }
        form->epsilon_in = form->f1->epsilon_in || form->f2->epsilon_in;
    }
}

void ContinuationSwitcher::visit(ASTForm_Ex1 *form) {
    form->epsilon_in = form->f->epsilon_in;
}

void ContinuationSwitcher::visit(ASTForm_Ex2 *form) {
    form->epsilon_in = form->f->epsilon_in;
}

void ContinuationSwitcher::visit(ASTForm_Not *form) {
    form->epsilon_in = !form->f->epsilon_in;
}