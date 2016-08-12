//
// Created by Raph on 07/10/2015.
//

#include "../Frontend/ast.h"
#include "../Frontend/timer.h"
#include "../Frontend/offsets.h"
#include "../Frontend/env.h"
#include "../DecisionProcedure/containers/SymbolicAutomata.h"
#include "../DecisionProcedure/environment.hh"
#include "../DecisionProcedure/automata.hh"
#include "../DecisionProcedure/visitors/restricters/NegationUnfolder.h"
#include <memory>

extern Timer timer_base;
extern VarToTrackMap varMap;
extern Offsets offsets;
extern Options options;

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
        DagNodeCache* cache = (doComplement ? SymbolicAutomaton::dagNegNodeCache : SymbolicAutomaton::dagNodeCache);
        auto key = this;
        // First look into the dag, if there is already something structurally similar
        if(!cache->retrieveFromCache(key, this->sfa)) {
            // If yes, return the automaton (the mapping will be constructed internally)
#       endif
#           if (OPT_USE_BASE_PROJECTION_AUTOMATA == true)
            if(this->tag == 0 || this->fixpoint_number == 0) {
#           else
            if(this->tag == 0) {
#           endif
                // It was tagged to be constructed by MONA
                this->sfa = baseToSymbolicAutomaton<GenericBaseAutomaton>(this, doComplement);
            } else {
#               if (OPT_CREATE_QF_AUTOMATON == TRUE)
                assert(this->fixpoint_number > 0 || this->tag == 1 || options.inverseFixLimit != -1);
#               endif
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

template<class FormClass>
bool has_consecutive_products(FormClass* form) {
    ASTKind k = form->kind;
    if(form->f1->kind == k && form->f1->fixpoint_number > 0) {
        FormClass* f1_form = static_cast<FormClass*>(form->f1);
        return    (   form->f2->kind == k &&    form->f2->fixpoint_number > 0)
               || (f1_form->f1->kind == k && f1_form->f1->fixpoint_number > 0)
               || (f1_form->f2->kind == k && f1_form->f2->fixpoint_number > 0);
    } else if(form->f2->kind == k) {
        assert(form->f1->kind != k || form->f1->fixpoint_number == 0);
        FormClass* f2_form = static_cast<FormClass*>(form->f2);
        return    (f2_form->f1->kind == k && f2_form->f1->fixpoint_number > 0)
               || (f2_form->f2->kind == k && f2_form->f2->fixpoint_number > 0);
    } else {
        return false;
    }
}

template<class FormClass, class BinaryProduct, class TernaryProduct, class NaryProduct>
SymbolicAutomaton* product_to_automaton(FormClass* form, bool doComplement) {
    ASTKind k = form->kind;
#   if (OPT_USE_NARY_AUTOMATA == true)
    if(has_consecutive_products<FormClass>(form)) {
#   else
    if(false) {
#   endif
        // Create nary representation
        return new NaryProduct(form, doComplement);
#   if (OPT_USE_TERNARY_AUTOMATA == true)
    } else if(form->f1->kind == k && form->f1->fixpoint_number > 0) {
        // Create ternary representation
        ASTForm_ff* form_f1 = static_cast<ASTForm_ff*>(form->f1);
        SymbolicAutomaton* lhs_aut;
        lhs_aut = form_f1->f1->toSymbolicAutomaton(doComplement);
        SymbolicAutomaton* mhs_aut;
        mhs_aut = form_f1->f2->toSymbolicAutomaton(doComplement);
        SymbolicAutomaton* rhs_aut;
        rhs_aut = form->f2->toSymbolicAutomaton(doComplement);
        return new TernaryProduct(lhs_aut, mhs_aut, rhs_aut, form);
    } else if(form->f2->kind == k && form->f1->kind != k && form->f2->fixpoint_number > 0) {
        // Create ternary representation
        ASTForm_ff* form_f2 = static_cast<ASTForm_ff*>(form->f2);
        SymbolicAutomaton* lhs_aut;
        lhs_aut = form->f1->toSymbolicAutomaton(doComplement);
        SymbolicAutomaton* mhs_aut;
        mhs_aut = form_f2->f1->toSymbolicAutomaton(doComplement);
        SymbolicAutomaton* rhs_aut;
        rhs_aut = form_f2->f2->toSymbolicAutomaton(doComplement);
        return new TernaryProduct(lhs_aut, mhs_aut, rhs_aut, form);
#   endif
    } else {
        // Create binary automaton
        SymbolicAutomaton* lhs_aut;
        lhs_aut = form->f1->toSymbolicAutomaton(doComplement);
        SymbolicAutomaton* rhs_aut;
#       if (OPT_EARLY_EVALUATION == true)
        rhs_aut = nullptr;
        form->f2->under_complement = doComplement;
#       else
        rhs_aut = form->f2->toSymbolicAutomaton(doComplement);
#       endif
        return new BinaryProduct(lhs_aut, rhs_aut, form);
    }

};

/**
 * Returns IntersectionAutomaton consisting of converted left and right automaton
 *
 * @param doComplement: true if we are making complementon
 */
SymbolicAutomaton* ASTForm_And::_toSymbolicAutomatonCore(bool doComplement) {
    return product_to_automaton<ASTForm_And, IntersectionAutomaton, TernaryIntersectionAutomaton, NaryIntersectionAutomaton>(this, doComplement);
}

SymbolicAutomaton* ASTForm_Or::_toSymbolicAutomatonCore(bool doComplement) {
    return product_to_automaton<ASTForm_Or, UnionAutomaton, TernaryUnionAutomaton, NaryUnionAutomaton>(this, doComplement);
}

SymbolicAutomaton* ASTForm_Biimpl::_toSymbolicAutomatonCore(bool doComplement) {
    return product_to_automaton<ASTForm_Biimpl, BiimplicationAutomaton, TernaryBiimplicationAutomaton, NaryBiimplicationAutomaton>(this, doComplement);
}

bool is_base_automaton(ASTForm* f) {
    return f->kind != aOr &&
           f->kind != aAnd &&
           f->kind != aEx2 &&
           f->kind != aNot;
}

SymbolicAutomaton* ASTForm_Not::_toSymbolicAutomatonCore(bool doComplement) {
#   if (OPT_DRAW_NEGATION_IN_BASE == true)
    if(is_base_automaton(this->f)) {
        return baseToSymbolicAutomaton<GenericBaseAutomaton>(this, !doComplement);
    }
#   endif
    SymbolicAutomaton* aut;
#   if (OPT_USE_BASE_PROJECTION_AUTOMATA == true)
    if( (this->tag == 0 || this->fixpoint_number == 1) && this->f->kind == aEx2) {
        SymbolicAutomaton* inner;
        inner = static_cast<ASTForm_Ex2*>(this->f)->f->toSymbolicAutomaton(!doComplement);
        return baseToSymbolicAutomaton<GenericBaseAutomaton>(this, !doComplement);
        //return new ComplementAutomaton(new ProjectionAutomaton(inner, this->f), this);
    } else {
        aut = this->f->toSymbolicAutomaton(!doComplement);
    }
#   else
    aut = this->f->toSymbolicAutomaton(!doComplement);
#   endif
    return new ComplementAutomaton(aut, this);
}

SymbolicAutomaton* ASTForm_Ex2::_toSymbolicAutomatonCore(bool doComplement) {
    SymbolicAutomaton* aut;
    aut = this->f->toSymbolicAutomaton(doComplement);
#   if (OPT_USE_BASE_PROJECTION_AUTOMATA == true)
    if(this->tag == 0 || this->fixpoint_number == 1) {
        assert(aut->type == AutType::BASE);
        return new BaseProjectionAutomaton(aut, this);
    } else {
        return new ProjectionAutomaton(aut, this);
    }
#   else
    return new ProjectionAutomaton(aut, this);
#   endif
}