//
// Created by Raph on 02/05/2016.
//

#include "FixpointGuide.h"
#include "SymbolicAutomata.h"
#include "Term.h"
#include "../containers/VarToTrackMap.hh"

extern VarToTrackMap varMap;

std::ostream &operator<<(std::ostream &out, const FixpointGuide &rhs) {
    out << "{";
    bool first = true;
    for(auto var : rhs._vars) {
        if(!first) {
            out << ", ";
        } else {
            first = false;
        }
        out << var;
    }
    out << "}";
    return out;
}

FixpointGuide::FixpointGuide(SymLink *link) : _link(link) {
    this->_InitializeVars(link->aut->_form);
}

void FixpointGuide::SetAutomaton(SymLink* link) {
    if(this->_link == nullptr) {
        this->_link = link;
    }
}

void FixpointGuide::_InitializeVars(ASTForm* form) {
    if(form->kind == aAnd || form->kind == aOr) {
        ASTForm_ff* ff_form = static_cast<ASTForm_ff*>(form);
        this->_InitializeVars(ff_form->f1);
        this->_InitializeVars(ff_form->f2);
    } else if(form->kind == aEx1 || form->kind == aEx2) {
        this->_InitializeVars(static_cast<ASTForm_uvf*>(form)->f);
    } else if(form->kind == aFirstOrder) {
        ASTForm_FirstOrder* fo_form = static_cast<ASTForm_FirstOrder*>(form);
        size_t var = static_cast<ASTTerm1_Var1*>(fo_form->t)->n;
        if(std::find_if(this->_vars.begin(), this->_vars.end(), [&var](size_t& i) { return var == i; }) == this->_vars.end()) {
            this->_vars.push_back(var);
        }
    }
}

GuideTip FixpointGuide::GiveTip(Term* term, Symbol* symbol) {
    if(term->type == TERM_EMPTY) {
        return GuideTip::G_THROW;
    } else if(term->link.succ == nullptr && symbol->IsZeroString()) {
        return GuideTip::G_THROW;
    } else if(this->_vars.size() > 0 && term->link.succ == nullptr) {
        symbol = this->_link->ReMapSymbol(symbol);
        // Fixme: i think this is maybe fishy, as there is DAG, but further at top, there is remapping
        for(auto var : this->_vars) {
            if (symbol->GetSymbolAt(varMap[var]) != '1') {
                return GuideTip::G_THROW;
            }
        }
    } else if(this->_link != nullptr && this->_link->aut->_form->kind == aFirstOrder && symbol != nullptr) {
        assert(this->_link->aut != nullptr);
        assert(this->_link->aut->type == AutType::BASE);
        ASTForm_FirstOrder *fo = static_cast<ASTForm_FirstOrder *>(this->_link->aut->_form);

        symbol = this->_link->ReMapSymbol(symbol);
        TermProduct *tp = static_cast<TermProduct *>(term);

        if (static_cast<TermBaseSet *>(tp->left)->Intersects(static_cast<TermBaseSet *>(this->_link->aut->GetInitialStates()))) {
            // Restriction holds, we'll look back at link and if the right side did not progress, we will throw it away
            if (tp->last_link.succ != nullptr) {
                TermProduct *tpp = static_cast<TermProduct *>(tp->last_link.succ);
                if (tpp->right == tp->right && tp->last_link.symbol == symbol) {
                    return GuideTip::G_BACK;
                }
            }
        }
    }
    return GuideTip::G_FRONT;
}