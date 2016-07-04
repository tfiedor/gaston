//
// Created by Raph on 28/06/2016.
//

#include "InverseFixpointDetagger.h"
#include "../../environment.hh"

void InverseFixpointDetagger::visit(ASTForm *form) {
    if(form->fixpoints_from_root > this->_cFixpointThreshold && !form->is_restriction) {
        form->tag = 0;
    }
}

void InverseFixpointDetagger::visit(ASTForm_ff *form) {
    form->f1->fixpoints_from_root = form->fixpoints_from_root;
    form->f2->fixpoints_from_root = form->fixpoints_from_root;

#   if(OPT_EXTRACT_MORE_AUTOMATA == true && OPT_CREATE_QF_AUTOMATON == true && MONA_FAIR_MODE == false)
    if(form->fixpoints_from_root > this->_cFixpointThreshold) {
        if(form->f1->is_restriction) {
            form->tag = 1;
        } else {
            form->tag = 0;
        }
    }
#   endif
}

void InverseFixpointDetagger::visit(ASTForm_Not *form) {
    form->f->fixpoints_from_root = form->fixpoints_from_root;

    if(form->fixpoints_from_root > this->_cFixpointThreshold)  {
        // Pull the negation in
        form->tag = 0;
    }
}

template<class FixpointFormula>
void InverseFixpointDetagger::_visitFixpointComputation(FixpointFormula *form) {
    form->fixpoints_from_root = form->fixpoints_from_root + 1;
    form->f->fixpoints_from_root = form->fixpoints_from_root;

#   if(OPT_EXTRACT_MORE_AUTOMATA == true)
    if(form->fixpoints_from_root > this->_cFixpointThreshold) {
        form->tag = 0;
    }
#   endif
}

void InverseFixpointDetagger::visit(ASTForm_Ex1 *form) {
    this->_visitFixpointComputation<ASTForm_Ex1>(form);
}

void InverseFixpointDetagger::visit(ASTForm_Ex2 *form) {
    this->_visitFixpointComputation<ASTForm_Ex2>(form);
}

void InverseFixpointDetagger::visit(ASTForm_All1 *form) {
    this->_visitFixpointComputation<ASTForm_All1>(form);
}

void InverseFixpointDetagger::visit(ASTForm_All2 *form) {
    this->_visitFixpointComputation<ASTForm_All2>(form);
}