/*****************************************************************************
 *  gaston - We pay homage to Gaston, an Africa-born brown fur seal who
 *    escaped the Prague Zoo during the floods in 2002 and made a heroic
 *    journey for freedom of over 300km all the way to Dresden. There he
 *    was caught and subsequently died due to exhaustion and infection.
 *    Rest In Piece, brave soldier.
 *
 *  Copyright (c) 2015  Tomas Fiedor <ifiedortom@fit.vutbr.cz>
 *      Notable mentions: Ondrej Lengal <ondra.lengal@gmail.com>
 *
 *  Description:
 *      Representation of Terms that are computed during the decision procedure
 *****************************************************************************/

#include "Term.h"
#include "TermEnumerator.h"
#include "../environment.hh"
#include <boost/functional/hash.hpp>
#include <future>
#include <algorithm>

extern Ident allPosVar;

namespace Gaston {
    size_t hash_value(Term* s) {
#       if (OPT_TERM_HASH_BY_APPROX == true)
        if (s->type == TERM_CONTINUATION) {
            // Todo: this is never hit fuck
            TermContinuation *sCont = static_cast<TermContinuation *>(s);
            if (sCont->IsUnfolded()) {
                return boost::hash_value(sCont->GetUnfoldedTerm());
            } else {
                return boost::hash_value(s);
            }
#       if (DEBUG_DONT_HASH_FIXPOINTS == true)
        } else if(s->type == TERM_FIXPOINT) {
            size_t seed = boost::hash_value(s->stateSpaceApprox);
            boost::hash_combine(seed, boost::hash_value(s->MeasureStateSpace()));
            return seed;
#       endif
        } else {
            return boost::hash_value(s);
        }
#       else
        return boost::hash_value(s->MeasureStateSpace());
#       endif
    }
}

// <<< STATIC MEMBER INITIALIZATION >>>

#define INIT_STATIC_MEASURE(prefix, measure) \
    size_t prefix::measure;

#define INIT_ALL_STATIC_MEASURES(measure) \
    TERM_TYPELIST(INIT_STATIC_MEASURE, measure)

TERM_MEASURELIST(INIT_ALL_STATIC_MEASURES)
#undef INIT_ALL_STATIC_MEASURES
#undef INIT_STATIC_MEASURE
size_t TermFixpoint::subsumedByHits = 0;
size_t TermFixpoint::preInstances = 0;
size_t TermFixpoint::isNotShared = 0;
size_t TermFixpoint::postponedTerms = 0;
size_t TermFixpoint::postponedProcessed = 0;
size_t TermContinuation::continuationUnfolding = 0;
size_t TermContinuation::unfoldInSubsumption = 0;
size_t TermContinuation::unfoldInIsectNonempty = 0;
size_t Term::partial_subsumption_hits = 0;
#if (MEASURE_BASE_SIZE == true)
size_t TermBaseSet::maxBaseSize = 0;
#endif

extern Ident lastPosVar, allPosVar;

// <<< TERM CONSTRUCTORS AND DESTRUCTORS >>>
Term::Term(Aut_ptr aut) : _aut(aut) {
    this->link = new link_t(nullptr, nullptr, 0);
}
Term::~Term() {
    delete this->link;
}

TermEmpty::TermEmpty(Aut_ptr aut, bool inComplement) : Term(aut) {
    #if (MEASURE_STATE_SPACE == true)
    ++TermEmpty::instances;
    #endif
    if(inComplement) {
        SET_IN_COMPLEMENT(this);
    }
    this->type = TERM_EMPTY;

    // Initialization of state space
    this->stateSpaceApprox = 0;

    #if (DEBUG_TERM_CREATION == true)
    std::cout << "[" << this << "]";
    std::cout << "TermEmpty::";
    this->dump();
    std::cout << "\n";
    #endif
}

/*Term::~Term() {
    this->_isSubsumedCache.clear();
    }*/

/**
    * Constructor of Term Product---construct intersecting product of
    * @p lhs and @p rhs
    *
    * @param[in] lhs:  left operand of term intersection
    * @param[in] rhs:  right operand of term intersection
    */
TermProduct::TermProduct(Aut_ptr aut, Term_ptr lhs, Term_ptr rhs, ProductType pt) : Term(aut), left(lhs), right(rhs) {
    #if (MEASURE_STATE_SPACE == true)
    ++TermProduct::instances;
    #endif

    this->type = TermType::TERM_PRODUCT;
    SET_PRODUCT_SUBTYPE(this, pt);

    // Initialization of state space
    this->stateSpaceApprox = this->left->stateSpaceApprox + (this->right != nullptr ? this->right->stateSpaceApprox : 0) + 1;
#   if (OPT_ENUMERATED_SUBSUMPTION_TESTING == true)
    this->enumerator = new ProductEnumerator(this);
#   endif

    #if (DEBUG_TERM_CREATION == true)
    std::cout << "TermProduct::";
    this->dump();
    std::cout << "\n";
    #endif
}

TermProduct::~TermProduct() {
    if(this->enumerator != nullptr) {
        delete this->enumerator;
    }
}

TermTernaryProduct::TermTernaryProduct(Aut_ptr aut, Term_ptr lhs, Term_ptr mhs, Term_ptr rhs, ProductType pt)
    : Term(aut), left(lhs), middle(mhs), right(rhs) {
#   if(MEASURE_STATE_SPACE == true)
    ++TermTernaryProduct::instances;
#   endif

    this->type = TermType::TERM_TERNARY_PRODUCT;
    SET_PRODUCT_SUBTYPE(this, pt);

    // Initialization of state space
    this->stateSpaceApprox = this->left->stateSpaceApprox + this->right->stateSpaceApprox + this->middle->stateSpaceApprox + 1;
    // Fixme: Add enumerator

#   if (DEBUG_TERM_CREATION == true)
    std::cout << "TermTernaryProduct::";
    this->dump();
    std::cout << "\n";
#   endif
}

TermTernaryProduct::~TermTernaryProduct() {

}

void TermNaryProduct::_InitNaryProduct(ProductType pt, size_t arity) {
#   if (MEASURE_STATE_SPACE == true)
    ++TermNaryProduct::instances;
#   endif
    this->arity = arity;
    this->access_vector = new size_t[this->arity];
    this->type = TermType::TERM_NARY_PRODUCT;
    SET_PRODUCT_SUBTYPE(this, pt);

    this->stateSpaceApprox = 0;
    for (int j = 0; j < this->arity; ++j) {
        this->access_vector[j] = static_cast<size_t>(j);
        this->stateSpaceApprox += this->terms[j]->stateSpaceApprox;
    }

#   if (DEBUG_TERM_CREATION == true)
    std::cout << "TermNaryProduct::";
    this->dump();
    std::cout << "\n";
#   endif
}

TermNaryProduct::TermNaryProduct(Aut_ptr aut, Term_ptr* terms, ProductType pt, size_t arity) : Term(aut) {
    this->terms = new Term_ptr[arity];
    std::copy(terms, terms+arity, this->terms);
    this->_InitNaryProduct(pt, arity);
}

TermNaryProduct::TermNaryProduct(Aut_ptr aut, SymLink* auts, StatesSetType st, ProductType pt, size_t arity) : Term(aut) {
    // Fixme: add link to auts
    this->terms = new Term_ptr[arity];
    for(auto i = 0; i < arity; ++i) {
        if(st == StatesSetType::E_INITIAL)
            this->terms[i] = auts[i].aut->GetInitialStates();
        else
            this->terms[i] = auts[i].aut->GetFinalStates();
    }
    this->_InitNaryProduct(pt, arity);
}

TermNaryProduct::TermNaryProduct(Aut_ptr aut, Term_ptr source, Symbol_ptr symbol, ProductType pt, size_t arity) : Term(aut) {
#   if (MEASURE_STATE_SPACE == true)
    ++TermNaryProduct::instances;
#   endif

    this->terms = new Term_ptr[arity];
    this->_InitNaryProduct(pt, arity);
}

TermNaryProduct::~TermNaryProduct() {
    delete[] this->access_vector;
    delete[] this->terms;
}

TermBaseSet::TermBaseSet(Aut_ptr aut, VATA::Util::OrdVector<size_t> && s) : Term(aut), states(std::move(s)) {
    #if (MEASURE_STATE_SPACE == true)
    ++TermBaseSet::instances;
    #endif
#   if (MEASURE_BASE_SIZE == true)
    size_t size = s.size();
    if(size > TermBaseSet::maxBaseSize) {
        TermBaseSet::maxBaseSize = size;
    }
#   endif
    type = TERM_BASE;
    assert(s.size() > 0);

    // Initialization of state space
    this->stateSpaceApprox = this->states.size();

    #if (DEBUG_TERM_CREATION == true)
    std::cout << "TermBaseSet::";
    this->dump();
    std::cout << "\n";
    #endif
}

TermBaseSet::~TermBaseSet() {
    this->states.clear();
}

TermContinuation::TermContinuation(Aut_ptr aut, SymLink* a, SymbolicAutomaton* init, Term* t, SymbolType* s, bool b, bool lazy)
        : Term(aut), aut(a), initAut(init), term(t), symbol(s), underComplement(b), lazyEval(lazy) {
    #if (DEBUG_TERM_CREATION == true)
    std::cout << "[" << this << "]";
    std::cout << "TermContinuation::";
    this->dump();
    std::cout << "\n";
    #endif
    #if (MEASURE_STATE_SPACE == true)
    ++TermContinuation::instances;
    #endif
    assert(t != nullptr || lazyEval);

    this->type = TERM_CONTINUATION;

    // Initialization of state space
    this->stateSpaceApprox = (t == nullptr ? 0 : t->stateSpaceApprox);

    #if (DEBUG_CONTINUATIONS == true)
    std::cout << "Postponing computation as [";
    t->dump();
    std::cout << "]\n";
    #endif
    SET_VALUE_NON_MEMBERSHIP_TESTING(this, b);
}

TermList::TermList(Aut_ptr aut, Term_ptr first, bool isCompl) : Term(aut) {
    #if (MEASURE_STATE_SPACE == true)
    ++TermList::instances;
    #endif

    this->type = TERM_LIST;

    // Initialization of state space
    this->stateSpaceApprox = first->stateSpaceApprox;

    this->list.push_back(first);
    SET_VALUE_NON_MEMBERSHIP_TESTING(this, isCompl);

    #if (DEBUG_TERM_CREATION == true)
    std::cout << "[" << this << "]";
    std::cout << "TermList::";
    this->dump();
    std::cout << "\n";
    #endif
}

TermFixpoint::TermFixpoint(Aut_ptr aut, Term_ptr startingTerm, Symbol* symbol, bool inComplement, bool initbValue, WorklistSearchType search = WorklistSearchType::E_DFS)
        : Term(aut), _sourceTerm(nullptr), _sourceSymbol(symbol), _sourceIt(nullptr), _baseAut(static_cast<ProjectionAutomaton*>(aut)->GetBase()),
          _guide(static_cast<ProjectionAutomaton*>(aut)->GetGuide()), _bValue(initbValue) {
    #if (MEASURE_STATE_SPACE == true)
    ++TermFixpoint::instances;
    #endif

#   if (ALT_SKIP_EMPTY_UNIVERSE == false)
    // Initialize the (counter)examples
    if(initbValue) {
        this->_satTerm = startingTerm;
    } else {
        this->_unsatTerm = startingTerm;
    }
#   endif

    // Initialize the aggregate function
    this->_InitializeAggregateFunction(inComplement);
    SET_VALUE_NON_MEMBERSHIP_TESTING(this, inComplement);
    this->type = TERM_FIXPOINT;
    this->_searchType = search;

    // Initialize the state space
    this->stateSpaceApprox = startingTerm->stateSpaceApprox;

    // Initialize the fixpoint as [nullptr, startingTerm]
    this->_fixpoint.push_front(std::make_pair(startingTerm, true));
    this->_fixpoint.push_front(std::make_pair(nullptr, true));

#   if (OPT_NO_SATURATION_FOR_M2L == true)
    // Push symbols to worklist
    if (static_cast<ProjectionAutomaton*>(aut)->IsRoot() || allPosVar == -1) {
#   endif
        this->_InitializeSymbols(&aut->symbolFactory, aut->GetFreeVars(),
                                     static_cast<ProjectionAutomaton *>(aut)->projectedVars, symbol);
#       if (DEBUG_RESTRICTION_DRIVEN_FIX == true)
        std::cout << "Created new fixpoint for '"; this->_baseAut->_form->dump(); std::cout << "'\n";
#       endif
#       if (OPT_WORKLIST_DRIVEN_BY_RESTRICTIONS == true)
        GuideTip tip;
        if(this->_guide == nullptr || (tip = this->_guide->GiveTip(startingTerm)) == GuideTip::G_PROJECT) {
#       endif
            for (auto symbol : this->_symList) {
#               if (OPT_WORKLIST_DRIVEN_BY_RESTRICTIONS == true)
#               if (DEBUG_RESTRICTION_DRIVEN_FIX == true)
                startingTerm->dump(); std::cout << " + " << (*symbol) << " -> ";
#               endif
                if (this->_guide != nullptr) {
                    switch (this->_guide->GiveTip(startingTerm, symbol)) {
                        case GuideTip::G_FRONT:
#                           if (DEBUG_RESTRICTION_DRIVEN_FIX == true)
                            std::cout << "G_FRONT\n";
#                          endif
                            this->_worklist.insert(this->_worklist.cbegin(), std::make_pair(startingTerm, symbol));
                            break;
                        case GuideTip::G_BACK:
#                           if (DEBUG_RESTRICTION_DRIVEN_FIX == true)
                            std::cout << "G_BACK\n";
#                           endif
                            this->_worklist.push_back(std::make_pair(startingTerm, symbol));
                            break;
                        case GuideTip::G_THROW:
#                           if (DEBUG_RESTRICTION_DRIVEN_FIX == true)
                            std::cout << "G_THROW\n";
#                           endif
                            break;
                        default:
                            assert(false && "Unsupported guiding tip\n");
                    }
                } else {
#                   if (DEBUG_RESTRICTION_DRIVEN_FIX == true)
                    std::cout << "insert\n";
#                   endif
                    this->_worklist.insert(this->_worklist.cbegin(), std::make_pair(startingTerm, symbol));
                }
#               else
                this->_worklist.insert(this->_worklist.cbegin(), std::make_pair(startingTerm, symbol));
#               endif
            }
#       if (OPT_WORKLIST_DRIVEN_BY_RESTRICTIONS == true)
        } else {
            assert(tip == GuideTip::G_PROJECT_ALL);
            this->_worklist.insert(this->_worklist.cbegin(), std::make_pair(startingTerm, this->_projectedSymbol));
        }
#       endif
        assert(this->_worklist.size() > 0 || startingTerm->type == TERM_EMPTY);
#   if (OPT_NO_SATURATION_FOR_M2L == true)
    }
#   endif

#   if (DEBUG_TERM_CREATION == true)
    std::cout << "[" << this << "]";
    std::cout << "TermFixpoint::";
    this->dump();
    std::cout << "\n";
#   endif
}

TermFixpoint::TermFixpoint(Aut_ptr aut, Term_ptr sourceTerm, Symbol* symbol, bool inComplement)
        : Term(aut), _sourceTerm(sourceTerm), _sourceSymbol(symbol), _sourceIt(static_cast<TermFixpoint*>(sourceTerm)->GetIteratorDynamic()),
          _guide(static_cast<ProjectionAutomaton*>(aut)->GetGuide()),
          _baseAut(static_cast<ProjectionAutomaton*>(aut)->GetBase()), _worklist(), _bValue(inComplement) {
    #if (MEASURE_STATE_SPACE == true)
    ++TermFixpoint::instances;
    ++TermFixpoint::preInstances;
    #endif
    assert(sourceTerm->type == TERM_FIXPOINT);

    // Initialize the state space
    this->stateSpaceApprox = sourceTerm->stateSpaceApprox;

    // Initialize the aggregate function
    this->_InitializeAggregateFunction(inComplement);
    this->type = TERM_FIXPOINT;
    this->_searchType = WorklistSearchType::E_DFS;
    SET_VALUE_NON_MEMBERSHIP_TESTING(this, inComplement);

    // Initialize the fixpoint
    this->_fixpoint.push_front(std::make_pair(nullptr, true));
    // Push things into worklist
    this->_InitializeSymbols(&aut->symbolFactory, aut->GetFreeVars(), static_cast<ProjectionAutomaton*>(aut)->projectedVars, symbol);

    #if (DEBUG_TERM_CREATION == true)
    std::cout << "[" << this << "]";
    std::cout << "TermFixpointPre::";
    this->dump();
    std::cout << "\n";
    #endif
}

TermFixpoint::~TermFixpoint() {
    this->_fixpoint.clear();
#   if (OPT_EARLY_EVALUATION == true)
    this->_postponed.clear();
#   endif
    this->_worklist.clear();
}

void Term::Complement() {
    FLIP_IN_COMPLEMENT(this);
}

/**
 * Sets the same link as for the @p term. If there is already some linked formed, we let it be.
 *
 * @param[in]  term  term we are aliasing link with
 */
void Term::SetSameSuccesorAs(Term* term) {
    if(this->link->succ == nullptr) {
        this->link->succ = term->link->succ;
        this->link->symbol = term->link->symbol;
        this->link->len = term->link->len;
    }
}

/**
 * Sets successor of the term to the pair of @p succ and @p symb and increments the lenght of the successor path.
 * If the successor is already set, we do nothing
 *
 * @param[in]  succ  successor of the term
 * @param[in]  symb  symbol we were subtracting from the @p succ
 */
void Term::SetSuccessor(Term* succ, Symbol* symb) {
    if(this->link->succ == nullptr) {
        this->link->succ = succ;
        this->link->symbol = symb;
        this->link->len = succ->link->len + 1;
    }
}

/**
 * Returns true if the term is not computed, i.e. it is continuation somehow
 */
bool Term::IsNotComputed() {
#if (OPT_EARLY_EVALUATION == true && MONA_FAIR_MODE == false)
    if(this->type == TERM_CONTINUATION) {
        return !static_cast<TermContinuation *>(this)->IsUnfolded();
    } else if(this->type == TERM_PRODUCT) {
        TermProduct* termProduct = static_cast<TermProduct*>(this);
        return termProduct->left->IsNotComputed() || (termProduct->right == nullptr || termProduct->right->IsNotComputed());
    } else {
        return false;
    }
#else
    return false;
#endif
}

/**
 * Tests the Term subsumption
 *
 * @param[in] t:    term we are testing subsumption against
 * @return:         true if this is subsumed by @p t
 */
SubsumptionResult Term::IsSubsumed(Term *t, int limit, Term** new_term, bool unfoldAll) {
    if(this == t) {
        return E_TRUE;
    }

#   if (OPT_PARTIALLY_LIMITED_SUBSUMPTION >= 0)
    if(!limit) {
        return (this == t ? E_TRUE : E_FALSE);
    }
#   endif

    // unfold the continuation
#   if (OPT_EARLY_EVALUATION == true)
    if(t->type == TERM_CONTINUATION) {
        TermContinuation *continuation = static_cast<TermContinuation *>(t);
        Term* unfoldedContinuation = continuation->unfoldContinuation(UnfoldedInType::E_IN_SUBSUMPTION);
        return this->IsSubsumed(unfoldedContinuation, limit, unfoldAll);
    } else if(this->type == TERM_CONTINUATION) {
        TermContinuation *continuation = static_cast<TermContinuation *>(this);
        Term* unfoldedContinuation = continuation->unfoldContinuation(UnfoldedInType::E_IN_SUBSUMPTION);
        return unfoldedContinuation->IsSubsumed(t, limit, unfoldAll);
    }
#   endif
    assert(GET_IN_COMPLEMENT(this) == GET_IN_COMPLEMENT(t));
    assert(this->type != TERM_CONTINUATION && t->type != TERM_CONTINUATION);
#   if (OPT_PARTIALLY_LIMITED_SUBSUMPTION > 0)
    --limit;
#   endif

    // Else if it is not continuation we first look into cache and then recompute if needed
    std::pair<SubsumptionResult, Term_ptr> result;
#   if (OPT_CACHE_SUBSUMES == true)
    //auto key = std::make_pair(&(*this), t);
    auto key = std::make_pair(static_cast<Term_ptr>(this), t);
    if(this->type == TERM_EMPTY || !this->_aut->_subCache.retrieveFromCache(key, result)) {
#   endif
        if (GET_IN_COMPLEMENT(this)) {
            if(this->type == TERM_EMPTY) {
                result.first = (t->type == TERM_EMPTY ? E_TRUE : E_FALSE);
            } else {
                result.first = t->_IsSubsumedCore(this, limit, new_term, unfoldAll);
            }
        } else {
            if(t->type == TERM_EMPTY) {
                result.first = (this->type == TERM_EMPTY ? E_TRUE : E_FALSE);
            } else {
                result.first = this->_IsSubsumedCore(t, limit, new_term, unfoldAll);
            }
        }
#   if (OPT_CACHE_SUBSUMES == true)
        if((result.first == E_TRUE || (result.first == E_PARTIALLY && new_term != nullptr)) && this->type != TERM_EMPTY) {
            if(result.first == E_PARTIALLY) {
                assert(*new_term != nullptr);
                result.second = *new_term;
            }
            this->_aut->_subCache.StoreIn(key, result);
        }
    } else if(new_term != nullptr && result.first == E_PARTIALLY) {
        assert(result.second != nullptr);
        *new_term = result.second;
    }
#   endif
    assert(!unfoldAll || result.first != E_PARTIALLY);
#   if (DEBUG_TERM_SUBSUMPTION == true)
    this->dump();
    std::cout << (result.first == E_TRUE ? " \u2291 " : " \u22E2 ");
    t->dump();
    std::cout << " = " << (result.first == E_TRUE ? "true" : "false") << "\n\n";
#   endif
    assert(!(result.first == E_PARTIALLY && (new_term != nullptr && *new_term == nullptr)));
    return result.first;
}

SubsumptionResult TermEmpty::_IsSubsumedCore(Term *t, int limit, Term** new_term, bool unfoldAll) {
    // Empty term is subsumed by everything (probably)
    return (GET_IN_COMPLEMENT(this)) ? (t->type == TERM_EMPTY ? E_TRUE : E_FALSE) : E_TRUE;
}

SubsumptionResult TermProduct::_IsSubsumedCore(Term *t, int limit, Term** new_term, bool unfoldAll) {
    assert(t->type == TERM_PRODUCT);

    // Retype and test the subsumption component-wise
    TermProduct *rhs = static_cast<TermProduct*>(t);
    Term *lhsl = this->left;
    Term *lhsr = this->right;
    Term *rhsl = rhs->left;
    Term *rhsr = rhs->right;

    // We are doing the partial testing
    if(lhsl->type == TermType::TERM_BASE && new_term != nullptr) {
        Term* inner_new_term = nullptr;
        SubsumptionResult result;

        // Fixme: There is some shit going on...
        if((result = lhsr->IsSubsumed(rhsr, limit, &inner_new_term, unfoldAll)) == E_FALSE) {

            return E_FALSE;
        } else if(result == E_TRUE) {
            assert(inner_new_term == nullptr);
            inner_new_term = lhsr;
        } else {
            assert(inner_new_term != nullptr);
            assert(result == E_PARTIALLY);
        }

        SubsumptionResult base_result = lhsl->IsSubsumed(rhsl, limit, new_term, unfoldAll);
        if(base_result == E_PARTIALLY) {
            assert(new_term != nullptr);
            assert(result != E_FALSE);
            *new_term = this->_aut->_factory.CreateProduct(*new_term, inner_new_term, IntToProductType(GET_PRODUCT_SUBTYPE(this)));
            (*new_term)->SetSameSuccesorAs(this);
            ++Term::partial_subsumption_hits;
            return E_PARTIALLY;
        } else {
            if (result == E_PARTIALLY && base_result != E_FALSE) {
                assert(base_result == E_TRUE);
                *new_term = this->_aut->_factory.CreateProduct(lhsl, inner_new_term, IntToProductType(GET_PRODUCT_SUBTYPE(this)));
                (*new_term)->SetSameSuccesorAs(this);
                ++Term::partial_subsumption_hits;
                return E_PARTIALLY;
            } else {
                assert(base_result == E_FALSE || base_result == E_TRUE);
                assert(result == E_TRUE || (result == E_PARTIALLY && base_result == E_FALSE));
                *new_term = nullptr;
                return base_result;
            }
        }
    }

    if(!unfoldAll && (lhsr->IsNotComputed() && rhsr->IsNotComputed())) {
        #if (OPT_EARLY_PARTIAL_SUB == true)
        if(lhsr->type == TERM_CONTINUATION && rhsr->type == TERM_CONTINUATION) {
            return (lhsl->IsSubsumed(rhsl, limit, nullptr, unfoldAll) == E_FALSE ? E_FALSE : E_PARTIALLY);
        } else {
            SubsumptionResult leftIsSubsumed = lhsl->IsSubsumed(rhsl, limit, nullptr, unfoldAll);
            if(leftIsSubsumed == E_TRUE) {
                return lhsr->IsSubsumed(rhsr, limit, nullptr, unfoldAll);
            } else {
                return leftIsSubsumed;
            }
        }
        #else
        return (lhsl->IsSubsumed(rhsl, limit) != E_FALSE && lhsr->IsSubsumed(rhsr, limit) != E_FALSE) ? E_TRUE : E_FALSE;
        #endif
    } if(!unfoldAll && lhsl == rhsl) {
        return lhsr->IsSubsumed(rhsr, limit, nullptr, unfoldAll);
    } else if(!unfoldAll && lhsr == rhsr) {
        return lhsl->IsSubsumed(rhsl, limit, nullptr, unfoldAll);
    } else {
        if(lhsl->stateSpaceApprox < lhsr->stateSpaceApprox || unfoldAll) {
            return (lhsl->IsSubsumed(rhsl, limit, nullptr, unfoldAll) != E_FALSE && lhsr->IsSubsumed(rhsr, limit, nullptr, unfoldAll) != E_FALSE) ? E_TRUE : E_FALSE;
        } else {
            return (lhsr->IsSubsumed(rhsr, limit, nullptr, unfoldAll) != E_FALSE && lhsl->IsSubsumed(rhsl, limit, nullptr, unfoldAll) != E_FALSE) ? E_TRUE : E_FALSE;
        }
    }
}

SubsumptionResult _ternary_subsumption_test(Term* f1, Term* f2, Term* s1, Term* s2, Term* l1, Term* l2, size_t approx_max, int limit, bool unfoldAll) {
    if(f1->IsSubsumed(f2, limit, nullptr, unfoldAll) != E_FALSE) {
        if(s1->stateSpaceApprox == approx_max) {
            return (l1->IsSubsumed(l2, limit, nullptr, unfoldAll) != E_FALSE && s1->IsSubsumed(s2, limit, nullptr, unfoldAll) != E_FALSE) ? E_TRUE : E_FALSE;
        } else {
            return (s1->IsSubsumed(s2, limit, nullptr, unfoldAll) != E_FALSE && l1->IsSubsumed(l2, limit, nullptr, unfoldAll) != E_FALSE) ? E_TRUE : E_FALSE;
        }
    } else {
        return E_FALSE;
    }
}

SubsumptionResult TermTernaryProduct::_IsSubsumedCore(Term *t, int limit, Term** new_term, bool unfoldAll) {
    assert(t->type == TERM_TERNARY_PRODUCT);

    // Retype and test the subsumption component-wise
    TermTernaryProduct *rhs = static_cast<TermTernaryProduct*>(t);
    Term *lhsl = this->left;
    Term *lhsm = this->middle;
    Term *lhsr = this->right;
    Term *rhsl = rhs->left;
    Term *rhsm = rhs->middle;
    Term *rhsr = rhs->right;

    size_t approx_max = std::max({lhsl->stateSpaceApprox, lhsm->stateSpaceApprox, lhsr->stateSpaceApprox});
    size_t approx_min = std::min({lhsl->stateSpaceApprox, lhsm->stateSpaceApprox, lhsr->stateSpaceApprox});
    if(lhsl->stateSpaceApprox == approx_min) {
        return _ternary_subsumption_test(lhsl, rhsl, lhsm, rhsm, lhsr, rhsr, approx_max, limit, unfoldAll);
    } else if(lhsr->stateSpaceApprox == approx_min) {
        return _ternary_subsumption_test(lhsr, rhsr, lhsl, rhsl, lhsm, rhsm, approx_max, limit, unfoldAll);
    } else {
        return _ternary_subsumption_test(lhsm, rhsm, lhsr, rhsr, lhsl, rhsl, approx_max, limit, unfoldAll);
    }
}

SubsumptionResult TermNaryProduct::_IsSubsumedCore(Term *t, int limit, Term** new_term, bool b) {
    assert(t->type == TERM_NARY_PRODUCT);

    // Retype and test the subsumption component-wise
    TermNaryProduct *rhs = static_cast<TermNaryProduct*>(t);
    assert(this->arity == rhs->arity);
    for (int i = 0; i < this->arity; ++i) {
        assert(this->access_vector[i] < this->arity);
        if(this->terms[this->access_vector[i]]->IsSubsumed(rhs->terms[this->access_vector[i]], limit, nullptr, b) == E_FALSE) {
            if(i != 0) {
                // Propagate the values towards 0 index
                this->access_vector[i] ^= this->access_vector[i-1];
                this->access_vector[i-1] ^= this->access_vector[i];
                this->access_vector[i] ^= this->access_vector[i-1];
            }
            return E_FALSE;
        }
    }

    return E_TRUE;
}

/*
 * Efficient integer comparison taken from: http://stackoverflow.com/a/10997428
 */
inline int compare_integers (int a, int b) {
    __asm__ __volatile__ (
    "sub %1, %0 \n\t"
            "jno 1f \n\t"
            "cmc \n\t"
            "rcr %0 \n\t"
            "1: "
    : "+r"(a)
    : "r"(b)
    : "cc");
    return a;
}

SubsumptionResult TermBaseSet::_IsSubsumedCore(Term *term, int limit, Term** new_term, bool unfoldAll) {
    assert(term->type == TERM_BASE);
    TermBaseSet *t = static_cast<TermBaseSet*>(term);

#   if (OPT_SUBSUMPTION_INTERSECTION == true)
    // Fixme: Create TermBaseSet
    if(new_term != nullptr && !this->_aut->_form->is_restriction) {
        TermBaseSetStates diff;
        bool is_nonempty_diff = this->states.SetDifference(t->states, diff);
        *new_term = nullptr;
        if(is_nonempty_diff) {
            if(diff.size() == this->states.size()) {
                return E_FALSE;
            } else {
                *new_term = this->_aut->_factory.CreateBaseSet(std::move(diff), 0, 0);
                (*new_term)->SetSameSuccesorAs(this);
                return E_PARTIALLY;
            }
        } else {
            return E_TRUE;
        }
    } else {
        if(t->stateSpaceApprox < this->stateSpaceApprox) {
            return E_FALSE;
        } else {
            return this->states.IsSubsetOf(t->states) ? E_TRUE : E_FALSE;
        }
    }
#   else
    // Test component-wise, not very efficient though
    // TODO: Change to bit-vectors if possible
    if(t->stateSpaceApprox < this->stateSpaceApprox) {
        return E_FALSE;
    } else {
        return this->states.IsSubsetOf(t->states) ? E_TRUE : E_FALSE;
    }
#   endif
}

SubsumptionResult TermContinuation::_IsSubsumedCore(Term *t, int limit, Term** new_term, bool unfoldAll) {
    // TODO: How to do this smartly?
    // TODO: Maybe if we have {} we can answer sooner, without unpacking
    assert(false);
    return E_FALSE;
}

SubsumptionResult TermList::_IsSubsumedCore(Term *t, int limit, Term** new_term, bool unfoldAll) {
    assert(t->type == TERM_LIST);

    // Reinterpret
    TermList* tt = static_cast<TermList*>(t);
    // Do the item-wise subsumption check
    for(auto& item : this->list) {
        bool subsumes = false;
        for(auto& tt_item : tt->list) {
            if(item->IsSubsumed(tt_item, limit, nullptr, unfoldAll)) {
                subsumes = true;
                break;
            }
        }
        if(!subsumes) return E_FALSE;
    }

    return E_TRUE;
}

SubsumptionResult TermFixpoint::_IsSubsumedCore(Term *t, int limit, Term** new_term, bool unfoldAll) {
    assert(t->type == TERM_FIXPOINT);

    // Reinterpret
    TermFixpoint* tt = static_cast<TermFixpoint*>(t);

#   if (OPT_UNFOLD_FIX_DURING_SUB == false)
    bool are_source_symbols_same = TermFixpoint::_compareSymbols(*this, *tt);
    // Worklists surely differ
    if(!are_source_symbols_same && (this->_worklist.size() != 0/* || tt->_worklist.size() != 0*/) ) {
        return E_FALSE;
    }
#   endif

#   if (OPT_SHORTTEST_FIXPOINT_SUB == true)
    // Will tests only generators and symbols
    if(are_source_symbols_same && this->_sourceTerm != nullptr && tt->_sourceTerm != nullptr) {
        return this->_sourceTerm->IsSubsumed(tt->_sourceTerm, limit, nullptr, unfoldAll);
    }
#   endif

    // Do the piece-wise comparison
    Term* tptr;
    for(auto& item : this->_fixpoint) {
        // Skip the nullptr
        if(item.first == nullptr || !item.second) continue;
        if((item.first)->IsSubsumedBy(tt->_fixpoint, tt->_worklist, tptr, true) == E_FALSE) {
            return E_FALSE;
        }
    }
#   if (OPT_UNFOLD_FIX_DURING_SUB == true)
    if(this->_worklist.size() == 0 && tt->_worklist.size() == 0) {
        return E_TRUE;
    } else {
        // We'll start to unfold the fixpoints;
        auto this_it = this->GetIterator();
        auto tt_it = tt->GetIterator();
        Term_ptr tit, ttit, temp;
        SubsumptionResult result;
        while( (tit = this_it.GetNext()) != nullptr && (ttit = tt_it.GetNext()) != nullptr) {
            if(tit != nullptr) {
                if((result = tit->IsSubsumedBy(tt->_fixpoint, temp)) == E_FALSE) {
                    return E_FALSE;
                }
                assert(result != E_PARTIALLY && "Continuations currently do not work with this optimizations!");
            }
            if(ttit != nullptr) {
                if((result = ttit->IsSubsumedBy(this->_fixpoint, temp)) == E_FALSE) {
                    return E_FALSE;
                }
                assert(result != E_PARTIALLY && "Continuations currently do not work with this optimizations!");
            }
        }
        return E_TRUE;
    }
#   else
    return ( (this->_worklist.size() == 0 /*&& tt->_worklist.size() == 0*/) ? E_TRUE : (are_source_symbols_same ? E_TRUE : E_FALSE));
    // Happy reading ^^                   ^---- Fixme: maybe this is incorrect?
#   endif
}

/**
 * Tests the subsumption over the list of terms
 *
 * @param[in] fixpoint:     list of terms contained as fixpoint
 */
SubsumptionResult TermEmpty::IsSubsumedBy(FixpointType& fixpoint, WorklistType& worklist, Term*& biggerTerm, bool no_prune) {
    // Empty term is subsumed by everything
    // Fixme: Complemented fixpoint should subsume everything right?
    return ( ( (fixpoint.size() == 1 && fixpoint.front().first == nullptr) || GET_IN_COMPLEMENT(this)) ? E_FALSE : E_TRUE);
}

void prune_worklist(WorklistType& worklist, Term*& item) {
    for(auto it = worklist.begin(); it != worklist.end();) {
        if(it->first == item) {
            it = worklist.erase(it);
        } else {
            ++it;
        }
    }
}

SubsumptionResult TermProduct::IsSubsumedBy(FixpointType& fixpoint, WorklistType& worklist, Term*& biggerTerm, bool no_prune) {
    //Fixme: is this true?
    if(this->IsEmpty()) {
        return E_TRUE;
#   if (OPT_ENUMERATED_SUBSUMPTION_TESTING == true)
    } else {
        bool isEmpty = true;
        for (auto &item : fixpoint) {
            if (item.first == nullptr || !item.second)
                continue;
            isEmpty = false;
            break;
        }
        if (isEmpty)
            return E_FALSE;
#   endif
    }

    // For each item in fixpoint
    Term* tested_term = this;
    Term* new_term = nullptr;
    for(auto& item : fixpoint) {
        // Nullptr is skipped
        if(item.first == nullptr || !item.second) continue;

        // Test the subsumption
        SubsumptionResult result;
        if(!no_prune) {
            result = tested_term->IsSubsumed(item.first, OPT_PARTIALLY_LIMITED_SUBSUMPTION, &new_term);
        } else {
            result = tested_term->IsSubsumed(item.first, OPT_PARTIALLY_LIMITED_SUBSUMPTION, nullptr);
        }

        if(result == SubsumptionResult::E_TRUE) {
            return E_TRUE;
        } else if(result == SubsumptionResult::E_PARTIALLY && !no_prune) {
            assert(new_term != tested_term);
            assert(new_term != nullptr);
            tested_term = new_term;
        }

        if(!no_prune) {
            if (item.first->IsSubsumed(tested_term, OPT_PARTIALLY_LIMITED_SUBSUMPTION)) {
#               if (OPT_PRUNE_WORKLIST == true)
                prune_worklist(worklist, item.first);
#               endif
                item.second = false;
            }
        }
    }

    if(no_prune) {
        return E_FALSE;
    }

#   if (OPT_ENUMERATED_SUBSUMPTION_TESTING == true)
    this->enumerator->FullReset();
    while (this->enumerator->IsNull() == false) {
        bool subsumed = false;
        for (auto &item : fixpoint) {
            if (item.first == nullptr || !item.second) continue;

            if (item.first->Subsumes(this->enumerator) != E_FALSE) {
                this->enumerator->Next();
                subsumed = true;
                break;
            }
        }

        if (!subsumed) {
            return E_FALSE;
        }
    }
    return E_TRUE;
#   else
    if(tested_term == this || no_prune) {
        return E_FALSE;
    } else {
        assert(tested_term != nullptr);
        assert(!tested_term->IsEmpty());
        biggerTerm = tested_term;
        return E_PARTIALLY;
    }
#   endif
}

SubsumptionResult TermTernaryProduct::IsSubsumedBy(FixpointType& fixpoint, WorklistType& worklist, Term *& biggerTerm, bool no_prune) {
    if(this->IsEmpty()) {
        return E_TRUE;
    }

    for(auto& item : fixpoint) {
        if(item.first == nullptr || !item.second) continue;

        if(this->IsSubsumed(item.first, OPT_PARTIALLY_LIMITED_SUBSUMPTION) != E_FALSE) {
            return E_TRUE;
        }

        if(!no_prune) {
            if(item.first->IsSubsumed(this, OPT_PARTIALLY_LIMITED_SUBSUMPTION) != E_FALSE) {
#               if (OPT_PRUNE_WORKLIST == true)
                prune_worklist(worklist, item.first);
#               endif
                item.second = false;
            }
        }
    }

    return E_FALSE;
}

SubsumptionResult TermNaryProduct::IsSubsumedBy(FixpointType& fixpoint, WorklistType& worklist, Term *& biggerTerm, bool no_prune) {
    if(this->IsEmpty()) {
        return E_TRUE;
    }

    for(auto& item : fixpoint) {
        if(item.first == nullptr || !item.second) continue;

        if(this->IsSubsumed(item.first, OPT_PARTIALLY_LIMITED_SUBSUMPTION)) {
            return E_TRUE;
        }

        if(!no_prune) {
            if(item.first->IsSubsumed(this, OPT_PARTIALLY_LIMITED_SUBSUMPTION)) {
#               if (OPT_PRUNE_WORKLIST == true)
                prune_worklist(worklist, item.first);
#               endif
                item.second = false;
            }
        }
    }

    return E_FALSE;
}

SubsumptionResult TermBaseSet::IsSubsumedBy(FixpointType& fixpoint, WorklistType& worklist, Term*& biggerTerm, bool no_prune) {
    if(this->IsEmpty()) {
        return E_TRUE;
    }
    // For each item in fixpoint
    Term* tested_term = this;
    Term* new_term = nullptr;
    for(auto& item : fixpoint) {
        // Nullptr is skipped
        if(item.first == nullptr || !item.second) continue;

        // Test the subsumption
        SubsumptionResult result;
        if(!no_prune) {
            result = tested_term->IsSubsumed(item.first, OPT_PARTIALLY_LIMITED_SUBSUMPTION, &new_term);
        } else {
            result = tested_term->IsSubsumed(item.first, OPT_PARTIALLY_LIMITED_SUBSUMPTION, nullptr);
        }
        switch(result) {
            case SubsumptionResult::E_TRUE:
                return E_TRUE;
            case SubsumptionResult::E_FALSE:
                break;
            case SubsumptionResult::E_PARTIALLY:
                assert(new_term != nullptr);
                assert(!no_prune);
                tested_term = new_term;
                break;
            default:
                assert(false && "Unsupported subsumption returned");
        }

        if(!no_prune) {
            if (item.first->IsSubsumed(tested_term, OPT_PARTIALLY_LIMITED_SUBSUMPTION)) {
#               if (OPT_PRUNE_WORKLIST == true)
                prune_worklist(worklist, item.first);
#               endif
                item.second = false;
            }
        }
    }

    if(tested_term == this || no_prune) {
        return E_FALSE;
    } else {
        assert(tested_term != nullptr);
        assert(!tested_term->IsEmpty());
        biggerTerm = tested_term;
        return E_PARTIALLY;
    }
}

SubsumptionResult TermContinuation::IsSubsumedBy(FixpointType& fixpoint, WorklistType& worklist, Term*& biggerTerm, bool no_prune) {
    assert(false && "TermContSubset.IsSubsumedBy() is impossible to happen~!");
}

SubsumptionResult TermList::IsSubsumedBy(FixpointType& fixpoint, WorklistType& worklist, Term*& biggerTerm, bool no_prune) {
    if(this->IsEmpty()) {
        return E_TRUE;
    }
    // For each item in fixpoint
    for(auto& item : fixpoint) {
        // Nullptr is skipped
        if(item.first == nullptr || !item.second) continue;

        if (this->IsSubsumed(item.first, OPT_PARTIALLY_LIMITED_SUBSUMPTION)) {
            return E_TRUE;
        }

        if(!no_prune) {
            if (item.first->IsSubsumed(this, OPT_PARTIALLY_LIMITED_SUBSUMPTION)) {
#               if (OPT_PRUNE_WORKLIST == true)
                prune_worklist(worklist, item.first);
#               endif
                item.second = false;
            }
        }
    }

    return E_FALSE;
}

SubsumptionResult TermFixpoint::IsSubsumedBy(FixpointType& fixpoint, WorklistType& worklist, Term*& biggerTerm, bool no_prune) {
    auto result = E_FALSE;
    // Component-wise comparison
    for(auto& item : fixpoint) {
        if(item.first == nullptr || !item.second) continue;

        if (this->IsSubsumed(item.first, OPT_PARTIALLY_LIMITED_SUBSUMPTION)) {
            result = E_TRUE;
            break;
        }

        if(!no_prune) {
            if (item.first->IsSubsumed(this, OPT_PARTIALLY_LIMITED_SUBSUMPTION)) {
#               if (OPT_PRUNE_WORKLIST == true)
                prune_worklist(worklist, item.first);
#               endif
                item.second = false;
            }
        }

    }
#if (DEBUG_TERM_SUBSUMED_BY == true)
    this->dump();
    std::cout << (result == E_TRUE ? " \u2286 " : " \u2288 ");
    std::cout << "{";
    for(auto& item : fixpoint) {
        if(item.first == nullptr || !item.second) continue;
        item.first->dump();
        std::cout << ",";
    }
    std::cout << "}";
    std::cout << "\n";
#endif
    return result;
}

/**
 * Tests if the single element is subsumed by the term
 */
SubsumptionResult Term::Subsumes(TermEnumerator* enumerator) {
    SubsumptionResult result;
#   if (OPT_ENUMERATED_SUBSUMPTION_TESTING == true)
    if(!this->_subsumesCache.retrieveFromCache(enumerator, result)) {
        result = this->_SubsumesCore(enumerator);
        if(result != E_FALSE)
            this->_subsumesCache.StoreIn(enumerator, result);
    }
#   endif
    return result;
}

SubsumptionResult Term::_SubsumesCore(TermEnumerator* enumerator) {
    assert(enumerator->type == ENUM_GENERIC);
    GenericEnumerator* genericEnumerator = static_cast<GenericEnumerator*>(enumerator);
    auto result = genericEnumerator->GetItem()->IsSubsumed(this, OPT_PARTIALLY_LIMITED_SUBSUMPTION, nullptr, false);
    return result;
}

SubsumptionResult TermProduct::_SubsumesCore(TermEnumerator* enumerator) {
    // TODO: Missing partial subsumption
    assert(enumerator->type == ENUM_PRODUCT);
    ProductEnumerator* productEnumerator = static_cast<ProductEnumerator*>(enumerator);
    if(this->left->stateSpaceApprox <= this->right->stateSpaceApprox) {
        return (this->left->Subsumes(productEnumerator->GetLeft()) != E_FALSE &&
                this->right->Subsumes(productEnumerator->GetRight()) != E_FALSE) ? E_TRUE : E_FALSE;
    } else {
        return (this->right->Subsumes(productEnumerator->GetRight()) != E_FALSE &&
                this->left->Subsumes(productEnumerator->GetLeft()) != E_FALSE) ? E_TRUE : E_FALSE;
    }
}

SubsumptionResult TermBaseSet::_SubsumesCore(TermEnumerator* enumerator) {
    assert(enumerator->type == ENUM_BASE);
    BaseEnumerator* baseEnumerator = static_cast<BaseEnumerator*>(enumerator);
    auto item = baseEnumerator->GetItem();

    for(auto state : this->states) {
        if(state == item) {
            return E_TRUE;
        } else if(state > item) {
            return E_FALSE;
        }
    }

    return E_FALSE;
}

/**
 * Tests the emptiness of Term
 */
bool TermEmpty::IsEmpty() {
    return !GET_IN_COMPLEMENT(this);
}

bool TermProduct::IsEmpty() {
    return this->left->IsEmpty() && this->right->IsEmpty();
}

bool TermTernaryProduct::IsEmpty() {
    return this->left->IsEmpty() && this->middle->IsEmpty() && this->right->IsEmpty();
}

bool TermNaryProduct::IsEmpty() {
    for (int i = 0; i < this->arity; ++i) {
        if(!this->terms[i]->IsEmpty()) {
            return false;
        }
    }

    return true;
}

bool TermBaseSet::IsEmpty() {
    return this->states.size() == 0;
}

bool TermContinuation::IsEmpty() {
    return false;
}

bool TermList::IsEmpty() {
    if(this->list.size() == 0) {
        return true;
    } else {
        for(auto& item : this->list) {
            if(!item->IsEmpty()) {
                return false;
            }
        }
        return true;
    }
}

bool TermFixpoint::IsEmpty() {
    return this->_fixpoint.empty() && this->_worklist.empty();
}

/**
 * Measures the state space as number of states
 */
unsigned int Term::MeasureStateSpace() {
    return this->_MeasureStateSpaceCore();
}

unsigned int TermEmpty::_MeasureStateSpaceCore() {
    return 0;
}

unsigned int TermProduct::_MeasureStateSpaceCore() {
    return this->left->MeasureStateSpace() + this->right->MeasureStateSpace() + 1;
}

unsigned int TermTernaryProduct::_MeasureStateSpaceCore() {
    return this->left->MeasureStateSpace() + this->middle->MeasureStateSpace() + this->right->MeasureStateSpace() + 1;
}

unsigned int TermNaryProduct::_MeasureStateSpaceCore() {
    size_t state_space = 0;
    for (int i = 0; i < this->arity; ++i) {
        state_space += this->terms[i]->MeasureStateSpace();
    }
    return state_space + 1;
}

unsigned int TermBaseSet::_MeasureStateSpaceCore() {
    return this->stateSpaceApprox;
}

unsigned int TermContinuation::_MeasureStateSpaceCore() {
    return 1;
}

unsigned int TermList::_MeasureStateSpaceCore() {
    unsigned int count = 1;
    // Measure state spaces of all subterms
    for(auto& item : this->list) {
        count += item->MeasureStateSpace();
    }

    return count;
}

unsigned int TermFixpoint::_MeasureStateSpaceCore() {
    unsigned count = 1;
    for(auto& item : this->_fixpoint) {
        if(item.first == nullptr || !item.second) {
            continue;
        }
        count += (item.first)->MeasureStateSpace();
    }

    return count;
}
/**
 * Dumping functions
 */
std::ostream& operator <<(std::ostream& osObject, Term& z) {
    z.dump();
    return osObject;
}

namespace Gaston {
    void dumpResultKey(std::pair<Term_ptr, Symbol_ptr> const& s) {
        assert(s.first != nullptr);

        if(s.second == nullptr) {
            //std::cout << "<" << (*s.first) << ", \u0437>";
            size_t seed = Gaston::hash_value(s.first);
            size_t seed2 = Gaston::hash_value(s.second);
            std::cout << "<" << "[" << s.first << "] {" << seed << "}";
            std::cout << (*s.first) << ", " << "[" << s.second << "] {" << seed2 << "} ";
            std::cout << "\u0437" << ">";
            boost::hash_combine(seed, seed2);
            std::cout << " {" << seed << "}";
        } else {
            size_t seed = Gaston::hash_value(s.first);
            size_t seed2 = Gaston::hash_value(s.second);
            std::cout << "<" << "[" << s.first << "] {" << seed << "}";
            std::cout << (*s.first) << ", " << "[" << s.second << "] {" << seed2 << "} ";
            std::cout << (*s.second) << ">";
            boost::hash_combine(seed, seed2);
            std::cout << " {" << seed << "}";
        }
    }

    void dumpResultData(std::pair<Term_ptr, bool> &s) {
        std::cout << "<" << (*s.first) << ", " << (s.second ? "True" : "False") << ">";
    }

    void dumpSubsumptionKey(std::pair<Term_ptr, Term_ptr> const& s) {
        assert(s.first != nullptr);
        assert(s.second != nullptr);

        std::cout << "<" << (*s.first) << ", " << (*s.second) << ">";
    }

    void dumpSubsumptionPairData(std::pair<SubsumptionResult, Term_ptr> &s) {
        switch(s.first) {
            case SubsumptionResult::E_TRUE:
                std::cout << "True";
                break;
            case SubsumptionResult::E_FALSE:
                std::cout << "False";
                break;
            case SubsumptionResult::E_PARTIALLY:
                std::cout << "Partially";
                break;
        }
        if(s.second) {
            std::cout << ", ";
            s.second->dump();
        }
    }

    void dumpSubsumptionData(SubsumptionResult &s) {
        std::cout << (s != E_FALSE ? "True" : "False");
    }

    void dumpPreKey(std::pair<size_t, Symbol_ptr> const& s) {
        std::cout << "(" << s.first << ", " << (*s.second) << ")";
    }

    void dumpSetPreKey(std::pair<VATA::Util::OrdVector<size_t>, Symbol_ptr> const& s) {
        std::cout << "(" << s.first << ", " << (*s.second) << ")";
    }

    void dumpPreData(Term_ptr& s) {
        std::cout << (*s);
    }

    void dumpDagKey(Formula_ptr const& form) {
        form->dump();
    }

    void dumpDagData(SymbolicAutomaton*& aut) {
        aut->DumpAutomaton();
    }

    void dumpEnumKey(TermEnumerator* const& key) {
        std::cout << key;
    }
}

void Term::dump(unsigned indent) {
    #if (DEBUG_TERM_UNIQUENESS == true)
    std::cout << "[" << this << "]";
    #endif
    if(GET_IN_COMPLEMENT(this)) {
        std::cout << "\033[1;31m{\033[0m";
    }
    this->_dumpCore(indent);
    if(GET_IN_COMPLEMENT(this)) {
        std::cout << "\033[1;31m}\033[0m";
    }
}

void TermEmpty::_dumpCore(unsigned indent) {
    std::cout << "\u2205";
}

void TermProduct::_dumpCore(unsigned indent) {
    const char* product_colour = ProductTypeToColour(GET_PRODUCT_SUBTYPE(this));
    const char* product_symbol = ProductTypeToTermSymbol(GET_PRODUCT_SUBTYPE(this));

    std::cout << "\033[" << product_colour << "{\033[0m";
    left->dump(indent);
    std::cout << "\033[" << product_colour << " " << product_symbol << " \033[0m";
    right->dump(indent);
    std::cout << "\033[" << product_colour << "}\033[0m";
}

void TermTernaryProduct::_dumpCore(unsigned int indent) {
    const char* product_colour = ProductTypeToColour(GET_PRODUCT_SUBTYPE(this));
    const char* product_symbol = ProductTypeToTermSymbol(GET_PRODUCT_SUBTYPE(this));

    std::cout << "\033[" << product_colour << "{\033[0m";
    left->dump(indent);
    std::cout << "\033[" << product_colour << " " << product_symbol << "\u00B3 \033[0m";
    middle->dump(indent);
    std::cout << "\033[" << product_colour << " " << product_symbol << "\u00B3 \033[0m";
    right->dump(indent);
    std::cout << "\033[" << product_colour << "}\033[0m";
}

void TermNaryProduct::_dumpCore(unsigned int indent) {
    const char* product_colour = ProductTypeToColour(GET_PRODUCT_SUBTYPE(this));
    const char* product_symbol = ProductTypeToTermSymbol(GET_PRODUCT_SUBTYPE(this));

    std::cout << "\033[" << product_colour << "{\033[0m";
    for (int i = 0; i < this->arity; ++i) {
        if(i != 0) {
            std::cout << "\033[" << product_colour << " " << product_symbol << "\u207F \033[0m";
        }
        this->terms[i]->dump(indent);
    }
    std::cout << "\033[" << product_colour << "}\033[0m";
}

void TermBaseSet::_dumpCore(unsigned indent) {
    std::cout << "\033[1;35m{";
    for (auto state : this->states) {
        std::cout << (state) << ",";
    }
    std::cout << "}\033[0m";
}

void TermContinuation::_dumpCore(unsigned indent) {
    if(this->_unfoldedTerm == nullptr) {
        std::cout << "?";
        term->dump(indent);
        std::cout << "?";
        std::cout << "'";
        if (symbol != nullptr) {
            std::cout << (*symbol);
        }
        std::cout << "'";
        std::cout << "?";
    } else {
        std::cout << "*";
        this->_unfoldedTerm->dump(indent);
        std::cout << "*";
    }
}

void TermList::_dumpCore(unsigned indent) {
    std::cout << "\033[1;36m{\033[0m";
    for(auto& state : this->list) {
        state->dump(indent);
        std::cout  << ",";
    }
    std::cout << "\033[1;36m}\033[0m";
}

void TermFixpoint::_dumpCore(unsigned indent) {
    std::cout << "\033[1;34m{\033[0m" << "\n";
    for(auto& item : this->_fixpoint) {
        if(item.first == nullptr || !(item.second)) {
            continue;
        }
        std::cout << std::string(indent+2, ' ');
        item.first->dump(indent + 2);
        std::cout << "\033[1;34m,\033[0m";
        std::cout << "\n";
    }
#   if (DEBUG_FIXPOINT_SYMBOLS == true)
    std::cout << "[";
    for(auto& item : this->_symList) {
        std::cout << "(" << (*item) << ") , ";
    }
    std::cout << "]";
#   endif
#   if (DEBUG_FIXPOINT_WORKLIST == true)
    std::cout << std::string(indent, ' ');
    std::cout << "\033[1;37m[";
    for(auto& workItem : this->_worklist) {
        std::cout << (*workItem.first) << " + " << (*workItem.second) << ", ";
    }
    std::cout << "\033[1;37m]\033[0m\n";
#   endif
    std::cout << std::string(indent, ' ');
    std::cout << "\033[1;34m}\033[0m";
    if(this->_bValue) {
        std::cout << "\033[1;34m\u22A8\033[0m";
    } else {
        std::cout << "\033[1;34m\u22AD\033[0m";
    }
}

// <<< ADDITIONAL TERMBASESET FUNCTIONS >>>

bool TermBaseSet::Intersects(TermBaseSet* rhs) {
    for (auto lhs_state : this->states) {
        for(auto& rhs_state : rhs->states) {
            if(lhs_state == rhs_state) {
                return true;
            }
        }
    }
    return false;
}

// <<< ADDITIONAL TERMFIXPOINT FUNCTIONS >>>
/**
 *
 */
bool TermFixpoint::_processOnePostponed() {
#   if (OPT_EARLY_EVALUATION == true)
    assert(!this->_postponed.empty());

    std::pair<Term_ptr, Term_ptr> postponedPair;

    // Get the front of the postponed (must be TermProduct with continuation)
    #if (OPT_FIND_POSTPONED_CANDIDATE == true)
    bool found = false;
    TermProduct* first_product, *second_product;
    // first is the postponed term, second is the thing from fixpoint!!!
    for(auto it = this->_postponed.begin(); it != this->_postponed.end(); ++it) {
        first_product = static_cast<TermProduct*>((*it).first);
        second_product = static_cast<TermProduct*>((*it).second);
        if(!second_product->right->IsNotComputed()) {
            // If left thing was not unfolded it means we don't need to compute more stuff
            postponedPair = (*it);
            it = this->_postponed.erase(it);
            found = true;
            break;
        }
    }
    if(!found) {
        return false;
    }
    #else
    postponedPair = this->_postponed.front();
    this->_postponed.pop_front();
    #endif

    TermProduct* postponedTerm = static_cast<TermProduct*>(postponedPair.first);
    TermProduct* postponedFixTerm = static_cast<TermProduct*>(postponedPair.second);

    assert(postponedPair.second != nullptr);

    // Test the subsumption
    SubsumptionResult result;
    // Todo: this could be softened to iterators
    if( (result = postponedTerm->IsSubsumed(postponedFixTerm, OPT_PARTIALLY_LIMITED_SUBSUMPTION, true)) == E_FALSE) {
        // Push new term to fixpoint
        // Fixme: But there is probably something other that could subsume this crap
        for(auto item : this->_fixpoint) {
            if(item.first == nullptr)
                continue;
            if((result = postponedTerm->IsSubsumed(item.first, OPT_PARTIALLY_LIMITED_SUBSUMPTION, true)) != E_FALSE) {
                assert(result != E_PARTIALLY);
                return false;
            }
        }

        this->_fixpoint.push_back(std::make_pair(postponedTerm, true));
        // Push new symbols from _symList, if we are in Fixpoint semantics
        if (this->GetSemantics() == E_FIXTERM_FIXPOINT) {
            for (auto &symbol : this->_symList) {
                this->_worklist.insert(this->_worklist.cbegin(), std::make_pair(postponedTerm, symbol));
            }
        }
        #if (MEASURE_POSTPONED == TRUE)
        ++TermFixpoint::postponedProcessed;
        #endif
        return true;
    } else {
        assert(result != E_PARTIALLY);
        return false;
    }
#   else
    return false;
#   endif
}

std::pair<SubsumptionResult, Term_ptr> TermFixpoint::_fixpointTest(Term_ptr const &term) {
    if(this->_searchType == WorklistSearchType::E_UNGROUND_ROOT) {
        // Fixme: Not sure if this is really correct, but somehow I still feel that the Root search is special and
        //   subsumption is maybe not enough? But maybe this simply does not work for fixpoints of negated thing.
        return this->_testIfIn(term);
        // ^--- however i still feel that this is wrong and may cause loop.
        if(this->_unsatTerm == nullptr && this->_satTerm == nullptr) {
            return this->_testIfIn(term);
        } else if(this->_satTerm == nullptr) {
            return (!term->InComplement() ? this->_testIfBiggerExists(term) : this->_testIfSmallerExists(term));
        } else {
            assert(this->_unsatTerm == nullptr);
            return (!term->InComplement() ? this->_testIfSmallerExists(term) : this->_testIfBiggerExists(term));
        }
    } else {
        return this->_testIfSubsumes(term);
    }
}

std::pair<SubsumptionResult, Term_ptr >TermFixpoint::_testIfBiggerExists(Term_ptr const &term) {
    return (std::find_if(this->_fixpoint.begin(), this->_fixpoint.end(), [&term](FixpointMember const& member) {
        if(!member.second || member.first == nullptr) {
            return false;
        } else {
            return term->IsSubsumed(member.first, OPT_PARTIALLY_LIMITED_SUBSUMPTION, nullptr, false) != E_FALSE;
        }
    }) == this->_fixpoint.end() ? std::make_pair(E_FALSE, term) : std::make_pair(E_TRUE, term));
}

std::pair<SubsumptionResult, Term_ptr> TermFixpoint::_testIfSmallerExists(Term_ptr const &term) {
    return (std::find_if(this->_fixpoint.begin(), this->_fixpoint.end(), [&term](FixpointMember const& member) {
        if(!member.second || member.first == nullptr) {
            return false;
        } else {
            return member.first->IsSubsumed(term, OPT_PARTIALLY_LIMITED_SUBSUMPTION, nullptr, false) != E_FALSE;
        }
    }) == this->_fixpoint.end() ? std::make_pair(E_FALSE, term) : std::make_pair(E_TRUE, term));
}

std::pair<SubsumptionResult, Term_ptr> TermFixpoint::_testIfIn(Term_ptr const &term) {
    return (std::find_if(this->_fixpoint.begin(), this->_fixpoint.end(), [&term](FixpointMember const& member){
        if(member.second) {
            return member.first == term;
        } else {
            return false;
        }
    }) == this->_fixpoint.end() ? std::make_pair(E_FALSE, term) : std::make_pair(E_TRUE, term));
}

/**
 * Tests if term is subsumed by fixpoint, either it is already computed in cache
 * or we have to compute the subsumption testing for each of the fixpoint members
 * and finally store the results in the cache.
 *
 * @param[in] term:     term we are testing subsumption for
 * @return:             true if term is subsumed by fixpoint
 */
std::pair<SubsumptionResult, Term_ptr> TermFixpoint::_testIfSubsumes(Term_ptr const& term) {
    #if (OPT_CACHE_SUBSUMED_BY == true)
    SubsumptionResult result;
    Term* key = term, *subsumedByTerm = nullptr;
    if(!this->_subsumedByCache.retrieveFromCache(key, result)) {
        // True/Partial results are stored in cache
        if((result = term->IsSubsumedBy(this->_fixpoint, this->_worklist, subsumedByTerm)) != E_FALSE) {
            // E_PARTIALLY is considered as E_TRUE, as it was partitioned already
            this->_subsumedByCache.StoreIn(key, E_TRUE);
        }

        if(result == E_PARTIALLY) {
            assert(subsumedByTerm != nullptr);
#           if (OPT_SUBSUMPTION_INTERSECTION == true)
            return std::make_pair(E_FALSE, subsumedByTerm);
#           elif (OPT_EARLY_EVALUATION == true)
            this->_postponed.insert(this->_postponed.begin(), std::make_pair(term, subsumedByTerm));
#           endif
            #if (MEASURE_POSTPONED == true)
            ++TermFixpoint::postponedTerms;
            #endif
        }
    }
    #if (MEASURE_SUBSUMEDBY_HITS == true)
    else {
        ++TermFixpoint::subsumedByHits;
    }
    #endif
    return std::make_pair(result, term);
    #else
    return std::make_pair(term->IsSubsumedBy(this->_fixpoint, this->_worklist, subsumedByTerm), term);
    #endif
}

WorklistItemType TermFixpoint::_popFromWorklist() {
    assert(_worklist.size() > 0);
    if(this->_searchType != WorklistSearchType::E_BFS) {
        WorklistItemType item = _worklist.front();
        _worklist.pop_front();
        return item;
    } else {
        WorklistItemType item = _worklist.back();
        _worklist.pop_back();
        return item;
    }
}

/**
 * Does the computation of the next fixpoint, i.e. the next iteration.
 */
void TermFixpoint::ComputeNextFixpoint() {
    if(_worklist.empty())
        return;

    // Pop the front item from worklist
    WorklistItemType item = this->_popFromWorklist();

    // Compute the results
    ResultType result = this->_baseAut->IntersectNonEmpty(item.second, item.first, GET_NON_MEMBERSHIP_TESTING(this));
    this->_updateExamples(result);

    // If it is subsumed by fixpoint, we don't add it
    auto fix_result = this->_fixpointTest(result.first);
    if(fix_result.first != E_FALSE) {
        assert(fix_result.first != E_PARTIALLY);
        return;
    }

    // Push new term to fixpoint
    if(result.second == this->_shortBoolValue && _iteratorNumber == 0) {
        _fixpoint.push_front(std::make_pair(fix_result.second, true));
    } else {
        _fixpoint.push_back(std::make_pair(fix_result.second, true));
    }
    _updated = true;
    // Aggregate the result of the fixpoint computation
    _bValue = this->_AggregateResult(_bValue,result.second);
    // Push new symbols from _symList
#   if (DEBUG_RESTRICTION_DRIVEN_FIX == true)
    std::cout << "ComputeNextFixpoint()\n";
#   endif
#   if (OPT_WORKLIST_DRIVEN_BY_RESTRICTIONS == true)
    GuideTip tip;
    if (this->_guide == nullptr || (tip = this->_guide->GiveTip(fix_result.second)) == GuideTip::G_PROJECT) {
#   endif
        for (auto &symbol : _symList) {
#           if (OPT_WORKLIST_DRIVEN_BY_RESTRICTIONS == true)
#           if (DEBUG_RESTRICTION_DRIVEN_FIX == true)
            fix_result.second->dump(); std::cout << " + " << (*symbol) << " -> ";
#           endif
            if (this->_guide != nullptr) {
                bool break_from_for = false;
                switch (this->_guide->GiveTip(fix_result.second, symbol)) {
                    case GuideTip::G_FRONT:
#                       if (DEBUG_RESTRICTION_DRIVEN_FIX == true)
                        std::cout << "G_FRONT\n";
#                       endif
                        _worklist.insert(_worklist.cbegin(), std::make_pair(fix_result.second, symbol));
                        break;
                    case GuideTip::G_BACK:
#                       if (DEBUG_RESTRICTION_DRIVEN_FIX == true)
                        std::cout << "G_BACK\n";
#                       endif
                        _worklist.push_back(std::make_pair(fix_result.second, symbol));
                        break;
                    case GuideTip::G_THROW:
#                       if (DEBUG_RESTRICTION_DRIVEN_FIX == true)
                        std::cout << "G_THROW\n";
#                       endif
                        break;
                    case GuideTip::G_PROJECT:
                        _worklist.insert(_worklist.cbegin(), std::make_pair(fix_result.second, symbol));
                        break;
                    default:
                        assert(false && "Unsupported guide tip");
                }
            } else {
#               if (DEBUG_RESTRICTION_DRIVEN_FIX == true)
                std::cout << "insert\n";
#               endif
                _worklist.insert(_worklist.cbegin(), std::make_pair(fix_result.second, symbol));
            }
#           elif (OPT_FIXPOINT_BFS_SEARCH == true)
            _worklist.push_back(std::make_pair(fix_result.second, symbol));
#          else
            _worklist.insert(_worklist.cbegin(), std::make_pair(fix_result.second, symbol));
#           endif
        }
#   if (OPT_WORKLIST_DRIVEN_BY_RESTRICTIONS == true)
    } else {
        assert(tip == GuideTip::G_PROJECT_ALL);
        _worklist.insert(_worklist.cbegin(), std::make_pair(fix_result.second, this->_projectedSymbol));
    }
#   endif
}

/**
 * Computes the pre of the fixpoint, i.e. it does pre on all the things
 * as we had already the fixpoint computed from previous step
 */
void TermFixpoint::ComputeNextPre() {
    if(_worklist.empty())
        return;

    // Pop item from worklist
    WorklistItemType item = this->_popFromWorklist();

    // Compute the results
    ResultType result = this->_baseAut->IntersectNonEmpty(item.second, item.first, GET_NON_MEMBERSHIP_TESTING(this));
    this->_updateExamples(result);

    // If it is subsumed we return
    auto fix_result = this->_fixpointTest(result.first);
    if(fix_result.first != E_FALSE) {
        assert(fix_result.first != E_PARTIALLY);
        return;
    }

    // Push the computed thing and aggregate the result
    if(result.second == this->_shortBoolValue && _iteratorNumber == 0) {
        _fixpoint.push_front(std::make_pair(fix_result.second, true));
    } else {
        _fixpoint.push_back(std::make_pair(fix_result.second, true));
    }
    _updated = true;
    _bValue = this->_AggregateResult(_bValue,result.second);
}

/**
 * When we are computing the fixpoint, we aggregate the results by giant
 * OR, as we are doing the epsilon check in the union. However, if we are
 * under the complement we have to compute the AND of the function, as we
 * are computing the epsilon check not in the union.
 *
 * @param[in] inComplement:     whether we are in complement
 */
void TermFixpoint::_InitializeAggregateFunction(bool inComplement) {
    if(!inComplement) {
        this->_shortBoolValue = true;
    } else {
        this->_shortBoolValue = false;
    }
}

bool TermFixpoint::_AggregateResult(bool a, bool b) {
    return (GET_NON_MEMBERSHIP_TESTING(this)) ? (a && b) : (a || b);
}

/**
 * Transforms @p symbols according to the bound variable in @p vars, by pumping
 *  0 and 1 on the tracks
 *
 * @param[in,out] symbols:  list of symbols, that will be transformed
 * @param[in] vars:         list of used vars, that are projected
 */
void TermFixpoint::_InitializeSymbols(Workshops::SymbolWorkshop* workshop, Gaston::VarList* freeVars, IdentList* vars, Symbol *startingSymbol) {
    // The input symbol is first trimmed, then if the AllPosition Variable exist, we generate only the trimmed stuff
    // TODO: Maybe for Fixpoint Pre this should be done? But nevertheless this will happen at topmost
    // Reserve symbols for at least 2^|freeVars|
    this->_symList.reserve(2 << vars->size());
    Symbol* trimmed = workshop->CreateTrimmedSymbol(startingSymbol, freeVars);
    if (allPosVar != -1) {
        trimmed = workshop->CreateSymbol(trimmed, varMap[allPosVar], '1');
    }
    this->_symList.push_back(trimmed);
    // TODO: Optimize, this sucks
    unsigned int symNum = 1;
#   if (DEBUG_FIXPOINT_SYMBOLS_INIT == true)
    std::cout << "[F] Initializing symbols of '"; this->dump(); std::cout << "\n";
#   endif
    this->_projectedSymbol = startingSymbol;
    for(auto var = vars->begin(); var != vars->end(); ++var) {
        // Pop symbol;
        this->_projectedSymbol = workshop->CreateSymbol(this->_projectedSymbol, varMap[(*var)], 'X');
        if(*var == allPosVar)
            continue;
        int i = 0;
        for(auto it = this->_symList.begin(); i < symNum; ++it, ++i) {
            Symbol* symF = *it;
            // #SYMBOL_CREATION
            this->_symList.push_back(workshop->CreateSymbol(symF, varMap[(*var)], '1'));
        }
        symNum <<= 1;// times 2
    }
#   if (DEBUG_FIXPOINT_SYMBOLS_INIT == true)
    for(auto sym : this->_symList) {
        std::cout << "[*] " << (*sym) << "\n";
    }
#   endif
}

/**
 * @return: result of fixpoint
 */
bool TermFixpoint::GetResult() {
    return this->_bValue;
}

void TermFixpoint::_updateExamples(ResultType& result) {
    if(this->_searchType == E_UNGROUND_ROOT) {
#   if (ALT_SKIP_EMPTY_UNIVERSE == true)
        if (result.first->link->symbol == nullptr)
            return;
#   endif
        if (result.second) {
            if (this->_satTerm == nullptr) {
                this->_satTerm = result.first;
            }
        } else {
            if (this->_unsatTerm == nullptr && this->_baseAut->WasLastExampleValid()) {
                this->_unsatTerm = result.first;
            }
        }
    }
}

ExamplePair TermFixpoint::GetFixpointExamples() {
    return std::make_pair(this->_satTerm, this->_unsatTerm);
}

bool TermFixpoint::IsFullyComputed() const {
    if(this->_sourceTerm != nullptr) {
        // E_FIXTERM_PRE
        // Fixpoints with PreSemantics are fully computed when the source iterator is empty (so we can
        // get nothing new from the source and if the worklist is empty.
        return this->_sourceIt == nullptr && this->_worklist.empty();
    } else {
        // E_FIXTERM_FIXPOINT
        // Fixpoints with classic semantics are fully computed when the worklist is empty
        return this->_worklist.empty();
    }
}

bool TermFixpoint::IsShared() {
    return this->_iteratorNumber != 0;
}

void TermFixpoint::RemoveSubsumed() {
    if(!this->_iteratorNumber) {
        assert(this->_iteratorNumber == 0);
        auto end = this->_fixpoint.end();
        for (auto it = this->_fixpoint.begin(); it != end;) {
            if ((*it).first == nullptr) {
                ++it;
                continue;
            }
            if (!(*it).second) {
                it = this->_fixpoint.erase(it);
                continue;
            }
            ++it;
        }
    }
}

/**
 * Returns whether we are computing the Pre of the already finished fixpoint,
 * or if we are computing the fixpoint from scratch
 *
 * @return: semantics of the fixpoint
 */
FixpointTermSem TermFixpoint::GetSemantics() const {
    return (nullptr == _sourceTerm) ? E_FIXTERM_FIXPOINT : E_FIXTERM_PRE;
}

// <<< ADDITIONAL TERMCONTINUATION FUNCTIONS >>>
Term* TermContinuation::unfoldContinuation(UnfoldedInType t) {
    if(this->_unfoldedTerm == nullptr) {
        if(lazyEval) {
            assert(this->aut->aut->type == AutType::INTERSECTION || this->aut->aut->type == AutType::UNION);
            assert(this->initAut != nullptr);
            BinaryOpAutomaton* boAutomaton = static_cast<BinaryOpAutomaton*>(this->initAut);
            std::tie(this->aut, this->term) = boAutomaton->LazyInit(this->term);
            lazyEval = false;
        }

        this->_unfoldedTerm = (this->aut->aut->IntersectNonEmpty(
                (this->symbol == nullptr ? nullptr : this->aut->ReMapSymbol(this->symbol)), this->term, this->underComplement)).first;
        #if (MEASURE_CONTINUATION_EVALUATION == true)
        switch(t){
            case UnfoldedInType::E_IN_SUBSUMPTION:
                ++TermContinuation::unfoldInSubsumption;
                break;
            case UnfoldedInType::E_IN_ISECT_NONEMPTY:
                ++TermContinuation::unfoldInIsectNonempty;
                break;
            default:
                break;
        }
        ++TermContinuation::continuationUnfolding;
        #endif
    }
    return this->_unfoldedTerm;
}

// <<< ADDITIONAL TERMNARYOPERATOR FUNCTIONS >>>
Term_ptr TermNaryProduct::operator[](size_t idx) {
    assert(idx < this->arity);
    if(this->terms[idx] == nullptr) {
        // compute the value
    }
    return this->terms[idx];
}

// <<< EQUALITY MEASURING FUNCTIONS
#if (MEASURE_COMPARISONS == true)
void Term::comparedBySamePtr(TermType t) {
    ++Term::comparisonsBySamePtr;
    switch(t) {
        case TERM_EMPTY:
        case TERM_BASE:
            ++TermBaseSet::comparisonsBySamePtr;
            break;
        case TERM_LIST:
            ++TermList::comparisonsBySamePtr;
            break;
        case TERM_PRODUCT:
            ++TermProduct::comparisonsBySamePtr;
            break;
        case TERM_FIXPOINT:
            ++TermFixpoint::comparisonsBySamePtr;
            break;
        case TERM_CONTINUATION:
            ++TermContinuation::comparisonsBySamePtr;
            break;
        default:
            assert(false);
    }
}

void Term::comparedByDifferentType(TermType t) {
    ++Term::comparisonsByDiffType;
    switch(t) {
        case TERM_EMPTY:
        case TERM_BASE:
            ++TermBaseSet::comparisonsByDiffType;
            break;
        case TERM_LIST:
            ++TermList::comparisonsByDiffType;
            break;
        case TERM_PRODUCT:
            ++TermProduct::comparisonsByDiffType;
            break;
        case TERM_FIXPOINT:
            ++TermFixpoint::comparisonsByDiffType;
            break;
        case TERM_CONTINUATION:
            ++TermContinuation::comparisonsByDiffType;
            break;
        default:
            assert(false);
    }
}

void Term::comparedByStructure(TermType t, bool res) {
    if(res)
        ++Term::comparisonsByStructureTrue;
    else
        ++Term::comparisonsByStructureFalse;
    switch(t) {
        case TERM_EMPTY:
        case TERM_BASE:
            if(res) {
                ++TermBaseSet::comparisonsByStructureTrue;
            } else {
                ++TermBaseSet::comparisonsByStructureFalse;
            }
            break;
        case TERM_LIST:
            if(res) {
                ++TermList::comparisonsByStructureTrue;
            } else {
                ++TermList::comparisonsByStructureFalse;
            }
            break;
        case TERM_PRODUCT:
            if(res) {
                ++TermProduct::comparisonsByStructureTrue;
            } else {
                ++TermProduct::comparisonsByStructureFalse;
            }
            break;
        case TERM_FIXPOINT:
            if(res) {
                ++TermFixpoint::comparisonsByStructureTrue;
            } else {
                ++TermFixpoint::comparisonsByStructureFalse;
            }
            break;
        case TERM_CONTINUATION:
            if(res) {
                ++TermContinuation::comparisonsByStructureTrue;
            } else {
                ++TermContinuation::comparisonsByStructureFalse;
            }
            break;
        default:
            assert(false);
    }
}
#endif

// <<< EQUALITY CHECKING FUNCTIONS >>>
/**
 * Operation for equality checking, tests first whether the two pointers
 * are the same (this is for the (future) unique pointer cache), checks
 * if the types are the same to optimize the equality checking.
 *
 * Otherwise it calls the _eqCore() function for specific comparison of
 * terms
 *
 * @param[in] t:        tested term
 */
bool Term::operator==(const Term &t) {
    if(&t == nullptr) {
        return false;
    }

    Term* tt = const_cast<Term*>(&t);
    Term* tthis = this;
#   if (OPT_EARLY_EVALUATION == true)
    if(tt->type == TERM_CONTINUATION) {
        TermContinuation* ttCont = static_cast<TermContinuation*>(tt);
        tt = ttCont->unfoldContinuation(UnfoldedInType::E_IN_COMPARISON);
    }
    if(this->type == TERM_CONTINUATION) {
        TermContinuation* thisCont = static_cast<TermContinuation*>(this);
        tthis = thisCont->unfoldContinuation(UnfoldedInType::E_IN_COMPARISON);
    }
#   endif

    assert(GET_IN_COMPLEMENT(tthis) == GET_IN_COMPLEMENT(tt));
    if(tthis == tt) {
        // Same thing
        #if (MEASURE_COMPARISONS == true)
        Term::comparedBySamePtr(this->type);
        #endif
        return true;
    } else if (tthis->type != tt->type) {
        // Terms are of different type
        #if (MEASURE_COMPARISONS == true)
        Term::comparedByDifferentType(this->type);
        #endif
        return tthis == tt;
    } else {
        #if (MEASURE_COMPARISONS == true)
        bool result = tthis->_eqCore(*tt);
        Term::comparedByStructure(tthis->type, result);
        return result;
        #else
        return tthis->_eqCore(*tt);
        #endif
    }
}

bool TermEmpty::_eqCore(const Term &t) {
    assert(t.type == TERM_EMPTY && "Testing equality of different term types");
    return true;
}

bool TermProduct::_eqCore(const Term &t) {
    assert(t.type == TERM_PRODUCT && "Testing equality of different term types");

    #if (OPT_EQ_THROUGH_POINTERS == true)
    assert(this != &t);
    const TermProduct &tProduct = static_cast<const TermProduct&>(t);
    // TODO: We should consider that continuation can be on left side, if we chose the different computation strategy
    if(!this->right->IsNotComputed() && !tProduct.right->IsNotComputed()) {
        // If something was continuation we try the structural compare, as there could be something unfolded
        if(this->left->stateSpaceApprox < this->right->stateSpaceApprox) {
            return (*tProduct.left == *this->left) && (*tProduct.right == *this->right);
        } else {
            return (*tProduct.right == *this->right) && (*tProduct.left == *this->left);
        }
    } else {
        return false;
    }
    #else
    const TermProduct &tProduct = static_cast<const TermProduct&>(t);
    if(this->left->stateSpaceApprox < this->right->stateSpaceApprox) {
        return (*tProduct.left == *this->left) && (*tProduct.right == *this->right);
    } else {
        return (*tProduct.right == *this->right) && (*tProduct.left == *this->left);
    }
    #endif
}

bool TermTernaryProduct::_eqCore(const Term &t) {
    assert(t.type == TERM_TERNARY_PRODUCT && "Testing equality of different term types");

#   if (OPT_EQ_THROUGH_POINTERS == true)
    assert(this != &t);
    return false;
#   else
    const TermTernaryProduct &tProduct = static_cast<const TermTernaryProduct&>(t);
    return (*tProduct.left == *this->left) && (*tProduct.middle == *this->middle) && (*tProduct.right == *this->right);
#   endif
}

bool TermNaryProduct::_eqCore(const Term &t) {
    assert(t.type == TERM_NARY_PRODUCT && "Testing equality of different term types");

#   if (OPT_EQ_THROUGH_POINTERS == true)
    assert(this != &t);
    return false;
#   else
    const TermNaryProduct &tProduct = static_cast<const TermNaryProduct&>(t);
    for (int i = 0; i < this->arity; ++i) {
        if(!(*tProduct.terms[i] == *this->terms[i])) {
            return false;
        }
    }
    return true;
#   endif
}

bool TermBaseSet::_eqCore(const Term &t) {
    assert(t.type == TERM_BASE && "Testing equality of different term types");

    #if (OPT_EQ_THROUGH_POINTERS == true)
    // As we cannot get to _eqCore with unique pointers, it must mean t and this are different;
    assert(this != &t);
    return false;
    #else
    const TermBaseSet &tBase = static_cast<const TermBaseSet&>(t);
    if(this->states.size() != tBase.states.size()) {
        return false;
    } else {
        // check the things, should be sorted
        auto lhsIt = this->states.begin();
        auto rhsIt = tBase.states.begin();
        for(; lhsIt != this->states.end(); ++lhsIt, ++rhsIt) {
            if(*lhsIt != *rhsIt) {
                return false;
            }
        }
        return true;
    }
    #endif
}

bool TermContinuation::_eqCore(const Term &t) {
    assert(t.type == TERM_CONTINUATION && "Testing equality of different term types");

    const TermContinuation &tCont = static_cast<const TermContinuation&>(t);
    if(this->_unfoldedTerm == nullptr) {
        return (this->symbol == tCont.symbol) && (this->term == tCont.term);
    } else {
        return this->_unfoldedTerm == tCont._unfoldedTerm;
    }
}

bool TermList::_eqCore(const Term &t) {
    assert(t.type == TERM_LIST && "Testing equality of different term types");
    G_NOT_IMPLEMENTED_YET("TermList::_eqCore");
}

unsigned int TermFixpoint::ValidMemberSize() const {
    unsigned int members = 0;
    for(auto& item : this->_fixpoint) {
        members += (item.second && item.first != nullptr);
    }
    return members;
}

bool TermFixpoint::_eqCore(const Term &t) {
    assert(t.type == TERM_FIXPOINT && "Testing equality of different term types");

    const TermFixpoint &tFix = static_cast<const TermFixpoint&>(t);
    if(this->_bValue != tFix._bValue) {
        // If the values are different, we can automatically assume that there is some difference
        return false;
    }

    if(this->ValidMemberSize() != tFix.ValidMemberSize()) {
        return false;
    }

    bool are_symbols_the_same = TermFixpoint::_compareSymbols(*this, tFix);
    if(!are_symbols_the_same && (this->_worklist.size() != 0 || tFix._worklist.size() != 0)) {
        return false;
    }

    for(auto it = this->_fixpoint.begin(); it != this->_fixpoint.end(); ++it) {
        if((*it).first == nullptr || !(*it).second)
            continue;
        bool found = false;
        for(auto tit = tFix._fixpoint.begin(); tit != tFix._fixpoint.end(); ++tit) {
            if((*tit).first == nullptr || !(*tit).second)
                continue;
            if(*(*it).first == *(*tit).first) {
                found = true;
                break;
            }
        }
        if(!found) {
            return false;
        }
    }

    return true;
}

bool TermFixpoint::_compareSymbols(const TermFixpoint& lhs, const TermFixpoint& rhs) {
    for(auto symbol : lhs._symList) {
        if(std::find_if(rhs._symList.begin(), rhs._symList.end(), [&symbol](Symbol_ptr s) { return s == symbol;}) == rhs._symList.end()) {
            return false;
        }
    }
    return true;
}