/*****************************************************************************
 *  dWiNA - Deciding WSkS using non-deterministic automata
 *
 *  Copyright (c) 2015  Tomas Fiedor <ifiedortom1@fit.vutbr.cz>
 *
 *  Description:
 *    Visitor for flattening of the formula
 *
 *****************************************************************************/
#ifndef WSKS_FLATTENER_H
#define WSKS_FLATTENER_H

#include "../Frontend/ast.h"
#include "../Frontend/ast_visitor.h"

class Flattener : public ASTTransformer {
    AST* visit(ASTTerm1_Plus* term);
    AST* visit(ASTForm_Equal1* form);
    AST* visit(ASTForm_Equal2* form);
    AST* visit(ASTForm_NotEqual1* form);
    AST* visit(ASTForm_NotEqual2* form);
    AST* visit(ASTForm_Less* form);
    AST* visit(ASTForm_LessEq* form);
    AST* visit(ASTForm_In* form);
    AST* visit(ASTForm_Notin* form);
    AST* visit(ASTForm_Sub* form);
    AST* visit(ASTForm_Ex1* form);
    AST* visit(ASTForm_All1* form);
};


#endif //WSKS_FLATTENER_H
