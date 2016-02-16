//
// Created by Raph on 14/01/2016.
//

#include "BaseAutomataMerger.h"
#include "../../Frontend/ast.h"
#include "../../Frontend/ident.h"

template<class BinopClass, ASTKind k>
AST* BaseAutomataMerger::_mergeBaseOnPath(BinopClass* binopForm) {
    IdentList free1, bound1;
    IdentList free2, bound2;
    binopForm->f1->freeVars(&free1, &bound1);
    binopForm->f2->freeVars(&free2, &bound2);
    if(bound1.size() == 0 && bound2.size() != 0 && binopForm->f2->kind == k) {
        BinopClass* leftBinop = reinterpret_cast<BinopClass*>(binopForm->f2);
        AST *mergePoint = this->_findMergePoint<BinopClass, k>(leftBinop);
        if(mergePoint == nullptr) {
            return binopForm;
        } else if(mergePoint != binopForm) {
            ASTForm* temp;
            BinopClass* binMergePoint = reinterpret_cast<BinopClass*>(mergePoint);
            temp = binopForm->f1;
            binopForm->f1 = binopForm->f2;
            binopForm->f2 = binMergePoint->f2;
            binMergePoint->f2 = binMergePoint->f1;
            binMergePoint->f1 = temp;
        }
    }

    return binopForm;
}

template<class BinopClass, ASTKind k>
AST* BaseAutomataMerger::_findMergePoint(BinopClass* binopForm) {
    if(binopForm->kind == k) {
        IdentList free, bound;
        binopForm->f1->freeVars(&free, &bound);
        if (bound.size() == 0) {
            if (binopForm->f2->kind == k) {
                BinopClass *right = reinterpret_cast<BinopClass*>(binopForm->f2);
                free.reset();
                bound.reset();
                right->f1->freeVars(&free, &bound);
                if (bound.size() == 0) {
                    return this->_findMergePoint<BinopClass, k>(right);
                }
            }
        }
    }
    return binopForm;
}

/**
 * @param[in] form:     traversed And node
 */
AST* BaseAutomataMerger::visit(ASTForm_And* form) {
    assert(form != nullptr);
    return this->_mergeBaseOnPath<ASTForm_And, ASTKind::aAnd>(form);
}

/**
 * @param[in] form:     traversed Or node
 */
AST* BaseAutomataMerger::visit(ASTForm_Or* form) {
    assert(form != nullptr);
    return this->_mergeBaseOnPath<ASTForm_Or, ASTKind::aOr>(form);
}

/**
 * @param[in] form:     traversed Impl node
 */
AST* BaseAutomataMerger::visit(ASTForm_Impl* form) {
    assert(false && "Implication should not be in reordered formula");
    return form;
}

/**
 * @param[in] form:     traversed Biimpl node
 */
AST* BaseAutomataMerger::visit(ASTForm_Biimpl* form) {
    assert(false && "Biimplication should not be in reordered formula");
    return form;
}