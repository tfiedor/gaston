//
// Created by Raph on 29/01/2016.
//

#ifndef WSKS_FIXPOINTDETAGGER_H
#define WSKS_FIXPOINTDETAGGER_H

#include "../../../Frontend/ast.h"
#include "../../../Frontend/ast_visitor.h"

class FixpointDetagger : public VoidVisitor {
private:
    const size_t _cFixpointThreshold;    // < Everything with less than _cFixpointThreshold fixpoint computations will be converted to automaton

    template<class FixpointFormula>
    void _visitFixpointComputation(FixpointFormula*);
public:
    FixpointDetagger(size_t fixpoint_limit = 0) :
            VoidVisitor(Traverse::PostOrder), _cFixpointThreshold(fixpoint_limit) {}

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


#endif //WSKS_FIXPOINTDETAGGER_H
