//
// Created by Raph on 21/04/2016.
//

#ifndef WSKS_OCCURINGVARIABLEDECORATOR_H
#define WSKS_OCCURINGVARIABLEDECORATOR_H

#include "../../../Frontend/ast.h"
#include "../../../Frontend/ast_visitor.h"

class OccuringVariableDecorator : public VoidVisitor {
public:
    OccuringVariableDecorator() : VoidVisitor(Traverse::PostOrder) {}

    virtual void visit(ASTTerm* term);
    virtual void visit(ASTUniv* univ);

    // < ASTForm Derives > //
    virtual void visit(ASTForm_tT* form);
    virtual void visit(ASTForm_T* form);
    virtual void visit(ASTForm_TT* form);
    virtual void visit(ASTForm_tt* form);
    virtual void visit(ASTForm_nt* form);
    virtual void visit(ASTForm_nT* form);
    virtual void visit(ASTForm_f* form);
    virtual void visit(ASTForm_ff* form);
    virtual void visit(ASTForm_vf* form);
    virtual void visit(ASTForm_uvf* form);

    virtual void visit(ASTForm_FirstOrder* form);
    virtual void visit(ASTForm_Not* form);
    virtual void visit(ASTForm_True* form);
    virtual void visit(ASTForm_False* form);
private:
    template<class ASTNode>
    void _decorateUnaryNode(ASTNode*);
};


#endif //WSKS_OCCURINGVARIABLEDECORATOR_H
