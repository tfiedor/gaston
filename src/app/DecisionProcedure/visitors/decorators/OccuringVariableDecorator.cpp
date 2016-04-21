//
// Created by Raph on 21/04/2016.
//

#include "OccuringVariableDecorator.h"
#include "../../../Frontend/ident.h"
#include "../../../Frontend/ast.h"

template<class ASTNode>
void OccuringVariableDecorator::_decorateUnaryNode(ASTNode* node) {
    if(node->allVars == nullptr) {
        IdentList free, bound;
        node->freeVars(&free, &bound);
        node->allVars = ident_union(&free, &bound);
        if (node->allVars == nullptr) {
            node->allVars = new IdentList();
        }
    }
}

void OccuringVariableDecorator::visit(ASTForm_f *form) {
    assert(form->f->allVars != nullptr);
    if(form->allVars == nullptr) {
        form->allVars = copy(form->f->allVars);
    }
}

void OccuringVariableDecorator::visit(ASTForm_ff *form) {
    assert(form->f1->allVars != nullptr);
    assert(form->f2->allVars != nullptr);
    if(form->allVars == nullptr) {
        form->allVars = ident_union(form->f1->allVars, form->f2->allVars);
    }
}

void OccuringVariableDecorator::visit(ASTForm_nt *form) {
    assert(form->t->allVars != nullptr);
    if(form->allVars == nullptr) {
        form->allVars = copy(form->t->allVars);
    }
}

void OccuringVariableDecorator::visit(ASTForm_nT *form) {
    assert(form->T->allVars != nullptr);
    if(form->allVars == nullptr) {
        form->allVars = copy(form->T->allVars);
    }
}

void OccuringVariableDecorator::visit(ASTForm_T *form) {
    assert(form->T->allVars != nullptr);
    if(form->allVars == nullptr) {
        form->allVars = copy(form->T->allVars);
    }
}

void OccuringVariableDecorator::visit(ASTForm_tT *form) {
    assert(form->t1->allVars != nullptr);
    assert(form->T2->allVars != nullptr);
    if(form->allVars == nullptr) {
        form->allVars = ident_union(form->t1->allVars, form->T2->allVars);
    }
}

void OccuringVariableDecorator::visit(ASTForm_TT *form) {
    assert(form->T1->allVars != nullptr);
    assert(form->T2->allVars != nullptr);
    if(form->allVars == nullptr) {
        form->allVars = ident_union(form->T1->allVars, form->T2->allVars);
    }
}

void OccuringVariableDecorator::visit(ASTForm_tt *form) {
    assert(form->t1->allVars != nullptr);
    assert(form->t2->allVars != nullptr);
    if(form->allVars == nullptr) {
        form->allVars = ident_union(form->t1->allVars, form->t2->allVars);
    }
}

void OccuringVariableDecorator::visit(ASTForm_uvf *form) {
    assert(form->f->allVars != nullptr);
    if(form->allVars == nullptr) {
        form->allVars = copy(form->f->allVars);
    }
}

void OccuringVariableDecorator::visit(ASTForm_vf *form) {
    assert(form->f->allVars != nullptr);
    if(form->allVars == nullptr) {
        form->allVars = copy(form->f->allVars);
    }
}

void OccuringVariableDecorator::visit(ASTTerm *term) {
    if(term->allVars == nullptr) {
        this->_decorateUnaryNode<ASTTerm>(term);
    }
}

void OccuringVariableDecorator::visit(ASTUniv *univ) {
    if(univ->allVars == nullptr) {
        this->_decorateUnaryNode<ASTUniv>(univ);
    }
}

void OccuringVariableDecorator::visit(ASTForm_FirstOrder *form) {
    assert(form->t->allVars != nullptr);
    if(form->allVars == nullptr) {
        form->allVars = copy(form->t->allVars);
    }
}

void OccuringVariableDecorator::visit(ASTForm_Not *form) {
    assert(form->f->allVars != nullptr);
    if(form->allVars == nullptr) {
        form->allVars = copy(form->f->allVars);
    }
}

void OccuringVariableDecorator::visit(ASTForm_False *form) {
    if(form->allVars == nullptr) {
        form->allVars = new IdentList();
    }
}

void OccuringVariableDecorator::visit(ASTForm_True *form) {
    if(form->allVars == nullptr) {
        form->allVars = new IdentList();
    }
}