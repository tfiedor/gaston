//
// Created by Raph on 30/01/2016.
//

#ifndef WSKS_PREDICATEUNFOLDER_H
#define WSKS_PREDICATEUNFOLDER_H

#include "../Frontend/ast.h"
#include "../Frontend/ast_visitor.h"
#include "../Frontend/env.h"
#include "../Frontend/predlib.h"
#include "../Frontend/symboltable.h"
#include "../environment.hh"

class PredicateUnfolder : public TransformerVisitor {
public:
    PredicateUnfolder() : TransformerVisitor(Traverse::PostOrder) {}

    AST* visit(ASTForm_Call* form);
};

#endif //WSKS_PREDICATEUNFOLDER_H
