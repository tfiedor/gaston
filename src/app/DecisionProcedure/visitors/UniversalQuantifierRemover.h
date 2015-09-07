/*****************************************************************************
 *  dWiNA - Deciding WSkS using non-deterministic automata
 *
 *  Copyright (c) 2015  Tomas Fiedor <xfiedo01@stud.fit.vutbr.cz>
 *
 *  Description:
 *    Visitor for removing the universal quantifier
 *
 *****************************************************************************/

#ifndef WSKS_UNIVERSALQUANTIFIERREMOVER_H
#define WSKS_UNIVERSALQUANTIFIERREMOVER_H

#include "../Frontend/ast.h"
#include "../Frontend/ast_visitor.h"

class UniversalQuantifierRemover : public TransformerVisitor {
public:
    UniversalQuantifierRemover() : TransformerVisitor(Traverse::PostOrder) {}

    AST* visit(ASTForm_All0* form);
    AST* visit(ASTForm_All1* form);
    AST* visit(ASTForm_All2* form);
};


#endif //WSKS_UNIVERSALQUANTIFIERREMOVER_H
