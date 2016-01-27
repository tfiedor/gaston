//
// Created by Raph on 09/01/2016.
//

#ifndef WSKS_DOTWALKER_H
#define WSKS_DOTWALKER_H

#include "../../Frontend/ast.h"
#include "../../Frontend/ast_visitor.h"
#include <iostream>
#include <fstream>
#include <string>

class DotWalker : public VoidVisitor {
public:
    DotWalker(std::string filename);
    ~DotWalker();

    void visit(ASTForm_And*);
    void visit(ASTForm_Or*);
    void visit(ASTForm_Impl*);
    void visit(ASTForm_Biimpl*);

    void visit(ASTForm_Not*);
    void visit(ASTForm_Ex1*);
    void visit(ASTForm_Ex2*);
    void visit(ASTForm_All1*);
    void visit(ASTForm_All2*);

    void visit(ASTForm_tT*);
    void visit(ASTForm_tt*);
    void visit(ASTForm_TT*);
    void visit(ASTForm_T*);
    void visit(ASTForm_nt*);
    void visit(ASTForm_nT*);

protected:
    std::ofstream _dotFile;

    void _atomicToDot(ASTForm*);
    template<class ExistClass>
    void _existsToDot(ExistClass*);
    template<class ForallClass>
    void _forallToDot(ForallClass*);
};


#endif //WSKS_DOTWALKER_H
