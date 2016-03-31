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

template<class RetNode, class Node>
RetNode traverse_T(Node* node, TransformerVisitor &v) {
    assert(node != nullptr);
    assert(node->T != nullptr);

    Node* temp = nullptr;

    switch(v.traverseDirection) {
        // Here InOrder and PreOrder are actually the same
        case TransformerVisitor::Traverse::InOrder:
        case TransformerVisitor::Traverse::PreOrder:
            // First visit the root then the right child
            temp = static_cast<Node*>(v.visit(node));
            temp->T = static_cast<ASTTerm2*>(temp->T->accept(v));
            return temp;
        case TransformerVisitor::Traverse::PostOrder:
            // First visit the right child, then the root
            node->T = static_cast<ASTTerm2*>(node->T->accept(v));
            return v.visit(node);
        default:
            assert(false && "Traversing AST_T not implemented yet");
    }
}

template<class RetNode, class Node>
RetNode traverse_t(Node* node, TransformerVisitor &v) {
    assert(node != nullptr);
    assert(node->t != nullptr);

    Node* temp = nullptr;

    switch(v.traverseDirection) {
        // Here InOrder and PreOrder traversal is the same
        case TransformerVisitor::Traverse::InOrder:
        case TransformerVisitor::Traverse::PreOrder:
            // First traverse the root, then the right child
            temp = static_cast<Node*>(v.visit(node));
            temp->t = static_cast<ASTTerm1*>(temp->t->accept(v));
            return temp;
        case TransformerVisitor::Traverse::PostOrder:
            // First traverse the right childe, then the root
            node->t = static_cast<ASTTerm1*>(node->t->accept(v));
            return v.visit(node);
        default:
            assert(false && "Traversing AST_t not implemented yet");
    }
}

template<class RetNode, class Node>
RetNode traverse_tt(Node* node, TransformerVisitor &v) {
    assert(node != nullptr);
    assert(node->t1 != nullptr);
    assert(node->t2 != nullptr);

    Node* temp = nullptr;

    switch(v.traverseDirection) {
        case TransformerVisitor::Traverse::PreOrder:
            // First visit the root, then left and right child
            temp = static_cast<Node*>(v.visit(node));
            temp->t1 = static_cast<ASTTerm1*>(temp->t1->accept(v));
            temp->t2 = static_cast<ASTTerm1*>(temp->t2->accept(v));
            return temp;
        case TransformerVisitor::Traverse::PostOrder:
            // First visit childs, then the root
            node->t1 = static_cast<ASTTerm1*>(node->t1->accept(v));
            node->t2 = static_cast<ASTTerm1*>(node->t2->accept(v));
            return v.visit(node);
        case TransformerVisitor::Traverse::InOrder:
            // First visit left child, then root and then right child
            node->t1 = static_cast<ASTTerm1*>(node->t1->accept(v));
            temp = static_cast<Node*>(v.visit(node));
            temp->t2 = static_cast<ASTTerm1*>(temp->t2->accept(v));
            return temp;
        default:
            assert(false && "Traversing AST_tt not implemented yet");
    }
}

template<class RetNode, class Node>
RetNode traverse_TT(Node* node, TransformerVisitor &v) {
    assert(node != nullptr);
    assert(node->T1 != nullptr);
    assert(node->T2 != nullptr);

    Node* temp = nullptr;

    switch(v.traverseDirection) {
        case TransformerVisitor::Traverse::PreOrder:
            // First visit the root, then childs
            temp = static_cast<Node*>(v.visit(node));
            temp->T1 = static_cast<ASTTerm2*>(temp->T1->accept(v));
            temp->T2 = static_cast<ASTTerm2*>(temp->T2->accept(v));
            return temp;
        case TransformerVisitor::Traverse::PostOrder:
            // First visit childs, then the root
            node->T1 = static_cast<ASTTerm2*>(node->T1->accept(v));
            node->T2 = static_cast<ASTTerm2*>(node->T2->accept(v));
            return v.visit(node);
        case TransformerVisitor::Traverse::InOrder:
            // First visit left child, then root and then right child
            node->T1 = static_cast<ASTTerm2*>(node->T1->accept(v));
            temp = static_cast<Node*>(v.visit(node));
            temp->T2 = static_cast<ASTTerm2*>(temp->T2->accept(v));
            return temp;
        default:
            assert(false && "Traversing AST_TT not implemented yet");
    }
}

template<class RetNode, class Node>
RetNode traverse_tT(Node* node, TransformerVisitor &v) {
    assert(node != nullptr);
    assert(node->t1 != nullptr);
    assert(node->T2 != nullptr);

    Node* temp = nullptr;

    switch(v.traverseDirection) {
        case TransformerVisitor::Traverse::PreOrder:
            // First traverse the root, then childs
            temp = static_cast<Node*>(v.visit(node));
            temp->t1 = static_cast<ASTTerm1*>(temp->t1->accept(v));
            temp->T2 = static_cast<ASTTerm2*>(temp->T2->accept(v));
            return temp;
        case TransformerVisitor::Traverse::PostOrder:
            // First traverse the childs then the root
            node->t1 = static_cast<ASTTerm1*>(node->t1->accept(v));
            node->T2 = static_cast<ASTTerm2*>(node->T2->accept(v));
            return v.visit(node);
        case TransformerVisitor::Traverse::InOrder:
            // First traverse the left child, then root, then right child
            node->t1 = static_cast<ASTTerm1*>(node->t1->accept(v));
            temp = static_cast<Node*>(v.visit(node));
            temp->T2 = static_cast<ASTTerm2*>(temp->T2->accept(v));
            return temp;
        default:
            assert(false && "Traversing AST_tT not implemented yet");
    }
}

template<class RetNode, class Node>
RetNode traverse_f(Node* node, TransformerVisitor &v) {
    assert(node != nullptr);
    assert(node->f != nullptr);

    Node* temp = nullptr;

    switch(v.traverseDirection) {
        // InOrder and PreOrder traversal is the same
        case TransformerVisitor::Traverse::InOrder:
        case TransformerVisitor::Traverse::PreOrder:
            // First traverse the root, then the child
            temp = static_cast<Node*>(v.visit(node));
            temp->f = static_cast<ASTForm*>(temp->f->accept(v));
            return temp;
        case TransformerVisitor::Traverse::PostOrder:
            // First traverse the child, then the root
            node->f = static_cast<ASTForm*>(node->f->accept(v));
            return v.visit(node);
        default:
            assert(false && "Traversing AST_t not implemented yet");
    }
}

template<class RetNode, class Node>
RetNode traverse_ff(Node* node, TransformerVisitor &v) {
    assert(node != nullptr);
    assert(node->f1 != nullptr);
    assert(node->f2 != nullptr);

    Node* temp = nullptr;

    switch(v.traverseDirection) {
        case TransformerVisitor::Traverse::PreOrder:
            // First traverse the root, then the childs
            temp = static_cast<Node*>(v.visit(node));
            temp->f1 = static_cast<ASTForm*>(temp->f1->accept(v));
            temp->f2 = static_cast<ASTForm*>(temp->f2->accept(v));
            return temp;
        case TransformerVisitor::Traverse::PostOrder:
            // First traverse the childs, then the root
            node->f1 = static_cast<ASTForm*>(node->f1->accept(v));
            node->f2 = static_cast<ASTForm*>(node->f2->accept(v));
            return v.visit(node);
        case TransformerVisitor::Traverse::InOrder:
            // First traverse the left child, then the root and then right child
            node->f1 = static_cast<ASTForm*>(node->f1->accept(v));
            temp = static_cast<Node*>(v.visit(node));
            temp->f2 = static_cast<ASTForm*>(temp->f2->accept(v));
            return temp;
        default:
            assert(false && "Traversing AST_tT not implemented yet");
    }
}

AST* ASTTerm::accept(TransformerVisitor &v) {
    return this;
}

AST* ASTForm::accept(TransformerVisitor &v) {
    return this;
}

AST* ASTUniv::accept(TransformerVisitor &v) {
    return this;
}

// < ASTTerm1 Accepts > //
AST* ASTTerm1_n::accept(TransformerVisitor &v) {
    return v.visit(this);
}

AST* ASTTerm1_T::accept(TransformerVisitor &v) {
    return traverse_T<AST*, ASTTerm1_T>(this, v);
}

AST* ASTTerm1_t::accept(TransformerVisitor &v) {
    return traverse_t<AST*, ASTTerm1_t>(this, v);
}

AST* ASTTerm1_tn::accept(TransformerVisitor &v) {
    return traverse_t<AST*, ASTTerm1_tn>(this, v);
}

AST* ASTTerm1_tnt::accept(TransformerVisitor &v) {
    return traverse_tt<AST*, ASTTerm1_tnt>(this, v);
}

// < ASTTerm2 Accepts > //
AST* ASTTerm2_TT::accept(TransformerVisitor &v) {
    return traverse_TT<AST*, ASTTerm2_TT>(this, v);
}

AST* ASTTerm2_Tn::accept(TransformerVisitor &v) {
    return traverse_T<AST*, ASTTerm2_Tn>(this, v);
}

// < ASTForm Accepts > //
AST* ASTForm_tT::accept(TransformerVisitor &v) {
    return traverse_tT<AST*, ASTForm_tT>(this, v);
}

AST* ASTForm_T::accept(TransformerVisitor &v) {
    return traverse_T<AST*, ASTForm_T>(this, v);
}

AST* ASTForm_TT::accept(TransformerVisitor &v){
    return traverse_TT<AST*, ASTForm_TT>(this, v);
}

AST* ASTForm_tt::accept(TransformerVisitor &v) {
    return traverse_tt<AST*, ASTForm_tt>(this, v);
}

AST* ASTForm_nt::accept(TransformerVisitor &v) {
    return traverse_t<AST*, ASTForm_nt>(this, v);
}

AST* ASTForm_nT::accept(TransformerVisitor &v) {
    return traverse_T<AST*, ASTForm_nT>(this, v);
}

AST* ASTForm_f::accept(TransformerVisitor &v) {
    return traverse_f<AST*, ASTForm_f>(this, v);
}

AST* ASTForm_ff::accept(TransformerVisitor &v) {
    return traverse_ff<AST*, ASTForm_ff>(this, v);
}

AST* ASTForm_vf::accept(TransformerVisitor &v) {
    return traverse_f<AST*, ASTForm_vf>(this, v);
}

AST* ASTForm_uvf::accept(TransformerVisitor &v) {
    return traverse_f<AST*, ASTForm_uvf>(this, v);
}

AST* ASTTerm1_Var1::accept(TransformerVisitor &v) {
    return v.visit(this);
}

AST* ASTTerm1_Dot::accept(TransformerVisitor &v) {
    return v.visit(this);
}

AST* ASTTerm1_Up::accept(TransformerVisitor &v) {
    return v.visit(this);
}

AST* ASTTerm1_Root::accept(TransformerVisitor &v) {
    return v.visit(this);
}

AST* ASTTerm1_Int::accept(TransformerVisitor &v) {
    return v.visit(this);
}

AST* ASTTerm1_Plus::accept(TransformerVisitor &v) {
    return v.visit(this);
}

AST* ASTTerm1_Minus::accept(TransformerVisitor &v) {
    return v.visit(this);
}

AST* ASTTerm1_PlusModulo::accept(TransformerVisitor &v) {
    return v.visit(this);
}

AST* ASTTerm1_MinusModulo::accept(TransformerVisitor &v) {
    return v.visit(this);
}

AST* ASTTerm1_Min::accept(TransformerVisitor &v) {
    return v.visit(this);
}

AST* ASTTerm1_Max::accept(TransformerVisitor &v) {
    return v.visit(this);
}

AST* ASTTerm1_TreeRoot::accept(TransformerVisitor &v) {
    return v.visit(this);
}

AST* ASTTerm2_Var2::accept(TransformerVisitor &v) {
    return v.visit(this);
}

AST* ASTTerm2_VarTree::accept(TransformerVisitor &v) {
    return v.visit(this);
}

AST* ASTTerm2_Dot::accept(TransformerVisitor &v) {
    return v.visit(this);
}

AST* ASTTerm2_Up::accept(TransformerVisitor &v) {
    return v.visit(this);
}

AST* ASTTerm2_Empty::accept(TransformerVisitor &v) {
    return v.visit(this);
}

AST* ASTTerm2_Union::accept(TransformerVisitor &v) {
    return v.visit(this);
}

AST* ASTTerm2_Inter::accept(TransformerVisitor &v) {
    return v.visit(this);
}

AST* ASTTerm2_Setminus::accept(TransformerVisitor &v) {
    return v.visit(this);
}

AST* ASTTerm2_Set::accept(TransformerVisitor &v) {
    return v.visit(this);
}

AST* ASTTerm2_Plus::accept(TransformerVisitor &v) {
    return v.visit(this);
}

AST* ASTTerm2_Minus::accept(TransformerVisitor &v) {
    return v.visit(this);
}

AST* ASTTerm2_Interval::accept(TransformerVisitor &v) {
    return v.visit(this);
}

AST* ASTTerm2_PresbConst::accept(TransformerVisitor &v) {
    return v.visit(this);
}

AST* ASTTerm2_Formula::accept(TransformerVisitor &v) {
    return v.visit(this);
}

AST* ASTForm_Var0::accept(TransformerVisitor &v) {
    return v.visit(this);
}

AST* ASTForm_AllPosVar::accept(TransformerVisitor &v) {
    return v.visit(this);
}

AST* ASTForm_True::accept(TransformerVisitor &v) {
    return v.visit(this);
}

AST* ASTForm_False::accept(TransformerVisitor &v) {
    return v.visit(this);
}

AST* ASTForm_In::accept(TransformerVisitor &v) {
    return traverse_tT<AST*, ASTForm_In>(this, v);
}

AST* ASTForm_Notin::accept(TransformerVisitor &v) {
    return traverse_tT<AST*, ASTForm_Notin>(this, v);
}

AST* ASTForm_RootPred::accept(TransformerVisitor &v) {
    return traverse_t<AST*, ASTForm_RootPred>(this, v);
}

AST* ASTForm_EmptyPred::accept(TransformerVisitor &v) {
    return traverse_T<AST*, ASTForm_EmptyPred>(this, v);
}

AST* ASTForm_FirstOrder::accept(TransformerVisitor &v) {
    return traverse_t<AST*, ASTForm_FirstOrder>(this, v);
}

AST* ASTForm_Sub::accept(TransformerVisitor &v) {
    return traverse_TT<AST*, ASTForm_Sub>(this, v);
}

AST* ASTForm_Equal1::accept(TransformerVisitor &v) {
    return traverse_tt<AST*, ASTForm_Equal1>(this, v);
}

AST* ASTForm_Equal2::accept(TransformerVisitor &v) {
    return traverse_TT<AST*, ASTForm_Equal2>(this, v);
}

AST* ASTForm_NotEqual1::accept(TransformerVisitor &v) {
    return traverse_tt<AST*, ASTForm_NotEqual1>(this, v);
}

AST* ASTForm_NotEqual2::accept(TransformerVisitor &v) {
    return traverse_TT<AST*, ASTForm_NotEqual2>(this, v);
}

AST* ASTForm_Less::accept(TransformerVisitor &v) {
    return traverse_tt<AST*, ASTForm_Less>(this, v);
}

AST* ASTForm_LessEq::accept(TransformerVisitor &v) {
    return traverse_tt<AST*, ASTForm_LessEq>(this, v);
}

AST* ASTForm_WellFormedTree::accept(TransformerVisitor &v) {
    return traverse_T<AST*, ASTForm_WellFormedTree>(this, v);
}

AST* ASTForm_Impl::accept(TransformerVisitor &v) {
    return traverse_ff<AST*, ASTForm_Impl>(this, v);
}

AST* ASTForm_Biimpl::accept(TransformerVisitor &v) {
    return traverse_ff<AST*, ASTForm_Biimpl>(this, v);
}

AST* ASTForm_And::accept(TransformerVisitor &v) {
    return traverse_ff<AST*, ASTForm_And>(this, v);
}

AST* ASTForm_IdLeft::accept(TransformerVisitor &v) {
    return traverse_ff<AST*, ASTForm_IdLeft>(this, v);
}

AST* ASTForm_Or::accept(TransformerVisitor &v) {
    return traverse_ff<AST*, ASTForm_Or>(this, v);
}

AST* ASTForm_Not::accept(TransformerVisitor &v) {
    return traverse_f<AST*, ASTForm_Not>(this, v);
}

AST* ASTForm_Ex0::accept(TransformerVisitor &v) {
    return traverse_f<AST*, ASTForm_Ex0>(this, v);
}

AST* ASTForm_Ex1::accept(TransformerVisitor &v) {
    return traverse_f<AST*, ASTForm_Ex1>(this, v);
}

AST* ASTForm_Ex2::accept(TransformerVisitor &v) {
    return traverse_f<AST*, ASTForm_Ex2>(this, v);
}

AST* ASTForm_All0::accept(TransformerVisitor &v) {
    return traverse_f<AST*, ASTForm_All0>(this, v);
}

AST* ASTForm_All1::accept(TransformerVisitor &v) {
    return traverse_f<AST*, ASTForm_All1>(this, v);
}

AST* ASTForm_All2::accept(TransformerVisitor &v) {
    return traverse_f<AST*, ASTForm_All2>(this, v);
}

AST* ASTForm_Let0::accept(TransformerVisitor &v) {
    return traverse_f<AST*, ASTForm_Let0>(this, v);
}

AST* ASTForm_Let1::accept(TransformerVisitor &v) {
    return traverse_f<AST*, ASTForm_Let1>(this, v);
}

AST* ASTForm_Let2::accept(TransformerVisitor &v) {
    return traverse_f<AST*, ASTForm_Let2>(this, v);
}

AST* ASTForm_Call::accept(TransformerVisitor &v) {
    return v.visit(this);
}

AST* ASTForm_Import::accept(TransformerVisitor &v) {
    return v.visit(this);
}

AST* ASTForm_Export::accept(TransformerVisitor &v) {
    return traverse_f<AST*, ASTForm_Export>(this, v);
}

AST* ASTForm_Prefix::accept(TransformerVisitor &v) {
    return traverse_f<AST*, ASTForm_Prefix>(this, v);
}

AST* ASTForm_Restrict::accept(TransformerVisitor &v) {
    return traverse_f<AST*, ASTForm_Restrict>(this, v);
}

AST* ASTForm_InStateSpace1::accept(TransformerVisitor &v) {
    return traverse_t<AST*, ASTForm_InStateSpace1>(this, v);
}

AST* ASTForm_InStateSpace2::accept(TransformerVisitor &v) {
    return traverse_T<AST*, ASTForm_InStateSpace2>(this, v);
}

AST* ASTForm_SomeType::accept(TransformerVisitor &v) {
    return traverse_t<AST*, ASTForm_SomeType>(this, v);
}

/**
 * Traverses sons and visits the nodes before
 *
 * @param[in] v:    visitor without parameters returning void
 */

template<class Node>
void void_traverse_T(Node* node, VoidVisitor &v) {
    assert(node != nullptr);
    assert(node->T != nullptr);

    switch(v.traverseDirection) {
        case TransformerVisitor::Traverse::InOrder:
        case TransformerVisitor::Traverse::PreOrder:
            // First traverse the root then the child
            v.visit(node);
            node->T->accept(v);
            break;
        case TransformerVisitor::Traverse::PostOrder:
            // First traverse the child then the root
            node->T->accept(v);
            v.visit(node);
            break;
        default:
            assert(false && "Traversing AST_T not implemented yet");
    }
}

template<class Node>
void void_traverse_t(Node* node, VoidVisitor &v) {
    assert(node != nullptr);
    assert(node->t != nullptr);

    switch(v.traverseDirection) {
        case TransformerVisitor::Traverse::InOrder:
        case TransformerVisitor::Traverse::PreOrder:
            // First traverse the root, then the child
            v.visit(node);
            node->t->accept(v);
            break;
        case TransformerVisitor::Traverse::PostOrder:
            // First traverse the child then the root
            node->t->accept(v);
            v.visit(node);
            break;
        default:
            assert(false && "Traversing AST_t not implemented yet");
    }
}

template<class Node>
void void_traverse_tt(Node* node, VoidVisitor &v) {
    assert(node != nullptr);
    assert(node->t1 != nullptr);
    assert(node->t2 != nullptr);

    switch(v.traverseDirection) {
        case TransformerVisitor::Traverse::PreOrder:
            // First traverse the root, then childs
            v.visit(node);
            node->t1->accept(v);
            node->t2->accept(v);
            break;
        case TransformerVisitor::Traverse::PostOrder:
            // First traverse childs, then the root
            node->t1->accept(v);
            node->t2->accept(v);
            v.visit(node);
            break;
        case TransformerVisitor::Traverse::InOrder:
            // First traverse left child, then root and last right child
            node->t1->accept(v);
            v.visit(node);
            node->t2->accept(v);
            break;
        default:
            assert(false && "Traversing AST_tt not implemented yet");
    }
}

template<class Node>
void void_traverse_TT(Node* node, VoidVisitor &v) {
    assert(node != nullptr);
    assert(node->T1 != nullptr);
    assert(node->T2 != nullptr);

    switch(v.traverseDirection) {
        case TransformerVisitor::Traverse::PreOrder:
            // First visit the root then childs
            v.visit(node);
            node->T1->accept(v);
            node->T2->accept(v);
            break;
        case TransformerVisitor::Traverse::PostOrder:
            // First visit childs, then the root
            node->T1->accept(v);
            node->T2->accept(v);
            v.visit(node);
            break;
        case TransformerVisitor::Traverse::InOrder:
            // First left child, then root, then right child
            node->T1->accept(v);
            v.visit(node);
            node->T2->accept(v);
            break;
        default:
            assert(false && "Traversing AST_TT not implemented yet");
    }
}

template<class Node>
void void_traverse_tT(Node* node, VoidVisitor &v) {
    assert(node != nullptr);
    assert(node->t1 != nullptr);
    assert(node->T2 != nullptr);

    switch(v.traverseDirection) {
        case TransformerVisitor::Traverse::PreOrder:
            // First traverse the root, then childs
            v.visit(node);
            node->t1->accept(v);
            node->T2->accept(v);
            break;
        case TransformerVisitor::Traverse::PostOrder:
            // First traverse childs, then the root
            node->t1->accept(v);
            node->T2->accept(v);
            v.visit(node);
            break;
        case TransformerVisitor::Traverse::InOrder:
            // First traverse left child, then root, then right child
            node->t1->accept(v);
            v.visit(node);
            node->T2->accept(v);
            break;
        default:
            assert(false && "Traversing AST_tT not implemented yet");
    }
}

template<class Node>
void void_traverse_f(Node* node, VoidVisitor &v) {
    assert(node != nullptr);
    assert(node->f != nullptr);

    switch(v.traverseDirection) {
        case TransformerVisitor::Traverse::InOrder:
        case TransformerVisitor::Traverse::PreOrder:
            // First traverse the root, then childs
            v.visit(node);
            node->f->accept(v);
            break;
        case TransformerVisitor::Traverse::PostOrder:
            // First traverse childs, then the root
            node->f->accept(v);
            v.visit(node);
            break;
        default:
            assert(false && "Traversing AST_t not implemented yet");
    }
}

template<class Node>
void void_traverse_ff(Node* node, VoidVisitor &v) {
    assert(node != nullptr);
    assert(node->f1 != nullptr);
    assert(node->f2 != nullptr);

    switch(v.traverseDirection) {
        case TransformerVisitor::Traverse::PreOrder:
            // First traverse the root, then childs
            v.visit(node);
            node->f1->accept(v);
            node->f2->accept(v);
            break;
        case TransformerVisitor::Traverse::PostOrder:
            // First traverse childs, then the root
            node->f1->accept(v);
            node->f2->accept(v);
            v.visit(node);
            break;
        case TransformerVisitor::Traverse::InOrder:
            // First traverse left childs, then root, then right child
            node->f1->accept(v);
            v.visit(node);
            node->f2->accept(v);
            break;
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

void ASTForm_AllPosVar::accept(VoidVisitor &v) {
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