//
// Created by Raph on 29/01/2016.
//

#include "FixpointDetagger.h"
#include "../environment.hh"

void FixpointDetagger::visit(ASTForm *form) {
    if(form->fixpoint_number <= this->_cFixpointThreshold) {
        form->tag = 0;
    }
}

void FixpointDetagger::visit(ASTForm_And *form) {
    form->fixpoint_number = std::max(form->f1->fixpoint_number, form->f2->fixpoint_number);
    form->height = std::max(form->f1->height, form->f2->height) + 1;
    form->size = form->f1->size + form->f2->size + 1;

#   if(OPT_EXTRACT_MORE_AUTOMATA == true && OPT_CREATE_QF_AUTOMATON == true && MONA_FAIR_MODE == false)
    if(form->fixpoint_number <= this->_cFixpointThreshold) {
        form->tag = 0;
    }
#   endif
}

void FixpointDetagger::visit(ASTForm_Or *form) {
    form->fixpoint_number = std::max(form->f1->fixpoint_number, form->f2->fixpoint_number);
    form->height = std::max(form->f1->height, form->f2->height) + 1;
    form->size = form->f1->size + form->f2->size + 1;

#   if(OPT_EXTRACT_MORE_AUTOMATA == true && OPT_CREATE_QF_AUTOMATON == true && MONA_FAIR_MODE == false)
    if(form->fixpoint_number <= this->_cFixpointThreshold) {
        form->tag = 0;
    }
#   endif
}

void FixpointDetagger::visit(ASTForm_Impl *form) {
    form->fixpoint_number = std::max(form->f1->fixpoint_number, form->f2->fixpoint_number);
    form->height = std::max(form->f1->height, form->f2->height) + 1;
    form->size = form->f1->size + form->f2->size + 1;

#   if(OPT_EXTRACT_MORE_AUTOMATA == true && OPT_CREATE_QF_AUTOMATON == true && MONA_FAIR_MODE == false)
    if(form->fixpoint_number <= this->_cFixpointThreshold) {
        form->tag = 0;
    }
#   endif
}

void FixpointDetagger::visit(ASTForm_Biimpl *form) {
    form->fixpoint_number = std::max(form->f1->fixpoint_number, form->f2->fixpoint_number);
    form->height = std::max(form->f1->height, form->f2->height) + 1;
    form->size = form->f1->size + form->f2->size + 1;

#   if(OPT_EXTRACT_MORE_AUTOMATA == true && OPT_CREATE_QF_AUTOMATON == true && MONA_FAIR_MODE == false)
    if(form->fixpoint_number <= this->_cFixpointThreshold) {
        form->tag = 0;
    }
#   endif
}

void FixpointDetagger::visit(ASTForm_Not *form) {
    form->fixpoint_number = form->f->fixpoint_number;
    form->height = form->height + 1;
    form->size = form->f->size + 1;

    if(form->f->tag == 0)  {
        // Pull the negation in
        form->tag = 0;
    }
}

template<class FixpointFormula>
void FixpointDetagger::_visitFixpointComputation(FixpointFormula *form) {
    form->fixpoint_number = form->f->fixpoint_number + 1;
    form->height = form->height + 1;
    form->size = form->f->size + 1;

#   if(OPT_EXTRACT_MORE_AUTOMATA == true)
    if(form->fixpoint_number <= this->_cFixpointThreshold) {
        form->tag = 0;
    }
#   endif
}

void FixpointDetagger::visit(ASTForm_Ex1 *form) {
    this->_visitFixpointComputation<ASTForm_Ex1>(form);
}

void FixpointDetagger::visit(ASTForm_Ex2 *form) {
    this->_visitFixpointComputation<ASTForm_Ex2>(form);
}

void FixpointDetagger::visit(ASTForm_All1 *form) {
    this->_visitFixpointComputation<ASTForm_All1>(form);
}

void FixpointDetagger::visit(ASTForm_All2 *form) {
    this->_visitFixpointComputation<ASTForm_All2>(form);
}