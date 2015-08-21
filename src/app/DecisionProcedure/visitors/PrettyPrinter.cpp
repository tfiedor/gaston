//
// Created by Raph on 19/08/2015.
//

#include "PrettyPrinter.h"

/**
 * Prints the name of the
 */
void PrettyPrinter::visit(ASTForm_True *form) {
    std::cout << "True";
}

void PrettyPrinter::visit(ASTForm_And *form) {
    std::cout << " and ";
}