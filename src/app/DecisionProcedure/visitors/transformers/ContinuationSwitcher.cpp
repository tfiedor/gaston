//
// Created by Raph on 28/04/2016.
//

#include "ContinuationSwitcher.h"
#include "../../../Frontend/symboltable.h"

extern SymbolTable symbolTable;

AST* ContinuationSwitcher::visit(ASTForm_ff* form) {
    int left_rank = 0, right_rank = 0;
    for(auto it = form->f1->allVars->begin(); it != form->f1->allVars->end(); ++it) {
        left_rank += (symbolTable.lookupType(*it) == MonaTypeTag::Varname1 ? 0 : 1);
    }
    for(auto tit = form->f2->allVars->begin(); tit != form->f2->allVars->end(); ++tit) {
        right_rank += (symbolTable.lookupType(*tit) == MonaTypeTag::Varname1 ? 0 : 1);
    }
    if(left_rank > right_rank && form->f2->kind != aNot) {
        ASTForm* temp = form->f1;
        form->f1 = form->f2;
        form->f2 = temp;
    }

    return static_cast<AST*>(form);
}
