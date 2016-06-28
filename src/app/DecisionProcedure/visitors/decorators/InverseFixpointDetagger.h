//
// Created by Raph on 28/06/2016.
//

#ifndef WSKS_INVERSEFIXPOINTDETAGGER_H
#define WSKS_INVERSEFIXPOINTDETAGGER_H

#include "../../../Frontend/ast.h"
#include "../../../Frontend/ast_visitor.h"

class InverseFixpointDetagger : public VoidVisitor {
private:
    const size_t _cFixpointThreshold;    // < Everything with less than _cFixpointThreshold fixpoint computations will be converted to automaton

    template<class FixpointFormula>
    void _visitFixpointComputation(FixpointFormula*);
public:
    InverseFixpointDetagger(size_t fixpointLimit = 0) : VoidVisitor(Traverse::PreOrder), _cFixpointThreshold(fixpointLimit) {}

    void visit(ASTForm* form);
    void visit(ASTTerm* term) {};
    void visit(ASTUniv* univ) {};

    void visit(ASTForm_ff*);

    void visit(ASTForm_Not*);
    void visit(ASTForm_Ex1*);
    void visit(ASTForm_Ex2*);
    void visit(ASTForm_All1*);
    void visit(ASTForm_All2*);
};


#endif //WSKS_INVERSEFIXPOINTDETAGGER_H
