//
// Created by Raph on 07/10/2015.
//

#include "../Frontend/ast.h"
#include "../Frontend/timer.h"
#include "../Frontend/offsets.h"
#include "../DecisionProcedure/containers/SymbolicAutomata.h"
#include "../DecisionProcedure/environment.hh"
#include "../DecisionProcedure/automata.hh"
#include "../DecisionProcedure/visitors/restricters/NegationUnfolder.h"
#include <memory>

extern Timer timer_base;
extern VarToTrackMap varMap;
extern Offsets offsets;

template<class TemplatedAutomaton>
SymbolicAutomaton* baseToSymbolicAutomaton(ASTForm* form, bool doComplement) {
#   if (AUT_CONSTRUCT_BY_MONA == true)
    assert(form != nullptr);
    DFA *dfa = nullptr;
    int numVars = varMap.TrackLength();

    IdentList free, bound;
    form->freeVars(&free, &bound);

    toMonaAutomaton(form, dfa, true);
    assert(dfa != nullptr);

    // Fixme: free.empty() should be composed here somehow
    bool hasEmptyTracks = (free.empty() && form->kind != aTrue && form->kind != aFalse) || dfa->ns == 1;
    return new TemplatedAutomaton(dfa, varMap.TrackLength(), form, hasEmptyTracks);
#   else
    assert(false);
#   endif
}

SymbolicAutomaton* ASTForm::toSymbolicAutomaton(bool doComplement) {
    // If the sfa was already initialized (it is thus shared somehow),
    // we return the pointer, otherwise we first create the symbolic
    // automaton and return the thing.
    if(this->sfa == nullptr) {
#       if (OPT_USE_DAG == true)
        bool isComplemented;
        DagNodeCache* cache = (doComplement ? SymbolicAutomaton::dagNegNodeCache : SymbolicAutomaton::dagNodeCache);
        auto key = this;
        // First look into the dag, if there is already something structurally similar
        if(!cache->retrieveFromCache(key, this->sfa)) {
            // If yes, return the automaton (the mapping will be constructed internally)
#       endif
            if (this->tag == 0) {
                // It was tagged to be constructed by MONA
                this->sfa = baseToSymbolicAutomaton<GenericBaseAutomaton>(this, doComplement);
            } else {
                assert(this->fixpoint_number > 0 || this->tag == 1);
                this->sfa = this->_toSymbolicAutomatonCore(doComplement);
            }
#       if (OPT_USE_DAG == true)
            cache->StoreIn(key, this->sfa);
        }
#       endif
        if(this->is_restriction) {
            this->sfa->MarkAsRestriction();
        }
    }
    return this->sfa;
}

SymbolicAutomaton* ASTForm::_toSymbolicAutomatonCore(bool doComplement) {
#   if (AUT_CONSTRUCT_BY_MONA == true)
    return baseToSymbolicAutomaton<GenericBaseAutomaton>(this, doComplement);
#   else
    this->dump();
    assert(false && "Missing automaton for this automaton");
#   endif
}

SymbolicAutomaton* ASTForm_True::_toSymbolicAutomatonCore(bool doComplement) {
    return baseToSymbolicAutomaton<TrueAutomaton>(this, doComplement);
}

SymbolicAutomaton* ASTForm_False::_toSymbolicAutomatonCore(bool doComplement) {
    return baseToSymbolicAutomaton<FalseAutomaton>(this, doComplement);
}

SymbolicAutomaton* ASTForm_In::_toSymbolicAutomatonCore(bool doComplement) {
    return baseToSymbolicAutomaton<InAutomaton>(this, doComplement);
}

SymbolicAutomaton* ASTForm_FirstOrder::_toSymbolicAutomatonCore(bool doComplement) {
    return baseToSymbolicAutomaton<FirstOrderAutomaton>(this, doComplement);
}

SymbolicAutomaton* ASTForm_Equal1::_toSymbolicAutomatonCore(bool doComplement) {
    return baseToSymbolicAutomaton<EqualFirstAutomaton>(this, doComplement);
}

SymbolicAutomaton* ASTForm_Equal2::_toSymbolicAutomatonCore(bool doComplement) {
    return baseToSymbolicAutomaton<EqualSecondAutomaton>(this, doComplement);
}

SymbolicAutomaton* ASTForm_Less::_toSymbolicAutomatonCore(bool doComplement) {
    return baseToSymbolicAutomaton<LessAutomaton>(this, doComplement);
}

SymbolicAutomaton* ASTForm_LessEq::_toSymbolicAutomatonCore(bool doComplement) {
    return baseToSymbolicAutomaton<LessEqAutomaton>(this, doComplement);
}

SymbolicAutomaton* ASTForm_Sub::_toSymbolicAutomatonCore(bool doComplement) {
    return baseToSymbolicAutomaton<SubAutomaton>(this, doComplement);
}

/**
 * Returns IntersectionAutomaton consisting of converted left and right automaton
 *
 * @param doComplement: true if we are making complementon
 */
SymbolicAutomaton* ASTForm_And::_toSymbolicAutomatonCore(bool doComplement) {
    SymbolicAutomaton* lhs_aut;
    lhs_aut = this->f1->toSymbolicAutomaton(doComplement);
    SymbolicAutomaton* rhs_aut;
    rhs_aut = this->f2->toSymbolicAutomaton(doComplement);
    return new IntersectionAutomaton(lhs_aut, rhs_aut, this);
}

SymbolicAutomaton* ASTForm_Or::_toSymbolicAutomatonCore(bool doComplement) {
    SymbolicAutomaton* lhs_aut;
    lhs_aut = this->f1->toSymbolicAutomaton(doComplement);
    SymbolicAutomaton* rhs_aut;
    rhs_aut = this->f2->toSymbolicAutomaton(doComplement);
    return new UnionAutomaton(lhs_aut, rhs_aut, this);
}

bool is_base_automaton(ASTForm* f) {
    return f->kind != aOr &&
           f->kind != aAnd &&
           f->kind != aEx2;
}

SymbolicAutomaton* ASTForm_Not::_toSymbolicAutomatonCore(bool doComplement) {
#   if (OPT_DRAW_NEGATION_IN_BASE == true)
    if(is_base_automaton(this->f)) {
        return baseToSymbolicAutomaton<GenericBaseAutomaton>(this, !doComplement);
    }
#   endif
    SymbolicAutomaton* aut;
    aut = this->f->toSymbolicAutomaton(!doComplement);
    return new ComplementAutomaton(aut, this);
}

SymbolicAutomaton* ASTForm_Ex2::_toSymbolicAutomatonCore(bool doComplement) {
    SymbolicAutomaton* aut;
    aut = this->f->toSymbolicAutomaton(doComplement);
    return new ProjectionAutomaton(aut, this);
}