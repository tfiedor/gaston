//
// Created by Raph on 11/04/2016.
//

#ifndef WSKS_MONASERIALIZER_H
#define WSKS_MONASERIALIZER_H

#include <fstream>
#include <string>
#include <iostream>
#include "../../../Frontend/ast.h"
#include "../../../Frontend/ast_visitor.h"

class MonaSerializer {
public:
    explicit MonaSerializer(std::string filename, ASTForm* form);
    ~MonaSerializer();

private:
    std::ofstream _monaFile;
};

#endif //WSKS_MONASERIALIZER_H
