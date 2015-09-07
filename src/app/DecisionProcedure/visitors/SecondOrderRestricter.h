/*****************************************************************************
 *  dWiNA - Deciding WSkS using non-deterministic automata
 *
 *  Copyright (c) 2015  Tomas Fiedor <ifiedortom1@fit.vutbr.cz>
 *
 *  Description:
 *    Visitor for restricting the orders of the formulae to second order only
 *
 *****************************************************************************/

#ifndef WSKS_SECONDORDERRESTRICTER_H
#define WSKS_SECONDORDERRESTRICTER_H

#include "../Frontend/ast.h"
#include "../Frontend/ast_visitor.h"

class SecondOrderRestricter : public TransformerVisitor {
public:
    SecondOrderRestricter() : TransformerVisitor(Traverse::PostOrder) {}

    AST* visit(ASTForm_Ex1* form);
    AST* visit(ASTForm_All1* form);
    AST* visit(ASTForm_Equal1* form);
};


#endif //WSKS_SECONDORDERRESTRICTER_H
