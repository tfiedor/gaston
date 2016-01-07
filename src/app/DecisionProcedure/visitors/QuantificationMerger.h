//
// Created by Raph on 07/01/2016.
//

#ifndef WSKS_QUANTIFICATIONMERGER_H
#define WSKS_QUANTIFICATIONMERGER_H

#include "../Frontend/ast.h"
#include "../Frontend/ast_visitor.h"

class QuantificationMerger : public TransformerVisitor {
    QuantificationMerger() : TransformerVisitor(Traverse::PostOrder) {}

    
};

#endif //WSKS_QUANTIFICATIONMERGER_H
