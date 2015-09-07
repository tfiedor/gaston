/*****************************************************************************
 *  dWiNA - Deciding WSkS using non-deterministic automata
 *
 *  Copyright (c) 2015  Tomas Fiedor <ifiedortom1@fit.vutbr.cz>
 *
 *  Description:
 *    Visitor for transforming the formula to Prenex Normal Form, i.e. in
 *    the form of:
 *      ex Xn: ... not ex X1: phi
 *
 *****************************************************************************/

#ifndef WSKS_PRENEXNORMALFORMTRANSFORMER_H
#define WSKS_PRENEXNORMALFORMTRANSFORMER_H

#include "../Frontend/ast.h"
#include "../Frontend/ast_visitor.h"

class PrenexNormalFormTransformer : public ASTTransformer {
public:
    PrenexNormalFormTransformer() : ASTTransformer(Traverse::PostOrder) {}

    AST* visit(ASTForm_ff* form);
    AST* visit(ASTForm_And* form) { return this->visit(static_cast<ASTForm_ff*>(form));}
    AST* visit(ASTForm_Or* form) { return this->visit(static_cast<ASTForm_ff*>(form));}
    AST* visit(ASTForm_Not* form);
};


#endif //WSKS_PRENEXNORMALFORMTRANSFORMER_H
