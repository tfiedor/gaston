//
// Created by Raph on 02/05/2016.
//

#include "UnderComplementDecorator.h"

void UnderComplementDecorator::visit(ASTForm_And *form) {
    form->f1->under_complement = form->under_complement;
    form->f2->under_complement = form->under_complement;
}

void UnderComplementDecorator::visit(ASTForm_Or *form) {
    form->f1->under_complement = form->under_complement;
    form->f2->under_complement = form->under_complement;
}

void UnderComplementDecorator::visit(ASTForm_Impl *form) {
    form->f1->under_complement = form->under_complement;
    form->f2->under_complement = form->under_complement;
}

void UnderComplementDecorator::visit(ASTForm_Biimpl *form) {
    form->f1->under_complement = form->under_complement;
    form->f2->under_complement = form->under_complement;
}

void UnderComplementDecorator::visit(ASTForm_Not *form) {
    form->f->under_complement = !form->under_complement;
}

void UnderComplementDecorator::visit(ASTForm_Ex1 *form) {
    form->f->under_complement = form->under_complement;
}

void UnderComplementDecorator::visit(ASTForm_Ex2 *form) {
    form->f->under_complement = form->under_complement;
}

void UnderComplementDecorator::visit(ASTForm_All1 *form) {
    form->f->under_complement = form->under_complement;
}

void UnderComplementDecorator::visit(ASTForm_All2 *form) {
    form->f->under_complement = form->under_complement;
}