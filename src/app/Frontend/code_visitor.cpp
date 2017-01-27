//
// Created by Raph on 15/09/2015.
//

#include "code_visitor.h"

template<class Code>
void void_traverse_c(Code* code, VoidCodeVisitor &v) {
    assert(code != nullptr);
    assert(code->vc.code != nullptr);

    switch(v.traverseDirection) {
        case VoidCodeVisitor::Traverse::InOrder:
        case VoidCodeVisitor::Traverse::PreOrder:
            // First traverse the root, then childs
            v.visit(code);
            code->vc.code->accept(v);
            break;
        case VoidCodeVisitor::Traverse::PostOrder:
            // First traverse childs, then the root
            code->vc.code->accept(v);
            v.visit(code);
            break;
        default:
            assert(false && "Traversing Code_c not implemented yet");
            abort();
    }
}

template<class Code>
void void_traverse_cc(Code* code, VoidCodeVisitor &v) {
    assert(code != nullptr);
    assert(code->vc1.code != nullptr);
    assert(code->vc2.code != nullptr);

    switch(v.traverseDirection) {
        case VoidCodeVisitor::Traverse::PreOrder:
            // First traverse the root, then childs
            v.visit(code);
            code->vc1.code->accept(v);
            code->vc2.code->accept(v);
            break;
        case VoidCodeVisitor::Traverse::PostOrder:
            // First traverse childs, then the root
            code->vc1.code->accept(v);
            code->vc2.code->accept(v);
            v.visit(code);
            break;
        case VoidCodeVisitor::Traverse::InOrder:
            // First traverse left childs, then root, then right child
            code->vc1.code->accept(v);
            v.visit(code);
            code->vc2.code->accept(v);
            break;
        default:
            assert(false && "Traversing Code_cc not implemented yet");
            abort();
    }
}

void Code::accept(VoidCodeVisitor &v) {
    v.visit(this);
}

// < Code Derives >
void Code_n::accept(VoidCodeVisitor &v) {
    v.visit(this);
}

void Code_ni::accept(VoidCodeVisitor &v) {
    v.visit(this);
}

void Code_nn::accept(VoidCodeVisitor &v) {
    v.visit(this);
}

void Code_nni::accept(VoidCodeVisitor &v) {
    v.visit(this);
}

void Code_nnn::accept(VoidCodeVisitor &v) {
    v.visit(this);
}

void Code_c::accept(VoidCodeVisitor &v) {
    void_traverse_c<Code_c>(this, v);
}

void Code_cc::accept(VoidCodeVisitor &v) {
    void_traverse_cc<Code_cc>(this, v);
}

// < Code Derives - specific >
void Code_True::accept(VoidCodeVisitor &v) {
    v.visit(this);
}

void Code_False::accept(VoidCodeVisitor &v) {
    v.visit(this);
}

// < Code_n(i) Derives >
void Code_EqEmpty::accept(VoidCodeVisitor &v) {
    v.visit(this);
}

void Code_EqRoot::accept(VoidCodeVisitor &v) {
    v.visit(this);
}

void Code_FirstOrder::accept(VoidCodeVisitor &v) {
    v.visit(this);
}

void Code_Singleton::accept(VoidCodeVisitor &v) {
    v.visit(this);
}

void Code_BoolVar::accept(VoidCodeVisitor &v) {
    v.visit(this);
}

void Code_InStateSpace::accept(VoidCodeVisitor &v) {
    v.visit(this);
}

void Code_SomeType::accept(VoidCodeVisitor &v) {
    v.visit(this);
}

void Code_EqConst::accept(VoidCodeVisitor &v) {
    v.visit(this);
}

void Code_WellFormedTree::accept(VoidCodeVisitor &v) {
    v.visit(this);
}

// < Code_nn(i) Derives >
void Code_In::accept(VoidCodeVisitor &v) {
    v.visit(this);
}

void Code_Eq1::accept(VoidCodeVisitor &v) {
    v.visit(this);
}

void Code_Eq2::accept(VoidCodeVisitor &v) {
    v.visit(this);
}

void Code_Sub2::accept(VoidCodeVisitor &v) {
    v.visit(this);
}

void Code_Less1::accept(VoidCodeVisitor &v) {
    v.visit(this);
}

void Code_LessEq1::accept(VoidCodeVisitor &v) {
    v.visit(this);
}

void Code_EqDot0::accept(VoidCodeVisitor &v) {
    v.visit(this);
}

void Code_EqDot1::accept(VoidCodeVisitor &v) {
    v.visit(this);
}

void Code_EqUp::accept(VoidCodeVisitor &v) {
    v.visit(this);
}

void Code_EqPlus2::accept(VoidCodeVisitor &v) {
    v.visit(this);
}

void Code_EqMinus2::accept(VoidCodeVisitor &v) {
    v.visit(this);
}

void Code_EqMin::accept(VoidCodeVisitor &v) {
    v.visit(this);
}

void Code_EqMax::accept(VoidCodeVisitor &v) {
    v.visit(this);
}

void Code_EqMinus1::accept(VoidCodeVisitor &v) {
    v.visit(this);
}

void Code_EqPlus1::accept(VoidCodeVisitor &v) {
    v.visit(this);
}

void Code_EqPresbConst::accept(VoidCodeVisitor &v) {
    v.visit(this);
}

// < Code_nnn Derives >
void Code_EqUnion::accept(VoidCodeVisitor &v) {
    v.visit(this);
}

void Code_EqInter::accept(VoidCodeVisitor &v) {
    v.visit(this);
}

void Code_EqSetMinus::accept(VoidCodeVisitor &v) {
    v.visit(this);
}

void Code_EqPlusModulo::accept(VoidCodeVisitor &v) {
    v.visit(this);
}

void Code_EqMinusModulo::accept(VoidCodeVisitor &v) {
    v.visit(this);
}

// < Code_c Derives >
void Code_Restrict::accept(VoidCodeVisitor &v) {
    void_traverse_c<Code_Restrict>(this, v);
}

void Code_Project::accept(VoidCodeVisitor &v) {
    void_traverse_c<Code_Project>(this, v);
}

void Code_Negate::accept(VoidCodeVisitor &v) {
    void_traverse_c<Code_Negate>(this, v);
}

void Code_Prefix::accept(VoidCodeVisitor &v) {
    void_traverse_c<Code_Prefix>(this, v);
}

void Code_PredCall::accept(VoidCodeVisitor &v) {
    void_traverse_c<Code_PredCall>(this, v);
}

void Code_Import::accept(VoidCodeVisitor &v) {
    v.visit(this);
}

void Code_Export::accept(VoidCodeVisitor &v) {
    void_traverse_c<Code_Export>(this, v);
}

// < Code_cc Derives >
void Code_And::accept(VoidCodeVisitor &v) {
    void_traverse_cc<Code_And>(this, v);
}

void Code_IdLeft::accept(VoidCodeVisitor &v) {
    void_traverse_cc<Code_IdLeft>(this, v);
}

void Code_Or::accept(VoidCodeVisitor &v) {
    void_traverse_cc<Code_Or>(this, v);
}

void Code_Impl::accept(VoidCodeVisitor &v) {
    void_traverse_cc<Code_Impl>(this, v);
}

void Code_Biimpl::accept(VoidCodeVisitor &v) {
    void_traverse_cc<Code_Biimpl>(this, v);
}

// <--------------------->
// < Transformer visitor >
// <--------------------->


template<class RetCode, class Code>
RetCode traverse_c(Code* code, TransformerCodeVisitor &v) {
    assert(code != nullptr);
    assert(code->vc.code != nullptr);

    Code* temp = nullptr;

    switch(v.traverseDirection) {
        // Here InOrder and PreOrder traversal is the same
        case TransformerCodeVisitor::Traverse::InOrder:
        case TransformerCodeVisitor::Traverse::PreOrder:
            // First traverse the root, then the right child
            temp = static_cast<Code*>(v.visit(code));
            temp->vc.code = temp->vc.code->accept(v);
            return temp;
        case TransformerCodeVisitor::Traverse::PostOrder:
            // First traverse the right childe, then the root
            code->vc.code = code->vc.code->accept(v);
            return v.visit(code);
        default:
            assert(false && "Traversing Code_c not implemented yet");
            abort();
    }
}

template<class RetCode, class Code>
RetCode traverse_cc(Code* code, TransformerCodeVisitor &v) {
    assert(code != nullptr);
    assert(code->vc1.code != nullptr);
    assert(code->vc2.code != nullptr);

    Code* temp = nullptr;

    switch(v.traverseDirection) {
        case TransformerCodeVisitor::Traverse::PreOrder:
            // First visit the root, then left and right child
            temp = static_cast<Code*>(v.visit(code));
            temp->vc1.code = temp->vc1.code->accept(v);
            temp->vc2.code = temp->vc2.code->accept(v);
            return temp;
        case TransformerCodeVisitor::Traverse::PostOrder:
            // First visit childs, then the root
            code->vc1.code = code->vc1.code->accept(v);
            code->vc2.code = code->vc2.code->accept(v);
            return v.visit(code);
        case TransformerCodeVisitor::Traverse::InOrder:
            // First visit left child, then root and then right child
            code->vc1.code = code->vc1.code->accept(v);
            temp = static_cast<Code*>(v.visit(code));
            temp->vc2.code = temp->vc2.code->accept(v);
            return temp;
        default:
            assert(false && "Traversing Code_cc not implemented yet");
            abort();
    }
}

Code* Code::accept(TransformerCodeVisitor &v) {
    return v.visit(this);
}

// < Code Derives >
Code* Code_n::accept(TransformerCodeVisitor &v) {
    return v.visit(this);
}

Code* Code_ni::accept(TransformerCodeVisitor &v) {
    return v.visit(this);
}

Code* Code_nn::accept(TransformerCodeVisitor &v) {
    return v.visit(this);
}

Code* Code_nni::accept(TransformerCodeVisitor &v) {
    return v.visit(this);
}

Code* Code_nnn::accept(TransformerCodeVisitor &v) {
    return v.visit(this);
}

Code* Code_c::accept(TransformerCodeVisitor &v) {
    return traverse_c<Code*, Code_c>(this, v);
}

Code* Code_cc::accept(TransformerCodeVisitor &v) {
    return traverse_cc<Code*, Code_cc>(this, v);
}

// < Code Derives - specific >
Code* Code_True::accept(TransformerCodeVisitor &v) {
    return v.visit(this);
}

Code* Code_False::accept(TransformerCodeVisitor &v) {
    return v.visit(this);
}

// < Code_n(i) Derives >
Code* Code_EqEmpty::accept(TransformerCodeVisitor &v) {
    return v.visit(this);
}

Code* Code_EqRoot::accept(TransformerCodeVisitor &v) {
    return v.visit(this);
}

Code* Code_FirstOrder::accept(TransformerCodeVisitor &v) {
    return v.visit(this);
}

Code* Code_Singleton::accept(TransformerCodeVisitor &v) {
    return v.visit(this);
}

Code* Code_BoolVar::accept(TransformerCodeVisitor &v) {
    return v.visit(this);
}

Code* Code_InStateSpace::accept(TransformerCodeVisitor &v) {
    return v.visit(this);
}

Code* Code_SomeType::accept(TransformerCodeVisitor &v) {
    return v.visit(this);
}

Code* Code_EqConst::accept(TransformerCodeVisitor &v) {
    return v.visit(this);
}

Code* Code_WellFormedTree::accept(TransformerCodeVisitor &v) {
    return v.visit(this);
}

// < Code_nn(i) Derives >
Code* Code_In::accept(TransformerCodeVisitor &v) {
    return v.visit(this);
}

Code* Code_Eq1::accept(TransformerCodeVisitor &v) {
    return v.visit(this);
}

Code* Code_Eq2::accept(TransformerCodeVisitor &v) {
    return v.visit(this);
}

Code* Code_Sub2::accept(TransformerCodeVisitor &v) {
    return v.visit(this);
}

Code* Code_Less1::accept(TransformerCodeVisitor &v) {
    return v.visit(this);
}

Code* Code_LessEq1::accept(TransformerCodeVisitor &v) {
    return v.visit(this);
}

Code* Code_EqDot0::accept(TransformerCodeVisitor &v) {
    return v.visit(this);
}

Code* Code_EqDot1::accept(TransformerCodeVisitor &v) {
    return v.visit(this);
}

Code* Code_EqUp::accept(TransformerCodeVisitor &v) {
    return v.visit(this);
}

Code* Code_EqPlus2::accept(TransformerCodeVisitor &v) {
    return v.visit(this);
}

Code* Code_EqMinus2::accept(TransformerCodeVisitor &v) {
    return v.visit(this);
}

Code* Code_EqMin::accept(TransformerCodeVisitor &v) {
    return v.visit(this);
}

Code* Code_EqMax::accept(TransformerCodeVisitor &v) {
    return v.visit(this);
}

Code* Code_EqMinus1::accept(TransformerCodeVisitor &v) {
    return v.visit(this);
}

Code* Code_EqPlus1::accept(TransformerCodeVisitor &v) {
    return v.visit(this);
}

Code* Code_EqPresbConst::accept(TransformerCodeVisitor &v) {
    return v.visit(this);
}

// < Code_nnn Derives >
Code* Code_EqUnion::accept(TransformerCodeVisitor &v) {
    return v.visit(this);
}

Code* Code_EqInter::accept(TransformerCodeVisitor &v) {
    return v.visit(this);
}

Code* Code_EqSetMinus::accept(TransformerCodeVisitor &v) {
    return v.visit(this);
}

Code* Code_EqPlusModulo::accept(TransformerCodeVisitor &v) {
    return v.visit(this);
}

Code* Code_EqMinusModulo::accept(TransformerCodeVisitor &v) {
    return v.visit(this);
}

// < Code_c Derives >
Code* Code_Restrict::accept(TransformerCodeVisitor &v) {
    return traverse_c<Code*, Code_Restrict>(this, v);
}

Code* Code_Project::accept(TransformerCodeVisitor &v) {
    return traverse_c<Code*, Code_Project>(this, v);
}

Code* Code_Negate::accept(TransformerCodeVisitor &v) {
    return traverse_c<Code*, Code_Negate>(this, v);
}

Code* Code_Prefix::accept(TransformerCodeVisitor &v) {
    return traverse_c<Code*, Code_Prefix>(this, v);
}

Code* Code_PredCall::accept(TransformerCodeVisitor &v) {
    return traverse_c<Code*, Code_PredCall>(this, v);
}

Code* Code_Import::accept(TransformerCodeVisitor &v) {
    return v.visit(this);
}

Code* Code_Export::accept(TransformerCodeVisitor &v) {
    return traverse_c<Code*, Code_Export>(this, v);
}

// < Code_cc Derives >
Code* Code_And::accept(TransformerCodeVisitor &v) {
    return traverse_cc<Code*, Code_And>(this, v);
}

Code* Code_IdLeft::accept(TransformerCodeVisitor &v) {
    return traverse_cc<Code*, Code_IdLeft>(this, v);
}

Code* Code_Or::accept(TransformerCodeVisitor &v) {
    return traverse_cc<Code*, Code_Or>(this, v);
}

Code* Code_Impl::accept(TransformerCodeVisitor &v) {
    return traverse_cc<Code*, Code_Impl>(this, v);
}

Code* Code_Biimpl::accept(TransformerCodeVisitor &v) {
    return traverse_cc<Code*, Code_Biimpl>(this, v);
}