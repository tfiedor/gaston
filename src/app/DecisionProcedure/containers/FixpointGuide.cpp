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
 * @brief Constructs guide of the fixpoints that controls the computation
 *
 * Note: Currently the guide only throws aways 0* strings out of the computation.
 * Note: Still this yields a huge gain
 *
 * @param[in]  identList  list of identifiers of quantified formula
 */
FixpointGuide::FixpointGuide(IdentList* identList) {
    this->_InitializeVars(identList);
}

/**
 * @brief Initializes the fixpoint guide out of the list of variables
 *
 * Initializes the fixpoint with first order variables that are bound by fixpoint
 *
 * @param[in]  vars  variables used for initialization from quantifier
 */
void FixpointGuide::_InitializeVars(IdentList* vars) {
    for(auto it = vars->begin(); it != vars->end(); ++it) {
        if(symbolTable.lookupType(*it) == MonaTypeTag::Varname1)
            this->_vars.push_back(*it);
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
    if(term->type == TermType::EMPTY) {
        return GuideTip::G_THROW;
    // The 0* chains are removed from the queue, so every zero string is not gonna be computed
    } else if(term->link->succ == nullptr && symbol->IsZeroString()) {
        return GuideTip::G_THROW;
    // This tries to enforce to subtract the '1' so the FirstOrder constraint holds
    } else if(this->_vars.size() > 0 && term->link->succ == nullptr) {
        //symbol = this->_link->ReMapSymbol(symbol);
        // Fixme: i think this is maybe fishy, as there is DAG, but further at top, there is remapping
        for(auto var : this->_vars) {
            if (symbol->GetSymbolAt(varMap[var]) != '1') {
                return GuideTip::G_THROW;
            }
        }
        return GuideTip::G_FRONT;
    // Guide for first order variables
    } else {
        return GuideTip::G_FRONT;
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
    if(term->type == TermType::EMPTY) {
        return GuideTip::G_THROW;
    } else {
        return GuideTip::G_PROJECT;
    }
}