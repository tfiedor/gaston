//
// Created by Raph on 11/04/2016.
//

#ifndef WSKS_MONASERIALIZER_H
#define WSKS_MONASERIALIZER_H


//
// Created by Raph on 27/01/2016.
//

#ifndef WSKS_MONAAUTOMATADOTWALKER_H
#define WSKS_MONAAUTOMATADOTWALKER_H

#include <fstream>
#include <string>
#include <iostream>
#include "../../Frontend/ast.h"
#include "../../Frontend/ast_visitor.h"

class MonaSerializer : public VoidVisitor {
public:
    explicit MonaSerializer(std::string filename);
    ~MonaSerializer();

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
    std::ofstream _monaFile;
    void _atomicToMona(ASTForm*);
    template<class QuantifierClass>
    void _quantifierToMona(std::string, QuantifierClass*);
};

#endif //WSKS_MONASERIALIZER_H
