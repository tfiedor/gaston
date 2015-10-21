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

// <<< STATIC MEMBER INITIALIZATION >>>
int TermProduct::instances = 0;
int TermBaseSet::instances = 0;
int TermList::instances = 0;
int TermFixpoint::instances = 0;
int TermContinuation::instances = 0;

// <<< TERM CONSTRUCTORS >>>
/**
 * Constructor of Term Product---construct intersecting product of
 * @p lhs and @p rhs
 *
 * @param[in] lhs:  left operand of term intersection
 * @param[in] rhs:  right operand of term intersection
 */
TermProduct::TermProduct(Term_ptr lhs, Term_ptr rhs) : left(lhs), right(rhs) {
    #if (MEASURE_STATE_SPACE == true)
    ++TermProduct::instances;
    #endif
    type = TERM_PRODUCT;
}

/**
 * Constructor of Term Product---other type
 */
TermProduct::TermProduct(Term_ptr lhs, Term_ptr rhs, TermType t) : left(lhs), right(rhs) {
    #if (MEASURE_STATE_SPACE == true)
    ++TermProduct::instances;
    #endif
    type = t;
}

TermBaseSet::TermBaseSet() : states() {
    #if (MEASURE_STATE_SPACE == true)
    ++TermBaseSet::instances;
    #endif
    type = TERM_BASE;
}

TermBaseSet::TermBaseSet(TermBaseSetStates& s) : states()  {
    #if (MEASURE_STATE_SPACE == true)
    ++TermBaseSet::instances;
    #endif
    type = TERM_BASE;
    for(auto state : s) {
        this->states.push_back(state);
    }
}

TermBaseSet::TermBaseSet(VATA::Util::OrdVector<unsigned int>& s) : states()  {
    #if (MEASURE_STATE_SPACE == true)
    ++TermBaseSet::instances;
    #endif
    type = TERM_BASE;
    for(auto state : s) {
        this->states.push_back(state);
    }
}

TermContinuation::TermContinuation(std::shared_ptr<SymbolicAutomaton> a, Term_ptr t, std::shared_ptr<SymbolType> s, bool b) : aut(a), term(t), symbol(s), underComplement(b) {
    #if (MEASURE_STATE_SPACE == true)
    ++TermContinuation::instances;
    #endif
    this->type = TERM_CONTINUATION;
    #if (DEBUG_CONTINUATIONS == true)
    std::cout << "Postponing computation as [";
    t->dump();
    std::cout << "]\n";
    #endif
}

TermList::TermList() {
    #if (MEASURE_STATE_SPACE == true)
    ++TermList::instances;
    #endif
    type = TERM_LIST;
}

TermList::TermList(Term_ptr first, bool isCompl) : isComplement(isCompl) {
    #if (MEASURE_STATE_SPACE == true)
    ++TermList::instances;
    #endif
    this->type = TERM_LIST;
    this->list.push_back(first);
}

TermList::TermList(Term_ptr f, Term_ptr s, bool isCompl) : isComplement(isCompl) {
    #if (MEASURE_STATE_SPACE == true)
    ++TermList::instances;
    #endif
    this->type = TERM_LIST;
    this->list.push_back(f);
    this->list.push_back(s);
}

TermFixpoint::TermFixpoint(std::shared_ptr<SymbolicAutomaton> aut, Term_ptr startingTerm, Symbols symList, bool inComplement, bool initbValue)
        : _sourceTerm(nullptr), _sourceIt(nullptr), _aut(aut), _bValue(initbValue), _inComplement(inComplement) {
    #if (MEASURE_STATE_SPACE == true)
    ++TermFixpoint::instances;
    #endif

    // Initialize the aggregate function
    this->_InitializeAggregateFunction(inComplement);
    this->type = TERM_FIXPOINT;

    // Initialize the fixpoint as [nullptr, startingTerm]
    this->_fixpoint.push_front(startingTerm);
    this->_fixpoint.push_front(nullptr);

    // Push symbols to worklist
    for(auto symbol : symList) {
        this->_symList.push_back(symbol);
        this->_worklist.insert(this->_worklist.cbegin(), std::make_pair(startingTerm, symbol));
    }
}

TermFixpoint::TermFixpoint(std::shared_ptr<SymbolicAutomaton> aut, Term_ptr sourceTerm, Symbols symList,bool inComplement)
        : _sourceTerm(sourceTerm), _sourceIt(reinterpret_cast<TermFixpoint*>(sourceTerm.get())->GetIteratorDynamic()),
        _aut(aut), _worklist(), _bValue(false), _inComplement(inComplement) {
    #if (MEASURE_STATE_SPACE == true)
    ++TermFixpoint::instances;
    #endif
    assert(sourceTerm->type == TERM_FIXPOINT);

    // Initialize the aggregate function
    this->_InitializeAggregateFunction(inComplement);
    this->type = TERM_FIXPOINT;

    // Initialize the fixpoint
    this->_fixpoint.push_front(nullptr);
    // Push things into worklist
    for(auto symbol : symList) {
        this->_symList.push_back(symbol);
    }
}

/**
 * Tests the Term subsumption
 *
 * @param[in] t:    term we are testing subsumption against
 * @return:         true if this is subsumed by @p t
 */
bool TermProduct::IsSubsumed(Term* t) {
    // TODO: Add continuations
    // TODO: Add caching
    #if (DEBUG_TERM_SUBSUMPTION == true)
    this->dump();
    std::cout << " <?= ";
    t->dump();
    std::cout << "\n";
    #endif
    assert(t->type == TERM_PRODUCT);

    // Retype and test the subsumption component-wise
    TermProduct *rhs = reinterpret_cast<TermProduct*>(t);
    return (this->left->IsSubsumed(rhs->left.get())) && (this->right->IsSubsumed(rhs->right.get()));
}

bool TermBaseSet::IsSubsumed(Term* term) {
    #if (DEBUG_TERM_SUBSUMPTION == true)
    this->dump();
    std::cout << " <?= ";
    term->dump();
    std::cout;
    #endif

    assert(term->type == TERM_BASE);

    // Test component-wise, not very efficient though
    // TODO: Change to bit-vectors if possible
    TermBaseSet *t = reinterpret_cast<TermBaseSet*>(term);
    if(t->states.size() < this->states.size()) {
        return false;
    } else {
        // TODO: Maybe we could exploit that we have ordered vectors
        for(auto state : this->states) {
            auto isIn = std::find(t->states.begin(), t->states.end(), state);
            if(isIn == t->states.end()) {
                // Not in, false
                return false;
            }
        }
        return true;
    }
}

bool TermContinuation::IsSubsumed(Term *t) {
    // TODO: How to do this smartly?
    // TODO: Maybe if we have {} we can answer sooner, without unpacking
    //assert(t->type != TERM_CONT_SUBSET);

    // We unpack this term
    auto unfoldedTerm = (this->aut->IntersectNonEmpty(this->symbol.get(), this->term, false)).first;
    if(t->type == TERM_CONTINUATION) {
        // @p t is also folded in continuation so we have to unfold it as well
        TermContinuation* continuationT = reinterpret_cast<TermContinuation*>(t);
        assert(continuationT->underComplement == this->underComplement);

        auto unfoldedT = (continuationT->aut->IntersectNonEmpty(continuationT->symbol.get(), continuationT->term, false)).first;

        // Test the subsumption over the unfolded stuff
        return unfoldedTerm->IsSubsumed(unfoldedT.get());
    } else {
        // Test the subsumption over the unfolded stuff
        return unfoldedTerm->IsSubsumed(t);
    }
}

bool TermList::IsSubsumed(Term* t) {
    // Unfold continuation
    if(t->type == TERM_CONTINUATION) {
        TermContinuation* productContinuation = reinterpret_cast<TermContinuation*>(t);
        t = (productContinuation->aut->IntersectNonEmpty((productContinuation->symbol == nullptr ? nullptr : productContinuation->symbol.get()), productContinuation->term, false)).first.get();
    }

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


bool TermFixpoint::IsSubsumed(Term* t) {
    // Unfold continuation
    if(t->type == TERM_CONTINUATION) {
        TermContinuation* productContinuation = reinterpret_cast<TermContinuation*>(t);
        t = (productContinuation->aut->IntersectNonEmpty((productContinuation->symbol == nullptr ? nullptr : productContinuation->symbol.get()), productContinuation->term, productContinuation->underComplement)).first.get();
    }

    if(t->type != TERM_FIXPOINT) {
        assert(false && "Testing subsumption of incompatible terms\n");
    }

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
bool TermProduct::IsSubsumedBy(std::list<Term_ptr>& fixpoint) {
    // TODO: Add caching
    // Empty things is always subsumed
    if(this->IsEmpty()) {
        return true;
    }

    // For each item in fixpoint
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

bool TermContinuation::IsSubsumedBy(std::list<Term_ptr>& fixpoint) {
    assert(false && "TermContSubset.IsSubsumedBy() is impossible to happen~!");
}

bool TermList::IsSubsumedBy(std::list<Term_ptr>& fixpoint) {
    // TODO: Add caching
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
    if(this->IsEmpty()) {
        return true;
    }

    // For each item in fixpoint
    for(auto item : fixpoint) {
        // Nullptr is skipped
        if(item == nullptr) continue;

        // Test the subsumption of terms, if we are under complement
        //   we switch the subsumption relation
        if(this->isComplement) {
            if (item->IsSubsumed(this)) {
                return true;
            }
        } else {
            if (this->IsSubsumed(item.get())) {
                return true;
            }
        }
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
unsigned int TermProduct::MeasureStateSpace() {
    return this->left->MeasureStateSpace() + this->right->MeasureStateSpace() + 1;
}

unsigned int TermBaseSet::MeasureStateSpace() {
    return this->states.size();
}

unsigned int TermContinuation::MeasureStateSpace() {
    return 1;
}

unsigned int TermList::MeasureStateSpace() {
    unsigned int count = 0;
    // Measure state spaces of all subterms
    for(auto item : this->list) {
        count += item->MeasureStateSpace();
    }

    return count;
}

unsigned int TermFixpoint::MeasureStateSpace() {
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
void TermProduct::dump() {
    std::cout << "{";
    left->dump();
    std::cout << " x ";
    right->dump();
    std::cout << "}";
}

void TermBaseSet::dump() {
    std::cout << "{";
    for(auto state : this->states) {
        std::cout << (state) << ",";
    }
    std::cout << "}";
}

void TermContinuation::dump() {
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

void TermList::dump() {
    std::cout << "{";
    for(auto state : this->list) {
        state->dump();
        std::cout  << ",";
    }
    std::cout << "}";
}

void TermFixpoint::dump() {
    std::cout << "{";
    for(auto item : this->_fixpoint) {
        if(item == nullptr) {
            continue;
        }
        item->dump();
        std::cout << ",";
    }
    std::cout << "}";
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
 * Does the computation of the next fixpoint, i.e. the next iteration.
 */
void TermFixpoint::ComputeNextFixpoint() {
    assert(!_worklist.empty());

    // Pop the front item from worklist
    WorklistItemType item = _worklist.front();
    _worklist.pop_front();

    // Compute the results
    ResultType result = _aut->IntersectNonEmpty(&item.second, item.first, this->_inComplement);

    // If it is subsumed by fixpoint, we don't add it
    if(result.first->IsSubsumedBy(_fixpoint)) {
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
    ResultType result = _aut->IntersectNonEmpty(&item.second, item.first, this->_inComplement);

    // If it is subsumed we return
    if(result.first->IsSubsumedBy(_fixpoint)) {
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