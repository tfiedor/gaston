//
// Created by Raph on 27/01/2016.
//

#ifndef WSKS_MONAAUTOMATADOTWALKER_H
#define WSKS_MONAAUTOMATADOTWALKER_H

#include <string>
#include "DotWalker.h"

using TimeType = std::pair<size_t, size_t>;

class MonaAutomataDotWalker : public DotWalker {
public:
    explicit MonaAutomataDotWalker(std::string filename) : DotWalker(filename) {}

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

private:
    TimeType _constructAutomaton(ASTForm*);
    void _atomicToDot(ASTForm*);
    template<class ExistClass>
    void _existsToDot(ExistClass*);
    template<class ForallClass>
    void _forallToDot(ForallClass*);
};

#endif //WSKS_MONAAUTOMATADOTWALKER_H