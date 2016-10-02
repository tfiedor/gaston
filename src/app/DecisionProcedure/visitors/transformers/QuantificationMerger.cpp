//
// Created by Raph on 07/01/2016.
//

#include "QuantificationMerger.h"
#include "../../environment.hh"
#include "../../../Frontend/ast.h"
#include "../../../Frontend/ident.h"

template<class ForallClass>
AST* mergeUniversal(ForallClass* form) {
    if(form->f->kind == aAll1 || form->f->kind == aAll2) {
        ASTForm_uvf* innerQuantifier = reinterpret_cast<ASTForm_uvf*>(form->f);
        form->f = innerQuantifier->f;
        form->vl = ident_union(form->vl, innerQuantifier->vl);

        innerQuantifier->detach();
        delete innerQuantifier;
    }
    return form;
}

AST* QuantificationMerger::visit(ASTForm_All1 *form) {
    return mergeUniversal<ASTForm_All1>(form);
}

AST* QuantificationMerger::visit(ASTForm_All2 *form) {
    return mergeUniversal<ASTForm_All2>(form);
}

template<class ExistClass>
AST* mergeExistential(ExistClass* form) {
    if(form->f->kind == aEx1 || form->f->kind == aEx2) {
        ASTForm_uvf* innerQuantifier = reinterpret_cast<ASTForm_uvf*>(form->f);
        form->f = innerQuantifier->f;
        IdentList* oldList = form->vl;
        form->vl = ident_union(form->vl, innerQuantifier->vl);

        // Clean up
        delete oldList;
        delete innerQuantifier->vl;
        innerQuantifier->detach();
        delete innerQuantifier;
#   if (OPT_MERGE_FIRST_ORDER_QUANTIFIERS == true)
    } else if(form->f->kind == aAnd) {
        ASTForm_ff* ff_form = static_cast<ASTForm_ff*>(form->f);
        if(ff_form->f1->kind == aFirstOrder && (ff_form->f2->kind == aEx1 || ff_form->f2->kind == aEx2)) {
            ASTForm_uvf* innerQuantifier = reinterpret_cast<ASTForm_uvf*>(ff_form->f2);
            ff_form->f2 = innerQuantifier->f;
            innerQuantifier->f = ff_form;
            form->f = innerQuantifier;
            return mergeExistential<ExistClass>(form);
        }
#   endif
    }
    return form;
}

AST* QuantificationMerger::visit(ASTForm_Ex1 *form) {
    return mergeExistential<ASTForm_Ex1>(form);
}

AST* QuantificationMerger::visit(ASTForm_Ex2 *form) {
    return mergeExistential<ASTForm_Ex2>(form);
}
