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
    IdentList free, bound;
    form->freeVars(&free, &bound);
    for(auto it = free.begin(); it != free.end(); ++it) {
        if(*it == allPosVar)
            continue;
        switch(symbolTable.lookupType(*it)) {
            case MonaTypeTag::Varname1:
                this->_monaFile << "var1";
                break;
            case MonaTypeTag::Varname0:
            case MonaTypeTag::Varname2:
                this->_monaFile << "var2";
                break;
        }
        this->_monaFile << " " << symbolTable.lookupSymbol(*it) << ";\n";
    }
    this->_monaFile << form->ToString(true) << ";\n";
}

MonaSerializer::~MonaSerializer() {
    this->_monaFile.close();
}

