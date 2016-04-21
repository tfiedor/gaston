#ifndef __WSKS_AST_VISITOR_H
#define __WSKS_AST_VISITOR_H

#include "ast.h"
#include <iostream>

template <typename R = void>
class ASTVisitor {
public:
    enum Traverse {PreOrder, PostOrder, InOrder, CustomOrder};
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

    virtual void visit(ASTTerm* term) { std::cerr << "[!] Warning called base VoidVisitor visit(ASTTerm*) for: "; term->dump(); std::cerr<< "\n"; assert(false); }
    virtual void visit(ASTForm* form) { std::cerr << "[!] Warning called base VoidVisitor visit(ASTForm*) for: "; form->dump(); std::cerr<< "\n"; assert(false); }
    virtual void visit(ASTUniv* univ) { std::cerr << "[!] Warning called base VoidVisitor visit(ASTUniv*) for: "; univ->dump(); std::cerr<< "\n"; assert(false); }

    // < ASTTerm1 Derives > //
    virtual void visit(ASTTerm1_n* term) { this->visit(static_cast<ASTTerm*>(term)); }
    virtual void visit(ASTTerm1_T* term) { this->visit(static_cast<ASTTerm*>(term)); }
    virtual void visit(ASTTerm1_t* term) { this->visit(static_cast<ASTTerm*>(term)); }
    virtual void visit(ASTTerm1_tn* term) { this->visit(static_cast<ASTTerm*>(term)); }
    virtual void visit(ASTTerm1_tnt* term) { this->visit(static_cast<ASTTerm*>(term)); }

    // < ASTTerm2 Derives > //
    virtual void visit(ASTTerm2_TT* Term) { this->visit(static_cast<ASTTerm*>(Term)); }
    virtual void visit(ASTTerm2_Tn* Term) { this->visit(static_cast<ASTTerm*>(Term)); }

    // < ASTForm Derives > //
    virtual void visit(ASTForm_tT* form) { this->visit(static_cast<ASTForm*>(form)); }
    virtual void visit(ASTForm_T* form) { this->visit(static_cast<ASTForm*>(form)); }
    virtual void visit(ASTForm_TT* form) { this->visit(static_cast<ASTForm*>(form)); }
    virtual void visit(ASTForm_tt* form) { this->visit(static_cast<ASTForm*>(form)); }
    virtual void visit(ASTForm_nt* form) { this->visit(static_cast<ASTForm*>(form)); }
    virtual void visit(ASTForm_nT* form) { this->visit(static_cast<ASTForm*>(form)); }
    virtual void visit(ASTForm_f* form) { this->visit(static_cast<ASTForm*>(form)); }
    virtual void visit(ASTForm_ff* form) { this->visit(static_cast<ASTForm*>(form)); }
    virtual void visit(ASTForm_vf* form) { this->visit(static_cast<ASTForm*>(form)); }
    virtual void visit(ASTForm_uvf* form) { this->visit(static_cast<ASTForm*>(form)); }

    // < ASTTerm1 Specific > //
    virtual void visit(ASTTerm1_Var1* term) { this->visit(static_cast<ASTTerm1_n*>(term)); }
    virtual void visit(ASTTerm1_Dot* term) { this->visit(static_cast<ASTTerm1_t*>(term)); }
    virtual void visit(ASTTerm1_Up* term) { this->visit(static_cast<ASTTerm1_t*>(term)); }
    virtual void visit(ASTTerm1_Root* term) { this->visit(static_cast<ASTTerm1*>(term)); }
    virtual void visit(ASTTerm1_Int* term) { this->visit(static_cast<ASTTerm1_n*>(term)); }
    virtual void visit(ASTTerm1_Plus* term) { this->visit(static_cast<ASTTerm1_tn*>(term)); }
    virtual void visit(ASTTerm1_Minus* term) { this->visit(static_cast<ASTTerm1_tn*>(term)); }
    virtual void visit(ASTTerm1_PlusModulo* term) { this->visit(static_cast<ASTTerm1_tnt*>(term)); }
    virtual void visit(ASTTerm1_MinusModulo* term) { this->visit(static_cast<ASTTerm1_tnt*>(term)); }
    virtual void visit(ASTTerm1_Min* term) { this->visit(static_cast<ASTTerm1_T*>(term)); }
    virtual void visit(ASTTerm1_Max* term) { this->visit(static_cast<ASTTerm1_T*>(term)); }
    virtual void visit(ASTTerm1_TreeRoot* term) { this->visit(static_cast<ASTTerm1_T*>(term)); }

    // < ASTTerm2 Specific > //
    virtual void visit(ASTTerm2_Var2* Term) { this->visit(static_cast<ASTTerm2*>(Term)); }
    virtual void visit(ASTTerm2_VarTree* Term) { this->visit(static_cast<ASTTerm2*>(Term)); }
    virtual void visit(ASTTerm2_Dot* Term) { this->visit(static_cast<ASTTerm2*>(Term)); }
    virtual void visit(ASTTerm2_Up* Term) { this->visit(static_cast<ASTTerm2*>(Term)); }
    virtual void visit(ASTTerm2_Empty* Term) { this->visit(static_cast<ASTTerm2*>(Term)); }
    virtual void visit(ASTTerm2_Union* Term) { this->visit(static_cast<ASTTerm2_TT*>(Term)); }
    virtual void visit(ASTTerm2_Inter* Term) { this->visit(static_cast<ASTTerm2_TT*>(Term)); }
    virtual void visit(ASTTerm2_Setminus* Term) { this->visit(static_cast<ASTTerm2_TT*>(Term)); }
    virtual void visit(ASTTerm2_Set* Term) { this->visit(static_cast<ASTTerm2*>(Term)); }
    virtual void visit(ASTTerm2_Plus* Term) { this->visit(static_cast<ASTTerm2_Tn*>(Term)); }
    virtual void visit(ASTTerm2_Minus* Term) { this->visit(static_cast<ASTTerm2_Tn*>(Term)); }
    virtual void visit(ASTTerm2_Interval* Term) { this->visit(static_cast<ASTTerm2*>(Term)); }
    virtual void visit(ASTTerm2_PresbConst* Term) { this->visit(static_cast<ASTTerm2*>(Term)); }
    virtual void visit(ASTTerm2_Formula* Term) { this->visit(static_cast<ASTTerm2*>(Term)); }

    // < ASTForm Specific > //
    virtual void visit(ASTForm_Var0* form) { this->visit(static_cast<ASTForm*>(form)); }
    virtual void visit(ASTForm_AllPosVar* form) { this->visit(static_cast<ASTForm*>(form)); }
    virtual void visit(ASTForm_True* form) { this->visit(static_cast<ASTForm*>(form)); }
    virtual void visit(ASTForm_False* form) { this->visit(static_cast<ASTForm*>(form)); }
    virtual void visit(ASTForm_In* form) { this->visit(static_cast<ASTForm_tT*>(form)); }
    virtual void visit(ASTForm_Notin* form) { this->visit(static_cast<ASTForm_tT*>(form)); }
    virtual void visit(ASTForm_RootPred* form) { this->visit(static_cast<ASTForm*>(form)); }
    virtual void visit(ASTForm_EmptyPred* form) { this->visit(static_cast<ASTForm_T*>(form)); }
    virtual void visit(ASTForm_FirstOrder* form) { this->visit(static_cast<ASTForm*>(form)); }
    virtual void visit(ASTForm_Sub* form) { this->visit(static_cast<ASTForm_TT*>(form)); }
    virtual void visit(ASTForm_Equal1* form) { this->visit(static_cast<ASTForm_tt*>(form)); }
    virtual void visit(ASTForm_Equal2* form) { this->visit(static_cast<ASTForm_TT*>(form)); }
    virtual void visit(ASTForm_NotEqual1* form) { this->visit(static_cast<ASTForm_tt*>(form)); }
    virtual void visit(ASTForm_NotEqual2* form) { this->visit(static_cast<ASTForm_TT*>(form)); }
    virtual void visit(ASTForm_Less* form) { this->visit(static_cast<ASTForm_tt*>(form)); }
    virtual void visit(ASTForm_LessEq* form) { this->visit(static_cast<ASTForm_tt*>(form)); }
    virtual void visit(ASTForm_WellFormedTree* form) { this->visit(static_cast<ASTForm_T*>(form)); }
    virtual void visit(ASTForm_Impl* form) { this->visit(static_cast<ASTForm_ff*>(form)); }
    virtual void visit(ASTForm_Biimpl* form) { this->visit(static_cast<ASTForm_ff*>(form)); }
    virtual void visit(ASTForm_And* form) { this->visit(static_cast<ASTForm_ff*>(form));}
    virtual void visit(ASTForm_IdLeft* form) { this->visit(static_cast<ASTForm_ff*>(form)); }
    virtual void visit(ASTForm_Or* form) { this->visit(static_cast<ASTForm_ff*>(form));}
    virtual void visit(ASTForm_Not* form) { this->visit(static_cast<ASTForm*>(form)); }
    virtual void visit(ASTForm_Ex0* form) { this->visit(static_cast<ASTForm_vf*>(form)); }
    virtual void visit(ASTForm_Ex1* form) { this->visit(static_cast<ASTForm_uvf*>(form)); }
    virtual void visit(ASTForm_Ex2* form) { this->visit(static_cast<ASTForm_uvf*>(form)); }
    virtual void visit(ASTForm_All0* form) { this->visit(static_cast<ASTForm_vf*>(form)); }
    virtual void visit(ASTForm_All1* form) { this->visit(static_cast<ASTForm_uvf*>(form)); }
    virtual void visit(ASTForm_All2* form) { this->visit(static_cast<ASTForm_uvf*>(form)); }
    virtual void visit(ASTForm_Let0* form) { this->visit(static_cast<ASTForm*>(form)); }
    virtual void visit(ASTForm_Let1* form) { this->visit(static_cast<ASTForm*>(form)); }
    virtual void visit(ASTForm_Let2* form) { this->visit(static_cast<ASTForm*>(form)); }
    virtual void visit(ASTForm_Call* form) { this->visit(static_cast<ASTForm*>(form)); }
    virtual void visit(ASTForm_Import* form) { this->visit(static_cast<ASTForm*>(form)); }
    virtual void visit(ASTForm_Export* form) { this->visit(static_cast<ASTForm_f*>(form)); }
    virtual void visit(ASTForm_Prefix* form) { this->visit(static_cast<ASTForm_f*>(form)); }
    virtual void visit(ASTForm_Restrict* form) { this->visit(static_cast<ASTForm_f*>(form)); }
    virtual void visit(ASTForm_InStateSpace1* form) { this->visit(static_cast<ASTForm*>(form)); }
    virtual void visit(ASTForm_InStateSpace2* form) { this->visit(static_cast<ASTForm*>(form)); }
    virtual void visit(ASTForm_SomeType* form) { this->visit(static_cast<ASTForm*>(form)); }
};

#endif //WSKS_AST_VISITOR_H