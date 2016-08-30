//
// Created by Raph on 15/08/2016.
//

#ifndef WSKS_DERESTRICTER_H
#define WSKS_DERESTRICTER_H

#include "../../../Frontend/ast.h"
#include "../../../Frontend/ast_visitor.h"
#include "../../../Frontend/ident.h"

class Derestricter : public TransformerVisitor {
private:
    template<class BinaryForm>
    AST* _visitBinaryForm(BinaryForm*);
public:
    Derestricter() : TransformerVisitor(Traverse::PostOrder) {}

    AST* visit(ASTForm_And*);
    AST* visit(ASTForm_Or*);
    AST* visit(ASTForm_Impl*);
    AST* visit(ASTForm_Biimpl*);
    AST* visit(ASTForm_Ex2*);
};

class Defirstorderer : public TransformerVisitor {
private:
    template<class BinaryForm>
    AST* _visitBinaryForm(BinaryForm*);

public:
    Defirstorderer() : TransformerVisitor(Traverse::CustomOrder) {}

    AST* visit(ASTForm_And*);
    AST* visit(ASTForm_Or*);
    AST* visit(ASTForm_Impl*);
    AST* visit(ASTForm_Biimpl*);
    AST* visit(ASTForm_FirstOrder*);
};

#endif //WSKS_DERESTRICTER_H
