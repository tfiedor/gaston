//
// Created by Raph on 02/05/2016.
//

#include "FixpointGuide.h"
#include "SymbolicAutomata.h"
#include "Term.h"
#include "../containers/VarToTrackMap.hh"
#include "../../Frontend/symboltable.h"

extern VarToTrackMap varMap;
extern SymbolTable symbolTable;

std::ostream &operator<<(std::ostream &out, const FixpointGuide &rhs) {
    out << "{";
    bool first = true;
    for(auto var : rhs._vars) {
        if(!first) {
            out << ", ";
        } else {
            first = false;
        }
        out << symbolTable.lookupSymbol(var);
    }
    out << "}";
    return out;
}

/**
 * Initialize FixpointGuide out of the symlink
 *
 * @param[in] link:         SymLink to automaton that is wrapped by fixpoint guide
 */
FixpointGuide::FixpointGuide(SymLink *link, bool qf) : _link(link), _isQuantifierFree(qf) {
    this->_InitializeVars(link->aut->_form);
}

/**
 * Sets SymLink to the link
 *
 * @param[in] link:         Symlink to automaton that is wrapped by fixpoint guide
 */
void FixpointGuide::SetAutomaton(SymLink* link) {
    if(this->_link == nullptr) {
        this->_link = link;
        this->_InitializeVars(this->_link->aut->_form);
    } else {
        assert(false && "Trying to reinitialize Guide\n");
    }
}

/**
 * Initilization of the variables from the formulae. The formula is recursively
 * traversed and first order variables are collected that are used to guide
 * the outer fixpoint computation
 *
 * @param[in] form:         formula that guides the link
 */
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
        if(std::find_if(this->_vars.begin(), this->_vars.end(), [&var](size_t& i) {
                return var == i; }) == this->_vars.end()) {
            this->_vars.push_back(var);
        }
    }
}

/**
 * Returns tip, what to do with the @p term and @p symbol combination during the fixpoint computation.
 * The pair is either recommended to be enqueued in the front of the worklist, to the back of the
 * worklist or completely thrown away.
 *
 * @param[in] term:         term we are adding to the worklist
 * @param[in] symbol:       symbol we are subtracting from the term
 * @return:                 tip what to do with the @p term and @p symbol pair
 */
GuideTip FixpointGuide::GiveTip(Term* term, Symbol* symbol) {
    // Empty terms are not enqueued in worklist
    if(term->type == TERM_EMPTY) {
        return GuideTip::G_THROW;
    // The 0* chains are removed from the queue, so every zero string is not gonna be computed
    } else if(term->link->succ == nullptr && symbol->IsZeroString()) {
        return GuideTip::G_THROW;
    // This tries to enforce to subtract the '1' so the FirstOrder constraint holds
    } else if(this->_vars.size() > 0 && term->link->succ == nullptr) {
        symbol = this->_link->ReMapSymbol(symbol);
        // Fixme: i think this is maybe fishy, as there is DAG, but further at top, there is remapping
        for(auto var : this->_vars) {
            if (symbol->GetSymbolAt(varMap[var]) != '1') {
                return GuideTip::G_THROW;
            }
        }
    // Guide for first order variables
    } else if(this->_link != nullptr && this->_link->aut->_form->kind == aFirstOrder && symbol != nullptr) {
        assert(this->_link->aut != nullptr);
        assert(this->_link->aut->type == AutType::BASE);
        ASTForm_FirstOrder *fo = static_cast<ASTForm_FirstOrder *>(this->_link->aut->_form);

        symbol = this->_link->ReMapSymbol(symbol);
        TermProduct *tp = static_cast<TermProduct *>(term);
        // FIXME: FIX THIS SHIT
        //assert(term->type == TermType::TERM_PRODUCT);
    }
    return GuideTip::G_FRONT;
}

TermBaseSet* get_base_states(Term* term) {
    assert(term != nullptr);
    if(term->type == TermType::TERM_PRODUCT) {
        TermProduct *tp = static_cast<TermProduct *>(term);
        assert(tp->left->type == TermType::TERM_BASE);
        return static_cast<TermBaseSet *>(tp->left);
    } else if(term->type == TermType::TERM_TERNARY_PRODUCT) {
        TermTernaryProduct *ttp = static_cast<TermTernaryProduct *>(term);
        assert(ttp->left->type == TermType::TERM_BASE);
        return static_cast<TermBaseSet *>(ttp->left);
    } else if(term->type == TermType::TERM_NARY_PRODUCT) {
        TermNaryProduct *tnp = static_cast<TermNaryProduct *>(term);
        assert(tnp->terms[0]->type == TermType::TERM_BASE);
        return static_cast<TermBaseSet *>(tnp->terms[0]);
    } else {
        assert(false && "We have something different than Product for giving tip");
    }
}

/**
 * Returns tip what to do with the @p term. If the restriction on the left holds, we don't have to project
 * everything away and simply push everything we can down, so simply project everything we can.
 *
 * @param[in] term:         term we are potentially adding to the worklist
 * @return:                 tip what to do with the @p term
 */
GuideTip FixpointGuide::GiveTip(Term* term) {
    assert(term != nullptr);
    if (this->_isQuantifierFree && this->_link != nullptr && this->_link->aut->type == AutType::BASE && term->type != TERM_EMPTY) {
        TermBaseSet* initial = static_cast<TermBaseSet*>(this->_link->aut->GetInitialStates());
        if (initial->Intersects(get_base_states(term))) {
            return GuideTip::G_PROJECT_ALL;
        }
    }

    return GuideTip::G_PROJECT;
}