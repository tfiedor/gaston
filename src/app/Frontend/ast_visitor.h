#ifndef WSKS_AST_VISITOR_H
#define WSKS_AST_VISITOR_H

#include "ast.h"

template <typename R = void>
class ASTVisitor {
public:
    typedef R ReturnType;

    virtual ReturnType visit(ASTForm *e) {};
    virtual ReturnType visit(ASTTerm *e) {};
    virtual ReturnType visit(ASTUniv *e) {};
};

/**
 * An implementation of visitor pattern for traversing and modifing
 * of the structure of AST. Implements traversals for basic node
 */
class ASTTransformer : public ASTVisitor<AST*> {
public:
    // < ASTTerm1 Derives > //
    virtual AST* visit(ASTTerm1_n* term) { return term; }
    virtual AST* visit(ASTTerm1_T* term) { return term; }
    virtual AST* visit(ASTTerm1_t* term) { return term; }
    virtual AST* visit(ASTTerm1_tn* term) { return term; }
    virtual AST* visit(ASTTerm1_tnt* term) { return term; }

    // < ASTTerm2 Derives > //
    virtual AST* visit(ASTTerm2_TT* term) { return term; }
    virtual AST* visit(ASTTerm2_Tn* term) { return term; }

    // < ASTForm Derives > //
    virtual AST* visit(ASTForm_tT* form) { return form; }
    virtual AST* visit(ASTForm_T* form) { return form; }
    virtual AST* visit(ASTForm_TT* form) { return form; }
    virtual AST* visit(ASTForm_tt* form) { return form; }
    virtual AST* visit(ASTForm_nt* form) { return form; }
    virtual AST* visit(ASTForm_nT* form) { return form; }
    virtual AST* visit(ASTForm_f* form) { return form; }
    virtual AST* visit(ASTForm_ff* form) { return form; }
    virtual AST* visit(ASTForm_vf* form) { return form; }
    virtual AST* visit(ASTForm_uvf* form) { return form; }
};

/**
 * Traverses sons, modifies them and then return this node
 *
 * @param[in] v:    visitor returning AST without parameters
 * @return:         modified AST
 */
AST* ASTTerm::accept(ASTTransformer &v) {
    return this;
}

AST* ASTForm::accept(ASTTransformer &v) {
    return this;
}

AST* ASTUniv::accept(ASTTransformer &v) {
    return this;
}

// < ASTTerm1 Accepts > //
AST* ASTTerm1_n::accept(ASTTransformer &v) {
    return v.visit(this);
}

AST* ASTTerm1_T::accept(ASTTransformer &v) {
    this->T = static_cast<ASTTerm2*>(this->T->accept(v));

    return v.visit(this);
}

AST* ASTTerm1_t::accept(ASTTransformer &v) {
    this->t = static_cast<ASTTerm1*>(this->t->accept(v));

    return v.visit(this);
}

AST* ASTTerm1_tn::accept(ASTTransformer &v) {
    this->t = static_cast<ASTTerm1*>(this->t->accept(v));

    return v.visit(this);
}

AST* ASTTerm1_tnt::accept(ASTTransformer &v) {
    this->t1 = static_cast<ASTTerm1*>(this->t1->accept(v));
    this->t2 = static_cast<ASTTerm1*>(this->t2->accept(v));

    return v.visit(this);
}

// < ASTTerm2 Accepts > //
AST* ASTTerm2_TT::accept(ASTTransformer &v) {
    this->T1 = static_cast<ASTTerm2*>(this->T1->accept(v));
    this->T2 = static_cast<ASTTerm2*>(this->T2->accept(v));

    return v.visit(this);
}

AST* ASTTerm2_Tn::accept(ASTTransformer &v) {
    this->T = static_cast<ASTTerm2*>(this->T->accept(v));

    return v.visit(this);
}

// < ASTForm Accepts > //
AST* ASTForm_tT::accept(ASTTransformer &v) {
    this->t1 = static_cast<ASTTerm1*>(this->t1->accept(v));
    this->T2 = static_cast<ASTTerm2*>(this->T2->accept(v));

    return v.visit(this);
}

AST* ASTForm_T::accept(ASTTransformer &v) {
    this->T = static_cast<ASTTerm2*>(this->T->accept(v));

    return v.visit(this);
}

AST* ASTForm_TT::accept(ASTTransformer &v){
    this->T1 = static_cast<ASTTerm2*>(this->T1->accept(v));
    this->T2 = static_cast<ASTTerm2*>(this->T2->accept(v));

    return v.visit(this);
}

AST* ASTForm_tt::accept(ASTTransformer &v) {
    this->t1 = static_cast<ASTTerm1*>(this->t1->accept(v));
    this->t2 = static_cast<ASTTerm1*>(this->t2->accept(v));

    return v.visit(this);
}

AST* ASTForm_nt::accept(ASTTransformer &v) {
    this->t = static_cast<ASTTerm1*>(this->t->accept(v));

    return v.visit(this);
}

AST* ASTForm_nT::accept(ASTTransformer &v) {
    this->T = static_cast<ASTTerm2*>(this->T->accept(v));

    return v.visit(this);
}

AST* ASTForm_f::accept(ASTTransformer &v) {
    this->f = static_cast<ASTForm*>(this->f->accept(v));

    return v.visit(this);
}

AST* ASTForm_ff::accept(ASTTransformer &v) {
    this->f1 = static_cast<ASTForm*>(this->f1->accept(v));
    this->f2 = static_cast<ASTForm*>(this->f2->accept(v));

    return v.visit(this);
}

AST* ASTForm_vf::accept(ASTTransformer &v) {
    this->f = static_cast<ASTForm*>(this->f->accept(v));

    return v.visit(this);
}

AST* ASTForm_uvf::accept(ASTTransformer &v) {
    this->f = static_cast<ASTForm*>(this->f->accept(v));

    return v.visit(this);
}

/**
 * An implementation of visitor pattern for traversing without
 * the modification of the structure of AST. Implements traversals
 * for basic types of the node
 */
class VoidVisitor : public ASTVisitor<> {
public:
    // < ASTTerm1 Derives > //
    virtual void visit(ASTTerm1_n* term) {}
    virtual void visit(ASTTerm1_T* term) {}
    virtual void visit(ASTTerm1_t* term) {}
    virtual void visit(ASTTerm1_tn* term) {}
    virtual void visit(ASTTerm1_tnt* term) {}

    // < ASTTerm2 Derives > //
    virtual void visit(ASTTerm2_TT* term) {}
    virtual void visit(ASTTerm2_Tn* term) {}

    // < ASTForm Derives > //
    virtual void visit(ASTForm_tT* form) {}
    virtual void visit(ASTForm_T* form) {}
    virtual void visit(ASTForm_TT* form) {}
    virtual void visit(ASTForm_tt* form) {}
    virtual void visit(ASTForm_nt* form) {}
    virtual void visit(ASTForm_nT* form) {}
    virtual void visit(ASTForm_f* form) {}
    virtual void visit(ASTForm_ff* form) {}
    virtual void visit(ASTForm_vf* form) {}
    virtual void visit(ASTForm_uvf* form) {}
};

/**
 * Traverses sons and visits the nodes before
 *
 * @param[in] v:    visitor without parameters returning void
 */

void ASTTerm::accept(VoidVisitor &v) {}
void ASTForm::accept(VoidVisitor &v) {}
void ASTUniv::accept(VoidVisitor &v) {}

// < ASTTerm1 Derives > //
void ASTTerm1_n::accept(VoidVisitor &v) {
    v.visit(this);
}

void ASTTerm1_T::accept(VoidVisitor &v) {
    v.visit(this);

    this->T->accept(v);
}

void ASTTerm1_t::accept(VoidVisitor &v) {
    v.visit(this);

    this->t->accept(v);
}

void ASTTerm1_tn::accept(VoidVisitor &v) {
    v.visit(this);

    this->t->accept(v);
}

void ASTTerm1_tnt::accept(VoidVisitor &v) {
    v.visit(this);

    this->t1->accept(v);
    this->t2->accept(v);
}

// < ASTTerm2 Derives > //
void ASTTerm2_TT::accept(VoidVisitor &v) {
    v.visit(this);

    this->T1->accept(v);
    this->T2->accept(v);
}

void ASTTerm2_Tn::accept(VoidVisitor &v) {
    v.visit(this);

    this->T->accept(v);
}

// < ASTForm Derives > //
void ASTForm_tT::accept(VoidVisitor &v) {
    v.visit(this);

    this->t1->accept(v);
    this->T2->accept(v);
}

void ASTForm_T::accept(VoidVisitor &v) {
    v.visit(this);

    this->T->accept(v);
}

void ASTForm_TT::accept(VoidVisitor &v) {
    v.visit(this);

    this->T1->accept(v);
    this->T2->accept(v);
}

void ASTForm_tt::accept(VoidVisitor &v) {
    v.visit(this);

    this->t1->accept(v);
    this->t2->accept(v);
}

void ASTForm_nt::accept(VoidVisitor &v) {
    v.visit(this);

    this->t->accept(v);
}

void ASTForm_nT::accept(VoidVisitor &v) {
    v.visit(this);

    this->T->accept(v);
}

void ASTForm_f::accept(VoidVisitor &v) {
    v.visit(this);

    this->f->accept(v);
}

void ASTForm_ff::accept(VoidVisitor &v) {
    v.visit(this);

    this->f1->accept(v);
    this->f2->accept(v);
}

void ASTForm_vf::accept(VoidVisitor &v) {
    v.visit(this);

    this->f->accept(v);
}

void ASTForm_uvf::accept(VoidVisitor &v) {
    v.visit(this);

    this->f->accept(v);
}

#endif //WSKS_AST_VISITOR_H