//
// Created by Raph on 15/09/2015.
//

#include "code_visitor.h"

void Code::accept(VoidCodeVisitor &v) {
    // TODO;
}

// < Code Derives >
void Code_n::accept(VoidCodeVisitor &v) {
    // TODO;
}

void Code_ni::accept(VoidCodeVisitor &v) {
    // TODO;
}

void Code_nn::accept(VoidCodeVisitor &v) {
    // TODO;
}

void Code_nni::accept(VoidCodeVisitor &v) {
    // TODO;
}

void Code_nnn::accept(VoidCodeVisitor &v) {
    // TODO;
}

void Code_c::accept(VoidCodeVisitor &v) {
    // TODO;
}

void Code_cc::accept(VoidCodeVisitor &v) {
    // TODO;
}

// < Code Derives - specific >
void Code_True::accept(VoidCodeVisitor &v) {
    // TODO;
}

void Code_False::accept(VoidCodeVisitor &v) {
    // TODO;
}

// < Code_n(i) Derives >
void Code_EqEmpty::accept(VoidCodeVisitor &v) {
    // TODO;
}

void Code_EqRoot::accept(VoidCodeVisitor &v) {
    // TODO;
}

void Code_FirstOrder::accept(VoidCodeVisitor &v) {
    // TODO;
}

void Code_Singleton::accept(VoidCodeVisitor &v) {
    // TODO;
}

void Code_BoolVar::accept(VoidCodeVisitor &v) {
    // TODO;
}

void Code_InStateSpace::accept(VoidCodeVisitor &v) {
    // TODO;
}

void Code_SomeType::accept(VoidCodeVisitor &v) {
    // TODO;
}

void Code_EqConst::accept(VoidCodeVisitor &v) {
    // TODO;
}

void Code_WellFormedTree::accept(VoidCodeVisitor &v) {
    // TODO;
}

// < Code_nn(i) Derives >
void Code_In::accept(VoidCodeVisitor &v) {
    // TODO;
}

void Code_Eq1::accept(VoidCodeVisitor &v) {
    // TODO;
}

void Code_Eq2::accept(VoidCodeVisitor &v) {
    // TODO;
}

void Code_Sub2::accept(VoidCodeVisitor &v) {
    // TODO;
}

void Code_Less1::accept(VoidCodeVisitor &v) {
    // TODO;
}

void Code_LessEq1::accept(VoidCodeVisitor &v) {
    // TODO;
}

void Code_EqDot0::accept(VoidCodeVisitor &v) {
    // TODO;
}

void Code_EqDot1::accept(VoidCodeVisitor &v) {
    // TODO;
}

void Code_EqUp::accept(VoidCodeVisitor &v) {
    // TODO;
}

void Code_EqPlus2::accept(VoidCodeVisitor &v) {
    // TODO;
}

void Code_EqMinus2::accept(VoidCodeVisitor &v) {
    // TODO;
}

void Code_EqMin::accept(VoidCodeVisitor &v) {
    // TODO;
}

void Code_EqMax::accept(VoidCodeVisitor &v) {
    // TODO;
}

void Code_EqMinus1::accept(VoidCodeVisitor &v) {
    // TODO;
}

void Code_EqPlus1::accept(VoidCodeVisitor &v) {
    // TODO;
}

void Code_EqPresbConst::accept(VoidCodeVisitor &v) {
    // TODO;
}

// < Code_nnn Derives >
void Code_EqUnion::accept(VoidCodeVisitor &v) {
    // TODO;
}

void Code_EqInter::accept(VoidCodeVisitor &v) {
    // TODO;
}

void Code_EqSetMinus::accept(VoidCodeVisitor &v) {
    // TODO;
}

void Code_EqPlusModulo::accept(VoidCodeVisitor &v) {
    // TODO;
}

void Code_EqMinusModulo::accept(VoidCodeVisitor &v) {
    // TODO;
}

// < Code_c Derives >
void Code_Restrict::accept(VoidCodeVisitor &v) {
    // TODO;
}

void Code_Project::accept(VoidCodeVisitor &v) {
    // TODO;
}

void Code_Negate::accept(VoidCodeVisitor &v) {
    // TODO;
}

void Code_Prefix::accept(VoidCodeVisitor &v) {
    // TODO;
}

void Code_PredCall::accept(VoidCodeVisitor &v) {
    // TODO;
}

void Code_Import::accept(VoidCodeVisitor &v) {
    // TODO;
}

void Code_Export::accept(VoidCodeVisitor &v) {
    // TODO;
}

// < Code_cc Derives >
void Code_And::accept(VoidCodeVisitor &v) {
    // TODO;
}

void Code_IdLeft::accept(VoidCodeVisitor &v) {
    // TODO;
}

void Code_Or::accept(VoidCodeVisitor &v) {
    // TODO;
}

void Code_Impl::accept(VoidCodeVisitor &v) {
    // TODO;
}

void Code_Biimpl::accept(VoidCodeVisitor &v) {
    // TODO;
}

// <--------------------->
// < Transformer visitor >
// <--------------------->

Code* Code::accept(TransformerCodeVisitor &v) {
    return nullptr;
}

// < Code Derives >
Code* Code_n::accept(TransformerCodeVisitor &v) {
    return nullptr;
}

Code* Code_ni::accept(TransformerCodeVisitor &v) {
    return nullptr;
}

Code* Code_nn::accept(TransformerCodeVisitor &v) {
    return nullptr;
}

Code* Code_nni::accept(TransformerCodeVisitor &v) {
    return nullptr;
}

Code* Code_nnn::accept(TransformerCodeVisitor &v) {
    return nullptr;
}

Code* Code_c::accept(TransformerCodeVisitor &v) {
    return nullptr;
}

Code* Code_cc::accept(TransformerCodeVisitor &v) {
    return nullptr;
}

// < Code Derives - specific >
Code* Code_True::accept(TransformerCodeVisitor &v) {
    return nullptr;
}

Code* Code_False::accept(TransformerCodeVisitor &v) {
    return nullptr;
}

// < Code_n(i) Derives >
Code* Code_EqEmpty::accept(TransformerCodeVisitor &v) {
    return nullptr;
}

Code* Code_EqRoot::accept(TransformerCodeVisitor &v) {
    return nullptr;
}

Code* Code_FirstOrder::accept(TransformerCodeVisitor &v) {
    return nullptr;
}

Code* Code_Singleton::accept(TransformerCodeVisitor &v) {
    return nullptr;
}

Code* Code_BoolVar::accept(TransformerCodeVisitor &v) {
    return nullptr;
}

Code* Code_InStateSpace::accept(TransformerCodeVisitor &v) {
    return nullptr;
}

Code* Code_SomeType::accept(TransformerCodeVisitor &v) {
    return nullptr;
}

Code* Code_EqConst::accept(TransformerCodeVisitor &v) {
    return nullptr;
}

Code* Code_WellFormedTree::accept(TransformerCodeVisitor &v) {
    return nullptr;
}

// < Code_nn(i) Derives >
Code* Code_In::accept(TransformerCodeVisitor &v) {
    return nullptr;
}

Code* Code_Eq1::accept(TransformerCodeVisitor &v) {
    return nullptr;
}

Code* Code_Eq2::accept(TransformerCodeVisitor &v) {
    return nullptr;
}

Code* Code_Sub2::accept(TransformerCodeVisitor &v) {
    return nullptr;
}

Code* Code_Less1::accept(TransformerCodeVisitor &v) {
    return nullptr;
}

Code* Code_LessEq1::accept(TransformerCodeVisitor &v) {
    return nullptr;
}

Code* Code_EqDot0::accept(TransformerCodeVisitor &v) {
    return nullptr;
}

Code* Code_EqDot1::accept(TransformerCodeVisitor &v) {
    return nullptr;
}

Code* Code_EqUp::accept(TransformerCodeVisitor &v) {
    return nullptr;
}

Code* Code_EqPlus2::accept(TransformerCodeVisitor &v) {
    return nullptr;
}

Code* Code_EqMinus2::accept(TransformerCodeVisitor &v) {
    return nullptr;
}

Code* Code_EqMin::accept(TransformerCodeVisitor &v) {
    return nullptr;
}

Code* Code_EqMax::accept(TransformerCodeVisitor &v) {
    return nullptr;
}

Code* Code_EqMinus1::accept(TransformerCodeVisitor &v) {
    return nullptr;
}

Code* Code_EqPlus1::accept(TransformerCodeVisitor &v) {
    return nullptr;
}

Code* Code_EqPresbConst::accept(TransformerCodeVisitor &v) {
    return nullptr;
}

// < Code_nnn Derives >
Code* Code_EqUnion::accept(TransformerCodeVisitor &v) {
    return nullptr;
}

Code* Code_EqInter::accept(TransformerCodeVisitor &v) {
    return nullptr;
}

Code* Code_EqSetMinus::accept(TransformerCodeVisitor &v) {
    return nullptr;
}

Code* Code_EqPlusModulo::accept(TransformerCodeVisitor &v) {
    return nullptr;
}

Code* Code_EqMinusModulo::accept(TransformerCodeVisitor &v) {
    return nullptr;
}

// < Code_c Derives >
Code* Code_Restrict::accept(TransformerCodeVisitor &v) {
    return nullptr;
}

Code* Code_Project::accept(TransformerCodeVisitor &v) {
    return nullptr;
}

Code* Code_Negate::accept(TransformerCodeVisitor &v) {
    return nullptr;
}

Code* Code_Prefix::accept(TransformerCodeVisitor &v) {
    return nullptr;
}

Code* Code_PredCall::accept(TransformerCodeVisitor &v) {
    return nullptr;
}

Code* Code_Import::accept(TransformerCodeVisitor &v) {
    return nullptr;
}

Code* Code_Export::accept(TransformerCodeVisitor &v) {
    return nullptr;
}

// < Code_cc Derives >
Code* Code_And::accept(TransformerCodeVisitor &v) {
    return nullptr;
}

Code* Code_IdLeft::accept(TransformerCodeVisitor &v) {
    return nullptr;
}

Code* Code_Or::accept(TransformerCodeVisitor &v) {
    return nullptr;
}

Code* Code_Impl::accept(TransformerCodeVisitor &v) {
    return nullptr;
}

Code* Code_Biimpl::accept(TransformerCodeVisitor &v) {
    return nullptr;
}