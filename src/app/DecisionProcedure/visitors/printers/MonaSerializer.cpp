//
// Created by Raph on 27/01/2016.
//

#include "MonaSerializer.h"
#include "../../environment.hh"
#include "../../../Frontend/ident.h"
#include "../../../Frontend/symboltable.h"

extern Ident allPosVar;
extern SymbolTable symbolTable;

MonaSerializer::MonaSerializer(std::string filename, ASTForm* form) {
    this->_monaFile.open(filename);
    if(!this->_monaFile.is_open()) {
        std::cerr << "[!] Unable to open '" << filename << "'\n";
        throw std::ios_base::failure("Unable to open file");
    }

    // Print the header
    if(allPosVar == -1) {
        this->_monaFile << "ws1s;\n";
    } else {
        this->_monaFile << "m2l-str;\n";
    }
    this->_monaFile << form->ToString(true) << ";\n";
}

MonaSerializer::~MonaSerializer() {
    this->_monaFile.close();
}

