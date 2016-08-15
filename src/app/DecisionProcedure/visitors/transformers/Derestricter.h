//
// Created by Raph on 15/08/2016.
//

#ifndef WSKS_DERESTRICTER_H
#define WSKS_DERESTRICTER_H

#include "../../../Frontend/ast.h"
#include "../../../Frontend/ast_visitor.h"
#include "../../../Frontend/ident.h"

class Derestricter : public TransformerVisitor {
public:
    Derestricter() : TransformerVisitor(Traverse::PostOrder) {}

    AST* visit(ASTForm_ff*);
    AST* visit(ASTForm_Ex2*);
};


#endif //WSKS_DERESTRICTER_H
