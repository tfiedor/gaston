/*****************************************************************************
 *  dWiNA - Deciding WSkS using non-deterministic automata
 *
 *  Copyright (c) 2015  Tomas Fiedor <ifiedortom1@fit.vutbr.cz>
 *
 *  Description:
 *    Visitor for unfolding Calls, i.e. replaces the call by formula and extract
 *    the information.
 *
 *    TODO: think if we can omit this in our method
 *
 *****************************************************************************/

#ifndef WSKS_MACROUNFOLDER_H
#define WSKS_MACROUNFOLDER_H

#include "../Frontend/ast.h"
#include "../Frontend/ast_visitor.h"

class MacroUnfolder : public ASTTransformer {
private:
    // TODO: Switch to shared pointers?
    IdentList* formalParameters;
    ASTList* realParameters;

public:
    AST* visit(ASTForm_Var0* form);
    AST* visit(ASTTerm1_Var1* term);
    AST* visit(ASTTerm2_Var2* term);
    AST* visit(ASTForm_vf* form);
    AST* visit(ASTForm_uvf* form);
    AST* visit(ASTForm_Ex0* form);
    AST* visit(ASTForm_Ex1* form);
    AST* visit(ASTForm_Ex2* form);
    AST* visit(ASTForm_All0* form);
    AST* visit(ASTForm_All1* form);
    AST* visit(ASTForm_All2* form);
};


#endif //WSKS_MACROUNFOLDER_H
