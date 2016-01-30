//
// Created by Raph on 30/01/2016.
//

#include "PredicateUnfolder.h"

extern SymbolTable symbolTable;
extern PredicateLib predicateLib;
extern IdentList inFirstOrder;
extern Options options;

/**
 * Unfolds the called macro by substituting its formal parameters with real
 * parameters
 *
 * @param called: called macro
 * @param realParams: real parameters
 * @return: unfolded formula
 */
ASTForm* unfoldFormula(PredLibEntry* called, ASTList* realParams) {
    IdentList* formalParams = called->formals;

    ASTForm* clonnedFormula = (called->ast)->clone();
    ASTForm* unfoldedFormula = clonnedFormula->unfoldMacro(formalParams, realParams);

    PredicateUnfolder predicateUnfolder;
    return static_cast<ASTForm*>(unfoldedFormula->accept(predicateUnfolder));
}

/**
 * Expands the body of the called Predicate or macro
 *
 * @param[in] form:     traversed Call node
 */
AST* Flattener::visit(ASTForm_Call* form) {
    return unfoldFormula(predicateLib.lookup(form->n), form->args);
}
