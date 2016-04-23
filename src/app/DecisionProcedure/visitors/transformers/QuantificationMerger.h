//
// Created by Raph on 07/01/2016.
//

#ifndef WSKS_QUANTIFICATIONMERGER_H
#define WSKS_QUANTIFICATIONMERGER_H

#include "../../../Frontend/ast.h"
#include "../../../Frontend/ast_visitor.h"

class QuantificationMerger : public TransformerVisitor {
public:
    QuantificationMerger() : TransformerVisitor(Traverse::PostOrder) {}

    AST* visit(ASTForm_Ex1* form);
    AST* visit(ASTForm_Ex2* form);
    AST* visit(ASTForm_All1* form);
    AST* visit(ASTForm_All2* form);
};

#endif //WSKS_QUANTIFICATIONMERGER_H
