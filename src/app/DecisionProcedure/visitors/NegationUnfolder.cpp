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
#include "NegationUnfolder.h"

AST* NegationUnfolder::visit(ASTForm_Not* form) {
    // TODO: implement
    return form;
}