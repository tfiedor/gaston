//
// Created by Raph on 28/04/2016.
//

#ifndef WSKS_CONTINUATIONSWITCHER_H
#define WSKS_CONTINUATIONSWITCHER_H

#include "../../../Frontend/ast.h"
#include "../../../Frontend/ast_visitor.h"

class ContinuationSwitcher : public VoidVisitor {
public:
    ContinuationSwitcher() : VoidVisitor(Traverse::PostOrder) {}

    virtual void visit(ASTTerm* form) { form->epsilon_in = false; }

    // < ASTForm Specific > //
    virtual void visit(ASTForm_True* form) { form->epsilon_in = !form->under_complement; }
    virtual void visit(ASTForm_False* form) { form->epsilon_in = form->under_complement; }
    virtual void visit(ASTForm_In* form) { form->epsilon_in = form->under_complement; }
    virtual void visit(ASTForm_Notin* form) { form->epsilon_in = form->under_complement; }
    virtual void visit(ASTForm_FirstOrder* form) { form->epsilon_in = form->under_complement; }
    virtual void visit(ASTForm_Sub* form) {
        bool has_epsilon = form->T1->kind == aVar2 && form->T2->kind == aVar2;
        form->epsilon_in = has_epsilon != form->under_complement;
    }
    virtual void visit(ASTForm_Equal1* form) { form->epsilon_in = form->under_complement; }
    virtual void visit(ASTForm_Equal2* form) {
        bool has_epsilon = form->T1->kind == aVar2 && form->T2->kind == aVar2;
        form->epsilon_in = has_epsilon != form->under_complement;
    }
    virtual void visit(ASTForm_NotEqual1* form) { form->epsilon_in = form->under_complement; }
    virtual void visit(ASTForm_NotEqual2* form) {
        bool has_epsilon = form->T1->kind == aVar2 && form->T2->kind == aVar2;
        form->epsilon_in = has_epsilon == form->under_complement;
    }
    virtual void visit(ASTForm_Less* form) { form->epsilon_in = form->under_complement;}
    virtual void visit(ASTForm_LessEq* form) { form->epsilon_in = form->under_complement; }

    virtual void visit(ASTForm_And* form);
    virtual void visit(ASTForm_Or* form);
    virtual void visit(ASTForm_Not* form);
    virtual void visit(ASTForm_Ex1* form);
    virtual void visit(ASTForm_Ex2* form);

private:
    template<class Binop>
    void _switch(Binop*);
};


#endif //WSKS_CONTINUATIONSWITCHER_H
