#ifndef WSKS_CODE_VISITOR_H
#define WSKS_CODE_VISITOR_H

#include "code.h"

template<typename R = void>
class CodeVisitor {
public:
    // TODO: Consider moving this away and merging with ASTVisitor::Traversal
    enum Traverse {PreOrder, InOrder, PostOrder};
    const Traverse traverseDirection;

    typedef R ReturnType;

    // < Constructors >
    CodeVisitor(CodeVisitor::Traverse td) : traverseDirection(td) {}

    virtual ReturnType visit(Code* c) { };
};

// < Void Visitor >
class VoidCodeVisitor : public CodeVisitor<> {
public:
    VoidCodeVisitor(CodeVisitor::Traverse td) : CodeVisitor<>(td) {}
    virtual void visit(Code* c) { }

    // < Code Derives >
    virtual void visit(Code_n* c) { }
    virtual void visit(Code_ni* c) { }
    virtual void visit(Code_nn* c) { }
    virtual void visit(Code_nni* c) { }
    virtual void visit(Code_nnn* c) { }
    virtual void visit(Code_c* c) { }
    virtual void visit(Code_cc* c) { }
    // TODO: do we need it? virtual void visit(Code_clist* c) { }

    // < Code Derives - specific >
    virtual void visit(Code_True* c) { }
    virtual void visit(Code_False* c) { }

    // < Code_n(i) Derives >
    virtual void visit(Code_EqEmpty* c) { }
    virtual void visit(Code_EqRoot* c) { }
    virtual void visit(Code_FirstOrder* c) { }
    virtual void visit(Code_Singleton* c) { }
    virtual void visit(Code_BoolVar* c) { }
    virtual void visit(Code_InStateSpace* c) { }
    virtual void visit(Code_SomeType* c) { }
    virtual void visit(Code_EqConst* c) { }
    virtual void visit(Code_WellFormedTree* c) { }

    // < Code_nn(i) Derives >
    virtual void visit(Code_In* c) { }
    virtual void visit(Code_Eq1* c) { }
    virtual void visit(Code_Eq2* c) { }
    virtual void visit(Code_Sub2* c) { }
    virtual void visit(Code_Less1* c) { }
    virtual void visit(Code_LessEq1* c) { }
    virtual void visit(Code_EqDot0* c) { }
    virtual void visit(Code_EqDot1* c) { }
    virtual void visit(Code_EqUp* c) { }
    virtual void visit(Code_EqPlus2* c) { }
    virtual void visit(Code_EqMinus2* c) { }
    virtual void visit(Code_EqMin* c) { }
    virtual void visit(Code_EqMax* c) { }
    virtual void visit(Code_EqMinus1* c) { }
    virtual void visit(Code_EqPlus1* c) { }
    virtual void visit(Code_EqPresbConst* c) { }

    // < Code_nnn Derives >
    virtual void visit(Code_EqUnion* c) { }
    virtual void visit(Code_EqInter* c) { }
    virtual void visit(Code_EqSetMinus* c) { }
    virtual void visit(Code_EqPlusModulo* c) { }
    virtual void visit(Code_EqMinusModulo* c) { }

    // < Code_c Derives >
    virtual void visit(Code_Restrict* c) { }
    virtual void visit(Code_Project* c) { }
    virtual void visit(Code_Negate* c) { }
    virtual void visit(Code_Prefix* c) { }
    virtual void visit(Code_PredCall* c) { }
    virtual void visit(Code_Import* c) { }
    virtual void visit(Code_Export* c) { }

    // < Code_cc Derives >
    virtual void visit(Code_And* c) { }
    virtual void visit(Code_IdLeft* c) { }
    virtual void visit(Code_Or* c) { }
    virtual void visit(Code_Impl* c) { }
    virtual void visit(Code_Biimpl* c) { }
};

class TransformerCodeVisitor : public CodeVisitor<Code*> {
public:
    TransformerCodeVisitor(CodeVisitor::Traverse td) : CodeVisitor<Code*>(td) {}

    virtual Code* visit(Code* c) { return c; }

    // < Code Derives >
    virtual Code* visit(Code_n* c) { return c;}
    virtual Code* visit(Code_ni* c) { return c; }
    virtual Code* visit(Code_nn* c) { return c; }
    virtual Code* visit(Code_nni* c) { return c; }
    virtual Code* visit(Code_nnn* c) { return c; }
    virtual Code* visit(Code_c* c) { return c; }
    virtual Code* visit(Code_cc* c) { return c; }
    // TODO: do we need it? virtual Code* visit(Code_clist* c) { return c; }

    // < Code Derives - specific >
    virtual Code* visit(Code_True* c) { return c; }
    virtual Code* visit(Code_False* c) { return c; }

    // < Code_n(i) Derives >
    virtual Code* visit(Code_EqEmpty* c) { return c; }
    virtual Code* visit(Code_EqRoot* c) { return c; }
    virtual Code* visit(Code_FirstOrder* c) { return c; }
    virtual Code* visit(Code_Singleton* c) { return c; }
    virtual Code* visit(Code_BoolVar* c) { return c; }
    virtual Code* visit(Code_InStateSpace* c) { return c; }
    virtual Code* visit(Code_SomeType* c) { return c; }
    virtual Code* visit(Code_EqConst* c) { return c; }
    virtual Code* visit(Code_WellFormedTree* c) { return c; }

    // < Code_nn(i) Derives >
    virtual Code* visit(Code_In* c) { return c; }
    virtual Code* visit(Code_Eq1* c) { return c; }
    virtual Code* visit(Code_Eq2* c) { return c; }
    virtual Code* visit(Code_Sub2* c) { return c; }
    virtual Code* visit(Code_Less1* c) { return c; }
    virtual Code* visit(Code_LessEq1* c) { return c; }
    virtual Code* visit(Code_EqDot0* c) { return c; }
    virtual Code* visit(Code_EqDot1* c) { return c; }
    virtual Code* visit(Code_EqUp* c) { return c; }
    virtual Code* visit(Code_EqPlus2* c) { return c; }
    virtual Code* visit(Code_EqMinus2* c) { return c; }
    virtual Code* visit(Code_EqMin* c) { return c; }
    virtual Code* visit(Code_EqMax* c) { return c; }
    virtual Code* visit(Code_EqMinus1* c) { return c; }
    virtual Code* visit(Code_EqPlus1* c) { return c; }
    virtual Code* visit(Code_EqPresbConst* c) { return c; }

    // < Code_nnn Derives >
    virtual Code* visit(Code_EqUnion* c) { return c; }
    virtual Code* visit(Code_EqInter* c) { return c; }
    virtual Code* visit(Code_EqSetMinus* c) { return c; }
    virtual Code* visit(Code_EqPlusModulo* c) { return c; }
    virtual Code* visit(Code_EqMinusModulo* c) { return c; }

    // < Code_c Derives >
    virtual Code* visit(Code_Restrict* c) { return c; }
    virtual Code* visit(Code_Project* c) { return c; }
    virtual Code* visit(Code_Negate* c) { return c; }
    virtual Code* visit(Code_Prefix* c) { return c; }
    virtual Code* visit(Code_PredCall* c) { return c; }
    virtual Code* visit(Code_Import* c) { return c; }
    virtual Code* visit(Code_Export* c) { return c; }

    // < Code_cc Derives >
    virtual Code* visit(Code_And* c) { return c; }
    virtual Code* visit(Code_IdLeft* c) { return c; }
    virtual Code* visit(Code_Or* c) { return c; }
    virtual Code* visit(Code_Impl* c) { return c; }
    virtual Code* visit(Code_Biimpl* c) { return c; }
};

// < Transformer Visitor >

#endif //WSKS_CODE_VISITOR_H
