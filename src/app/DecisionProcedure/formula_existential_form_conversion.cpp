/*****************************************************************************
 *  dWiNA - Deciding WSkS using non-deterministic automata
 *
 *  Copyright (c) 2014  Tomas Fiedor <xfiedo01@stud.fit.vutbr.cz>
 *
 *  Description:
 *    Conversion of formula to exPNF
 *
 *****************************************************************************/

#include "../Frontend/ast.h"
#include "../Frontend/symboltable.h"
#include "visitors/SyntaxRestricter.h"
#include "visitors/NegationUnfolder.h"
#include "visitors/UniversalQuantifierRemover.h"
#include "visitors/PrenexNormalFormTransformer.h"

#include <cstring>

using std::cout;

extern SymbolTable symbolTable;

/**
 * Given AST tree for formula phi, it is first transformed to restricted syntax
 * then to Prenex Normal Form, moving all quantifiers to the leftmost of
 * formula. All universal quantifiers are transformed to existentional and
 * negation is shifted to atoms.
 *
 * @return: AST representation of formula in Existentional Normal Form
 */
ASTForm* ASTForm::toExistentionalPNF() {
    ASTForm* temp;

    SyntaxRestricter sr_visitor;
    temp = static_cast<ASTForm*>(this->accept(sr_visitor));

    PrenexNormalFormTransformer pnft_visitor;
    temp = static_cast<ASTForm*>(temp->accept(pnft_visitor));

    UniversalQuantifierRemover uqr_visitor;
    temp = static_cast<ASTForm*>(temp->accept(uqr_visitor));

    NegationUnfolder nu_visitor;
    return static_cast<ASTForm*>(temp->accept(nu_visitor));
}