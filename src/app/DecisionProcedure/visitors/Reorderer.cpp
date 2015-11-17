/*****************************************************************************
 *  dWiNA - Deciding WSkS using non-deterministic automata
 *
 *  Copyright (c) 2015  Tomas Fiedor <ifiedortom1@fit.vutbr.cz>
 *
 *  Description:
 *    Visitor for doing the reordering of the formulae. This could be
 *    optimization for our future method, so simpler formulae are traversed
 *    first achieving better performance
 *
 *****************************************************************************/

#include "Reorderer.h"
#include "../../Frontend/ast.h"

template<class BinopClass>
AST* switchOperands(BinopClass* binopForm) {
    IdentList freeLeft, boundLeft;
    IdentList freeRight, boundRight;

    binopForm->f1->freeVars(&freeLeft, &boundLeft);
    binopForm->f2->freeVars(&freeRight, &boundRight);

    if(boundRight.size() < boundLeft.size()) {
        // Switch, so leftmost is always thing with less quantifications
        std::cout << "[HIT]" << boundRight.size() << " vs " << boundLeft.size() << "\n";
        ASTForm* temp;
        temp = binopForm->f1;
        binopForm->f1 = binopForm->f2;
        binopForm->f2 = temp;
    }

    return binopForm;
}

template<class BinopClass>
AST* reorder(BinopClass* binopForm) {
    binopForm = static_cast<BinopClass*>(switchOperands(binopForm));

    IdentList freeLeft, boundLeft;
    IdentList freeRight, boundRight;

    binopForm->f1->freeVars(&freeLeft, &boundLeft);
    binopForm->f2->freeVars(&freeRight, &boundRight);

    if(binopForm->f1->kind == binopForm->kind && boundRight.size() == 0) {
        BinopClass* left = static_cast<BinopClass*>(binopForm->f1);
        // (f1 o f2) o f3
        IdentList freeLLeft, boundLLeft;
        IdentList freeLRight, boundLRight;

        left->f1->freeVars(&freeLLeft, &boundLLeft);
        left->f2->freeVars(&freeLRight, &boundLRight);

        auto lleftSize = boundLLeft.size();
        auto lrightsize = boundLRight.size();

        ASTForm* temp;
        if(lleftSize != 0 && lrightsize == 0) {
            // (Qx f1 o f2) o f3 => (f2 o f3) o Qx f1
            temp = binopForm->f2;
            binopForm->f2 = left->f1;
            left->f1 = temp;
        } else if(lleftSize == 0 && lrightsize != 0) {
            // (f1 o Qx f2) o f3 => (f1 o f3) o Qx f2
            temp = binopForm->f2;
            binopForm->f2 = left->f2;
            left->f2 = temp;
        };
    } else if(binopForm->f2->kind == binopForm->kind && boundLeft.size() == 0) {
        BinopClass* right = static_cast<BinopClass*>(binopForm->f2);
        // f1 o (f2 o f3)
        IdentList freeRLeft, boundRLeft;
        IdentList freeRRight, boundRRight;

        right->f1->freeVars(&freeRLeft, &boundRLeft);
        right->f2->freeVars(&freeRRight, &boundRRight);

        auto rleftSize = boundRLeft.size();
        auto rrightsize = boundRRight.size();

        ASTForm* temp;
        if(rleftSize != 0 && rrightsize == 0) {
            // f1 o (Qx f2 o f3) => (f1 o f3) o Qx f2
            temp = binopForm->f1;
            binopForm->f1 = right->f1;
            right->f1 = temp;

            temp = binopForm->f1;
            binopForm->f1 = binopForm->f2;
            binopForm->f2 = temp;
        } else if(rleftSize == 0 && rrightsize != 0) {
            // f1 o (f2 o Qx f3)=> (f1 o f2) o Qx f3
            temp = binopForm->f1;
            binopForm->f1 = right->f2;
            right->f2 = temp;

            temp = binopForm->f1;
            binopForm->f1 = binopForm->f2;
            binopForm->f2 = temp;
        };
    }

    return binopForm;
}

/**
 * @param[in] form:     traversed And node
 */
AST* Reorderer::visit(ASTForm_And* form) {
    assert(form != nullptr);
    return reorder<ASTForm_And>(form);
}

/**
 * @param[in] form:     traversed Or node
 */
AST* Reorderer::visit(ASTForm_Or* form) {
    assert(form != nullptr);
    return reorder<ASTForm_Or>(form);
}

/**
 * @param[in] form:     traversed Impl node
 */
AST* Reorderer::visit(ASTForm_Impl* form) {
    assert(false && "Implication should not be in reordered formula");
    return form;
}

/**
 * @param[in] form:     traversed Biimpl node
 */
AST* Reorderer::visit(ASTForm_Biimpl* form) {
    assert(false && "Biimplication should not be in reordered formula");
    return form;
}