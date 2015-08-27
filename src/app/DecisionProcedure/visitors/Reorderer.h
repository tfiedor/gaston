/*****************************************************************************
 *  dWiNA - Deciding WSkS using non-deterministic automata
 *
 *  Copyright (c) 2015  Tomas Fiedor <ifiedortom1@fit.vutbr.cz>
 *
 *  Description:
 *    Visitor for doing the reordering of the formulae. This could be
 *    optimization for our future method, so simpler formulae are traversed
 *    first achieving better performance
 *
 *****************************************************************************/

#ifndef WSKS_REORDERER_H
#define WSKS_REORDERER_H

#include "../Frontend/ast.h"
#include "../Frontend/ast_visitor.h"

class Reorderer : public ASTTransformer {
    // Postorder traversal
    // Assumption: are not Impl and Biimpls
    AST* visit(ASTForm_ff* form);
    AST* visit(ASTForm_And* form);
    AST* visit(ASTForm_Or* form);
    AST* visit(ASTForm_Impl* form);
    AST* visit(ASTForm_Biimpl* form);
};


#endif //WSKS_REORDERER_H
