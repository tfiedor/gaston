//
// Created by Raph on 27/01/2016.
//

#ifndef WSKS_TAGGER_H
#define WSKS_TAGGER_H

#include "../../Frontend/ast.h"
#include "../../Frontend/ast_visitor.h"

class Tagger : public VoidVisitor{
private:
    size_t _lastTag = 1;

public:
    Tagger() : VoidVisitor(Traverse::PreOrder) {}

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


#endif //WSKS_TAGGER_H
