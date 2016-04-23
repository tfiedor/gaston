//
// Created by Raph on 18/01/2016.
//

#ifndef WSKS_ZEROORDERREMOVER_H
#define WSKS_ZEROORDERREMOVER_H

#include "../../../Frontend/ast.h"
#include "../../../Frontend/ast_visitor.h"

class ZeroOrderRemover : public TransformerVisitor {
public:
    ZeroOrderRemover() : TransformerVisitor(Traverse::PostOrder) {}

    AST* visit(ASTForm_Ex0* form);
    AST* visit(ASTForm_All0* form);
    AST* visit(ASTForm_Var0* form);
};


#endif //WSKS_ZEROORDERREMOVER_H
