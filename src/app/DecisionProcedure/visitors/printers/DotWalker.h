//
// Created by Raph on 09/01/2016.
//

#ifndef WSKS_DOTWALKER_H
#define WSKS_DOTWALKER_H

#include "../../../Frontend/ast.h"
#include "../../../Frontend/ast_visitor.h"
#include <iostream>
#include <fstream>
#include <string>

class DotWalker : public VoidVisitor {
public:
    explicit DotWalker(std::string filename);
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

    // < ASTForm Specific > //
    void visit(ASTForm_True* form);
    void visit(ASTForm_False* form);
    void visit(ASTForm_In* form);
    void visit(ASTForm_Notin* form);
    void visit(ASTForm_RootPred* form);
    void visit(ASTForm_EmptyPred* form);
    void visit(ASTForm_FirstOrder* form);
    void visit(ASTForm_Sub* form);
    void visit(ASTForm_Equal1* form);
    void visit(ASTForm_Equal2* form);
    void visit(ASTForm_NotEqual1* form);
    void visit(ASTForm_NotEqual2* form);
    void visit(ASTForm_Less* form);
    void visit(ASTForm_LessEq* form);

protected:
    std::ofstream _dotFile;

    void _atomicToDot(ASTForm*);
    template<class ExistClass>
    void _existsToDot(ExistClass*);
    template<class ForallClass>
    void _forallToDot(ForallClass*);
};


#endif //WSKS_DOTWALKER_H
