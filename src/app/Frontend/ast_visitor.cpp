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

template<class ASTNode>
AST* traverse_T(ASTNode* node, ASTTransformer &v) {
    ASTNode* temp = nullptr;

    switch(v.traverseDirection) {
        case ASTTransformer::Traverse::PreOrder:
            temp = static_cast<ASTNode*>(v.visit(node));
            temp->T = static_cast<ASTTerm2*>(temp->T->accept(v));
            return temp;
        case ASTTransformer::Traverse::PostOrder:
            node->T = static_cast<ASTTerm2*>(node->T->accept(v));
            return v.visit(node);
        default:
            assert(false && "Traversing AST_T not implemented yet");
    }
}

template<class ASTNode>
AST* traverse_t(ASTNode* node, ASTTransformer &v) {
    ASTNode* temp = nullptr;

    switch(v.traverseDirection) {
        case ASTTransformer::Traverse::PreOrder:
            temp = static_cast<ASTNode*>(v.visit(node));
            temp->t = static_cast<ASTTerm1*>(temp->t->accept(v));
            return temp;
        case ASTTransformer::Traverse::PostOrder:
            node->t = static_cast<ASTTerm1*>(node->t->accept(v));
            return v.visit(node);
        default:
            assert(false && "Traversing AST_t not implemented yet");
    }
}

template<class ASTNode>
AST* traverse_tt(ASTNode* node, ASTTransformer &v) {
    ASTNode* temp = nullptr;

    switch(v.traverseDirection) {
        case ASTTransformer::Traverse::PreOrder:
            temp = static_cast<ASTNode*>(v.visit(node));
            temp->t1 = static_cast<ASTTerm1*>(temp->t1->accept(v));
            temp->t2 = static_cast<ASTTerm1*>(temp->t2->accept(v));
            return temp;
        case ASTTransformer::Traverse::PostOrder:
            node->t1 = static_cast<ASTTerm1*>(node->t1->accept(v));
            node->t2 = static_cast<ASTTerm1*>(node->t2->accept(v));
            return v.visit(node);
        default:
            assert(false && "Traversing AST_tt not implemented yet");
    }
}

template<class ASTNode>
AST* traverse_TT(ASTNode* node, ASTTransformer &v) {
    ASTNode* temp = nullptr;

    switch(v.traverseDirection) {
        case ASTTransformer::Traverse::PreOrder:
            temp = static_cast<ASTNode*>(v.visit(node));
            temp->T1 = static_cast<ASTTerm2*>(temp->T1->accept(v));
            temp->T2 = static_cast<ASTTerm2*>(temp->T2->accept(v));
            return temp;
        case ASTTransformer::Traverse::PostOrder:
            node->T1 = static_cast<ASTTerm2*>(node->T1->accept(v));
            node->T2 = static_cast<ASTTerm2*>(node->T2->accept(v));
            return v.visit(node);
        default:
            assert(false && "Traversing AST_TT not implemented yet");
    }
}

template<class ASTNode>
AST* traverse_tT(ASTNode* node, ASTTransformer &v) {
    ASTNode* temp = nullptr;

    switch(v.traverseDirection) {
        case ASTTransformer::Traverse::PreOrder:
            temp = static_cast<ASTNode*>(v.visit(node));
            temp->t1 = static_cast<ASTTerm1*>(temp->t1->accept(v));
            temp->T2 = static_cast<ASTTerm2*>(temp->T2->accept(v));
            return temp;
        case ASTTransformer::Traverse::PostOrder:
            node->t1 = static_cast<ASTTerm1*>(node->t1->accept(v));
            node->T2 = static_cast<ASTTerm2*>(node->T2->accept(v));
            return v.visit(node);
        default:
            assert(false && "Traversing AST_tT not implemented yet");
    }
}

template<class ASTNode>
AST* traverse_f(ASTNode* node, ASTTransformer &v) {
    ASTNode* temp = nullptr;

    switch(v.traverseDirection) {
        case ASTTransformer::Traverse::PreOrder:
            temp = static_cast<ASTNode*>(v.visit(node));
            temp->f = static_cast<ASTForm*>(temp->f->accept(v));
            return temp;
        case ASTTransformer::Traverse::PostOrder:
            node->f = static_cast<ASTForm*>(node->f->accept(v));
            return v.visit(node);
        default:
            assert(false && "Traversing AST_t not implemented yet");
    }
}

template<class ASTNode>
AST* traverse_ff(ASTNode* node, ASTTransformer &v) {
    ASTNode* temp = nullptr;

    switch(v.traverseDirection) {
        case ASTTransformer::Traverse::PreOrder:
            temp = static_cast<ASTNode*>(v.visit(node));
            temp->f1 = static_cast<ASTForm*>(temp->f1->accept(v));
            temp->f2 = static_cast<ASTForm*>(temp->f2->accept(v));
            return temp;
        case ASTTransformer::Traverse::PostOrder:
            node->f1 = static_cast<ASTForm*>(node->f1->accept(v));
            node->f2 = static_cast<ASTForm*>(node->f2->accept(v));
            return v.visit(node);
        default:
            assert(false && "Traversing AST_tT not implemented yet");
    }
}

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
    return traverse_T<ASTTerm1_T>(this, v);
}

AST* ASTTerm1_t::accept(ASTTransformer &v) {
    return traverse_t<ASTTerm1_t>(this, v);
}

AST* ASTTerm1_tn::accept(ASTTransformer &v) {
    return traverse_t<ASTTerm1_tn>(this, v);
}

AST* ASTTerm1_tnt::accept(ASTTransformer &v) {
    return traverse_tt<ASTTerm1_tnt>(this, v);
}

// < ASTTerm2 Accepts > //
AST* ASTTerm2_TT::accept(ASTTransformer &v) {
    return traverse_TT<ASTTerm2_TT>(this, v);
}

AST* ASTTerm2_Tn::accept(ASTTransformer &v) {
    return traverse_T<ASTTerm2_Tn>(this, v);
}

// < ASTForm Accepts > //
AST* ASTForm_tT::accept(ASTTransformer &v) {
    return traverse_tT<ASTForm_tT>(this, v);
}

AST* ASTForm_T::accept(ASTTransformer &v) {
    return traverse_T<ASTForm_T>(this, v);
}

AST* ASTForm_TT::accept(ASTTransformer &v){
    return traverse_TT<ASTForm_TT>(this, v);
}

AST* ASTForm_tt::accept(ASTTransformer &v) {
    return traverse_tt<ASTForm_tt>(this, v);
}

AST* ASTForm_nt::accept(ASTTransformer &v) {
    return traverse_t<ASTForm_nt>(this, v);
}

AST* ASTForm_nT::accept(ASTTransformer &v) {
    return traverse_T<ASTForm_nT>(this, v);
}

AST* ASTForm_f::accept(ASTTransformer &v) {
    return traverse_f<ASTForm_f>(this, v);
}

AST* ASTForm_ff::accept(ASTTransformer &v) {
    return traverse_ff<ASTForm_ff>(this, v);
}

AST* ASTForm_vf::accept(ASTTransformer &v) {
    return traverse_f<ASTForm_vf>(this, v);
}

AST* ASTForm_uvf::accept(ASTTransformer &v) {
    return traverse_f<ASTForm_uvf>(this, v);
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
    return traverse_tT<ASTForm_In>(this, v);
}

AST* ASTForm_Notin::accept(ASTTransformer &v) {
    return traverse_tT<ASTForm_Notin>(this, v);
}

AST* ASTForm_RootPred::accept(ASTTransformer &v) {
    return traverse_t<ASTForm_RootPred>(this, v);
}

AST* ASTForm_EmptyPred::accept(ASTTransformer &v) {
    return traverse_T<ASTForm_EmptyPred>(this, v);
}

AST* ASTForm_FirstOrder::accept(ASTTransformer &v) {
    return traverse_t<ASTForm_FirstOrder>(this, v);
}

AST* ASTForm_Sub::accept(ASTTransformer &v) {
    return traverse_TT<ASTForm_Sub>(this, v);
}

AST* ASTForm_Equal1::accept(ASTTransformer &v) {
    return traverse_tt<ASTForm_Equal1>(this, v);
}

AST* ASTForm_Equal2::accept(ASTTransformer &v) {
    return traverse_TT<ASTForm_Equal2>(this, v);
}

AST* ASTForm_NotEqual1::accept(ASTTransformer &v) {
    return traverse_tt<ASTForm_NotEqual1>(this, v);
}

AST* ASTForm_NotEqual2::accept(ASTTransformer &v) {
    return traverse_TT<ASTForm_NotEqual2>(this, v);
}

AST* ASTForm_Less::accept(ASTTransformer &v) {
    return traverse_tt<ASTForm_Less>(this, v);
}

AST* ASTForm_LessEq::accept(ASTTransformer &v) {
    return traverse_tt<ASTForm_LessEq>(this, v);
}

AST* ASTForm_WellFormedTree::accept(ASTTransformer &v) {
    return traverse_T<ASTForm_WellFormedTree>(this, v);
}

AST* ASTForm_Impl::accept(ASTTransformer &v) {
    return traverse_ff<ASTForm_Impl>(this, v);
}

AST* ASTForm_Biimpl::accept(ASTTransformer &v) {
    return traverse_ff<ASTForm_Biimpl>(this, v);
}

AST* ASTForm_And::accept(ASTTransformer &v) {
    return traverse_ff<ASTForm_And>(this, v);
}

AST* ASTForm_IdLeft::accept(ASTTransformer &v) {
    return traverse_ff<ASTForm_IdLeft>(this, v);
}

AST* ASTForm_Or::accept(ASTTransformer &v) {
    return traverse_ff<ASTForm_Or>(this, v);
}

AST* ASTForm_Not::accept(ASTTransformer &v) {
    return traverse_f<ASTForm_Not>(this, v);
}

AST* ASTForm_Ex0::accept(ASTTransformer &v) {
    return traverse_f<ASTForm_Ex0>(this, v);
}

AST* ASTForm_Ex1::accept(ASTTransformer &v) {
    return traverse_f<ASTForm_Ex1>(this, v);
}

AST* ASTForm_Ex2::accept(ASTTransformer &v) {
    return traverse_f<ASTForm_Ex2>(this, v);
}

AST* ASTForm_All0::accept(ASTTransformer &v) {
    return traverse_f<ASTForm_All0>(this, v);
}

AST* ASTForm_All1::accept(ASTTransformer &v) {
    return traverse_f<ASTForm_All1>(this, v);
}

AST* ASTForm_All2::accept(ASTTransformer &v) {
    return traverse_f<ASTForm_All2>(this, v);
}

AST* ASTForm_Let0::accept(ASTTransformer &v) {
    return traverse_f<ASTForm_Let0>(this, v);
}

AST* ASTForm_Let1::accept(ASTTransformer &v) {
    return traverse_f<ASTForm_Let1>(this, v);
}

AST* ASTForm_Let2::accept(ASTTransformer &v) {
    return traverse_f<ASTForm_Let2>(this, v);
}

AST* ASTForm_Call::accept(ASTTransformer &v) {
    return v.visit(this);
}

AST* ASTForm_Import::accept(ASTTransformer &v) {
    return v.visit(this);
}

AST* ASTForm_Export::accept(ASTTransformer &v) {
    return traverse_f<ASTForm_Export>(this, v);
}

AST* ASTForm_Prefix::accept(ASTTransformer &v) {
    return traverse_f<ASTForm_Prefix>(this, v);
}

AST* ASTForm_Restrict::accept(ASTTransformer &v) {
    return traverse_f<ASTForm_Restrict>(this, v);
}

AST* ASTForm_InStateSpace1::accept(ASTTransformer &v) {
    return traverse_t<ASTForm_InStateSpace1>(this, v);
}

AST* ASTForm_InStateSpace2::accept(ASTTransformer &v) {
    return traverse_T<ASTForm_InStateSpace2>(this, v);
}

AST* ASTForm_SomeType::accept(ASTTransformer &v) {
    return traverse_t<ASTForm_SomeType>(this, v);
}

/**
 * Traverses sons and visits the nodes before
 *
 * @param[in] v:    visitor without parameters returning void
 */

template<class ASTNode>
void void_traverse_T(ASTNode* node, VoidVisitor &v) {
    switch(v.traverseDirection) {
        case ASTTransformer::Traverse::PreOrder:
            v.visit(node);
            node->T->accept(v);
        case ASTTransformer::Traverse::PostOrder:
            node->T->accept(v);
            v.visit(node);
        default:
            assert(false && "Traversing AST_T not implemented yet");
    }
}

template<class ASTNode>
void void_traverse_t(ASTNode* node, VoidVisitor &v) {
    switch(v.traverseDirection) {
        case ASTTransformer::Traverse::PreOrder:
            v.visit(node);
            node->t->accept(v);
        case ASTTransformer::Traverse::PostOrder:
            node->t->accept(v);
            v.visit(node);
        default:
            assert(false && "Traversing AST_t not implemented yet");
    }
}

template<class ASTNode>
void void_traverse_tt(ASTNode* node, VoidVisitor &v) {
    switch(v.traverseDirection) {
        case ASTTransformer::Traverse::PreOrder:
            v.visit(node);
            node->t1->accept(v);
            node->t2->accept(v);
        case ASTTransformer::Traverse::PostOrder:
            node->t1->accept(v);
            node->t2->accept(v);
            v.visit(node);
        default:
            assert(false && "Traversing AST_tt not implemented yet");
    }
}

template<class ASTNode>
void void_traverse_TT(ASTNode* node, VoidVisitor &v) {
    switch(v.traverseDirection) {
        case ASTTransformer::Traverse::PreOrder:
            v.visit(node);
            node->T1->accept(v);
            node->T2->accept(v);
        case ASTTransformer::Traverse::PostOrder:
            node->T1->accept(v);
            node->T2->accept(v);
            v.visit(node);
        default:
            assert(false && "Traversing AST_TT not implemented yet");
    }
}

template<class ASTNode>
void void_traverse_tT(ASTNode* node, VoidVisitor &v) {
    switch(v.traverseDirection) {
        case ASTTransformer::Traverse::PreOrder:
            v.visit(node);
            node->t1->accept(v);
            node->T2->accept(v);
        case ASTTransformer::Traverse::PostOrder:
            node->t1->accept(v);
            node->T2->accept(v);
            v.visit(node);
        default:
            assert(false && "Traversing AST_tT not implemented yet");
    }
}

template<class ASTNode>
void void_traverse_f(ASTNode* node, VoidVisitor &v) {
    switch(v.traverseDirection) {
        case ASTTransformer::Traverse::PreOrder:
            v.visit(node);
            node->f->accept(v);
        case ASTTransformer::Traverse::PostOrder:
            node->f->accept(v);
            v.visit(node);
        default:
            assert(false && "Traversing AST_t not implemented yet");
    }
}

template<class ASTNode>
void void_traverse_ff(ASTNode* node, VoidVisitor &v) {
    switch(v.traverseDirection) {
        case ASTTransformer::Traverse::PreOrder:
            v.visit(node);
            node->f1->accept(v);
            node->f2->accept(v);
        case ASTTransformer::Traverse::PostOrder:
            node->f1->accept(v);
            node->f2->accept(v);
            v.visit(node);
        default:
            assert(false && "Traversing AST_tT not implemented yet");
    }
}


void ASTTerm::accept(VoidVisitor &v) { v.visit(this);}
void ASTForm::accept(VoidVisitor &v) { v.visit(this);}
void ASTUniv::accept(VoidVisitor &v) { v.visit(this);}

// < ASTTerm1 Derives > //
void ASTTerm1_n::accept(VoidVisitor &v) {
    v.visit(this);
}

void ASTTerm1_T::accept(VoidVisitor &v) {
    void_traverse_T<ASTTerm1_T>(this, v);
}

void ASTTerm1_t::accept(VoidVisitor &v) {
    void_traverse_t<ASTTerm1_t>(this, v);
}

void ASTTerm1_tn::accept(VoidVisitor &v) {
    void_traverse_t<ASTTerm1_tn>(this, v);
}

void ASTTerm1_tnt::accept(VoidVisitor &v) {
    void_traverse_tt<ASTTerm1_tnt>(this, v);
}

// < ASTTerm2 Derives > //
void ASTTerm2_TT::accept(VoidVisitor &v) {
    void_traverse_TT<ASTTerm2_TT>(this, v);
}

void ASTTerm2_Tn::accept(VoidVisitor &v) {
    void_traverse_T<ASTTerm2_Tn>(this, v);
}

// < ASTForm Derives > //
void ASTForm_tT::accept(VoidVisitor &v) {
    void_traverse_tT<ASTForm_tT>(this, v);
}

void ASTForm_T::accept(VoidVisitor &v) {
    void_traverse_T<ASTForm_T>(this, v);
}

void ASTForm_TT::accept(VoidVisitor &v) {
    void_traverse_TT<ASTForm_TT>(this, v);
}

void ASTForm_tt::accept(VoidVisitor &v) {
    void_traverse_tt<ASTForm_tt>(this, v);
}

void ASTForm_nt::accept(VoidVisitor &v) {
    void_traverse_t<ASTForm_nt>(this, v);
}

void ASTForm_nT::accept(VoidVisitor &v) {
    void_traverse_T<ASTForm_nT>(this, v);
}

void ASTForm_f::accept(VoidVisitor &v) {
    void_traverse_f<ASTForm_f>(this, v);
}

void ASTForm_ff::accept(VoidVisitor &v) {
    void_traverse_ff<ASTForm_ff>(this, v);
}

void ASTForm_vf::accept(VoidVisitor &v) {
    void_traverse_f<ASTForm_vf>(this, v);
}

void ASTForm_uvf::accept(VoidVisitor &v) {
    void_traverse_f<ASTForm_uvf>(this, v);
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
    void_traverse_tT<ASTForm_In>(this, v);
}

void ASTForm_Notin::accept(VoidVisitor &v) {
    void_traverse_tT<ASTForm_Notin>(this, v);
}

void ASTForm_RootPred::accept(VoidVisitor &v) {
    void_traverse_t<ASTForm_RootPred>(this, v);
}

void ASTForm_EmptyPred::accept(VoidVisitor &v) {
    void_traverse_T<ASTForm_EmptyPred>(this, v);
}

void ASTForm_FirstOrder::accept(VoidVisitor &v) {
    void_traverse_t<ASTForm_FirstOrder>(this, v);
}

void ASTForm_Sub::accept(VoidVisitor &v) {
    void_traverse_TT<ASTForm_Sub>(this, v);
}

void ASTForm_Equal1::accept(VoidVisitor &v) {
    void_traverse_tt<ASTForm_Equal1>(this, v);
}

void ASTForm_Equal2::accept(VoidVisitor &v) {
    void_traverse_TT<ASTForm_Equal2>(this, v);
}

void ASTForm_NotEqual1::accept(VoidVisitor &v) {
    void_traverse_tt<ASTForm_NotEqual1>(this, v);
}

void ASTForm_NotEqual2::accept(VoidVisitor &v) {
    void_traverse_TT<ASTForm_NotEqual2>(this, v);
}

void ASTForm_Less::accept(VoidVisitor &v) {
    void_traverse_tt<ASTForm_Less>(this, v);
}

void ASTForm_LessEq::accept(VoidVisitor &v) {
    void_traverse_tt<ASTForm_LessEq>(this, v);
}

void ASTForm_WellFormedTree::accept(VoidVisitor &v) {
    void_traverse_T<ASTForm_WellFormedTree>(this, v);
}

void ASTForm_Impl::accept(VoidVisitor &v) {
    void_traverse_ff<ASTForm_Impl>(this, v);
}

void ASTForm_Biimpl::accept(VoidVisitor &v) {
    void_traverse_ff<ASTForm_Biimpl>(this, v);
}

void ASTForm_And::accept(VoidVisitor &v) {
    void_traverse_ff<ASTForm_And>(this, v);
}

void ASTForm_IdLeft::accept(VoidVisitor &v) {
    void_traverse_ff<ASTForm_IdLeft>(this, v);
}

void ASTForm_Or::accept(VoidVisitor &v) {
    void_traverse_ff<ASTForm_Or>(this, v);
}

void ASTForm_Not::accept(VoidVisitor &v) {
    void_traverse_f<ASTForm_Not>(this, v);
}

void ASTForm_Ex0::accept(VoidVisitor &v) {
    void_traverse_f<ASTForm_Ex0>(this, v);
}

void ASTForm_Ex1::accept(VoidVisitor &v) {
    void_traverse_f<ASTForm_Ex1>(this, v);
}

void ASTForm_Ex2::accept(VoidVisitor &v) {
    void_traverse_f<ASTForm_Ex2>(this, v);
}

void ASTForm_All0::accept(VoidVisitor &v) {
    void_traverse_f<ASTForm_All0>(this, v);
}

void ASTForm_All1::accept(VoidVisitor &v) {
    void_traverse_f<ASTForm_All1>(this, v);
}

void ASTForm_All2::accept(VoidVisitor &v) {
    void_traverse_f<ASTForm_All2>(this, v);
}

void ASTForm_Let0::accept(VoidVisitor &v) {
    void_traverse_f<ASTForm_Let0>(this, v);;
}

void ASTForm_Let1::accept(VoidVisitor &v) {
    void_traverse_f<ASTForm_Let1>(this, v);
}

void ASTForm_Let2::accept(VoidVisitor &v) {
    void_traverse_f<ASTForm_Let2>(this, v);
}

void ASTForm_Call::accept(VoidVisitor &v) {
    v.visit(this);
}

void ASTForm_Import::accept(VoidVisitor &v) {
    v.visit(this);
}

void ASTForm_Export::accept(VoidVisitor &v) {
    void_traverse_f<ASTForm_Export>(this, v);
}

void ASTForm_Prefix::accept(VoidVisitor &v) {
    void_traverse_f<ASTForm_Prefix>(this, v);
}

void ASTForm_Restrict::accept(VoidVisitor &v) {
    void_traverse_f<ASTForm_Restrict>(this, v);
}

void ASTForm_InStateSpace1::accept(VoidVisitor &v) {
    void_traverse_t<ASTForm_InStateSpace1>(this, v);
}

void ASTForm_InStateSpace2::accept(VoidVisitor &v) {
    void_traverse_T<ASTForm_InStateSpace2>(this, v);
}

void ASTForm_SomeType::accept(VoidVisitor &v) {
    void_traverse_t<ASTForm_SomeType>(this, v);
}