//
// Created by Raph on 02/05/2016.
//

#include "FixpointGuide.h"
#include "SymbolicAutomata.h"
#include "Term.h"
#include "../containers/VarToTrackMap.hh"

extern VarToTrackMap varMap;

void FixpointGuide::SetAutomaton(SymbolicAutomaton *aut) {
    if(this->_aut == nullptr) {
        this->_aut = aut;
    }
}

GuideTip FixpointGuide::GiveTip(Term* term, Symbol* symbol) {
    assert(this->_aut != nullptr);
    assert(this->_aut->type == AutType::BASE);
    if(term->type == TERM_EMPTY) {
        return GuideTip::G_THROW;
    } else if(this->_aut->_form->kind == aFirstOrder && symbol != nullptr) {
        TermProduct* tp = static_cast<TermProduct*>(term);
        if(static_cast<TermBaseSet*>(tp->left)->Intersects(static_cast<TermBaseSet*>(this->_aut->GetInitialStates()))) {
            ASTForm_FirstOrder* fo = static_cast<ASTForm_FirstOrder*>(this->_aut->_form);
            if(symbol->GetSymbolAt(varMap[static_cast<ASTTerm1_Var1*>(fo->t)->n]) != '1') {
                return GuideTip::G_FRONT;
            } else {
                return GuideTip::G_FRONT;
            }
        } else if(tp->left->link.succ != nullptr && tp->left->stateSpace == tp->left->link.succ->stateSpace) {
            return GuideTip::G_BACK;
        } else {
            if(tp->link.succ == nullptr) {
                ASTForm_FirstOrder* fo = static_cast<ASTForm_FirstOrder*>(this->_aut->_form);
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