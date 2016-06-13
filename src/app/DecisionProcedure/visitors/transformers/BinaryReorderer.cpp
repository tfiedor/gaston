//
// Created by Raph on 01/01/2016.
//

#include "BinaryReorderer.h"
#include "../../../Frontend/ast.h"

template<class BinopClass>
AST* binary_reorder(BinopClass* binopForm) {
    // (f1 op f2) op f3 -> f1 op (f2 op f3)
    if(binopForm->f1->kind == binopForm->kind) {
        BinopClass* left = reinterpret_cast<BinopClass*>(binopForm->f1);
        ASTForm* temp;
        temp = binopForm->f2;
        binopForm->f2 = binopForm->f1;
        binopForm->f1 = left->f1;
        left->f1 = left->f2;
        left->f2 = temp;
        return binary_reorder(binopForm);
    } else {
        return binopForm;
    }
}

/**
 * @param[in] form:     traversed And node
 */
AST* BinaryReorderer::visit(ASTForm_And* form) {
    assert(form != nullptr);
    return binary_reorder<ASTForm_And>(form);
}

/**
 * @param[in] form:     traversed Or node
 */
AST* BinaryReorderer::visit(ASTForm_Or* form) {
    assert(form != nullptr);
    return binary_reorder<ASTForm_Or>(form);
}

/**
 * @param[in] form:     traversed Impl node
 */
AST* BinaryReorderer::visit(ASTForm_Impl* form) {
    assert(false && "Implication should not be in reordered formula");
    return form;
}

/**
 * @param[in] form:     traversed Biimpl node
 */
AST* BinaryReorderer::visit(ASTForm_Biimpl* form) {
    return binary_reorder<ASTForm_Biimpl>(form);
}