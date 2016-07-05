//
// Created by Raph on 27/01/2016.
//

#include "MonaSerializer.h"
#include "../../environment.hh"
#include "../../../Frontend/env.h"
#include "../../../Frontend/ident.h"

extern Ident allPosVar;
extern SymbolTable symbolTable;
extern Options options;

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
        this->_serializeFreeVariable(*it, symbolTable.lookupType(*it));
    }
    if(options.test == TestType::UNSATISFIABILITY && false) {
        // Looks like i was wrong and this is not needed?
        this->_monaFile << "~(" << form->ToString(true) << ");\n";
    } else {
        this->_monaFile << form->ToString(true) << ";\n";
    }
}

void MonaSerializer::_serializeFreeVariable(Ident ident, MonaTypeTag type) {
    switch(options.test) {
        case TestType::EVERYTHING:
            this->_serializeAsFree(ident, type);
            break;
            // Is free
        case TestType::SATISFIABILITY:
        case TestType::UNSATISFIABILITY:
            this->_serializeAsBound(ident, type, "ex");
            break;
            // Existential
        case TestType::VALIDITY:
            this->_serializeAsBound(ident, type, "all");
            // Universal
        default:
            assert(false);
    }
}

void MonaSerializer::_serializeAsFree(Ident i, MonaTypeTag type) {
    if(type == MonaTypeTag::Varname1) {
        this->_monaFile << "var1 " << symbolTable.lookupSymbol(i) << ";\n";
    } else {
        this->_monaFile << "var2 " << symbolTable.lookupSymbol(i) << ";\n";
    }
}

void MonaSerializer::_serializeAsBound(Ident i, MonaTypeTag type, std::string quant) {
    if(type == MonaTypeTag::Varname1) {
        this->_monaFile << quant << "1 " << symbolTable.lookupSymbol(i) << ":";
    } else {
        this->_monaFile << quant << "2 " << symbolTable.lookupSymbol(i) << ":";
    }
}

MonaSerializer::~MonaSerializer() {
    this->_monaFile.close();
}

