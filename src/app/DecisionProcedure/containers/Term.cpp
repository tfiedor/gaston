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
        if (s->type == TERM_CONTINUATION) {
            // Todo: this is never hit fuck
            TermContinuation* sCont = reinterpret_cast<TermContinuation*>(s);
            if(sCont->IsUnfolded()) {
                return boost::hash_value(sCont->GetUnfoldedTerm());
            } else {
                return boost::hash_value(s);
            }
        } /*else if(s->type == TERM_FIXPOINT || s->type == TERM_LIST) {
            size_t seed = boost::hash_value(s->stateSpaceApprox);
            boost::hash_combine(seed, boost::hash_value(s->MeasureStateSpace()));
            return seed;
        }*/ else {
            return boost::hash_value(s);
        }
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
size_t TermFixpoint::preInstances = 0;
size_t TermFixpoint::isNotShared = 0;
size_t TermFixpoint::postponedTerms = 0;
size_t TermFixpoint::postponedProcessed = 0;
size_t TermContinuation::continuationUnfolding = 0;
size_t TermContinuation::unfoldInSubsumption = 0;
size_t TermContinuation::unfoldInIsectNonempty = 0;

extern Ident lastPosVar, allPosVar;

// <<< TERM CONSTRUCTORS AND DESTRUCTORS >>>
Term::~Term() {}

TermEmpty::TermEmpty(bool inComplement) {
    #if (MEASURE_STATE_SPACE == true)
    ++TermEmpty::instances;
    #endif
    this->_inComplement = inComplement;
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
TermProduct::TermProduct(Term_ptr lhs, Term_ptr rhs, ProductType pt) : left(lhs), right(rhs) {
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

TermBaseSet::~TermBaseSet() {
    this->states.clear();
}

TermContinuation::TermContinuation(SymbolicAutomaton* a, Term* t, SymbolType* s, bool b) : aut(a), term(t), symbol(s), underComplement(b) {
    #if (DEBUG_TERM_CREATION == true)
    std::cout << "[" << this << "]";
    std::cout << "TermContinuation::";
    this->dump();
    std::cout << "\n";
    #endif
    #if (MEASURE_STATE_SPACE == true)
    ++TermContinuation::instances;
    #endif
    assert(t != nullptr);

    this->type = TERM_CONTINUATION;

    // Initialization of state space
    this->stateSpace = 0;
    this->stateSpaceApprox = t->stateSpaceApprox;

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

TermFixpoint::TermFixpoint(Aut_ptr aut, Term_ptr startingTerm, Symbol* symbol, bool inComplement, bool initbValue, WorklistSearchType search = WorklistSearchType::E_DFS)
        : _sourceTerm(nullptr), _sourceIt(nullptr), _aut(reinterpret_cast<ProjectionAutomaton*>(aut)->GetBase()), _bValue(initbValue) {
    #if (MEASURE_STATE_SPACE == true)
    ++TermFixpoint::instances;
    #endif

    // Initialize the aggregate function
    this->_InitializeAggregateFunction(inComplement);
    this->_nonMembershipTesting = inComplement;
    this->_inComplement = false;
    this->type = TERM_FIXPOINT;
    this->_searchType = search;

    // Initialize the state space
    this->stateSpace = 0;
    this->stateSpaceApprox = startingTerm->stateSpaceApprox;

    // Initialize the fixpoint as [nullptr, startingTerm]
    this->_fixpoint.push_front(std::make_pair(startingTerm, true));
    this->_fixpoint.push_front(std::make_pair(nullptr, true));

    // Push symbols to worklist
    this->_InitializeSymbols(aut->symbolFactory, aut->GetFreeVars(), reinterpret_cast<ProjectionAutomaton*>(aut)->projectedVars, symbol);
    for(auto symbol : this->_symList) {
        this->_worklist.insert(this->_worklist.cbegin(), std::make_pair(startingTerm, symbol));
    }

    #if (DEBUG_TERM_CREATION == true)
    std::cout << "[" << this << "]";
    std::cout << "TermFixpoint::";
    this->dump();
    std::cout << "\n";
    #endif
}

TermFixpoint::TermFixpoint(Aut_ptr aut, Term_ptr sourceTerm, Symbol* symbol, bool inComplement)
        : _sourceTerm(sourceTerm), _sourceIt(reinterpret_cast<TermFixpoint*>(sourceTerm)->GetIteratorDynamic()),
          _aut(reinterpret_cast<ProjectionAutomaton*>(aut)->GetBase()), _worklist(), _bValue(inComplement) {
    #if (MEASURE_STATE_SPACE == true)
    ++TermFixpoint::instances;
    ++TermFixpoint::preInstances;
    #endif
    assert(sourceTerm->type == TERM_FIXPOINT);

    // Initialize the state space
    this->stateSpace = 0;
    this->stateSpaceApprox = sourceTerm->stateSpaceApprox;

    // Initialize the aggregate function
    this->_InitializeAggregateFunction(inComplement);
    this->type = TERM_FIXPOINT;
    this->_searchType = WorklistSearchType::E_DFS;
    this->_nonMembershipTesting = inComplement;
    this->_inComplement = false;

    // Initialize the fixpoint
    this->_fixpoint.push_front(std::make_pair(nullptr, true));
    // Push things into worklist
    this->_InitializeSymbols(aut->symbolFactory, aut->GetFreeVars(), reinterpret_cast<ProjectionAutomaton*>(aut)->projectedVars, symbol);

    #if (DEBUG_TERM_CREATION == true)
    std::cout << "[" << this << "]";
    std::cout << "TermFixpointPre::";
    this->dump();
    std::cout << "\n";
    #endif
}

TermFixpoint::~TermFixpoint() {
    this->_fixpoint.clear();
    this->_postponed.clear();
    this->_worklist.clear();
}

void Term::Complement() {
    this->_inComplement = !this->_inComplement;
}

void Term::SetSuccessor(Term* succ, Symbol* symb) {
    if(this->link.succ == nullptr) {
        this->link.succ = succ;
        this->link.symbol = symb;
        this->link.len = succ->link.len + 1;
    }
}

/**
 * Returns true if the term is not computed, i.e. it is continuation somehow
 */
bool Term::IsNotComputed() {
#if (OPT_EARLY_EVALUATION == true)
    if(this->type == TERM_CONTINUATION) {
        return !reinterpret_cast<TermContinuation *>(this)->IsUnfolded();
    } else if(this->type == TERM_PRODUCT) {
        TermProduct* termProduct = reinterpret_cast<TermProduct*>(this);
        return termProduct->left->IsNotComputed() || termProduct->right->IsNotComputed();
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
SubsumptionResult Term::IsSubsumed(Term *t, bool unfoldAll) {
    #if (DEBUG_TERM_SUBSUMPTION == true)
    this->dump();
    std::cout << " <?= ";
    t->dump();
    std::cout << "\n";
    #endif
    // unfold the continuation
    if(this == t) {
        return E_TRUE;
    }

    if(t->type == TERM_CONTINUATION) {
        TermContinuation *continuation = reinterpret_cast<TermContinuation *>(t);
        Term* unfoldedContinuation = continuation->unfoldContinuation(UnfoldedInType::E_IN_SUBSUMPTION);
        return this->IsSubsumed(unfoldedContinuation, unfoldAll);
    } else if(this->type == TERM_CONTINUATION) {
        TermContinuation *continuation = reinterpret_cast<TermContinuation *>(this);
        Term* unfoldedContinuation = continuation->unfoldContinuation(UnfoldedInType::E_IN_SUBSUMPTION);
        return unfoldedContinuation->IsSubsumed(t, unfoldAll);
    }
    assert(this->_inComplement == t->_inComplement);
    assert(this->type != TERM_CONTINUATION && t->type != TERM_CONTINUATION);

    // Else if it is not continuation we first look into cache and then recompute if needed
    SubsumptionResult result;
    #if (OPT_CACHE_SUBSUMES == true)
    if(this->type != TERM_FIXPOINT || !this->_isSubsumedCache.retrieveFromCache(t, result)) {
    #endif
        if (this->_inComplement) {
            if (this->type == TERM_EMPTY) {
                result = (t->type == TERM_EMPTY) ? E_TRUE : E_FALSE;
            } else {
                result = t->_IsSubsumedCore(this, unfoldAll);
            }
        } else {
            if (t->type == TERM_EMPTY) {
                result = (this->type == TERM_EMPTY) ? E_TRUE : E_FALSE;
            } else {
                result = this->_IsSubsumedCore(t, unfoldAll);
            }
        }
    #if (OPT_CACHE_SUBSUMES == true)
        if(result != E_PARTIALLY && this->type == TERM_FIXPOINT)
            this->_isSubsumedCache.StoreIn(t, result);
    }
    #endif
    assert(!unfoldAll || result != E_PARTIALLY);
    return result;
}

SubsumptionResult TermEmpty::_IsSubsumedCore(Term *t, bool unfoldAll) {
    // Empty term is subsumed by everything (probably)
    return E_TRUE;
}

SubsumptionResult TermProduct::_IsSubsumedCore(Term* t, bool unfoldAll) {
    assert(t->type == TERM_PRODUCT);

    // Retype and test the subsumption component-wise
    TermProduct *rhs = reinterpret_cast<TermProduct*>(t);
    Term *lhsl = this->left;
    Term *lhsr = this->right;
    Term *rhsl = rhs->left;
    Term *rhsr = rhs->right;

    if(!unfoldAll && (lhsr->IsNotComputed() && rhsr->IsNotComputed())) {
        #if (OPT_EARLY_PARTIAL_SUB == true)
        if(lhsr->type == TERM_CONTINUATION && rhsr->type == TERM_CONTINUATION) {
            return (lhsl->IsSubsumed(rhsl, unfoldAll) == E_FALSE ? E_FALSE : E_PARTIALLY);
        } else {
            SubsumptionResult leftIsSubsumed = lhsl->IsSubsumed(rhsl, unfoldAll);
            if(leftIsSubsumed == E_TRUE) {
                return lhsr->IsSubsumed(rhsr, unfoldAll);
            } else {
                return leftIsSubsumed;
            }
        }
        #else
        return (lhsl->IsSubsumed(rhsl) != E_FALSE && lhsr->IsSubsumed(rhsr) != E_FALSE) ? E_TRUE : E_FALSE;
        #endif
    } if(!unfoldAll && lhsl == rhsl) {
        return lhsr->IsSubsumed(rhsr, unfoldAll);
    } else if(!unfoldAll && lhsr == rhsr) {
        return lhsl->IsSubsumed(rhsl, unfoldAll);
    } else {
        if(lhsl->stateSpaceApprox < lhsr->stateSpaceApprox || unfoldAll) {
            return (lhsl->IsSubsumed(rhsl, unfoldAll) != E_FALSE && lhsr->IsSubsumed(rhsr, unfoldAll) != E_FALSE) ? E_TRUE : E_FALSE;
        } else {
            return (lhsr->IsSubsumed(rhsr, unfoldAll) != E_FALSE && lhsl->IsSubsumed(rhsl, unfoldAll) != E_FALSE) ? E_TRUE : E_FALSE;
        }
    }
}

SubsumptionResult TermBaseSet::_IsSubsumedCore(Term* term, bool unfoldAll) {
    assert(term->type == TERM_BASE);

    // Test component-wise, not very efficient though
    // TODO: Change to bit-vectors if possible
    TermBaseSet *t = reinterpret_cast<TermBaseSet*>(term);
    if(t->states.size() < this->states.size()) {
        return E_FALSE;
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
                return E_FALSE;
            }
        }
        return (it == end) ? E_TRUE : E_FALSE;
    }
}

SubsumptionResult TermContinuation::_IsSubsumedCore(Term *t, bool unfoldAll) {
    // TODO: How to do this smartly?
    // TODO: Maybe if we have {} we can answer sooner, without unpacking
    assert(false);

    // We unpack this term
    auto unfoldedTerm = (this->aut->IntersectNonEmpty(this->symbol, this->term, this->underComplement)).first;
    return unfoldedTerm->IsSubsumed(t, unfoldAll);
}

SubsumptionResult TermList::_IsSubsumedCore(Term* t, bool unfoldAll) {
    assert(t->type == TERM_LIST);

    // Reinterpret
    TermList* tt = reinterpret_cast<TermList*>(t);
    // Do the item-wise subsumption check
    for(auto& item : this->list) {
        bool subsumes = false;
        for(auto& tt_item : tt->list) {
            if(item->IsSubsumed(tt_item, unfoldAll)) {
                subsumes = true;
                break;
            }
        }
        if(!subsumes) return E_FALSE;
    }

    return E_TRUE;
}

SubsumptionResult TermFixpoint::_IsSubsumedCore(Term* t, bool unfoldAll) {
    assert(t->type == TERM_FIXPOINT);

    // Reinterpret
    TermFixpoint* tt = reinterpret_cast<TermFixpoint*>(t);
    if(this->_bValue != tt->_bValue) {
        // we can automatically assume that these two are different
        return E_FALSE;
    }

    // Do the piece-wise comparison
    for(auto& item : this->_fixpoint) {
        // Skip the nullptr
        if(item.first == nullptr || !item.second) continue;
        bool subsumes = false;
        for(auto& tt_item : tt->_fixpoint) {
            if(tt_item.first == nullptr || !item.second) continue;
            if((item.first)->IsSubsumed(tt_item.first, unfoldAll)) {
                subsumes = true;
                break;
            }
        }
        if(!subsumes) return E_FALSE;
    }

    #if (DEBUG_COMPARE_WORKLISTS == true)
    for(auto it = this->_worklist.begin(); it != this->_worklist.end(); ++it) {
        bool found = false;
        for(auto tit = tt->_worklist.begin(); tit != tt->_worklist.end(); ++tit) {
            if(*(it->first) == *(tit->first) && (it->second == tit->second)) {
                found = true;
                break;
            }
        }
        if(!found) {
            return E_FALSE;
        }
    }
    #endif

    return E_TRUE;
}

/**
 * Tests the subsumption over the list of terms
 *
 * @param[in] fixpoint:     list of terms contained as fixpoint
 */
SubsumptionResult TermEmpty::IsSubsumedBy(FixpointType& fixpoint, Term*& biggerTerm) {
    // Empty term is subsumed by everything
    return ( (fixpoint.size() == 1 && fixpoint.front().first == nullptr) ? E_FALSE : E_TRUE);
}

SubsumptionResult TermProduct::IsSubsumedBy(FixpointType& fixpoint, Term*& biggerTerm) {
    if(this->IsEmpty()) {
        return E_TRUE;
    }
    // For each item in fixpoint
    // TODO: This should be holikoptimized
    SubsumptionResult result;
    for(auto& item : fixpoint) {
        // Nullptr is skipped
        if(item.first == nullptr || !item.second) continue;

        // Test the subsumption
        if((result = this->IsSubsumed(item.first)) != E_FALSE) {
            if(result == E_PARTIALLY)
                biggerTerm = item.first;
            return result;
        }

        if(item.first->IsSubsumed(this)) {
            item.second = false;
        }
    }

    return E_FALSE;
}

SubsumptionResult TermBaseSet::IsSubsumedBy(FixpointType& fixpoint, Term*& biggerTerm) {
    if(this->IsEmpty()) {
        return E_TRUE;
    }
    // For each item in fixpoint
    // TODO: Maybe during this shit we could remove some of the things from fixpoint?
    for(auto& item : fixpoint) {
        // Nullptr is skipped
        if(item.first == nullptr || !item.second) continue;

        // Test the subsumption
        if(this->IsSubsumed(item.first)) {
            return E_TRUE ;
        }
        if(item.first->IsSubsumed(this)) {
            item.second = false;
        }
    }

    return E_FALSE;
}

SubsumptionResult TermContinuation::IsSubsumedBy(FixpointType& fixpoint, Term*& biggerTerm) {
    assert(false && "TermContSubset.IsSubsumedBy() is impossible to happen~!");
}

SubsumptionResult TermList::IsSubsumedBy(FixpointType& fixpoint, Term*& biggerTerm) {
    if(this->IsEmpty()) {
        return E_TRUE;
    }
    // For each item in fixpoint
    for(auto& item : fixpoint) {
        // Nullptr is skipped
        if(item.first == nullptr || !item.second) continue;

        if (this->IsSubsumed(item.first)) {
            return E_TRUE;
        }
    }

    return E_FALSE;
}

SubsumptionResult TermFixpoint::IsSubsumedBy(FixpointType& fixpoint, Term*& biggerTerm) {
    // TODO: There should be unfolding of fixpoint probably
    #if (DEBUG_TERM_SUBSUMPTION == true)
    this->dump();
    std::cout << " <?= ";
    std::cout << "{";
    for(auto& item : fixpoint) {
        if(item == nullptr) continue;
        item->dump();
        std::cout << ",";
    }
    std::cout << "}";
    std::cout << "\n";
    #endif

    // Component-wise comparison
    for(auto& item : fixpoint) {
        if(item.first == nullptr) continue;
        if (this->IsSubsumed(item.first)) {
            return E_TRUE ;
        }

    }
    return E_FALSE;
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
    void dumpResultKey(std::pair<Term*, Symbol*> const& s) {
        assert(s.first != nullptr);

        if(s.second == nullptr) {
            std::cout << "<" << (*s.first) << ", \u0437>";
        } else {
            std::cout << "<" << (*s.first) << ", " << (*s.second) << ">";
        }
    }

    void dumpResultData(std::pair<Term*, bool> &s) {
        std::cout << "<" << (*s.first) << ", " << (s.second ? "True" : "False") << ">";
    }

    void dumpSubsumptionKey(std::pair<Term*, Term*> const& s) {
        assert(s.first != nullptr);
        assert(s.second != nullptr);

        std::cout << "<" << (*s.first) << ", " << (*s.second) << ">";
    }

    void dumpSubsumptionData(SubsumptionResult &s) {
        std::cout << (s != E_FALSE ? "True" : "False");
    }

    void dumpPreKey(std::pair<size_t, ZeroSymbol*> const& s) {
        std::cout << "(" << s.first << ", " << (*s.second) << ")";
    }

    void dumpPreData(VATA::Util::OrdVector<size_t>& s) {
        std::cout << s;
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
    if(this->_unfoldedTerm == nullptr) {
        std::cout << "?";
        term->dump();
        std::cout << "?";
        std::cout << "'";
        if (symbol != nullptr) {
            std::cout << (*symbol);
        }
        std::cout << "'";
        std::cout << "?";
    } else {
        std::cout << "*";
        this->_unfoldedTerm->dump();
        std::cout << "*";
    }
}

void TermList::_dumpCore() {
    std::cout << "\033[1;36m{\033[0m";
    for(auto& state : this->list) {
        state->dump();
        std::cout  << ",";
    }
    std::cout << "\033[1;36m}\033[0m";
}

void TermFixpoint::_dumpCore() {
    std::cout << "[" << this << "]";
    std::cout << "\033[1;34m{\033[0m";
    for(auto& item : this->_fixpoint) {
        if(item.first == nullptr || !(item.second)) {
            continue;
        }
        item.first->dump();
        std::cout << "\033[1;34m,\033[0m";
    }
    #if (DEBUG_FIXPOINT_SYMBOLS == true)
        std::cout << "[";
        for(auto& item : this->_worklist) {
            std::cout << "(" << item.first << "+" << item.second << ") , ";
        }
        std::cout << "]";
    #endif
    std::cout << "\033[1;37m[";
    for(auto& workItem : this->_worklist) {
        std::cout << (workItem.first) << " + " << (workItem.second) << ", ";
    }
    std::cout << "]\033[0m";
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
    assert(!this->_postponed.empty());

    std::pair<Term_ptr, Term_ptr> postponedPair;
    TermProduct* temp = nullptr, *temp2 = nullptr;

    // Get the front of the postponed (must be TermProduct with continuation)
    #if (OPT_FIND_POSTPONED_CANDIDATE == true)
    bool found = false;
    // first is the postponed term, second is the thing from fixpoint!!!
    for(auto it = this->_postponed.begin(); it != this->_postponed.end(); ++it) {
        temp = reinterpret_cast<TermProduct*>((*it).first);
        temp2 = reinterpret_cast<TermProduct*>((*it).second);
        if(!temp->right->IsNotComputed() && !temp2->right->IsNotComputed()) {
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

    TermProduct* postponedTerm = reinterpret_cast<TermProduct*>(postponedPair.first);
    TermProduct* postponedFixTerm = reinterpret_cast<TermProduct*>(postponedPair.second);

    assert(postponedPair.second != nullptr);

    // Test the subsumption
    SubsumptionResult result;
    // Todo: this could be softened to iterators
    if( (result = postponedTerm->IsSubsumed(postponedFixTerm, true)) == E_FALSE) {
        // Push new term to fixpoint
        // Fixme: But there is probably something other that could subsume this crap
        for(auto item : this->_fixpoint) {
            if(item.first == nullptr)
                continue;
            if((result = postponedTerm->IsSubsumed(item.first, true)) != E_FALSE) {
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
}

/**
 * Tests if term is subsumed by fixpoint, either it is already computed in cache
 * or we have to compute the subsumption testing for each of the fixpoint members
 * and finally store the results in the cache.
 *
 * @param[in] term:     term we are testing subsumption for
 * @return:             true if term is subsumed by fixpoint
 */
SubsumptionResult TermFixpoint::_testIfSubsumes(Term_ptr const& term) {
    #if (OPT_CACHE_SUBSUMED_BY == true)
    SubsumptionResult result;
    Term* key = term, *subsumedByTerm;
    if(!this->_subsumedByCache.retrieveFromCache(key, result)) {
        // True/Partial results are stored in cache
        if((result = term->IsSubsumedBy(this->_fixpoint, subsumedByTerm))!= E_FALSE) {
            this->_subsumedByCache.StoreIn(key, result);
        }

        if(result == E_PARTIALLY) {
            assert(subsumedByTerm != nullptr);
            this->_postponed.insert(this->_postponed.begin(), std::make_pair(term, subsumedByTerm));
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
    return result;
    #else
    return term->IsSubsumedBy(this->_fixpoint);
    #endif
}

WorklistItemType TermFixpoint::_popFromWorklist() {
    if(this->_searchType == WorklistSearchType::E_DFS) {
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
    assert(!_worklist.empty());

    // Pop the front item from worklist
    WorklistItemType item = this->_popFromWorklist();

    // Compute the results
    ResultType result = _aut->IntersectNonEmpty(item.second, item.first, this->_nonMembershipTesting);
    this->_updateExamples(result);

    // If it is subsumed by fixpoint, we don't add it
    if(this->_testIfSubsumes(result.first) != E_FALSE) {
        return;
    }

    // Push new term to fixpoint
    _fixpoint.push_back(std::make_pair(result.first, true));
    // Aggregate the result of the fixpoint computation
    _bValue = this->_aggregate_result(_bValue,result.second);
    // Push new symbols from _symList
    for(auto& symbol : _symList) {
#       if (OPT_FIXPOINT_BFS_SEARCH == true)
        _worklist.push_back(std::make_pair(result.first, symbol));
#       else
        _worklist.insert(_worklist.cbegin(), std::make_pair(result.first, symbol));
#       endif
    }
}

/**
 * Computes the pre of the fixpoint, i.e. it does pre on all the things
 * as we had already the fixpoint computed from previous step
 */
void TermFixpoint::ComputeNextPre() {
    assert(!_worklist.empty());

    // Pop item from worklist
    WorklistItemType item = this->_popFromWorklist();

    // Compute the results
    ResultType result = _aut->IntersectNonEmpty(item.second, item.first, this->_nonMembershipTesting);
    this->_updateExamples(result);

    // If it is subsumed we return
    if(this->_testIfSubsumes(result.first)) {
        return;
    }

    // Push the computed thing and aggregate the result
    _fixpoint.push_back(std::make_pair(result.first, true));
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
 * Transforms @p symbols according to the bound variable in @p vars, by pumping
 *  0 and 1 on the tracks
 *
 * @param[in,out] symbols:  list of symbols, that will be transformed
 * @param[in] vars:         list of used vars, that are projected
 */
void TermFixpoint::_InitializeSymbols(Workshops::SymbolWorkshop* workshop, Gaston::VarList* freeVars, IdentList* vars, Symbol *startingSymbol) {
    this->_symList.push_back(workshop->CreateTrimmedSymbol(startingSymbol, freeVars));
    // TODO: Optimize, this sucks
    unsigned int symNum = 1;
#   if (DEBUG_FIXPOINT_SYMBOLS == true)
    std::cout << "[F] Initializing symbols of '"; this->dump(); std::cout << "\n";
#   endif
    for(auto var = vars->begin(); var != vars->end(); ++var) {
        // Pop symbol;
        for(auto i = symNum; i != 0; --i) {
            Symbol* symF = this->_symList.front();
            this->_symList.pop_front();
            // #SYMBOL_CREATION
            this->_symList.push_back(workshop->CreateSymbol(symF, varMap[(*var)], '0'));
            this->_symList.push_back(workshop->CreateSymbol(symF, varMap[(*var)], '1'));
        }

        symNum <<= 1;// times 2
    }
#   if (DEBUG_FIXPOINT_SYMBOLS == true)
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
    if(result.second) {
        if(this->_satTerm == nullptr) {
            this->_satTerm = result.first;
        }
    } else {
        if(this->_unsatTerm == nullptr) {
            this->_unsatTerm = result.first;
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
        this->_unfoldedTerm = (this->aut->IntersectNonEmpty(
                (this->symbol == nullptr ? nullptr : this->symbol), this->term, this->underComplement)).first;
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
    if(tt->type == TERM_CONTINUATION) {
        TermContinuation* ttCont = reinterpret_cast<TermContinuation*>(tt);
        tt = ttCont->unfoldContinuation(UnfoldedInType::E_IN_COMPARISON);
    }
    if(this->type == TERM_CONTINUATION) {
        TermContinuation* thisCont = reinterpret_cast<TermContinuation*>(this);
        tthis = thisCont->unfoldContinuation(UnfoldedInType::E_IN_COMPARISON);
    }

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

    const TermList &tList = static_cast<const TermList&>(t);
    G_NOT_IMPLEMENTED_YET("TermList::_eqCore");
}

bool TermFixpoint::_eqCore(const Term &t) {
    assert(t.type == TERM_FIXPOINT && "Testing equality of different term types");

    const TermFixpoint &tFix = static_cast<const TermFixpoint&>(t);
    if(this->_bValue != tFix._bValue) {
        // If the values are different, we can automatically assume that there is some difference
        return false;
    } else if(this->_fixpoint.size() != tFix._fixpoint.size()) {
        // TODO: Can this happen: that we have something twice in fixpoint?
        return false;
    } else {
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
        #if (DEBUG_COMPARE_WORKLISTS == true)
            for(auto it = this->_worklist.begin(); it != this->_worklist.end(); ++it) {
                bool found = false;
                for(auto tit = tFix._worklist.begin(); tit != tFix._worklist.end(); ++tit) {
                    if(*(it->first) == *(tit->first) && (it->second == tit->second)) {
                        found = true;
                        break;
                    }
                }
                if(!found) {
                    return false;
                }
            }
        #endif
        return true;
    }
}