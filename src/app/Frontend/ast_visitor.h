#ifndef WSKS_AST_VISITOR_H
#define WSKS_AST_VISITOR_H

#include "ast.h"

template <typename R>
class ASTVisitor {
public:
    typedef R ReturnType;
    virtual ReturnType visit(ASTForm *e) {};
    virtual ReturnType visit(ASTTerm *e) {};
    virtual ReturnType visit(ASTUniv *e) {};

    virtual ReturnType visit(ASTForm_And *e) {};
    virtual ReturnType visit(ASTForm_Or *e) {};

    virtual ReturnType visit(ASTForm_True *e) {};
};

void ASTForm_And::accept(ASTVisitor<> &v) {
    v.visit(this);
    this->f1->accept(v);
    this->f2->accept(v);
}

AST* ASTForm_And::accept(ASTVisitor<AST*> &v) {
    this->f1 = (ASTForm*) (this->f1)->accept(v);
    this->f2 = (ASTForm*) (this->f2)->accept(v);
    this->dump();
    std::cout << "\n";
    return v.visit(this);
}

void ASTForm_Or::accept(ASTVisitor<> &v) {
    v.visit(this);
    this->f1->accept(v);
    this->f2->accept(v);
}

AST* ASTForm_Or::accept(ASTVisitor<AST*> &v) {
    this->f1 = (ASTForm*) (this->f1)->accept(v);
    this->f2 = (ASTForm*) (this->f2)->accept(v);
    this->dump();
    std::cout << "\n";
    return v.visit(this);
}

void ASTForm::accept(ASTVisitor<> &v) {
    v.visit(this);
}

AST* ASTForm::accept(ASTVisitor<AST*> &v) {
    return v.visit(this);
}

void ASTTerm::accept(ASTVisitor<> &v) {
    v.visit(this);
}

AST* ASTTerm::accept(ASTVisitor<AST*> &v) {
    return v.visit(this);
}

void ASTUniv::accept(ASTVisitor<> &v) {
    v.visit(this);
}

AST* ASTUniv::accept(ASTVisitor<AST*> &v) {
    return v.visit(this);
}

void ASTForm_True::accept(ASTVisitor<> &v) {
    v.visit(this);
}

AST* ASTForm_True::accept(ASTVisitor<AST*> &v) {
    return v.visit(this);
}

class PrettyPrintVisitor : public ASTVisitor<> {
public:
    void visit(ASTForm_True *e) {
        std::cout << "Visiting True\n";
    }

    void visit(ASTForm *e) {
        std::cout << "Visiting ASTForm\n";
    }

    void visit(ASTTerm *e) {
        std::cout << "Visiting ASTTerm\n";
    }

    void visit(ASTUniv *e) {
        std::cout << "Visiting ASTUniv\n";
    }
};

class NegateTruthVisitor : public ASTVisitor<AST*> {
public:
    AST* visit(AST *e) {
        return e;
    }

    AST* visit(ASTForm_And *e) {
        return e;
    }

    AST* visit(ASTForm_True *e) {
        std::cout << "> Visiting True\n";
        return new ASTForm_False(Pos());
    }
};

#endif //WSKS_AST_VISITOR_H
