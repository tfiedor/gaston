/*****************************************************************************
 *  dWiNA - Deciding WSkS using non-deterministic automata
 *
 *  Copyright (c) 2015  Tomas Fiedor <ifiedortom1@fit.vutbr.cz>
 *
 *  Description:
 *    Visitor for restricting the orders of the formulae to second order only
 *
 *****************************************************************************/

#include "SecondOrderRestricter.h"
#include "../../Frontend/ast.h"
#include "../../Frontend/ident.h"

template<class FirstOrderQuantification, class SecondOrderQuantification, class BinaryFormula>
AST* SecondOrderRestricter::_firstOrderRestrict(FirstOrderQuantification* form) {
    assert(form != nullptr);
    assert(form->f != nullptr);

    Ident* it = form->vl->begin();
    assert(it != form->vl->end());

    BinaryFormula* binaryForm;
    ASTForm* restrictedFormula = new ASTForm_FirstOrder(new ASTTerm1_Var1(*it, form->pos), form->pos);

    while ((++it) != form->vl->end()) {
        ASTForm_FirstOrder *singleton = new ASTForm_FirstOrder(new ASTTerm1_Var1(*it, form->pos), form->pos);
        restrictedFormula = new BinaryFormula(restrictedFormula, singleton, form->pos);
    }

    restrictedFormula = new BinaryFormula(restrictedFormula, form->f, form->pos);

    // Free the previous form
    form->f = nullptr;
    IdentList* ul = form->ul;
    form->ul = nullptr;
    IdentList* vl = form->vl;
    form->vl = nullptr;
    delete form;

    return new SecondOrderQuantification(ul, vl, restrictedFormula, Pos());
};

/**
 * Restricts the first order quantifer to second order one
 *
 * @param[in] form:     traversed Ex1 node
 */
AST* SecondOrderRestricter::visit(ASTForm_Ex1* form) {
    return this->_firstOrderRestrict<ASTForm_Ex1, ASTForm_Ex2, ASTForm_And>(form);
}

/**
 * Restricts the first order quantifier to second order one
 *
 * @param[in] form:     traversed All1 node
 */
AST* SecondOrderRestricter::visit(ASTForm_All1* form) {
    return this->_firstOrderRestrict<ASTForm_All1, ASTForm_All2, ASTForm_Impl>(form);
}