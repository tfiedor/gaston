//
// Created by Raph on 28/04/2016.
//

#ifndef WSKS_CONTINUATIONSWITCHER_H
#define WSKS_CONTINUATIONSWITCHER_H

#include "../../../Frontend/ast.h"
#include "../../../Frontend/ast_visitor.h"

class ContinuationSwitcher : public TransformerVisitor {
public:
    ContinuationSwitcher() : TransformerVisitor(Traverse::PostOrder) {}

    AST* visit(ASTForm_ff*);
};


#endif //WSKS_CONTINUATIONSWITCHER_H
