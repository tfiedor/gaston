//
// Created by Raph on 14/01/2016.
//

#ifndef WSKS_BASEAUTOMATAMERGER_H
#define WSKS_BASEAUTOMATAMERGER_H

#include "../Frontend/ast.h"
#include "../Frontend/ast_visitor.h"

class BaseAutomataMerger : public TransformerVisitor {
public:
    BaseAutomataMerger() : TransformerVisitor(Traverse::PreOrder) {}

    // Postorder traversal
    // Assumption: are not Impl and Biimpls
    AST* visit(ASTForm_And* form);
    AST* visit(ASTForm_Or* form);
    AST* visit(ASTForm_Impl* form);
    AST* visit(ASTForm_Biimpl* form);

private:
    template<class BinopClass, ASTKind k>
    AST* _mergeBaseOnPath(BinopClass* binopForm);
    template<class BinopClass, ASTKind k>
    AST* _findMergePoint(BinopClass* binop);
};


#endif //WSKS_BASEAUTOMATAMERGER_H
