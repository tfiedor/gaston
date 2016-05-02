//
// Created by Raph on 02/05/2016.
//

#ifndef WSKS_UNDERCOMPLEMENTDECORATOR_H
#define WSKS_UNDERCOMPLEMENTDECORATOR_H

#include "../../../Frontend/ast.h"
#include "../../../Frontend/ast_visitor.h"

class UnderComplementDecorator : public VoidVisitor {
public:
    UnderComplementDecorator() : VoidVisitor(Traverse::PreOrder) {}

    void visit(ASTForm* form) {};
    void visit(ASTTerm* term) {};
    void visit(ASTUniv* univ) {};

    void visit(ASTForm_And*);
    void visit(ASTForm_Or*);
    void visit(ASTForm_Impl*);
    void visit(ASTForm_Biimpl*);

    void visit(ASTForm_Not*);
    void visit(ASTForm_Ex1*);
    void visit(ASTForm_Ex2*);
    void visit(ASTForm_All1*);
    void visit(ASTForm_All2*);
};


#endif //WSKS_UNDERCOMPLEMENTDECORATOR_H
