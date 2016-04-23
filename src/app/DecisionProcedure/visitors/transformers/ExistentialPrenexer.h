//
// Created by Raph on 17/01/2016.
//

#ifndef WSKS_EXISTENTIALPRENEXER_H
#define WSKS_EXISTENTIALPRENEXER_H

#include "../../../Frontend/ast.h"
#include "../../../Frontend/ast_visitor.h"

class ExistentialPrenexer : public TransformerVisitor {
public:
    ExistentialPrenexer() : TransformerVisitor(Traverse::PostOrder) {}

    template<class BinopClass>
    AST* moveExistUp(AST* form);

    AST* visit(ASTForm_And* form);
    AST* visit(ASTForm_Or* form);
};


#endif //WSKS_EXISTENTIALPRENEXER_H
