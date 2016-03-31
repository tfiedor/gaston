#ifndef __WSKS_AST_VISITOR_H
#define __WSKS_AST_VISITOR_H

#include "ast.h"

template <typename R = void>
class ASTVisitor {
public:
    enum Traverse {PreOrder, PostOrder, InOrder};
    const Traverse traverseDirection;

    // < Constructors >
    ASTVisitor(Traverse tD) : traverseDirection(tD) {}

    typedef R ReturnType;

    virtual ReturnType visit(ASTForm *e) { };
    virtual ReturnType visit(ASTTerm *e) { };
    virtual ReturnType visit(ASTUniv *e) { };
};

/**
 * An implementation of visitor pattern for traversing and modifing
 * of the structure of AST. Implements traversals for basic node
 */
class TransformerVisitor : public ASTVisitor<AST*> {
public:
    // < Constructors >
    TransformerVisitor(Traverse tD) : ASTVisitor<AST*>(tD) {}

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

    // < ASTTerm1 Specific > //
    virtual AST* visit(ASTTerm1_Var1* term) { return term; }
    virtual AST* visit(ASTTerm1_Dot* term) { return term; }
    virtual AST* visit(ASTTerm1_Up* term) { return term; }
    virtual AST* visit(ASTTerm1_Root* term) { return term; }
    virtual AST* visit(ASTTerm1_Int* term) { return term; }
    virtual AST* visit(ASTTerm1_Plus* term) { return term; }
    virtual AST* visit(ASTTerm1_Minus* term) { return term; }
    virtual AST* visit(ASTTerm1_PlusModulo* term) { return term; }
    virtual AST* visit(ASTTerm1_MinusModulo* term) { return term; }
    virtual AST* visit(ASTTerm1_Min* term) { return term; }
    virtual AST* visit(ASTTerm1_Max* term) { return term; }
    virtual AST* visit(ASTTerm1_TreeRoot* term) { return term; }

    // < ASTTerm2 Specific > //
    virtual AST* visit(ASTTerm2_Var2* Term) { return Term; }
    virtual AST* visit(ASTTerm2_VarTree* Term) { return Term; }
    virtual AST* visit(ASTTerm2_Dot* Term) { return Term; }
    virtual AST* visit(ASTTerm2_Up* Term) { return Term; }
    virtual AST* visit(ASTTerm2_Empty* Term) { return Term; }
    virtual AST* visit(ASTTerm2_Union* Term) { return Term; }
    virtual AST* visit(ASTTerm2_Inter* Term) { return Term; }
    virtual AST* visit(ASTTerm2_Setminus* Term) { return Term; }
    virtual AST* visit(ASTTerm2_Set* Term) { return Term; }
    virtual AST* visit(ASTTerm2_Plus* Term) { return Term; }
    virtual AST* visit(ASTTerm2_Minus* Term) { return Term; }
    virtual AST* visit(ASTTerm2_Interval* Term) { return Term; }
    virtual AST* visit(ASTTerm2_PresbConst* Term) { return Term; }
    virtual AST* visit(ASTTerm2_Formula* Term) { return Term; }

    // < ASTForm Specific > //
    virtual AST* visit(ASTForm_Var0* form) { return form; }
    virtual AST* visit(ASTForm_AllPosVar* form) { return form; }
    virtual AST* visit(ASTForm_True* form) { return form; }
    virtual AST* visit(ASTForm_False* form) { return form; }
    virtual AST* visit(ASTForm_In* form) { return form; }
    virtual AST* visit(ASTForm_Notin* form) { return form; }
    virtual AST* visit(ASTForm_RootPred* form) { return form; }
    virtual AST* visit(ASTForm_EmptyPred* form) { return form; }
    virtual AST* visit(ASTForm_FirstOrder* form) { return form; }
    virtual AST* visit(ASTForm_Sub* form) { return form; }
    virtual AST* visit(ASTForm_Equal1* form) { return form; }
    virtual AST* visit(ASTForm_Equal2* form) { return form; }
    virtual AST* visit(ASTForm_NotEqual1* form) { return form; }
    virtual AST* visit(ASTForm_NotEqual2* form) { return form; }
    virtual AST* visit(ASTForm_Less* form) { return form; }
    virtual AST* visit(ASTForm_LessEq* form) { return form; }
    virtual AST* visit(ASTForm_WellFormedTree* form) { return form; }
    virtual AST* visit(ASTForm_Impl* form) { return form; }
    virtual AST* visit(ASTForm_Biimpl* form) { return form; }
    virtual AST* visit(ASTForm_And* form) { return form; }
    virtual AST* visit(ASTForm_IdLeft* form) { return form; }
    virtual AST* visit(ASTForm_Or* form) { return form; }
    virtual AST* visit(ASTForm_Not* form) { return form; }
    virtual AST* visit(ASTForm_Ex0* form) { return form; }
    virtual AST* visit(ASTForm_Ex1* form) { return form; }
    virtual AST* visit(ASTForm_Ex2* form) { return form; }
    virtual AST* visit(ASTForm_All0* form) { return form; }
    virtual AST* visit(ASTForm_All1* form) { return form; }
    virtual AST* visit(ASTForm_All2* form) { return form; }
    virtual AST* visit(ASTForm_Let0* form) { return form; }
    virtual AST* visit(ASTForm_Let1* form) { return form; }
    virtual AST* visit(ASTForm_Let2* form) { return form; }
    virtual AST* visit(ASTForm_Call* form) { return form; }
    virtual AST* visit(ASTForm_Import* form) { return form; }
    virtual AST* visit(ASTForm_Export* form) { return form; }
    virtual AST* visit(ASTForm_Prefix* form) { return form; }
    virtual AST* visit(ASTForm_Restrict* form) { return form; }
    virtual AST* visit(ASTForm_InStateSpace1* form) { return form; }
    virtual AST* visit(ASTForm_InStateSpace2* form) { return form; }
    virtual AST* visit(ASTForm_SomeType* form) { return form;}
};

/**
 * An implementation of visitor pattern for traversing without
 * the modification of the structure of AST. Implements traversals
 * for basic types of the node
 */
class VoidVisitor : public ASTVisitor<> {
public:
    // < Constructors >
    VoidVisitor(Traverse td) : ASTVisitor<>(td) {}

    virtual void visit(ASTTerm* term) {}
    virtual void visit(ASTForm* form) {}
    virtual void visit(ASTUniv* univ) {}

    // < ASTTerm1 Derives > //
    virtual void visit(ASTTerm1_n* term) {}
    virtual void visit(ASTTerm1_T* term) {}
    virtual void visit(ASTTerm1_t* term) {}
    virtual void visit(ASTTerm1_tn* term) {}
    virtual void visit(ASTTerm1_tnt* term) {}

    // < ASTTerm2 Derives > //
    virtual void visit(ASTTerm2_TT* Term) {}
    virtual void visit(ASTTerm2_Tn* Term) {}

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

    // < ASTTerm1 Specific > //
    virtual void visit(ASTTerm1_Var1* term) {}
    virtual void visit(ASTTerm1_Dot* term) {}
    virtual void visit(ASTTerm1_Up* term) {}
    virtual void visit(ASTTerm1_Root* term) {}
    virtual void visit(ASTTerm1_Int* term) {}
    virtual void visit(ASTTerm1_Plus* term) {}
    virtual void visit(ASTTerm1_Minus* term) {}
    virtual void visit(ASTTerm1_PlusModulo* term) {}
    virtual void visit(ASTTerm1_MinusModulo* term) {}
    virtual void visit(ASTTerm1_Min* term) {}
    virtual void visit(ASTTerm1_Max* term) {}
    virtual void visit(ASTTerm1_TreeRoot* term) {}

    // < ASTTerm2 Specific > //
    virtual void visit(ASTTerm2_Var2* Term) {}
    virtual void visit(ASTTerm2_VarTree* Term) {}
    virtual void visit(ASTTerm2_Dot* Term) {}
    virtual void visit(ASTTerm2_Up* Term) {}
    virtual void visit(ASTTerm2_Empty* Term) {}
    virtual void visit(ASTTerm2_Union* Term) {}
    virtual void visit(ASTTerm2_Inter* Term) {}
    virtual void visit(ASTTerm2_Setminus* Term) {}
    virtual void visit(ASTTerm2_Set* Term) {}
    virtual void visit(ASTTerm2_Plus* Term) {}
    virtual void visit(ASTTerm2_Minus* Term) {}
    virtual void visit(ASTTerm2_Interval* Term) {}
    virtual void visit(ASTTerm2_PresbConst* Term) {}
    virtual void visit(ASTTerm2_Formula* Term) {}

    // < ASTForm Specific > //
    virtual void visit(ASTForm_Var0* form) {}
    virtual void visit(ASTForm_AllPosVar* form) {}
    virtual void visit(ASTForm_True* form) {}
    virtual void visit(ASTForm_False* form) {}
    virtual void visit(ASTForm_In* form) {}
    virtual void visit(ASTForm_Notin* form) {}
    virtual void visit(ASTForm_RootPred* form) {}
    virtual void visit(ASTForm_EmptyPred* form) {}
    virtual void visit(ASTForm_FirstOrder* form) {}
    virtual void visit(ASTForm_Sub* form) {}
    virtual void visit(ASTForm_Equal1* form) {}
    virtual void visit(ASTForm_Equal2* form) {}
    virtual void visit(ASTForm_NotEqual1* form) {}
    virtual void visit(ASTForm_NotEqual2* form) {}
    virtual void visit(ASTForm_Less* form) {}
    virtual void visit(ASTForm_LessEq* form) {}
    virtual void visit(ASTForm_WellFormedTree* form) {}
    virtual void visit(ASTForm_Impl* form) {}
    virtual void visit(ASTForm_Biimpl* form) {}
    virtual void visit(ASTForm_And* form) {}
    virtual void visit(ASTForm_IdLeft* form) {}
    virtual void visit(ASTForm_Or* form) {}
    virtual void visit(ASTForm_Not* form) {}
    virtual void visit(ASTForm_Ex0* form) {}
    virtual void visit(ASTForm_Ex1* form) {}
    virtual void visit(ASTForm_Ex2* form) {}
    virtual void visit(ASTForm_All0* form) {}
    virtual void visit(ASTForm_All1* form) {}
    virtual void visit(ASTForm_All2* form) {}
    virtual void visit(ASTForm_Let0* form) {}
    virtual void visit(ASTForm_Let1* form) {}
    virtual void visit(ASTForm_Let2* form) {}
    virtual void visit(ASTForm_Call* form) {}
    virtual void visit(ASTForm_Import* form) {}
    virtual void visit(ASTForm_Export* form) {}
    virtual void visit(ASTForm_Prefix* form) {}
    virtual void visit(ASTForm_Restrict* form) {}
    virtual void visit(ASTForm_InStateSpace1* form) {}
    virtual void visit(ASTForm_InStateSpace2* form) {}
    virtual void visit(ASTForm_SomeType* form) {}
};

#endif //WSKS_AST_VISITOR_H