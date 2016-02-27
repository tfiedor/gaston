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
#include "AntiPrenexer.h"
#include "../../Frontend/ast.h"
#include "../../Frontend/ident.h"
#include "../../Frontend/symboltable.h"

extern SymbolTable symbolTable;

template<class FirstOrderQuantification, class SecondOrderQuantification, class BinaryFormula>
AST* SecondOrderRestricter::_firstOrderRestrict(FirstOrderQuantification* form) {
    assert(form != nullptr);
    assert(form->f != nullptr);

    ASTForm* restrictedFormula = form->f;

    while(form->vl->size() != 0) {
        auto it = form->vl->pop_back();
        ASTForm_FirstOrder *singleton = new ASTForm_FirstOrder(new ASTTerm1_Var1(it, form->pos), form->pos);
        BinaryFormula* binopForm = new BinaryFormula(singleton, restrictedFormula, form->pos);
        // #RESTRICTION#
        ASTForm* restriction = symbolTable.lookupRestriction(it);
        if(restriction == nullptr) {
            Ident formal;
            ASTList* list = new ASTList();
            list->push_back(new ASTTerm1_Var1(it, Pos()));
            restriction = symbolTable.getDefault1Restriction(&formal);
            restriction = restriction->clone()->unfoldMacro(new IdentList(formal), list);
        }
        if(restriction->kind != aTrue) {
            binopForm = new BinaryFormula(restriction, binopForm, form->pos);
        }
        restrictedFormula = new SecondOrderQuantification(nullptr, new IdentList(it), binopForm, Pos());
    }

    // Free the previous form
    form->detach();
    delete form;

    return restrictedFormula;
};

template<class SecondOrderQuantification, class BinaryFormula>
AST* SecondOrderRestricter::_secondOrderRestrict(SecondOrderQuantification *form) {
    assert(form != nullptr);
    assert(form->f != nullptr);

    ASTForm* restrictedFormula = form->f;
    ASTForm* binopForm;

    while(form->vl->size() != 0) {
        binopForm = restrictedFormula;
        auto it = form->vl->pop_back();
        ASTForm* restriction = symbolTable.lookupRestriction(it);
        if(restriction == nullptr) {
            Ident formal;
            ASTList* list = new ASTList();
            list->push_back(new ASTTerm2_Var2(it, Pos()));
            restriction = symbolTable.getDefault2Restriction(&formal);
            restriction = restriction->clone()->unfoldMacro(new IdentList(formal), list);
        }

        if(restriction->kind != aTrue) {
            binopForm = new BinaryFormula(restriction, binopForm, form->pos);
        }
        restrictedFormula = new SecondOrderQuantification(nullptr, new IdentList(it), binopForm, form->pos);
    }

    form->detach();
    delete form;

    return restrictedFormula;
}

/**
 * Restricts the first order quantifer to second order one
 *
 * @param[in] form:     traversed Ex1 node
 */
AST* SecondOrderRestricter::visit(ASTForm_Ex1* form) {
    return this->_firstOrderRestrict<ASTForm_Ex1, ASTForm_Ex2, ASTForm_And>(form);
}

AST* SecondOrderRestricter::visit(ASTForm_Ex2* form) {
    return this->_secondOrderRestrict<ASTForm_Ex2, ASTForm_And>(form);
}

/**
 * Restricts the first order quantifier to second order one
 *
 * @param[in] form:     traversed All1 node
 */
AST* SecondOrderRestricter::visit(ASTForm_All1* form) {
    return this->_firstOrderRestrict<ASTForm_All1, ASTForm_All2, ASTForm_Impl>(form);
}

AST* SecondOrderRestricter::visit(ASTForm_All2* form) {
    return this->_secondOrderRestrict<ASTForm_All2, ASTForm_Impl>(form);
}