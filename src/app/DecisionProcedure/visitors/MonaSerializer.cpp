//
// Created by Raph on 27/01/2016.
//

#include "MonaSerializer.h"
#include "../environment.hh"
#include "../../Frontend/ident.h"
#include "../../Frontend/symboltable.h"

extern Ident allPosVar;
extern SymbolTable symbolTable;

MonaSerializer::MonaSerializer(std::string filename) : VoidVisitor(Traverse::InOrder) {
    this->_monaFile.open(filename);
    if(!this->_monaFile.is_open()) {
        std::err << "[!] Unable to open '" << filename << "'\n";
        throw std::ios_base::failure("Unable to open file");
    }

    // Print the header
    if(allPosVar == -1) {
        this->_monaFile << "ws1s;\n";
    } else {
        this->_monaFile << "m2l-str;\n";
    }
}

MonaSerializer::~MonaSerializer() {
    this->_monaFile << ";\n";
    this->_monaFile.close();
}

void MonaSerializer::visit(ASTForm_And* form) {
    this->_monaFile << " & ";
}

void MonaSerializer::visit(ASTForm_Or* form) {
    this->_monaFile << " | ";
}

void MonaSerializer::visit(ASTForm_Impl* form) {
    this->_monaFile << " => ";
}

void MonaSerializer::visit(ASTForm_Biimpl* form) {
    this->_monaFile << " <=> ";
}

void MonaSerializer::visit(ASTForm_Not* form) {
    this->_monaFile << "~";
}

template<class QuantifierClass>
void MonaSerializer::_quantifierToMona(std::string quantifier, QuantifierClass* form) {
    this->_monaFile << quantifier << " ";
    bool first = true;
    for(auto it = form->vl->begin(); it != form->vl->end(); ++it) {
        if(first) {
            first = false;
        } else {
            this->_monaFile << ", ";
        }
        this->_monaFile << symbolTable.lookupSymbol(*it);
    }
    this->_monaFile << ": ";
}

void MonaSerializer::visit(ASTForm_Ex1* form) {
    this->_quantifierToMona<ASTForm_Ex1>("ex1", form);
}

void MonaSerializer::visit(ASTForm_Ex2* form) {
    this->_quantifierToMona<ASTForm_Ex2>("ex2", form);
}

void MonaSerializer::visit(ASTForm_All1* form) {
    this->_quantifierToMona<ASTForm_All1>("all1", form);
}

void MonaSerializer::visit(ASTForm_All2* form) {
    this->_quantifierToMona<ASTForm_All2>("all2", form);
}

void MonaSerializer::_atomicToMona(ASTForm* form) {
    this->_monaFile << form->ToString();
}


void MonaSerializer::visit(ASTForm_True* form) {
    this->_atomicToMona(form);
}

void MonaSerializer::visit(ASTForm_False* form) {
    this->_atomicToMona(form);
}

void MonaSerializer::visit(ASTForm_In* form) {
    this->_atomicToMona(form);
}

void MonaSerializer::visit(ASTForm_Notin* form) {
    this->_atomicToMona(form);
}

void MonaSerializer::visit(ASTForm_RootPred* form) {
    this->_atomicToMona(form);
}

void MonaSerializer::visit(ASTForm_EmptyPred* form) {
    this->_atomicToMona(form);
}

void MonaSerializer::visit(ASTForm_FirstOrder* form) {
    this->_atomicToMona(form);
}

void MonaSerializer::visit(ASTForm_Sub* form) {
    this->_atomicToMona(form);
}

void MonaSerializer::visit(ASTForm_Equal1* form) {
    this->_atomicToMona(form);
}

void MonaSerializer::visit(ASTForm_Equal2* form) {
    this->_atomicToMona(form);
}

void MonaSerializer::visit(ASTForm_NotEqual1* form) {
    this->_atomicToMona(form);
}

void MonaSerializer::visit(ASTForm_NotEqual2* form) {
    this->_atomicToMona(form);
}

void MonaSerializer::visit(ASTForm_Less* form) {
    this->_atomicToMona(form);
}

void MonaSerializer::visit(ASTForm_LessEq* form) {
    this->_atomicToMona(form);
}
