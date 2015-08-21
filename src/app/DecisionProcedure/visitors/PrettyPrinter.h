//
// Created by Raph on 19/08/2015.
//

#ifndef WSKS_PRETTYPRINTER_H
#define WSKS_PRETTYPRINTER_H

#include "../Frontend/ast_visitor.h"

/**
 * Class for printing the info about the structure for debug mostly
 */
class PrettyPrinter : public VoidVisitor {
    void visit(ASTForm_True *form);
    void visit(ASTForm_And *form);
};

#endif //WSKS_PRETTYPRINTER_H
