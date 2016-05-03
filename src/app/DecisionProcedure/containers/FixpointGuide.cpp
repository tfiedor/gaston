//
// Created by Raph on 02/05/2016.
//

#include "FixpointGuide.h"
#include "SymbolicAutomata.h"
#include "Term.h"
#include "../containers/VarToTrackMap.hh"

extern VarToTrackMap varMap;

void FixpointGuide::SetAutomaton(SymLink* link) {
    if(this->_link == nullptr) {
        this->_link = link;
    }
}

GuideTip FixpointGuide::GiveTip(Term* term, Symbol* symbol) {
    if(term->type == TERM_EMPTY) {
        return GuideTip::G_THROW;
    } else if(this->_link == nullptr) {
        if(term->link.succ == nullptr && symbol->IsZeroString()) {
            return GuideTip::G_THROW;
        } else {
            return GuideTip::G_FRONT;
        }
    } else if(this->_link->aut->_form->kind == aFirstOrder && symbol != nullptr) {
        assert(this->_link->aut != nullptr);
        symbol = this->_link->ReMapSymbol(symbol);
        assert(this->_link->aut->type == AutType::BASE);
        TermProduct* tp = static_cast<TermProduct*>(term);
        if(static_cast<TermBaseSet*>(tp->left)->Intersects(static_cast<TermBaseSet*>(this->_link->aut->GetInitialStates()))) {
            // Restriction holds, we'll look back at link and if the right side did not progress, we will throw it away
            if(tp->link.succ != nullptr) {
                TermProduct* tpp = static_cast<TermProduct*>(tp->link.succ);
                if(tpp->right == tp->right && tp->link.symbol == symbol) {
                    return GuideTip::G_BACK;
                } else {
                    return GuideTip::G_FRONT;
                }
            } else {
                return GuideTip::G_FRONT;
            }
        } else if(tp->left->link.succ != nullptr && tp->left->stateSpace == tp->left->link.succ->stateSpace) {
            return GuideTip::G_BACK;
        } else {
            if(tp->link.succ == nullptr) {
                ASTForm_FirstOrder* fo = static_cast<ASTForm_FirstOrder*>(this->_link->aut->_form);
                if(symbol->GetSymbolAt(varMap[static_cast<ASTTerm1_Var1*>(fo->t)->n]) != '1') {
                    return GuideTip::G_THROW;
                } else {
                    return GuideTip::G_FRONT;
                }
            } else {
                return GuideTip::G_FRONT;
            }
        }
    } else {
        return GuideTip::G_FRONT;
    }
}