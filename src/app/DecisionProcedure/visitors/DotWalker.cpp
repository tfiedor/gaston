//
// Created by Raph on 09/01/2016.
//

#include "DotWalker.h"
#include <ios>
#include <stdint.h>

DotWalker::DotWalker(std::string filename) : VoidVisitor(Traverse::PreOrder) {
    this->_dotFile.open(filename);
    if(!this->_dotFile.is_open()) {
        std::cerr << "[!] Unable to open '" << filename << "'\n";
        throw std::ios_base::failure("Unable to open file");
    }
    this->_dotFile << "graph " << "formulae" << " {\n";
}

DotWalker::~DotWalker() {
    this->_dotFile << "}\n";
    this->_dotFile.close();
}

void DotWalker::visit(ASTForm_And* form) {
    this->_dotFile << "\t" << (uintptr_t) form << " [label=\"\u2227\"];\n";
    this->_dotFile << "\t" << (uintptr_t) form << " -- " << (uintptr_t) form->f1 << ";\n";
    this->_dotFile << "\t" << (uintptr_t) form << " -- " << (uintptr_t) form->f2 << ";\n";
}

void DotWalker::visit(ASTForm_Or* form) {
    this->_dotFile << "\t" << (uintptr_t) form << " [label=\"\u2228\"];\n";
    this->_dotFile << "\t" << (uintptr_t) form << " -- " << (uintptr_t) form->f1 << ";\n";
    this->_dotFile << "\t" << (uintptr_t) form << " -- " << (uintptr_t) form->f2 << ";\n";
}

void DotWalker::visit(ASTForm_Impl* form) {
    this->_dotFile << "\t" << (uintptr_t) form << " [label=\"\u21D2\"];\n";
    this->_dotFile << "\t" << (uintptr_t) form << " -- " << (uintptr_t) form->f1 << ";\n";
    this->_dotFile << "\t" << (uintptr_t) form << " -- " << (uintptr_t) form->f2 << ";\n";
}

void DotWalker::visit(ASTForm_Biimpl* form) {
    this->_dotFile << "\t" << (uintptr_t) form << " [label=\"\u21D4\"];\n";
    this->_dotFile << "\t" << (uintptr_t) form << " -- " << (uintptr_t) form->f1 << ";\n";
    this->_dotFile << "\t" << (uintptr_t) form << " -- " << (uintptr_t) form->f2 << ";\n";
}

void DotWalker::visit(ASTForm_Not* form) {
    this->_dotFile << "\t" << (uintptr_t) form << " [label=\"\u00AC\"];\n";
    this->_dotFile << "\t" << (uintptr_t) form << " -- " << (uintptr_t) form->f << ";\n";
}

template<class ExistClass>
void DotWalker::_existsToDot(ExistClass* form) {
    this->_dotFile << "\t" << (uintptr_t) form << " [label=\"\u2203";
    for(auto it = form->vl->begin(); it != form->vl->end(); ++it) {
        this->_dotFile << (*it) << ", ";
    }
    this->_dotFile << "\"];\n";
    this->_dotFile << "\t" << (uintptr_t) form << " -- " << (uintptr_t) form->f << ";\n";
}

void DotWalker::visit(ASTForm_Ex1* form) {
    this->_existsToDot<ASTForm_Ex1>(form);
}
void DotWalker::visit(ASTForm_Ex2* form) {
    this->_existsToDot<ASTForm_Ex2>(form);
}

template<class ForallClass>
void DotWalker::_forallToDot(ForallClass* form) {
    this->_dotFile << "\t" << (uintptr_t) form << " [label=\"\u2200";
    for(auto it = form->vl->begin(); it != form->vl->end(); ++it) {
        this->_dotFile << (*it) << ", ";
    }
    this->_dotFile << "\"];\n";
    this->_dotFile << "\t" << (uintptr_t) form << " -- " << (uintptr_t) form->f << ";\n";
}

void DotWalker::visit(ASTForm_All1* form) {
    this->_forallToDot<ASTForm_All1>(form);
}

void DotWalker::visit(ASTForm_All2* form) {
    this->_forallToDot<ASTForm_All2>(form);
}

void DotWalker::_atomicToDot(ASTForm* form) {
    this->_dotFile << "\t" << (uintptr_t) form << " [label=\"atomic\"];\n";
}

void DotWalker::visit(ASTForm_tT* form) {
    this->_atomicToDot(form);
}

void DotWalker::visit(ASTForm_tt* form) {
    this->_atomicToDot(form);
}

void DotWalker::visit(ASTForm_TT* form) {
    this->_atomicToDot(form);
}

void DotWalker::visit(ASTForm_T* form) {
    this->_atomicToDot(form);
}

void DotWalker::visit(ASTForm_nt* form) {
    this->_atomicToDot(form);
}

void DotWalker::visit(ASTForm_nT* form) {
    this->_atomicToDot(form);
}