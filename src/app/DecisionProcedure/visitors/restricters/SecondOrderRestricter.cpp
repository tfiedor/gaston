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
#include "../../environment.hh"
#include "../../../Frontend/ast.h"
#include "../../../Frontend/ident.h"
#include "../../../Frontend/symboltable.h"

extern SymbolTable symbolTable;

template<class BinaryFormula, class VarType>
ASTForm* SecondOrderRestricter::RestrictFormula(Ident var, ASTForm* form) {
    ASTForm* restriction = symbolTable.lookupRestriction(var);
#   if (DEBUG_RESTRICTIONS == true)
    std::cout << "Restricting '" << symbolTable.lookupSymbol(var) << "' of type " << symbolTable.lookupType(var) << " by ";
    if(restriction == nullptr) {
        std::cout << "True";
    } else {
        restriction->dump();
    }
    std::cout << "\n";
#   endif
    // Fixme: Default restrictions should be handled as well
    if((restriction = symbolTable.lookupRestriction(var)) == nullptr) {
        // Get default restriction instead
        Ident formal;
        ASTList* list = new ASTList();
        list->push_back(new VarType(var, Pos()));

        if(symbolTable.lookupType(var) == MonaTypeTag::Varname1) {
            restriction = symbolTable.getDefault1Restriction(&formal);
        } else {
            restriction = symbolTable.getDefault2Restriction(&formal);
        }
        if(restriction != nullptr)
            restriction = restriction->clone()->unfoldMacro(new IdentList(formal), list);
        delete list;
    } else {
        restriction = restriction->clone();
    }

    ASTForm_FirstOrder *singleton;
    if(symbolTable.lookupType(var) == MonaTypeTag::Varname1) {
        singleton = new ASTForm_FirstOrder(new ASTTerm1_Var1(var, form->pos), form->pos);
        restriction = (restriction != nullptr && restriction->kind != aTrue) ?
                      static_cast<ASTForm*>(new BinaryFormula(singleton, restriction, form->pos)) : singleton;
    }

    if(restriction != nullptr && restriction->kind != aTrue) {
#       if (ALT_EXPLICIT_RESTRICTIONS == true)
        restriction->is_restriction = true;
        restriction->tag = 0;
#       endif
        return static_cast<ASTForm*>(new BinaryFormula(restriction, form, form->pos));
    } else {
        return form;
    }
}

template<class FirstOrderQuantification, class SecondOrderQuantification, class BinaryFormula>
AST* SecondOrderRestricter::_firstOrderRestrict(FirstOrderQuantification* form) {
    assert(form != nullptr);
    assert(form->f != nullptr);

    ASTForm* restrictedFormula = form->f;

    while(form->vl->size() != 0) {
        auto it = form->vl->pop_back();
        assert(symbolTable.lookupType(it) == MonaTypeTag::Varname1);
        BinaryFormula* binopForm = reinterpret_cast<BinaryFormula*>(SecondOrderRestricter::RestrictFormula<BinaryFormula, ASTTerm1_Var1>(it, restrictedFormula));
        restrictedFormula = new SecondOrderQuantification(nullptr, new IdentList(it), binopForm, Pos());
    }

    restrictedFormula->fixpoint_number = form->fixpoint_number;

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

    while(form->vl->size() != 0) {
        auto it = form->vl->pop_back();
        assert(symbolTable.lookupType(it) != MonaTypeTag::Varname1);
        ASTForm* binopForm = restrictedFormula;
        binopForm = reinterpret_cast<BinaryFormula*>(SecondOrderRestricter::RestrictFormula<BinaryFormula, ASTTerm2_Var2>(it, binopForm));
        restrictedFormula = new SecondOrderQuantification(nullptr, new IdentList(it), binopForm, form->pos);
    }

    restrictedFormula->fixpoint_number = form->fixpoint_number;

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
    // This will be further constructed by MONA, do not fucking touch it
    if(form->tag == 0) {
        return form;
    } else {
        return this->_firstOrderRestrict<ASTForm_Ex1, ASTForm_Ex2, ASTForm_And>(form);
    }
}

AST* SecondOrderRestricter::visit(ASTForm_Ex2* form) {
    if(form->tag == 0) {
        return form;
    } else {
        return this->_secondOrderRestrict<ASTForm_Ex2, ASTForm_And>(form);
    }
}

/**
 * Restricts the first order quantifier to second order one
 *
 * @param[in] form:     traversed All1 node
 */
AST* SecondOrderRestricter::visit(ASTForm_All1* form) {
    // This will be further constructed by MONA, do not fucking touch it
    if(form->tag == 0) {
        return form;
    } else {
        return this->_firstOrderRestrict<ASTForm_All1, ASTForm_All2, ASTForm_Impl>(form);

    }
}

AST* SecondOrderRestricter::visit(ASTForm_All2* form) {
    if(form->tag == 0) {
        return form;
    } else {
        return this->_secondOrderRestrict<ASTForm_All2, ASTForm_Impl>(form);
    }
}