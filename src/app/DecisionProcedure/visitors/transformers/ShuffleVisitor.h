//
// Created by Raph on 18/04/2016.
//

#ifndef WSKS_SHUFFLEVISITOR_H
#define WSKS_SHUFFLEVISITOR_H

#include <deque>
#include <vector>
#include "../../../Frontend/ast.h"
#include "../../../Frontend/ast_visitor.h"

using AST_ptr = AST*;
using LeafBuffer = std::vector<AST_ptr>;

class ShuffleVisitor  : public TransformerVisitor {
public:
    ShuffleVisitor() : TransformerVisitor(Traverse::CustomOrder) {}

    virtual AST* visit(ASTForm_Ex2* form);
    virtual AST* visit(ASTForm_Ex1* form);
    virtual AST* visit(ASTForm_All2* form);
    virtual AST* visit(ASTForm_All1* form);
    virtual AST* visit(ASTForm_Not* form);
    virtual AST* visit(ASTForm_And* form);
    virtual AST* visit(ASTForm_Or* form);
    virtual AST* visit(ASTForm_Impl* form);
    virtual AST* visit(ASTForm_Biimpl* form);
    virtual AST* visit(ASTForm_FirstOrder* form) { return static_cast<AST*>(form); }

private:
    template<class QuantifierClass>
    AST* _visitQuantifier(QuantifierClass* form);
    template<class BinopClass>
    AST* _visitBinary(BinopClass* form);
    void _CollectLeaves(AST*, ASTKind, LeafBuffer&);
    void _AddFormToBuffer(AST*&, LeafBuffer&);
    void _AddToBuffer(AST*&, LeafBuffer&);
};


#endif //WSKS_SHUFFLEVISITOR_H
