#include "ast_visitor.h"

/**
 * NOTE: Yes, this is exhausting, but this way we should achieve maximum efficiency
 *   without using dynamic_cast<> and complex templates to achieve the same.
 */

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

AST* ASTTerm1_Var1::accept(ASTTransformer &v) {
    return v.visit(this);
}

AST* ASTTerm1_Dot::accept(ASTTransformer &v) {
    return v.visit(this);
}

AST* ASTTerm1_Up::accept(ASTTransformer &v) {
    return v.visit(this);
}

AST* ASTTerm1_Root::accept(ASTTransformer &v) {
    return v.visit(this);
}

AST* ASTTerm1_Int::accept(ASTTransformer &v) {
    return v.visit(this);
}

AST* ASTTerm1_Plus::accept(ASTTransformer &v) {
    return v.visit(this);
}

AST* ASTTerm1_Minus::accept(ASTTransformer &v) {
    return v.visit(this);
}

AST* ASTTerm1_PlusModulo::accept(ASTTransformer &v) {
    return v.visit(this);
}

AST* ASTTerm1_MinusModulo::accept(ASTTransformer &v) {
    return v.visit(this);
}

AST* ASTTerm1_Min::accept(ASTTransformer &v) {
    return v.visit(this);
}

AST* ASTTerm1_Max::accept(ASTTransformer &v) {
    return v.visit(this);
}

AST* ASTTerm1_TreeRoot::accept(ASTTransformer &v) {
    return v.visit(this);
}

AST* ASTTerm2_Var2::accept(ASTTransformer &v) {
    return v.visit(this);
}

AST* ASTTerm2_VarTree::accept(ASTTransformer &v) {
    return v.visit(this);
}

AST* ASTTerm2_Dot::accept(ASTTransformer &v) {
    return v.visit(this);
}

AST* ASTTerm2_Up::accept(ASTTransformer &v) {
    return v.visit(this);
}

AST* ASTTerm2_Empty::accept(ASTTransformer &v) {
    return v.visit(this);
}

AST* ASTTerm2_Union::accept(ASTTransformer &v) {
    return v.visit(this);
}

AST* ASTTerm2_Inter::accept(ASTTransformer &v) {
    return v.visit(this);
}

AST* ASTTerm2_Setminus::accept(ASTTransformer &v) {
    return v.visit(this);
}

AST* ASTTerm2_Set::accept(ASTTransformer &v) {
    return v.visit(this);
}

AST* ASTTerm2_Plus::accept(ASTTransformer &v) {
    return v.visit(this);
}

AST* ASTTerm2_Minus::accept(ASTTransformer &v) {
    return v.visit(this);
}

AST* ASTTerm2_Interval::accept(ASTTransformer &v) {
    return v.visit(this);
}

AST* ASTTerm2_PresbConst::accept(ASTTransformer &v) {
    return v.visit(this);
}

AST* ASTTerm2_Formula::accept(ASTTransformer &v) {
    return v.visit(this);
}

AST* ASTForm_Var0::accept(ASTTransformer &v) {
    return v.visit(this);
}

AST* ASTForm_True::accept(ASTTransformer &v) {
    return v.visit(this);
}

AST* ASTForm_False::accept(ASTTransformer &v) {
    return v.visit(this);
}

AST* ASTForm_In::accept(ASTTransformer &v) {
    this->t1 = static_cast<ASTTerm1*>(this->t1->accept(v));
    this->T2 = static_cast<ASTTerm2*>(this->T2->accept(v));
    return v.visit(this);
}

AST* ASTForm_Notin::accept(ASTTransformer &v) {
    this->t1 = static_cast<ASTTerm1*>(this->t1->accept(v));
    this->T2 = static_cast<ASTTerm2*>(this->T2->accept(v));
    v.visit(this);
}

AST* ASTForm_RootPred::accept(ASTTransformer &v) {
    this->t = static_cast<ASTTerm1*>(this->t->accept(v));
    return v.visit(this);
}

AST* ASTForm_EmptyPred::accept(ASTTransformer &v) {
    this->T = static_cast<ASTTerm2*>(this->T->accept(v));
    return v.visit(this);
}

AST* ASTForm_FirstOrder::accept(ASTTransformer &v) {
    this->t = static_cast<ASTTerm1*>(this->t->accept(v));
    return v.visit(this);
}

AST* ASTForm_Sub::accept(ASTTransformer &v) {
    this->T1 = static_cast<ASTTerm2*>(this->T1->accept(v));
    this->T2 = static_cast<ASTTerm2*>(this->T2->accept(v));
    return v.visit(this);
}

AST* ASTForm_Equal1::accept(ASTTransformer &v) {
    this->t1 = static_cast<ASTTerm1*>(this->t1->accept(v));
    this->t2 = static_cast<ASTTerm1*>(this->t2->accept(v));
    return v.visit(this);
}

AST* ASTForm_Equal2::accept(ASTTransformer &v) {
    this->T1 = static_cast<ASTTerm2*>(this->T1->accept(v));
    this->T2 = static_cast<ASTTerm2*>(this->T2->accept(v));
    return v.visit(this);
}

AST* ASTForm_NotEqual1::accept(ASTTransformer &v) {
    this->t1 = static_cast<ASTTerm1*>(this->t1->accept(v));
    this->t2 = static_cast<ASTTerm1*>(this->t2->accept(v));
    return v.visit(this);
}

AST* ASTForm_NotEqual2::accept(ASTTransformer &v) {
    this->T1 = static_cast<ASTTerm2*>(this->T1->accept(v));
    this->T2 = static_cast<ASTTerm2*>(this->T2->accept(v));
    return v.visit(this);
}

AST* ASTForm_Less::accept(ASTTransformer &v) {
    this->t1 = static_cast<ASTTerm1*>(this->t1->accept(v));
    this->t2 = static_cast<ASTTerm1*>(this->t2->accept(v));
    return v.visit(this);
}

AST* ASTForm_LessEq::accept(ASTTransformer &v) {
    this->t1 = static_cast<ASTTerm1*>(this->t1->accept(v));
    this->t2 = static_cast<ASTTerm1*>(this->t2->accept(v));
    return v.visit(this);
}

AST* ASTForm_WellFormedTree::accept(ASTTransformer &v) {
    this->T = static_cast<ASTTerm2*>(this->T->accept(v));
    return v.visit(this);
}

AST* ASTForm_Impl::accept(ASTTransformer &v) {
    this->f1 = static_cast<ASTForm*>(this->f1->accept(v));
    this->f2 = static_cast<ASTForm*>(this->f2->accept(v));
    return v.visit(this);
}

AST* ASTForm_Biimpl::accept(ASTTransformer &v) {
    this->f1 = static_cast<ASTForm*>(this->f1->accept(v));
    this->f2 = static_cast<ASTForm*>(this->f2->accept(v));
    return v.visit(this);
}

AST* ASTForm_And::accept(ASTTransformer &v) {
    this->f1 = static_cast<ASTForm*>(this->f1->accept(v));
    this->f2 = static_cast<ASTForm*>(this->f2->accept(v));
    return v.visit(this);
}

AST* ASTForm_IdLeft::accept(ASTTransformer &v) {
    this->f1 = static_cast<ASTForm*>(this->f1->accept(v));
    this->f2 = static_cast<ASTForm*>(this->f2->accept(v));
    return v.visit(this);
}

AST* ASTForm_Or::accept(ASTTransformer &v) {
    this->f1 = static_cast<ASTForm*>(this->f1->accept(v));
    this->f2 = static_cast<ASTForm*>(this->f2->accept(v));
    return v.visit(this);
}

AST* ASTForm_Not::accept(ASTTransformer &v) {
    this->f = static_cast<ASTForm*>(this->f->accept(v));
    return v.visit(this);
}

AST* ASTForm_Ex0::accept(ASTTransformer &v) {
    this->f = static_cast<ASTForm*>(this->f->accept(v));
    return v.visit(this);
}

AST* ASTForm_Ex1::accept(ASTTransformer &v) {
    this->f = static_cast<ASTForm*>(this->f->accept(v));
    return v.visit(this);
}

AST* ASTForm_Ex2::accept(ASTTransformer &v) {
    this->f = static_cast<ASTForm*>(this->f->accept(v));
    return v.visit(this);
}

AST* ASTForm_All0::accept(ASTTransformer &v) {
    this->f = static_cast<ASTForm*>(this->f->accept(v));
    return v.visit(this);
}

AST* ASTForm_All1::accept(ASTTransformer &v) {
    this->f = static_cast<ASTForm*>(this->f->accept(v));
    return v.visit(this);
}

AST* ASTForm_All2::accept(ASTTransformer &v) {
    this->f = static_cast<ASTForm*>(this->f->accept(v));
    return v.visit(this);
}

AST* ASTForm_Let0::accept(ASTTransformer &v) {
    this->f = static_cast<ASTForm*>(this->f->accept(v));
    return v.visit(this);
}

AST* ASTForm_Let1::accept(ASTTransformer &v) {
    this->f = static_cast<ASTForm*>(this->f->accept(v));
    return v.visit(this);
}

AST* ASTForm_Let2::accept(ASTTransformer &v) {
    this->f = static_cast<ASTForm*>(this->f->accept(v));
    return v.visit(this);
}

AST* ASTForm_Call::accept(ASTTransformer &v) {
    return v.visit(this);
}

AST* ASTForm_Import::accept(ASTTransformer &v) {
    return v.visit(this);
}

AST* ASTForm_Export::accept(ASTTransformer &v) {
    this->f = static_cast<ASTForm*>(this->f->accept(v));
    return v.visit(this);
}

AST* ASTForm_Prefix::accept(ASTTransformer &v) {
    this->f = static_cast<ASTForm*>(this->f->accept(v));
    return v.visit(this);
}

AST* ASTForm_Restrict::accept(ASTTransformer &v) {
    this->f = static_cast<ASTForm*>(this->f->accept(v));
    return v.visit(this);
}

AST* ASTForm_InStateSpace1::accept(ASTTransformer &v) {
    this->t = static_cast<ASTTerm1*>(this->t->accept(v));
    return v.visit(this);
}

AST* ASTForm_InStateSpace2::accept(ASTTransformer &v) {
    this->T = static_cast<ASTTerm2*>(this->T->accept(v));
    return v.visit(this);
}

AST* ASTForm_SomeType::accept(ASTTransformer &v) {
    this->t = static_cast<ASTTerm1*>(this->t->accept(v));
    return v.visit(this);
}

/**
 * Traverses sons and visits the nodes before
 *
 * @param[in] v:    visitor without parameters returning void
 */

void ASTTerm::accept(VoidVisitor &v) { v.visit(this);}
void ASTForm::accept(VoidVisitor &v) { v.visit(this);}
void ASTUniv::accept(VoidVisitor &v) { v.visit(this);}

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
    this->t1->accept(v);
    v.visit(this);
    this->t2->accept(v);
}

// < ASTTerm2 Derives > //
void ASTTerm2_TT::accept(VoidVisitor &v) {
    this->T1->accept(v);
    v.visit(this);
    this->T2->accept(v);
}

void ASTTerm2_Tn::accept(VoidVisitor &v) {
    v.visit(this);

    this->T->accept(v);
}

// < ASTForm Derives > //
void ASTForm_tT::accept(VoidVisitor &v) {
    this->t1->accept(v);
    v.visit(this);
    this->T2->accept(v);
}

void ASTForm_T::accept(VoidVisitor &v) {
    v.visit(this);

    this->T->accept(v);
}

void ASTForm_TT::accept(VoidVisitor &v) {
    this->T1->accept(v);
    v.visit(this);
    this->T2->accept(v);
}

void ASTForm_tt::accept(VoidVisitor &v) {
    this->t1->accept(v);
    v.visit(this);
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
    this->f1->accept(v);
    v.visit(this);
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

void ASTTerm1_Var1::accept(VoidVisitor &v) {
    v.visit(this);
}

void ASTTerm1_Dot::accept(VoidVisitor &v) {
    v.visit(this);
}

void ASTTerm1_Up::accept(VoidVisitor &v) {
    v.visit(this);
}

void ASTTerm1_Root::accept(VoidVisitor &v) {
    v.visit(this);
}

void ASTTerm1_Int::accept(VoidVisitor &v) {
    v.visit(this);
}

void ASTTerm1_Plus::accept(VoidVisitor &v) {
    v.visit(this);
}

void ASTTerm1_Minus::accept(VoidVisitor &v) {
    v.visit(this);
}

void ASTTerm1_PlusModulo::accept(VoidVisitor &v) {
    v.visit(this);
}

void ASTTerm1_MinusModulo::accept(VoidVisitor &v) {
    v.visit(this);
}

void ASTTerm1_Min::accept(VoidVisitor &v) {
    v.visit(this);
}

void ASTTerm1_Max::accept(VoidVisitor &v) {
    v.visit(this);
}

void ASTTerm1_TreeRoot::accept(VoidVisitor &v) {
    v.visit(this);
}

void ASTTerm2_Var2::accept(VoidVisitor &v) {
    v.visit(this);
}

void ASTTerm2_VarTree::accept(VoidVisitor &v) {
    v.visit(this);
}

void ASTTerm2_Dot::accept(VoidVisitor &v) {
    v.visit(this);
}

void ASTTerm2_Up::accept(VoidVisitor &v) {
    v.visit(this);
}

void ASTTerm2_Empty::accept(VoidVisitor &v) {
    v.visit(this);
}

void ASTTerm2_Union::accept(VoidVisitor &v) {
    v.visit(this);
}

void ASTTerm2_Inter::accept(VoidVisitor &v) {
    v.visit(this);
}

void ASTTerm2_Setminus::accept(VoidVisitor &v) {
    v.visit(this);
}

void ASTTerm2_Set::accept(VoidVisitor &v) {
    v.visit(this);
}

void ASTTerm2_Plus::accept(VoidVisitor &v) {
    v.visit(this);
}

void ASTTerm2_Minus::accept(VoidVisitor &v) {
    v.visit(this);
}

void ASTTerm2_Interval::accept(VoidVisitor &v) {
    v.visit(this);
}

void ASTTerm2_PresbConst::accept(VoidVisitor &v) {
    v.visit(this);
}

void ASTTerm2_Formula::accept(VoidVisitor &v) {
    v.visit(this);
}

void ASTForm_Var0::accept(VoidVisitor &v) {
    v.visit(this);
}

void ASTForm_True::accept(VoidVisitor &v) {
    v.visit(this);
}

void ASTForm_False::accept(VoidVisitor &v) {
    v.visit(this);
}

void ASTForm_In::accept(VoidVisitor &v) {
    this->t1->accept(v);
    v.visit(this);
    this->T2->accept(v);
}

void ASTForm_Notin::accept(VoidVisitor &v) {
    this->t1->accept(v);
    v.visit(this);
    this->T2->accept(v);
}

void ASTForm_RootPred::accept(VoidVisitor &v) {
    this->t->accept(v);
    v.visit(this);
}

void ASTForm_EmptyPred::accept(VoidVisitor &v) {
    this->T->accept(v);
    v.visit(this);
}

void ASTForm_FirstOrder::accept(VoidVisitor &v) {
    this->t->accept(v);
    v.visit(this);
}

void ASTForm_Sub::accept(VoidVisitor &v) {
    this->T1->accept(v);
    v.visit(this);
    this->T2->accept(v);
}

void ASTForm_Equal1::accept(VoidVisitor &v) {
    this->t1->accept(v);
    v.visit(this);
    this->t2->accept(v);
}

void ASTForm_Equal2::accept(VoidVisitor &v) {
    this->T1->accept(v);
    v.visit(this);
    this->T2->accept(v);
}

void ASTForm_NotEqual1::accept(VoidVisitor &v) {
    this->t1->accept(v);
    v.visit(this);
    this->t2->accept(v);
}

void ASTForm_NotEqual2::accept(VoidVisitor &v) {
    this->T1->accept(v);
    v.visit(this);
    this->T2->accept(v);
}

void ASTForm_Less::accept(VoidVisitor &v) {
    this->t1->accept(v);
    v.visit(this);
    this->t2->accept(v);
}

void ASTForm_LessEq::accept(VoidVisitor &v) {
    this->t1->accept(v);
    v.visit(this);
    this->t2->accept(v);
}

void ASTForm_WellFormedTree::accept(VoidVisitor &v) {
    this->T->accept(v);
    v.visit(this);
}

void ASTForm_Impl::accept(VoidVisitor &v) {
    this->f1->accept(v);
    v.visit(this);
    this->f2->accept(v);
}

void ASTForm_Biimpl::accept(VoidVisitor &v) {
    this->f1->accept(v);
    v.visit(this);
    this->f2->accept(v);
}

void ASTForm_And::accept(VoidVisitor &v) {
    this->f1->accept(v);
    v.visit(this);
    this->f2->accept(v);
}

void ASTForm_IdLeft::accept(VoidVisitor &v) {
    this->f1->accept(v);
    v.visit(this);
    this->f2->accept(v);
}

void ASTForm_Or::accept(VoidVisitor &v) {
    this->f1->accept(v);
    v.visit(this);
    this->f2->accept(v);
}

void ASTForm_Not::accept(VoidVisitor &v) {
    v.visit(this);
    this->f->accept(v);
}

void ASTForm_Ex0::accept(VoidVisitor &v) {
    v.visit(this);
    this->f->accept(v);
}

void ASTForm_Ex1::accept(VoidVisitor &v) {
    v.visit(this);
    this->f->accept(v);
}

void ASTForm_Ex2::accept(VoidVisitor &v) {
    v.visit(this);
    this->f->accept(v);
}

void ASTForm_All0::accept(VoidVisitor &v) {
    v.visit(this);
    this->f->accept(v);
}

void ASTForm_All1::accept(VoidVisitor &v) {
    v.visit(this);
    this->f->accept(v);
}

void ASTForm_All2::accept(VoidVisitor &v) {
    v.visit(this);
    this->f->accept(v);
}

void ASTForm_Let0::accept(VoidVisitor &v) {
    v.visit(this);
    this->f->accept(v);
}

void ASTForm_Let1::accept(VoidVisitor &v) {
    v.visit(this);
    this->f->accept(v);
}

void ASTForm_Let2::accept(VoidVisitor &v) {
    v.visit(this);
    this->f->accept(v);
}

void ASTForm_Call::accept(VoidVisitor &v) {
    v.visit(this);
}

void ASTForm_Import::accept(VoidVisitor &v) {
    v.visit(this);
}

void ASTForm_Export::accept(VoidVisitor &v) {
    v.visit(this);
    this->f->accept(v);
}

void ASTForm_Prefix::accept(VoidVisitor &v) {
    v.visit(this);
    this->f->accept(v);
}

void ASTForm_Restrict::accept(VoidVisitor &v) {
    v.visit(this);
    this->f->accept(v);
}

void ASTForm_InStateSpace1::accept(VoidVisitor &v) {
    v.visit(this);
    this->t->accept(v);
}

void ASTForm_InStateSpace2::accept(VoidVisitor &v) {
    v.visit(this);
    this->T->accept(v);
}

void ASTForm_SomeType::accept(VoidVisitor &v) {
    v.visit(this);
    this->t->accept(v);
}