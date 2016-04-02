/*****************************************************************************
 *  dWiNA - Deciding WSkS using non-deterministic automata
 *
 *  Copyright (c) 2015  Tomas Fiedor <ifiedortom1@fit.vutbr.cz>
 *
 *  Description:
 *    Visitor for restricting the orders of the formulae to second order only
 *
 *****************************************************************************/

#ifndef WSKS_SECONDORDERRESTRICTER_H
#define WSKS_SECONDORDERRESTRICTER_H

#include "../../Frontend/ast.h"
#include "../../Frontend/ast_visitor.h"
#include "../../Frontend/ident.h"

class SecondOrderRestricter : public TransformerVisitor {
private:
    template<class FirstOrderQuantification, class SecondOrderQuantification, class BinaryFormula>
    AST* _firstOrderRestrict(FirstOrderQuantification* form);
    template<class SecondOrderQuantification, class BinaryFormula>
    AST* _secondOrderRestrict(SecondOrderQuantification* form);

public:
    SecondOrderRestricter() : TransformerVisitor(Traverse::PostOrder) {}

    AST* visit(ASTForm_Ex1* form);
    AST* visit(ASTForm_Ex2* form);
    AST* visit(ASTForm_All1* form);
    AST* visit(ASTForm_All2* form);

    template<class BinaryFormula, class VarType>
    static ASTForm* RestrictFormula(Ident var, ASTForm* form);
};


#endif //WSKS_SECONDORDERRESTRICTER_H
