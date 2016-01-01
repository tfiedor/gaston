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
#include <boost/functional/hash.hpp>

namespace Gaston {
    size_t hash_value(Term* s) {
        #if (OPT_TERM_HASH_BY_APPROX == true)
        size_t seed = 0;
        if(s->type == TERM_PRODUCT || s->type == TERM_BASE) {
            boost::hash_combine(seed, boost::hash_value(s));
        } else {
            boost::hash_combine(seed, boost::hash_value(s->stateSpaceApprox));
            boost::hash_combine(seed, boost::hash_value(s->MeasureStateSpace()));
        }
        return seed;
        #else
        return boost::hash_value(s->MeasureStateSpace());
        #endif
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

// <<< TERM CONSTRUCTORS >>>
TermEmpty::TermEmpty() {
    #if (MEASURE_STATE_SPACE == true)
    ++TermEmpty::instances;
    #endif
    this->_inComplement = false;
    this->type = TERM_EMPTY;

    // Initialization of state space
    this->stateSpace = 0;
    this->stateSpaceApprox = 0;

    #if (DEBUG_TERM_CREATION == true)
    std::cout << "[" << this << "]";
    std::cout << "TermEmpty::";
    this->dump();
    std::cout << "\n";
    #endif
}

/**
 * Constructor of Term Product---construct intersecting product of
 * @p lhs and @p rhs
 *
 * @param[in] lhs:  left operand of term intersection
 * @param[in] rhs:  right operand of term intersection
 */
TermProduct::TermProduct(Term_ptr lhs, Term_ptr rhs, ProductType pt) : left(move(lhs)), right(move(rhs)) {
    #if (MEASURE_STATE_SPACE == true)
    ++TermProduct::instances;
    #endif

    this->_inComplement = false;
    this->type = TermType::TERM_PRODUCT;
    this->subtype = pt;

    // Initialization of state space
    if(this->left->stateSpace != 0 && this->right->stateSpace != 0) {
        this->stateSpace = this->left->stateSpace + this->right->stateSpace +1;
    } else {
        this->stateSpace = 0;
    }
    this->stateSpaceApprox = this->left->stateSpaceApprox + this->right->stateSpaceApprox + 1;

    #if (DEBUG_TERM_CREATION == true)
    //std::cout << "[" << this << "]";
    std::cout << "TermProduct::";
    this->dump();
    std::cout << "\n";
    #endif
}

TermBaseSet::TermBaseSet(VATA::Util::OrdVector<unsigned int>& s, unsigned int offset, unsigned int stateNo) : states(), stateMask(stateNo)  {
    #if (MEASURE_STATE_SPACE == true)
    ++TermBaseSet::instances;
    #endif
    type = TERM_BASE;
    for(auto state : s) {
        this->states.push_back(state);
        this->stateMask.set(state-offset, true);
    }

    // Initialization of state space
    this->_inComplement = false;
    this->stateSpace = this->states.size();
    this->stateSpaceApprox = this->stateSpace;

    #if (DEBUG_TERM_CREATION == true)
    //std::cout << "[" << this << "]";
    std::cout << "TermBaseSet::";
    this->dump();
    std::cout << "\n";
    #endif
}

TermContinuation::TermContinuation(std::shared_ptr<SymbolicAutomaton> a, Term_ptr t, std::shared_ptr<SymbolType> s, bool b) : aut(a), term(t), symbol(s), underComplement(b) {
    #if (DEBUG_TERM_CREATION == true)
    std::cout << "[" << this << "]";
    std::cout << "TermContinuation::";
    this->dump();
    std::cout << "\n";
    #endif
    #if (MEASURE_STATE_SPACE == true)
    ++TermContinuation::instances;
    #endif

    this->type = TERM_CONTINUATION;

    // Initialization of state space
    this->stateSpace = 1;
    this->stateSpaceApprox = 1;

    #if (DEBUG_CONTINUATIONS == true)
    std::cout << "Postponing computation as [";
    t->dump();
    std::cout << "]\n";
    #endif
    this->_nonMembershipTesting = b;
    this->_inComplement = false;
}

TermList::TermList(Term_ptr first, bool isCompl) {
    #if (MEASURE_STATE_SPACE == true)
    ++TermList::instances;
    #endif

    this->type = TERM_LIST;

    // Initialization of state space
    if(first->stateSpace) {
        this->stateSpace = first->stateSpace + 1;
    }
    this->stateSpaceApprox = first->stateSpaceApprox;

    this->list.push_back(first);
    this->_nonMembershipTesting = isCompl;
    this->_inComplement = false;

    #if (DEBUG_TERM_CREATION == true)
    std::cout << "[" << this << "]";
    std::cout << "TermList::";
    this->dump();
    std::cout << "\n";
    #endif
}

TermFixpoint::TermFixpoint(std::shared_ptr<SymbolicAutomaton> aut, Term_ptr startingTerm, Symbols symList, bool inComplement, bool initbValue)
        : _sourceTerm(nullptr), _sourceIt(nullptr), _aut(aut), _bValue(initbValue) {
    #if (MEASURE_STATE_SPACE == true)
    ++TermFixpoint::instances;
    #endif

    // Initialize the aggregate function
    this->_InitializeAggregateFunction(inComplement);
    this->_nonMembershipTesting = inComplement;
    this->_inComplement = false;
    this->type = TERM_FIXPOINT;

    // Initialize the state space
    this->stateSpace = 0;
    this->stateSpaceApprox = startingTerm->stateSpaceApprox;

    // Initialize the fixpoint as [nullptr, startingTerm]
    this->_fixpoint.push_front(startingTerm);
    this->_fixpoint.push_front(nullptr);

    // Push symbols to worklist
    for(auto symbol : symList) {
        this->_symList.push_back(symbol);
        this->_worklist.insert(this->_worklist.cbegin(), std::make_pair(startingTerm, symbol));
    }

    #if (DEBUG_TERM_CREATION == true)
    std::cout << "[" << this << "]";
    std::cout << "TermFixpoint::";
    this->dump();
    std::cout << "\n";
    #endif
}

TermFixpoint::TermFixpoint(std::shared_ptr<SymbolicAutomaton> aut, Term_ptr sourceTerm, Symbols symList, bool inComplement)
        : _sourceTerm(sourceTerm), _sourceIt(reinterpret_cast<TermFixpoint*>(sourceTerm.get())->GetIteratorDynamic()),
        _aut(aut), _worklist(), _bValue(inComplement) {
    #if (MEASURE_STATE_SPACE == true)
    ++TermFixpoint::instances;
    #endif
    assert(sourceTerm->type == TERM_FIXPOINT);

    // Initialize the state space
    this->stateSpace = 0;
    this->stateSpaceApprox = sourceTerm->stateSpaceApprox;

    // Initialize the aggregate function
    this->_InitializeAggregateFunction(inComplement);
    this->type = TERM_FIXPOINT;
    this->_nonMembershipTesting = inComplement;
    this->_inComplement = false;

    // Initialize the fixpoint
    this->_fixpoint.push_front(nullptr);
    // Push things into worklist
    for(auto symbol : symList) {
        this->_symList.push_back(symbol);
    }

    #if (DEBUG_TERM_CREATION == true)
    std::cout << "[" << this << "]";
    std::cout << "TermFixpointPre::";
    this->dump();
    std::cout << "\n";
    #endif
}

/**
 * Tests the Term subsumption
 *
 * @param[in] t:    term we are testing subsumption against
 * @return:         true if this is subsumed by @p t
 */
bool Term::IsSubsumed(Term *t) {
    assert(this->_inComplement == t->_inComplement);
    // TODO: Add Caching
    #if (DEBUG_TERM_SUBSUMPTION == true)
    this->dump();
    std::cout << " <?= ";
    t->dump();
    std::cout << "\n";
    #endif
    // unfold the continuation
    if(this == t) {
        return true;
    }

    if(t->type == TERM_CONTINUATION) {
        // TODO: We should check that maybe we have different continuations
        TermContinuation *continuation = reinterpret_cast<TermContinuation *>(t);
        if (this->type == TERM_CONTINUATION) {
            TermContinuation *thisCont = reinterpret_cast<TermContinuation *>(this);
            assert(continuation->underComplement == thisCont->underComplement);
        }
        auto unfoldedContinuation = (continuation->aut->IntersectNonEmpty(continuation->symbol.get(),
                                                                          continuation->term.get(),
                                                                          continuation->underComplement)).first;
        return this->IsSubsumed(unfoldedContinuation.get());
    } else if(this->type == TERM_CONTINUATION) {
        TermContinuation *continuation = reinterpret_cast<TermContinuation *>(this);
        auto unfoldedContinuation = (continuation->aut->IntersectNonEmpty(continuation->symbol.get(),
                                                                          continuation->term.get(),
                                                                          continuation->underComplement)).first;
        return unfoldedContinuation->IsSubsumed(t);
    }

    // Else if it is not continuation we first look into cache and then recompute if needed
    bool result;
#if (OPT_CACHE_SUBSUMES == true)
    if(!this->_isSubsumedCache.retrieveFromCache(t, result)) {
#endif
        if (this->_inComplement) {
            if (this->type == TERM_EMPTY) {
                result = t->type == TERM_EMPTY;
            } else {
                result = t->_IsSubsumedCore(this);
            }
        } else {
            if (t->type == TERM_EMPTY) {
                result = this->type == TERM_EMPTY;
            } else {
                result = this->_IsSubsumedCore(t);
            }
        }
#if (OPT_CACHE_SUBSUMES == true)
        this->_isSubsumedCache.StoreIn(t, result);
    }
#endif
    return result;
}

bool TermEmpty::_IsSubsumedCore(Term *t) {
    // Empty term is subsumed by everything (probably)
    return true;
}

bool TermProduct::_IsSubsumedCore(Term* t) {
    assert(t->type == TERM_PRODUCT);

    // Retype and test the subsumption component-wise
    TermProduct *rhs = reinterpret_cast<TermProduct*>(t);
    Term *lhsl = this->left.get();
    Term *lhsr = this->right.get();
    Term *rhsl = rhs->left.get();
    Term *rhsr = rhs->right.get();

    if(lhsl == rhsl) {
        return lhsr->IsSubsumed(rhsr);
    } else if(lhsr == rhsr) {
        return lhsl->IsSubsumed(rhsl);
    } else {
        if(lhsl->stateSpaceApprox < lhsr->stateSpaceApprox) {
            return lhsl->IsSubsumed(rhsl) && lhsr->IsSubsumed(rhsr);
        } else {
            return lhsr->IsSubsumed(rhsr) && lhsl->IsSubsumed(rhsl);
        }
    }
}

bool TermBaseSet::_IsSubsumedCore(Term* term) {
    if(term->type != TERM_BASE) {
        std::cout << "Term type = " << term->type << "\n";
    }
    assert(term->type == TERM_BASE);

    // Test component-wise, not very efficient though
    // TODO: Change to bit-vectors if possible
    TermBaseSet *t = reinterpret_cast<TermBaseSet*>(term);
    if (this == term) {
        return true;
    } else if(t->states.size() < this->states.size()) {
        return false;
    } else {
        // TODO: Maybe we could exploit that we have ordered vectors
        auto it = this->states.begin();
        auto end = this->states.end();
        auto tit = t->states.begin();
        auto tend = t->states.end();
        while(it != end && tit != tend) {
            if(*it == *tit) {
                ++it;
                ++tit;
            } else if(*it > *tit) {
                ++tit;
            } else {
                // *it < *tit
                return false;
            }
        }
        return (it == end);
    }
}

bool TermContinuation::_IsSubsumedCore(Term *t) {
    // TODO: How to do this smartly?
    // TODO: Maybe if we have {} we can answer sooner, without unpacking

    // We unpack this term
    auto unfoldedTerm = (this->aut->IntersectNonEmpty(this->symbol.get(), this->term.get(), this->underComplement)).first;
    return unfoldedTerm->IsSubsumed(t);
}

bool TermList::_IsSubsumedCore(Term* t) {
    assert(t->type == TERM_LIST);

    // Reinterpret
    TermList* tt = reinterpret_cast<TermList*>(t);
    // Do the item-wise subsumption check
    for(auto item : this->list) {
        bool subsumes = false;
        for(auto tt_item : tt->list) {
            if(item->IsSubsumed(tt_item.get())) {
                subsumes = true;
                break;
            }
        }
        if(!subsumes) return false;
    }

    return true;
}

bool TermFixpoint::_IsSubsumedCore(Term* t) {
    assert(t->type == TERM_FIXPOINT);

    // Reinterpret
    TermFixpoint* tt = reinterpret_cast<TermFixpoint*>(t);

    // Do the piece-wise comparison
    for(auto item : this->_fixpoint) {
        // Skip the nullptr
        if(item == nullptr) continue;
        bool subsumes = false;
        for(auto tt_item : tt->_fixpoint) {
            if(tt_item == nullptr) continue;
            if(item->IsSubsumed(tt_item.get())) {
                subsumes = true;
                break;
            }
        }
        if(!subsumes) return false;
    }

    return true;
}

/**
 * Tests the subsumption over the list of terms
 *
 * @param[in] fixpoint:     list of terms contained as fixpoint
 */
bool TermEmpty::IsSubsumedBy(std::list < Term_ptr > &fixpoint) {
    // Empty term is subsumed by everything
    return true;
}

bool TermProduct::IsSubsumedBy(std::list<Term_ptr>& fixpoint) {
    // TODO: Add caching
    // Empty things is always subsumed
    if(this->IsEmpty()) {
        return true;
    }

    // For each item in fixpoint
    // TODO: This should be holikoptimized
    for(auto item : fixpoint) {
        // Nullptr is skipped
        if(item == nullptr) continue;

        // Test the subsumption
        if(this->IsSubsumed(item.get())) {
            return true;
        }
    }

    return false;
}

bool TermBaseSet::IsSubsumedBy(std::list<Term_ptr>& fixpoint) {
    // TODO: Add caching
    if(this->IsEmpty()) {
        return true;
    }

    // For each item in fixpoint
    // TODO: Maybe during this shit we could remove some of the things from fixpoint?
    for(auto item : fixpoint) {
        // Nullptr is skipped
        if(item == nullptr) continue;

        // Test the subsumption
        if(this->IsSubsumed(item.get())) {
            return true;
        #if (OPT_PRUNE_FIXPOINT == true)
        } else if(this->type == TERM_BASE && item->IsSubsumed(this)) {
            fixpoint.remove(item);
        #endif
        }
    }

    return false;
}

bool TermContinuation::IsSubsumedBy(std::list<Term_ptr>& fixpoint) {
    assert(false && "TermContSubset.IsSubsumedBy() is impossible to happen~!");
}

bool TermList::IsSubsumedBy(std::list<Term_ptr>& fixpoint) {
    // TODO: Add caching
    if(this->IsEmpty()) {
        return true;
    }

    // For each item in fixpoint
    for(auto item : fixpoint) {
        // Nullptr is skipped
        if(item == nullptr) continue;

        return this->IsSubsumed(item.get());
    }

    return false;
}

bool TermFixpoint::IsSubsumedBy(std::list<Term_ptr>& fixpoint) {
    // TODO: There should be unfolding of fixpoint probably
    #if (DEBUG_TERM_SUBSUMPTION == true)
    this->dump();
    std::cout << " <?= ";
    std::cout << "{";
    for(auto item : fixpoint) {
        if(item == nullptr) continue;
        item->dump();
        std::cout << ",";
    }
    std::cout << "}";
    std::cout << "\n";
    #endif

    // Component-wise comparison
    for(auto item : fixpoint) {
        if(item == nullptr) continue;
        if (this->IsSubsumed(item.get())) {
            return true;
        }
    }
    return false;
}

/**
 * Tests the emptiness of Term
 */
bool TermEmpty::IsEmpty() {
    return true;
}

bool TermProduct::IsEmpty() {
    return this->left->IsEmpty() && this->right->IsEmpty();
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
        for(auto item : this->list) {
            if(!item->IsEmpty()) {
                return false;
            }
        }
        return false;
    }
}

bool TermFixpoint::IsEmpty() {
    return this->_worklist.empty();
}

/**
 * Measures the state space as number of states
 */
unsigned int Term::MeasureStateSpace() {
    if(this->stateSpace != 0) {
        return this->stateSpace;
    } else {
        return this->_MeasureStateSpaceCore();
    }
}

unsigned int TermEmpty::_MeasureStateSpaceCore() {
    return 0;
}

unsigned int TermProduct::_MeasureStateSpaceCore() {
    return this->left->MeasureStateSpace() + this->right->MeasureStateSpace() + 1;
}

unsigned int TermBaseSet::_MeasureStateSpaceCore() {
    return this->stateSpace;
}

unsigned int TermContinuation::_MeasureStateSpaceCore() {
    return 1;
}

unsigned int TermList::_MeasureStateSpaceCore() {
    unsigned int count = 0;
    // Measure state spaces of all subterms
    for(auto item : this->list) {
        count += item->MeasureStateSpace();
    }

    return count;
}

unsigned int TermFixpoint::_MeasureStateSpaceCore() {
    unsigned count = 1;
    for(auto item : this->_fixpoint) {
        if(item == nullptr) {
            continue;
        }
        assert(item != nullptr);
        count += item->MeasureStateSpace();
    }

    return count;
}
/**
 * Dumping functions
 */
std::ostream& operator <<(std::ostream& osObject, Term& z) {
    // TODO: Optimize this you lazy fuck!
#if (DEBUG_TERM_UNIQUENESS == true)
    osObject << "[" << &z <<"]";
#endif
    z.dump();
    return osObject;
}

namespace Gaston {
    void dumpResultKey(std::pair<Term*, Symbol*> const& s) {
        assert(s.first != nullptr);
        assert(s.second != nullptr);

        std::cout << "<" << (*s.first) << ", " << (*s.second) << ">";
    }

    void dumpResultData(std::pair<std::shared_ptr<Term>, bool> &s) {
        std::cout << "<" << (*s.first) << ", " << (s.second ? "True" : "False") << ">";
    }

    void dumpSubsumptionKey(std::pair < Term * , Term * > const& s) {
        assert(s.first != nullptr);
        assert(s.second != nullptr);

        std::cout << "<" << (*s.first) << ", " << (*s.second) << ">";
    }

    void dumpSubsumptionData(bool &s) {
        std::cout << (s ? "True" : "False");
    }
}

void Term::dump() {
    #if (DEBUG_TERM_UNIQUENESS == true)
    std::cout << "[" << this << "]";
    #endif
    if(this->_inComplement) {
        std::cout << "\033[1;31m{\033[0m";
    }
    this->_dumpCore();
    if(this->_inComplement) {
        std::cout << "\033[1;31m}\033[0m";
    }
}

void TermEmpty::_dumpCore() {
    std::cout << "\u2205";
}

void TermProduct::_dumpCore() {
    if(this->subtype == ProductType::E_INTERSECTION) {
        std::cout << "\033[1;32m";
    } else {
        std::cout << "\033[1;33m";
    }
    std::cout << "{\033[0m";
    left->dump();
    if(this->subtype == ProductType::E_INTERSECTION) {
        std::cout << "\033[1;32m \u2293 \033[0m";
    } else {
        //std::cout << " \u22C3 ";
        std::cout << "\033[1;33m \u2294 \033[0m";
    };
    right->dump();
    if(this->subtype == ProductType::E_INTERSECTION) {
        std::cout << "\033[1;32m";
    } else {
        std::cout << "\033[1;33m";
    }
    std::cout << "}\033[0m";
}

void TermBaseSet::_dumpCore() {
    std::cout << "\033[1;35m{";
    for (auto state : this->states) {
        std::cout << (state) << ",";
    }
    std::cout << "}\033[0m";
}

void TermContinuation::_dumpCore() {
    std::cout << "?";
    term->dump();
    std::cout << "?";
    std::cout << "'";
    if(symbol != nullptr) {
        std::cout << (*symbol);
    }
    std::cout << "'";
    std::cout << "?";
}

void TermList::_dumpCore() {
    std::cout << "\033[1;36m{\033[0m";
    for(auto state : this->list) {
        state->dump();
        std::cout  << ",";
    }
    std::cout << "\033[1;36m}\033[0m";
}

void TermFixpoint::_dumpCore() {
    std::cout << "\033[1;34m{\033[0m";
    for(auto item : this->_fixpoint) {
        if(item == nullptr) {
            continue;
        }
        item->dump();
        std::cout << "\033[1;34m,\033[0m";
    }
    std::cout << "\033[1;34m}\033[0m";
}
// <<< ADDITIONAL TERMBASESET FUNCTIONS >>>

bool TermBaseSet::Intersects(TermBaseSet* rhs) {
    for (auto lhs_state : this->states) {
        for(auto rhs_state : rhs->states) {
            if(lhs_state == rhs_state) {
                return true;
            }
        }
    }
    return false;
}

// <<< ADDITIONAL TERMFIXPOINT FUNCTIONS >>>

/**
 * Tests if term is subsumed by fixpoint, either it is already computed in cache
 * or we have to compute the subsumption testing for each of the fixpoint members
 * and finally store the results in the cache.
 *
 * @param[in] term:     term we are testing subsumption for
 * @return:             true if term is subsumed by fixpoint
 */
bool TermFixpoint::_testIfSubsumes(Term_ptr &term) {
    #if (OPT_CACHE_SUBSUMED_BY == true)
    bool result;
    Term* key = term.get();
    if(!this->_subsumedByCache.retrieveFromCache(key, result)) {
        if(result = term->IsSubsumedBy(this->_fixpoint)) {
            this->_subsumedByCache.StoreIn(key, result);
        }
    }
    #if (MEASURE_SUBSUMEDBY_HITS == true)
    else {
        ++TermFixpoint::subsumedByHits;
    }
    #endif

    return result;
    #else
    if(term->IsSubsumedBy(this->_fixpoint)) {
        return true;
    }

    return false;
    #endif
}

/**
 * Does the computation of the next fixpoint, i.e. the next iteration.
 */
void TermFixpoint::ComputeNextFixpoint() {
    assert(!_worklist.empty());

    // Pop the front item from worklist
    WorklistItemType item = _worklist.front();
    _worklist.pop_front();

    // Compute the results
    ResultType result = _aut->IntersectNonEmpty(&item.second, item.first.get(), this->_nonMembershipTesting);

    // If it is subsumed by fixpoint, we don't add it
    if(this->_testIfSubsumes(result.first)) {
        return;
    }

    // Push new term to fixpoint
    _fixpoint.push_back(result.first);
    // Aggregate the result of the fixpoint computation
    _bValue = this->_aggregate_result(_bValue,result.second);
    // Push new symbols from _symList
    for(auto symbol : _symList) {
        _worklist.insert(_worklist.cbegin(), std::make_pair(result.first, symbol));
    }
}

/**
 * Computes the pre of the fixpoint, i.e. it does pre on all the things
 * as we had already the fixpoint computed from previous step
 */
void TermFixpoint::ComputeNextPre() {
    assert(!_worklist.empty());

    // Pop item from worklist
    WorklistItemType item = _worklist.front();
    _worklist.pop_front();

    // Compute the results
    ResultType result = _aut->IntersectNonEmpty(&item.second, item.first.get(), this->_nonMembershipTesting);

    // If it is subsumed we return
    //if(result.first->IsSubsumedBy(_fixpoint)) {
    if(this->_testIfSubsumes(result.first)) {
        return;
    }

    // Push the computed thing and aggregate the result
    _fixpoint.push_back(result.first);
    _bValue = this->_aggregate_result(_bValue,result.second);
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
        this->_aggregate_result = [](bool a, bool b) {return a || b;};
    } else {
        this->_aggregate_result = [](bool a, bool b) {return a && b;};
    }
}

/**
 * @return: result of fixpoint
 */
bool TermFixpoint::GetResult() {
    return this->_bValue;
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

// <<< EQUALITY MEASURING FUNCTIONS
#if (MEASURE_COMPARISONS == true)
void Term::comparedBySamePtr(TermType t) {
    ++Term::comparisonsBySamePtr;
    switch(t) {
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

void Term::comparedByStructure(TermType t) {
    ++Term::comparisonsByStructure;
    switch(t) {
        case TERM_BASE:
            ++TermBaseSet::comparisonsByStructure;
            break;
        case TERM_LIST:
            ++TermList::comparisonsByStructure;
            break;
        case TERM_PRODUCT:
            ++TermProduct::comparisonsByStructure;
            break;
        case TERM_FIXPOINT:
            ++TermFixpoint::comparisonsByStructure;
            break;
        case TERM_CONTINUATION:
            ++TermContinuation::comparisonsByStructure;
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
    // TODO: Add measure here
    if(this == &t) {
        // Same thing
        #if (MEASURE_COMPARISONS == true)
        Term::comparedBySamePtr(this->type);
        #endif
        return true;
    } else if (this->type != t.type) {
        // Terms are of different type
        #if (MEASURE_COMPARISONS == true)
        Term::comparedByDifferentType(this->type);
        #endif
        return false;
    } else {
        #if (MEASURE_COMPARISONS == true)
        Term::comparedByStructure(this->type);
        #endif
        return this->_eqCore(t);
    }
}

bool TermEmpty::_eqCore(const Term &t) {
    assert(t.type == TERM_EMPTY && "Testing equality of different term types");
    return true;
}

bool TermProduct::_eqCore(const Term &t) {
    assert(t.type == TERM_PRODUCT && "Testing equality of different term types");

    const TermProduct &tProduct = static_cast<const TermProduct&>(t);
    if(this->left->stateSpaceApprox < this->right->stateSpaceApprox) {
        return (*tProduct.left == *this->left) && (*tProduct.right == *this->right);
    } else {
        return (*tProduct.right == *this->right) && (*tProduct.left == *this->left);
    }
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
    return (this->symbol == tCont.symbol) && (this->term == tCont.term);
}

bool TermList::_eqCore(const Term &t) {
    assert(t.type == TERM_LIST && "Testing equality of different term types");

    const TermList &tList = static_cast<const TermList&>(t);
    G_NOT_IMPLEMENTED_YET("TermList::_eqCore");
}

bool TermFixpoint::_eqCore(const Term &t) {
    assert(t.type == TERM_FIXPOINT && "Testing equality of different term types");

    const TermFixpoint &tFix = static_cast<const TermFixpoint&>(t);
    if(this->_fixpoint.size() != tFix._fixpoint.size()) {
        return false;
    } else {
        for(auto it = this->_fixpoint.begin(); it != this->_fixpoint.end(); ++it) {
            if(*it == nullptr)
                continue;
            bool found = false;
            for(auto tit = tFix._fixpoint.begin(); tit != tFix._fixpoint.end(); ++tit) {
                if(*tit == nullptr)
                    continue;
                if(**it == **tit) {
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
}