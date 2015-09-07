/*****************************************************************************
 *  dWiNA - Deciding WSkS using non-deterministic automata
 *
 *  Copyright (c) 2015  Tomas Fiedor <xfiedo01@stud.fit.vutbr.cz>
 *
 *  Description:
 *    Visitor for unfolding the true and false from the formula
 *
 *  Implements application of various boolean laws:
 *      True or phi     = True
 *      True and phi    = phi
 *      False or phi    = phi
 *      False and phi   = False
 *      not True        = False
 *      not False       = True
 *      False => phi    = True
 *      True => phi     = phi
 *      phi => True     = True
 *      phi=> False     = not phi
 *      True <=> phi    = phi
 *      False <=> phi   = not phi
 *
 *****************************************************************************/

#ifndef WSKS_BOOLEANUNFOLDER_H
#define WSKS_BOOLEANUNFOLDER_H

#include "../Frontend/ast_visitor.h"
#include "../Frontend/ast.h"

class BooleanUnfolder : public TransformerVisitor {
public:
    BooleanUnfolder() : TransformerVisitor(Traverse::PostOrder) {}
    AST* visit(ASTForm_And* form);
    AST* visit(ASTForm_Or* form);
    AST* visit(ASTForm_Not* form);
    AST* visit(ASTForm_Impl* form);
    AST* visit(ASTForm_Biimpl* form);
};


#endif //WSKS_BOOLEANUNFOLDER_H
