/*****************************************************************************
 *  dWiNA - Deciding WSkS using non-deterministic automata
 *
 *  Copyright (c) 2015  Tomas Fiedor <ifiedortom@fit.vutbr.cz>
 *
 *  Description:
 *    Traverses the tree and unfolds negations, i.e. tries to fold them as
 *    deeply as possible to terms of logic. This way the formula is as simple
 *    as possible.
 *
 *****************************************************************************/

#ifndef WSKS_NEGATIONUNFOLDER_H
#define WSKS_NEGATIONUNFOLDER_H

#include "../Frontend/ast.h"
#include "../Frontend/ast_visitor.h"

class NegationUnfolder : public TransformerVisitor {
public:
    NegationUnfolder() : TransformerVisitor(Traverse::PostOrder) {}
    AST* visit(ASTForm_Not* form);
};


#endif //WSKS_NEGATIONUNFOLDER_H
