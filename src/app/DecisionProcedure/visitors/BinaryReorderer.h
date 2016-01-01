/*****************************************************************************
 *  dWiNA - Deciding WSkS using non-deterministic automata
 *
 *  Copyright (c) 2015  Tomas Fiedor <ifiedortom1@fit.vutbr.cz>
 *
 *  Description:
 *    Visitor for doing the anti-prenexing. This means instead of pushing
 *    the quantifiers higher to root, we push them deeper towards leaves.
 *    We do this if we see that some variable is not bound in the formula.
 *    Thus we can push the quantifier to the lhs or rhs.
 *
 *****************************************************************************/

#ifndef WSKS_BINARYREORDERER_H
#define WSKS_BINARYREORDERER_H


#include "../Frontend/ast.h"
#include "../Frontend/ast_visitor.h"

class BinaryReorderer : public TransformerVisitor {
public:
    BinaryReorderer() : TransformerVisitor(Traverse::PreOrder) {}
    // Postorder traversal
    // Assumption: are not Impl and Biimpls
    AST* visit(ASTForm_And* form);
    AST* visit(ASTForm_Or* form);
    AST* visit(ASTForm_Impl* form);
    AST* visit(ASTForm_Biimpl* form);
};


#endif //WSKS_BINARYREORDERER_H
