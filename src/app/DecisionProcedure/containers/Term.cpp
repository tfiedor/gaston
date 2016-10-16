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
#include <sstream>
#include <boost/functional/hash.hpp>
#include <future>
#include <algorithm>

extern Ident allPosVar;

namespace Gaston {
    size_t hash_value(Term* s) {
#       if (OPT_TERM_HASH_BY_APPROX == true)
        if (s->type == TermType::CONTINUATION && OPT_EARLY_EVALUATION) {
            // Todo: this is never hit fuck
            TermContinuation *sCont = static_cast<TermContinuation *>(s);
            if (sCont->IsUnfolded()) {
                return boost::hash_value(sCont->GetUnfoldedTerm());
            } else {
                return boost::hash_value(s);
            }
#       if (DEBUG_DONT_HASH_FIXPOINTS == true)
        } else if(s->type == TermType::FIXPOINT) {
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

    size_t hash_value_no_ptr(Term* s) {
        size_t seed = boost::hash_value(s->stateSpaceApprox);
        boost::hash_combine(seed, boost::hash_value(s));
        //boost::hash_combine(seed, boost::hash_value(s->MeasureStateSpace()));
        return seed;
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
size_t TermFixpoint::fullyComputedFixpoints = 0;
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

/**
 * @brief Constructor of the empty or complemented empty term
 *
 * @param[in]  aut  link to the automaton that created the empty (note: empty terms are unique through whole program!)
 * @param[in]  inComplement  whether the term is complemented
 */
TermEmpty::TermEmpty(Aut_ptr aut, bool inComplement) : Term(aut) {
#   if (MEASURE_STATE_SPACE == true)
    ++TermEmpty::instances;
#   endif
    if(inComplement) {
        SET_IN_COMPLEMENT(this);
    }
    this->type = TermType::EMPTY;

    // Initialization of state space
    this->stateSpaceApprox = 0;

#   if (DEBUG_TERM_CREATION == true)
    std::cout << "[" << this << "]";
    std::cout << "TermEmpty::";
    this->dump();
    std::cout << "\n";
#   endif
}

/**
 * @brief Constructor of the product of terms @p lhs and @p rhs
 *
 * Creates a classic binary product of @p pt type of the pair @p lhs and @rhs
 *
 * @param[in]  aut  link to the automaton that created the product
 * @param[in]  lhs  left operand of the binary product
 * @param[in]  rhs  right operand of the binary product
 * @param[in]  pt   product type
 */
TermProduct::TermProduct(Aut_ptr aut, Term_ptr lhs, Term_ptr rhs, ProductType pt) : Term(aut), left(lhs), right(rhs) {
#   if (MEASURE_STATE_SPACE == true)
    ++TermProduct::instances;
#   endif

    this->type = TermType::PRODUCT;
    SET_PRODUCT_SUBTYPE(this, pt);

    // Initialization of state space
    this->stateSpaceApprox = this->left->stateSpaceApprox + (this->right != nullptr ? this->right->stateSpaceApprox : 0) + 1;

    // Initialization of the enumerator
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
#   if (OPT_ENUMERATED_SUBSUMPTION_TESTING == true)
    if(this->enumerator != nullptr) {
        delete this->enumerator;
    }
#   endif
}

/**
 * @brief Constructor of the ternary product of terms @p lhs @p mhs @rhs of @pt type
 *
 * Creates a ternary product of @p pt type of the triple @p lhs @mhs and @rhs
 *
 * @param[in]  aut  link to the automaton that created the product
 * @param[in]  lhs  left operand of the ternary product
 * @param[in]  mhs  middle operand of the ternary product
 * @param[in]  rhs  right operand of the ternary product
 * @param[in]  pt   product type
 */
TermTernaryProduct::TermTernaryProduct(Aut_ptr aut, Term_ptr lhs, Term_ptr mhs, Term_ptr rhs, ProductType pt)
    : Term(aut), left(lhs), middle(mhs), right(rhs) {
#   if(MEASURE_STATE_SPACE == true)
    ++TermTernaryProduct::instances;
#   endif

    this->type = TermType::TERNARY_PRODUCT;
    SET_PRODUCT_SUBTYPE(this, pt);

    // Initialization of state space
    this->stateSpaceApprox = this->left->stateSpaceApprox + this->right->stateSpaceApprox + this->middle->stateSpaceApprox + 1;
#   if (OPT_ENUMERATED_SUBSUMPTION_TESTING == true)
    this->enumerator = new TernaryProductEnumerator(this);
#   endif

#   if (DEBUG_TERM_CREATION == true)
    std::cout << "TermTernaryProduct::";
    this->dump();
    std::cout << "\n";
#   endif
}

/**
 * @brief destructs the enumerator of the ternary product
 */
TermTernaryProduct::~TermTernaryProduct() {
#   if (OPT_ENUMERATED_SUBSUMPTION_TESTING == true)
    if(this->enumerator != nullptr)
        delete this->enumerator;
#   endif
}

/**
 * @brief Initialization function for the Nary product
 *
 * Initializes the type, arity, access vector and state space approximation for the Nary product
 *
 * @param[in]  pt  product type
 * @param[in]  arity  arity of the product
 */
void TermNaryProduct::_InitNaryProduct(ProductType pt, size_t arity) {
#   if (MEASURE_STATE_SPACE == true)
    ++TermNaryProduct::instances;
#   endif

    this->arity = arity;
    this->access_vector = new size_t[this->arity];
    this->type = TermType::NARY_PRODUCT;
    SET_PRODUCT_SUBTYPE(this, pt);

    // Initialization of the access vector and the state space approx
    this->stateSpaceApprox = 0;
    for (size_t j = 0; j < this->arity; ++j) {
        // Fixme: Is this used anywhere?
        this->access_vector[j] = j;
        this->stateSpaceApprox += this->terms[j]->stateSpaceApprox;
    }

#   if (DEBUG_TERM_CREATION == true)
    std::cout << "TermNaryProduct::";
    this->dump();
    std::cout << "\n";
#   endif

#   if (OPT_ENUMERATED_SUBSUMPTION_TESTING == true)
    this->enumerator = new NaryProductEnumerator(this);
    assert(this->enumerator->type == EnumeratorType::NARY);
#   endif
}

/**
 * @brief Constructor of the Nary product of @p pt type
 *
 * Creates a Nary product of the @p pt type of @p terms
 *
 * @param[in]  aut  link to the automaton that created the nary product
 * @param[in]  terms  terms of the product (note: on index [0] is the quantifier free part of the product)
 * @param[in]  pt  product type
 * @param[in]  arity  arity of the product
 */
TermNaryProduct::TermNaryProduct(Aut_ptr aut, Term_ptr* terms, ProductType pt, size_t arity) : Term(aut) {
#   if (MEASURE_STATE_SPACE == true)
    ++TermNaryProduct::instances;
#   endif

    this->terms = new Term_ptr[arity];
    std::copy(terms, terms+arity, this->terms);
    this->_InitNaryProduct(pt, arity);
}

/**
 * @brief Constructor of the Nary product of @p pt type from the list of automata
 *
 * Creates a Nary product of the @p pt type either from the initial states of the @p auts
 * or from the final states of the @p auts.
 *
 * @param[in]  aut  link to the automaton that created the nary product
 * @param[in]  auts  links to the automata of outlying terms
 * @param[in]  st  state set type (either final or initial)
 * @param[in]  pt  product type
 * @param[in]  arity  arity of the Nary product
 */
TermNaryProduct::TermNaryProduct(Aut_ptr aut, SymLink* auts, StatesSetType st, ProductType pt, size_t arity) : Term(aut) {
#   if (MEASURE_STATE_SPACE == true)
    ++TermNaryProduct::instances;
#   endif

    // Fixme: add link to auts
    this->terms = new Term_ptr[arity];
    for(auto i = 0; i < arity; ++i) {
        if(st == StatesSetType::INITIAL)
            this->terms[i] = auts[i].aut->GetInitialStates();
        else
            this->terms[i] = auts[i].aut->GetFinalStates();
    }
    this->_InitNaryProduct(pt, arity);
}

/**
 * @brief Constructor of the Nary product represented as fixpoint style of type @p pt
 *
 * Creates a Nary product that has backlink to its @p source and behaves like bounded fixpoint, i.e.
 * its members are computed by demand instead of fully everytime
 *
 * @param[in]  aut  link to the automaton that created the nary product
 * @param[in]  source  source term of the product
 * @param[in]  symbol  symbol we are subtracting from the source
 * @param[in]  pt  product type
 * @param[in]  arity  arity of the Nary product
 */
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

#   if (OPT_ENUMERATED_SUBSUMPTION_TESTING == true)
    if(this->enumerator != nullptr)
        delete this->enumerator;
#   endif
}

/**
 * @brief Constructor of the Base set of atomic states
 *
 * Creates a Base set as a set of atomic states
 *
 * @param[in]  aut  link to the automaton that created the base set
 * @param[in]  s  set of states that we are wrapping
 */
TermBaseSet::TermBaseSet(Aut_ptr aut, BaseAutomatonStateSet && s) : Term(aut), states(std::move(s)) {
#   if (MEASURE_STATE_SPACE == true)
    ++TermBaseSet::instances;
#   endif
#   if (MEASURE_BASE_SIZE == true)
    size_t size = s.size();
    if(size > TermBaseSet::maxBaseSize) {
        TermBaseSet::maxBaseSize = size;
    }
#   endif
    type = TermType::BASE;
    assert(s.size() > 0 && "Trying to create 'TermBaseSet' without any states");

    // Initialization of state space
    this->stateSpaceApprox = this->states.size();

#   if (DEBUG_TERM_CREATION == true)
    std::cout << "TermBaseSet::";
    this->dump();
    std::cout << "\n";
#   endif
}

TermBaseSet::~TermBaseSet() {
    this->states.clear();
}

/**
 * @brief Constructor of the Continuation, i.e. the postponing of the computations
 *
 * Creates a Continuation, i.e. the postponing of the computations---the subtracting
 * of the symbol @p s from the term @p t
 *
 * @param[in]  aut  link to the automaton that created the base set
 * @param[in]  a  link to the automaton
 * @param[in]  init  automaton used for lazy initialization of the continuation
 * @param[in]  t  term we are postponing the computation on
 * @param[in]  s  symbol we are subtracting from the term @p t
 * @param[in]  b  whether the term is under complement
 * @param[in]  lazy  whether this should be lazy evaluated
 */
TermContinuation::TermContinuation(Aut_ptr aut, SymLink* a, SymbolicAutomaton* init, Term* t, SymbolType* s, bool b, bool lazy)
        : Term(aut), aut(a), initAut(init), term(t), symbol(s), underComplement(b), lazyEval(lazy) {
#   if (DEBUG_TERM_CREATION == true)
    std::cout << "[" << this << "]";
    std::cout << "TermContinuation::";
    this->dump();
    std::cout << "\n";
#   endif
#   if (MEASURE_STATE_SPACE == true)
    ++TermContinuation::instances;
#   endif
    assert(t != nullptr || lazyEval);
    this->type = TermType::CONTINUATION;

    // Initialization of state space
    this->stateSpaceApprox = (t == nullptr ? 0 : t->stateSpaceApprox);

#   if (DEBUG_CONTINUATIONS == true)
    std::cout << "Postponing computation as [";
    t->dump();
    std::cout << "]\n";
#   endif
    SET_VALUE_NON_MEMBERSHIP_TESTING(this, b);
}

/**
 * @brief Constructor of the List of terms
 *
 * Creates a List of terms, note: this is used only for the initialization
 * of the initial and final states of the fixpoints.
 *
 * @param[in]  aut  link to the automaton that created the list
 * @param[in]  first  the term we are pushing to the list
 * @param[in]  isCompl  if the term is complemented
 */
TermList::TermList(Aut_ptr aut, Term_ptr first, bool isCompl) : Term(aut), list{first} {
#   if (MEASURE_STATE_SPACE == true)
    ++TermList::instances;
#   endif

    this->type = TermType::LIST;

    // Initialization of state space
    this->stateSpaceApprox = first->stateSpaceApprox;
    SET_VALUE_NON_MEMBERSHIP_TESTING(this, isCompl);

#   if (DEBUG_TERM_CREATION == true)
    std::cout << "[" << this << "]";
    std::cout << "TermList::";
    this->dump();
    std::cout << "\n";
#   endif
}

/**
 * @brief Constructor of the Fixpoint computation
 *
 * Creates a computation for the fixpoint of terms, the base case, i.e. the saturation
 * part of the projection.
 *
 * @param[in]  aut  link to the automaton that created the fixpoint
 * @param[in]  startingTerm  the term corresponding to the testing of the epsilon in the automaton of fixpoint
 * @param[in]  symbol  symbol that initializes the fixpoint (usually the zero symbol)
 * @param[in]  inComplement  whether we are computing the complement
 * @param[in]  initbValue  initial boolean value
 * @param[in]  search  type of the fixpoint search (either DFS or BFS or some other)
 */
TermFixpoint::TermFixpoint(Aut_ptr aut, Term_ptr startingTerm, Symbol* symbol, bool inComplement, bool initbValue, WorklistSearchType search = WorklistSearchType::DFS)
        : Term(aut),
          _fixpoint{FixpointMember(nullptr, true, 0), FixpointMember(startingTerm, true, 0)},
          _sourceTerm(nullptr),
          _sourceSymbol(symbol),
          _sourceIt(nullptr),
          _baseAut(static_cast<ProjectionAutomaton*>(aut)->GetBase()),
          _guide(static_cast<ProjectionAutomaton*>(aut)->GetGuide()),
          _validCount(1),
          _searchType(search),
          _bValue(initbValue) {
#   if (MEASURE_STATE_SPACE == true)
    ++TermFixpoint::instances;
#   endif

#   if (ALT_SKIP_EMPTY_UNIVERSE == false)
    // Initialize the (counter)examples
    if(initbValue) {
        this->_satTerm = startingTerm;
    } else {
        this->_unsatTerm = startingTerm;
    }
#   endif

    auto it = this->_fixpoint.begin();
    ++it;
    if(initbValue == inComplement || this->_guide == nullptr || this->_guide->CanEarlyTerminate()) {
        it->isValid = true;
    } else {
        it->isValid = false;
        --this->_validCount;
        this->_bValue = inComplement;
    }

    // Initialize the aggregate function
    this->_InitializeAggregateFunction(inComplement);
    SET_VALUE_NON_MEMBERSHIP_TESTING(this, inComplement);
    this->type = TermType::FIXPOINT;

    // Initialize the state space
    this->stateSpaceApprox = startingTerm->stateSpaceApprox;

    // Push symbols to worklist
    if (static_cast<ProjectionAutomaton*>(aut)->IsRoot() || allPosVar == -1) {
        // Fixme: Consider extracting this to different function
        if(OPT_INCREMENTAL_LEVEL_PRE) {
            this->_InitializeProjectedSymbol(&aut->symbolFactory, aut->GetNonOccuringVars(), static_cast<ProjectionAutomaton *>(aut)->projectedVars, symbol);
        } else {
            this->_InitializeSymbols(&aut->symbolFactory, aut->GetNonOccuringVars(), static_cast<ProjectionAutomaton *>(aut)->projectedVars, symbol);
        }

        IntersectNonEmptyParams params(inComplement);
#       if (OPT_INCREMENTAL_LEVEL_PRE == true)
        params.limitPre = true;
        params.variableLevel = 0;
#       endif
        this->_EnqueueInWorklist(startingTerm, params);
        assert(this->_worklist.size() > 0 || startingTerm->type == TermType::EMPTY);
    }

#   if (DEBUG_TERM_CREATION == true)
    std::cout << "[" << this << "]";
    std::cout << "TermFixpoint::";
    this->dump();
    std::cout << "\n";
#   endif
}

/**
 * @brief Constructor of the Pre Fixpoint computation
 *
 * Creates a computation for the fixpoint of terms, the pre case, i.e. the subtraction
 * of symbols from the already (partly) computed fixpoints.
 *
 * @param[in]  aut  link to the automaton that created the fixpoint
 * @param[in]  sourceTerm  source fixpoint we are subtracting @p symbol from
 * @param[in]  symbol  source symbol we are subtracting (the projected set)
 * @param[in]  inComplement  whether we are computing in complement
 */
TermFixpoint::TermFixpoint(Aut_ptr aut, Term_ptr sourceTerm, Symbol* symbol, bool inComplement)
        : Term(aut),
          _fixpoint{FixpointMember(nullptr, true, 0)},
          _sourceTerm(sourceTerm),
          _sourceSymbol(symbol),
          _sourceIt(static_cast<TermFixpoint*>(sourceTerm)->GetIteratorDynamic()),
          _guide(static_cast<ProjectionAutomaton*>(aut)->GetGuide()),
          _baseAut(static_cast<ProjectionAutomaton*>(aut)->GetBase()),
          _worklist(),
          _validCount(0),
          _searchType(WorklistSearchType::DFS),
          _bValue(inComplement) {
#   if (MEASURE_STATE_SPACE == true)
    ++TermFixpoint::instances;
    ++TermFixpoint::preInstances;
#   endif
    assert(sourceTerm->type == TermType::FIXPOINT && "Computing Pre fixpoint of something different than fixpoint");

    // Initialize the state space
    this->stateSpaceApprox = sourceTerm->stateSpaceApprox;

    // Initialize the aggregate function
    this->_InitializeAggregateFunction(inComplement);
    this->type = TermType::FIXPOINT;
    SET_VALUE_NON_MEMBERSHIP_TESTING(this, inComplement);

    // Push things into worklist
    if(OPT_INCREMENTAL_LEVEL_PRE == true) {
        this->_InitializeProjectedSymbol(&aut->symbolFactory, aut->GetNonOccuringVars(), static_cast<ProjectionAutomaton *>(aut)->projectedVars, symbol);
    } else {
        this->_InitializeSymbols(&aut->symbolFactory, aut->GetNonOccuringVars(), static_cast<ProjectionAutomaton *>(aut)->projectedVars, symbol);
    }

#   if (DEBUG_TERM_CREATION == true)
    std::cout << "[" << this << "]";
    std::cout << "TermFixpointPre[" << sourceTerm << "]::";
    this->dump();
    std::cout << "\n";
#   endif
}

TermFixpoint::~TermFixpoint() {
    this->_fixpoint.clear();
#   if (OPT_EARLY_EVALUATION == true)
    this->_postponed.clear();
#   endif
    this->_worklist.clear();
}

void Term::Complement() {
    assert(!this->IsIntermediate());
    FLIP_IN_COMPLEMENT(this);
}

/**
 * //Fixme: Rename this shit
 * Sets the same link as for the @p term. If there is already some link formed, we let it be.
 *
 * @param[in]  term  term we are aliasing link with
 */
void Term::SetSameSuccesorAs(Term* term) {
    if(this->link->succ == nullptr) {
        this->link->succ = term->link->succ;
        this->link->symbol = term->link->symbol;
        this->link->val = term->link->val;
        this->link->var = term->link->var;
        this->link->len = term->link->len;
    }
    if(term->IsIntermediate()) {
        this->SetIsIntermediate();
    } else {
        this->ResetIsIntermediate();
    }
}

/**
 * Sets successor of the term to the pair of @p succ and @p symb and increments the lenght of the successor path.
 * If the successor is already set, we do nothing
 *
 * @param[in]  succ  successor of the term
 * @param[in]  symb  symbol we were subtracting from the @p succ
 */
void Term::SetSuccessor(Term* succ, Symbol* symb, IntersectNonEmptyParams& params) {
    if(this->link->succ == nullptr) {
        this->link->succ = succ;
        this->link->symbol = symb;
        if(params.limitPre) {
            this->link->len = succ->link->len + 1*(params.variableLevel == 0);
            this->link->var = params.variableLevel;
            this->link->val += params.variableValue;
        } else {
            this->link->len = succ->link->len;
            assert(this->link->val == "");
        }
    } else if(succ == this && params.limitPre) {
        // Looping
        assert(this->link->val != "");
        int nextVar = this->link->var - this->link->val.size();
        if(nextVar >= 0 && nextVar == params.variableLevel) {
            this->link->val += params.variableValue;
        }
    }
}

/**
 * Returns true if the term is not computed, i.e. it is continuation somehow
 */
bool Term::IsNotComputed() {
#if (OPT_EARLY_EVALUATION == true && MONA_FAIR_MODE == false)
    if(this->type == TermType::CONTINUATION) {
        return !static_cast<TermContinuation *>(this)->IsUnfolded();
    } else if(this->type == TermType::PRODUCT) {
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
 * @brief Main function for the subsumption testing of two terms
 *
 * Tests whether this term is subsumed by the term @p t. This subsumption can be partly
 * limited with the @p limit parameter, that can be set by the OPT_PARTIALLY_LIMITED_SUBSUMPTION
 * define. Moreover, if there are continations, we can limit their unfoldings by the @p unfoldAll
 * parameter. From newer versions partial subsumptions are supported and can return the different
 * term through the @p new_term.
 *
 * @param[in]  t  term we are testing subsumption from
 * @param[in]  new_term  returned different term, e.g. for partial subsumption with set difference
 *   logic (for base sets)
 * @param[in]  params  parameters of subsumption testing (see Term.h)
 * @return:  whether this term is subsumed by @p t and how (full, not, partial, etc.)
 */
SubsumedType Term::IsSubsumed(Term *t, Term** new_term, SubsumptionTestParams params) {
    // Intermediate stuff is not subsumed
    if(this == t) {
        return SubsumedType::YES;
    }
    assert(t->type == TermType::EMPTY || this->type == TermType::EMPTY || t->IsIntermediate() == this->IsIntermediate());

#   if (OPT_PARTIALLY_LIMITED_SUBSUMPTION >= 0)
    if(!params.limit) {
        return (this == t ? SubsumedType::YES : SubsumedType::NOT);
    }
#   endif

    // unfold the continuation
#   if (OPT_EARLY_EVALUATION == true)
    if(t->type == TermType::CONTINUATION) {
        TermContinuation *continuation = static_cast<TermContinuation *>(t);
        Term* unfoldedContinuation = continuation->unfoldContinuation(UnfoldedIn::SUBSUMPTION);
        return this->IsSubsumed(unfoldedContinuation, params.limit, params.unfoldAll);
    } else if(this->type == TermType::CONTINUATION) {
        TermContinuation *continuation = static_cast<TermContinuation *>(this);
        Term* unfoldedContinuation = continuation->unfoldContinuation(UnfoldedIn::SUBSUMPTION);
        return unfoldedContinuation->IsSubsumed(t, params.limit, params.unfoldAll);
    }
#   endif

    assert(GET_IN_COMPLEMENT(this) == GET_IN_COMPLEMENT(t));
    assert(this->type != TermType::CONTINUATION && t->type != TermType::CONTINUATION);

#   if (OPT_PARTIALLY_LIMITED_SUBSUMPTION > 0)
    --params.limit;
#   endif

    // Else if it is not continuation we first look into cache and then recompute if needed
    std::pair<SubsumedType, Term_ptr> result;
#   if (OPT_CACHE_SUBSUMES == true)
    auto key = std::make_pair(static_cast<Term_ptr>(this), t);
    //if(this->type == TermType::EMPTY || !this->_aut->_subCache.retrieveFromCache(key, result)) {
    if(this->type == TermType::EMPTY || !this->_aut->_subCache.LookUp(key, result)) {
#   endif
        if (GET_IN_COMPLEMENT(this)) {
            if(this->type == TermType::EMPTY) {
                result.first = (t->type == TermType::EMPTY ? SubsumedType::YES : SubsumedType::NOT);
            } else {
                result.first = t->_IsSubsumedCore(this, new_term, params);
            }
        } else {
            if(t->type == TermType::EMPTY) {
                result.first = (this->type == TermType::EMPTY ? SubsumedType::YES : SubsumedType::NOT);
            } else {
                result.first = this->_IsSubsumedCore(t, new_term, params);
            }
        }
#   if (OPT_CACHE_SUBSUMES == true)
        if((result.first == SubsumedType::YES || result.first == SubsumedType::PARTIALLY) && this->type != TermType::EMPTY) {
            if(result.first == SubsumedType::PARTIALLY) {
                assert(*new_term != nullptr);
                result.second = *new_term;
            }
            this->_aut->_subCache.StoreIn(key, result);
        }
    }

    if(result.first == SubsumedType::PARTIALLY) {
        assert(result.second != nullptr);
        if(new_term != nullptr) {
            *new_term = result.second;
        } else {
            // We did not chose the partial stuff
            return SubsumedType::NOT;
        }
    }
#   endif
    assert(!params.unfoldAll || result.first != SubsumedType::PARTIALLY);
#   if (DEBUG_TERM_SUBSUMPTION == true)
    this->dump();
    std::cout << (result.first == SubsumedType::YES ? " \u2291 " : " \u22E2 ");
    t->dump();
    std::cout << " = " << (result.first == SubsumedType::YES ? "true" : "false") << "\n\n";
#   endif
    assert(!(result.first == SubsumedType::PARTIALLY && (new_term != nullptr && *new_term == nullptr)));
    return result.first;
}

SubsumedType TermEmpty::_IsSubsumedCore(Term* t, Term** new_term, SubsumptionTestParams params) {
    // Empty term is subsumed by everything (probably)
    return (GET_IN_COMPLEMENT(this)) ? (t->type == TermType::EMPTY ? SubsumedType::YES : SubsumedType::NOT) : SubsumedType::YES;
}

SubsumedType TermProduct::_IsSubsumedCore(Term* t, Term** new_term, SubsumptionTestParams params) {
    assert(t->type == TermType::PRODUCT);

    // Retype and test the subsumption component-wise
    TermProduct *rhs = static_cast<TermProduct*>(t);
    Term *lhsl = this->left;
    Term *lhsr = this->right;
    Term *rhsl = rhs->left;
    Term *rhsr = rhs->right;

    // We are doing the partial testing
#   if (OPT_SUBSUMPTION_INTERSECTION == true)
    if(lhsl->type == TermType::BASE && new_term != nullptr) {
        Term* inner_new_term = nullptr;
        SubsumedType result;

        // First test the left operands
        if((result = lhsl->IsSubsumed(rhsl, &inner_new_term, params)) == SubsumedType::NOT) {
            // Left is false, everything is false
            return SubsumedType::NOT;
        } else if(result == SubsumedType::YES) {
            // Operand is fully subsumed, no partialization
            assert(inner_new_term == nullptr);
            inner_new_term = lhsl;
        } else {
            assert(inner_new_term != nullptr);
            assert(result == SubsumedType::PARTIALLY);
        }

        // Test the right operand
        SubsumedType base_result = lhsr->IsSubsumed(rhsr, new_term, params);
        if(base_result == SubsumedType::PARTIALLY) {
            // Partially subsumed, return new partial product and set the successors as the same, so we keep the link
            assert(new_term != nullptr);
            assert(result != SubsumedType::NOT);
            *new_term = this->_aut->_factory.CreateProduct(inner_new_term, *new_term, IntToProductType(GET_PRODUCT_SUBTYPE(this)));
            (*new_term)->SetSameSuccesorAs(this);
            ++Term::partial_subsumption_hits;
            return SubsumedType::PARTIALLY;
        } else {
            if (result == SubsumedType::PARTIALLY && base_result != SubsumedType::NOT) {
                // Right operand is fully subsumed, left one is partially subsumed, create new partial product
                assert(base_result == SubsumedType::YES);
                *new_term = this->_aut->_factory.CreateProduct(inner_new_term, lhsr, IntToProductType(GET_PRODUCT_SUBTYPE(this)));
                (*new_term)->SetSameSuccesorAs(this);
                ++Term::partial_subsumption_hits;
                return SubsumedType::PARTIALLY;
            } else {
                // Either everything is fully subsumed, or right operand is not subsumed, return
                assert(base_result == SubsumedType::NOT || base_result == SubsumedType::YES);
                assert(result == SubsumedType::YES || (result == SubsumedType::PARTIALLY && base_result == SubsumedType::NOT));
                *new_term = nullptr;
                return base_result;
            }
        }
    }
#   endif

    if(OPT_EARLY_EVALUATION && !params.unfoldAll && (lhsr->IsNotComputed() && rhsr->IsNotComputed())) {
#       if (OPT_EARLY_PARTIAL_SUB == true)
        if(lhsr->type == TermType::CONTINUATION && rhsr->type == TermType::CONTINUATION) {
            return (lhsl->IsSubsumed(rhsl, nullptr, params) == SubsumedType::NOT ? SubsumedType::NOT : SubsumedType::PARTIALLY);
        } else {
            SubsumedType leftIsSubsumed = lhsl->IsSubsumed(rhsl, nullptr, params);
            if(leftIsSubsumed == SubsumedType::YES) {
                return lhsr->IsSubsumed(rhsr, nullptr, params);
            } else {
                return leftIsSubsumed;
            }
        }
#       else
        return (lhsl->IsSubsumed(rhsl, SubsumptionTestParams(params.limit, params.level)) != SubsumedType::NOT && lhsr->IsSubsumed(rhsr, SubsumptionTestParams(params.limit, params.level)) != SubsumedType::NOT) ? SubsumedType::YES : SubsumedType::NOT;
#       endif
    } if(!params.unfoldAll && lhsl == rhsl) {
        return lhsr->IsSubsumed(rhsr, nullptr, params);
    } else if(!params.unfoldAll && lhsr == rhsr) {
        return lhsl->IsSubsumed(rhsl, nullptr, params);
    } else {
        if(lhsl->stateSpaceApprox < lhsr->stateSpaceApprox || params.unfoldAll) {
            return (lhsl->IsSubsumed(rhsl, nullptr, params) != SubsumedType::NOT && lhsr->IsSubsumed(rhsr, nullptr, params) != SubsumedType::NOT) ? SubsumedType::YES : SubsumedType::NOT;
        } else {
            return (lhsr->IsSubsumed(rhsr, nullptr, params) != SubsumedType::NOT && lhsl->IsSubsumed(rhsl, nullptr, params) != SubsumedType::NOT) ? SubsumedType::YES : SubsumedType::NOT;
        }
    }
}

SubsumedType _ternary_subsumption_test(Term* f1, Term* f2, Term* s1, Term* s2, Term* l1, Term* l2, size_t approx_max, SubsumptionTestParams params) {
    if(f1->IsSubsumed(f2, nullptr, params) != SubsumedType::NOT) {
        if(s1->stateSpaceApprox == approx_max) {
            return (l1->IsSubsumed(l2, nullptr, params) != SubsumedType::NOT && s1->IsSubsumed(s2, nullptr, params) != SubsumedType::NOT) ? SubsumedType::YES : SubsumedType::NOT;
        } else {
            return (s1->IsSubsumed(s2, nullptr, params) != SubsumedType::NOT && l1->IsSubsumed(l2, nullptr, params) != SubsumedType::NOT) ? SubsumedType::YES : SubsumedType::NOT;
        }
    } else {
        return SubsumedType::NOT;
    }
}

SubsumedType TermTernaryProduct::_IsSubsumedCore(Term* t, Term** new_term, SubsumptionTestParams params) {
    assert(t->type == TermType::TERNARY_PRODUCT);

    // Retype and test the subsumption component-wise
    TermTernaryProduct *rhs = static_cast<TermTernaryProduct*>(t);
    Term *lhsl = this->left;
    Term *lhsm = this->middle;
    Term *lhsr = this->right;
    Term *rhsl = rhs->left;
    Term *rhsm = rhs->middle;
    Term *rhsr = rhs->right;

#   if (OPT_SUBSUMPTION_INTERSECTION == true)
    if(lhsl->type == TermType::BASE && new_term != nullptr) {
        // First test the left operands for subsumption
        Term* left_new_term = nullptr;
        SubsumedType left_result;
        if((left_result = lhsl->IsSubsumed(rhsl, &left_new_term, params)) == SubsumedType::NOT) {
            // Left operand is not subsumed, everything is not subsumed
            return SubsumedType::NOT;
        } else if(left_result == SubsumedType::YES) {
            assert(left_new_term == nullptr);
            left_new_term = lhsl;
        } else {
            assert(left_new_term != nullptr);
            assert(left_result == SubsumedType::PARTIALLY);
        }

        // Test the middle operands for subsumption
        Term* middle_new_term = nullptr;
        SubsumedType middle_result = SubsumedType::YES;
        if((middle_result = lhsm->IsSubsumed(rhsm, &middle_new_term, params)) == SubsumedType::NOT) {
            // Middle operand is not subsumed, everything is not subsumed
            return SubsumedType::NOT;
        } else if(middle_result == SubsumedType::YES) {
            assert(middle_new_term == nullptr);
            middle_new_term = lhsm;
        } else {
            assert(middle_new_term != nullptr);
            assert(middle_result == SubsumedType::PARTIALLY);
        }

        // Test the rightmost operands for subsumption
        Term* right_new_term = nullptr;
        SubsumedType right_result;
        if((right_result = lhsr->IsSubsumed(rhsr, &right_new_term, params)) == SubsumedType::NOT) {
            return SubsumedType::NOT;
        } else if(right_result == SubsumedType::YES) {
            assert(right_new_term == nullptr);
            if(left_result == SubsumedType::YES && middle_result == SubsumedType::YES) {
                *new_term = nullptr;
                return SubsumedType::YES;
            } else {
                // Create new partial product with successors set the same as the original source
                assert(left_result == SubsumedType::PARTIALLY || middle_result == SubsumedType::PARTIALLY);
                ++Term::partial_subsumption_hits;
                *new_term = this->_aut->_factory.CreateTernaryProduct(left_new_term, middle_new_term, lhsr, IntToProductType(GET_PRODUCT_SUBTYPE(this)));
                (*new_term)->SetSameSuccesorAs(this);
                return SubsumedType::PARTIALLY;
            }
        } else {
            // Create new partial product with successors set the same as the original source
            ++Term::partial_subsumption_hits;
            *new_term = this->_aut->_factory.CreateTernaryProduct(left_new_term, middle_new_term, right_new_term, IntToProductType(GET_PRODUCT_SUBTYPE(this)));
            (*new_term)->SetSameSuccesorAs(this);
            return SubsumedType::PARTIALLY;
        }
    }
#   endif

    // Test the subsumption according to the order induced by state space approximation
    size_t approx_max = std::max({lhsl->stateSpaceApprox, lhsm->stateSpaceApprox, lhsr->stateSpaceApprox});
    size_t approx_min = std::min({lhsl->stateSpaceApprox, lhsm->stateSpaceApprox, lhsr->stateSpaceApprox});
    if(lhsl->stateSpaceApprox == approx_min) {
        return _ternary_subsumption_test(lhsl, rhsl, lhsm, rhsm, lhsr, rhsr, approx_max, params);
    } else if(lhsr->stateSpaceApprox == approx_min) {
        return _ternary_subsumption_test(lhsr, rhsr, lhsl, rhsl, lhsm, rhsm, approx_max, params);
    } else {
        return _ternary_subsumption_test(lhsm, rhsm, lhsr, rhsr, lhsl, rhsl, approx_max, params);
    }
}

SubsumedType TermNaryProduct::_IsSubsumedCore(Term* t, Term** new_term, SubsumptionTestParams params) {
    assert(t->type == TermType::NARY_PRODUCT);
    TermNaryProduct *rhs = static_cast<TermNaryProduct*>(t);
    assert(this->arity == rhs->arity);

#   if (OPT_SUBSUMPTION_INTERSECTION == true)
    if(this->terms[0]->type == TermType::BASE && new_term != nullptr) {
        Term_ptr inner_term = nullptr;
        Term_ptr* new_terms = new Term_ptr[this->arity];
        bool terms_were_partitioned = false;
        SubsumedType result;

        for (int i = 0; i < this->arity; ++i) {
            if(this->terms[i] == rhs->terms[i]) {
                result = SubsumedType::YES;
            } else {
                result = this->terms[i]->IsSubsumed(rhs->terms[i], &inner_term, params);
            }
            if(result == SubsumedType::NOT) {
                delete[] new_terms;
                return SubsumedType::NOT;
            } else if(result == SubsumedType::YES) {
                new_terms[i] = this->terms[i];
            } else {
                assert(result == SubsumedType::PARTIALLY);
                new_terms[i] = inner_term;
                terms_were_partitioned = true;
            }
        }

        if(terms_were_partitioned) {
            ++Term::partial_subsumption_hits;
            *new_term = this->_aut->_factory.CreateNaryProduct(new_terms, this->arity, IntToProductType(GET_PRODUCT_SUBTYPE(this)));
            (*new_term)->SetSameSuccesorAs(this);
            delete[] new_terms;
            return SubsumedType::PARTIALLY;
        } else {
            delete[] new_terms;
            *new_term = nullptr;
            return SubsumedType::YES;
        }
    }
#   endif

    // Retype and test the subsumption component-wise
    for (int i = 0; i < this->arity; ++i) {
        assert(this->access_vector[i] < this->arity);
        if(this->terms[this->access_vector[i]]->IsSubsumed(rhs->terms[this->access_vector[i]], nullptr, params) == SubsumedType::NOT) {
            if(i != 0) {
                // Propagate the values towards 0 index
                this->access_vector[i] ^= this->access_vector[0];
                this->access_vector[0] ^= this->access_vector[i];
                this->access_vector[i] ^= this->access_vector[0];
            }
            return SubsumedType::NOT;
        }
    }

    return SubsumedType::YES;
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

SubsumedType TermBaseSet::_IsSubsumedCore(Term *term, Term** new_term, SubsumptionTestParams params) {
    assert(term->type == TermType::BASE);
    TermBaseSet *t = static_cast<TermBaseSet*>(term);
    assert(term->IsIntermediate() == this->IsIntermediate());

#   if (OPT_SUBSUMPTION_INTERSECTION == true)
    if(new_term != nullptr) {
        TermBaseSetStates diff;
        bool is_nonempty_diff = this->states.SetDifference(t->states, diff);
        *new_term = nullptr;
        if(is_nonempty_diff) {
            if(diff.size() == this->states.size()) {
                return SubsumedType::NOT;
            } else {
                *new_term = this->_aut->_factory.CreateBaseSet(std::move(diff));
                (*new_term)->SetSameSuccesorAs(this);
                return SubsumedType::PARTIALLY;
            }
        } else {
            return SubsumedType::YES;
        }
    }
#   endif
    // Test component-wise, not very efficient though
    // TODO: Change to bit-vectors if possible
    if(t->stateSpaceApprox < this->stateSpaceApprox) {
        return SubsumedType::NOT;
    } else {
        if(false && OPT_INCREMENTAL_LEVEL_PRE && this->IsIntermediate()) {
            auto lit = this->states.begin();
            auto lend = this->states.end();
            auto rit = t->states.begin();
            auto rend = t->states.end();

            while (lit != lend && rit != rend) {
                size_t lval = *lit;
                size_t rval = *rit;
                if (lval == rval) {
                    ++lit;
                    ++rit;
                    continue;
                } else if (this->IsIntermediate() && (lval >> 2) == (rval >> 2)) {
                    size_t lflags = (lval) & 3;
                    size_t rflags = (rval) & 3;

                    if (lflags == rflags || rflags == 3 || lflags == 0) {
                        ++lit;
                        ++rit;
                        continue;
                    }
                }

                if (lval < rval) {
                    return SubsumedType::NOT;
                } else {
                    ++rit;
                };
            }

            return (lit == lend) ? SubsumedType::YES : SubsumedType::NOT;
        } else {
            return this->states.IsSubsetOf(t->states) ? SubsumedType::YES : SubsumedType::NOT;
        }
    }
}

SubsumedType TermContinuation::_IsSubsumedCore(Term* t, Term** new_term, SubsumptionTestParams params) {
    assert(false);
    return SubsumedType::NOT;
}

SubsumedType TermList::_IsSubsumedCore(Term* t, Term** new_term, SubsumptionTestParams params) {
    assert(t->type == TermType::LIST);

    // Reinterpret
    TermList* tt = static_cast<TermList*>(t);
    // Do the item-wise subsumption check
    for(auto& item : this->list) {
        bool subsumes = false;
        for(auto& tt_item : tt->list) {
            if(item->IsSubsumed(tt_item, nullptr, params) == SubsumedType::YES) {
                subsumes = true;
                break;
            }
        }
        if(!subsumes) return SubsumedType::NOT;
    }

    return SubsumedType::YES;
}

SubsumedType TermFixpoint::_IsSubsumedCore(Term* t, Term** new_term, SubsumptionTestParams params) {
    assert(t->type == TermType::FIXPOINT);

    // Reinterpret
    TermFixpoint* tt = static_cast<TermFixpoint*>(t);

#   if (OPT_MORE_CONSERVATIVE_SUB_TEST == true)
    bool are_source_symbols_same = TermFixpoint::_compareSymbols(*this, *tt);
    // Worklists surely differ
    if(!are_source_symbols_same && (this->_worklist.size() != 0) ) {
        return SubsumedType::NOT;
    }
#   endif

    Term* tptr = nullptr;

    SubsumedType result;
#   if (OPT_UNFOLD_FIX_DURING_SUB == true)
    Term_ptr fixpointMember;
    auto it = this->GetIterator();
    size_t term_level = (this->GetSemantics() == FixpointSemanticType::FIXPOINT ? 0 : this->_symbolPart.level);
    while( (fixpointMember = it.GetNext()) != nullptr) {
        if( (result = fixpointMember->IsSubsumedBy(tt, tptr, SubsumedByParams(true, term_level))) == SubsumedType::NOT) {
            --this->_iteratorNumber;
            return SubsumedType::NOT;
        }
        assert(result == SubsumedType::YES);
    }

    return SubsumedType::YES;
#   else
    // Do the piece-wise comparison
    for(FixpointMember& item : this->_fixpoint) {
        // Skip the nullpt
        if(item.term == nullptr || !item.isValid) continue;

        if( (result = (item.term)->IsSubsumedBy(tt, tptr, SubsumedByParams(true, item.level))) == SubsumedType::NOT) {
            return SubsumedType::NOT;
        }
        assert(result == SubsumedType::YES);
    }

#   if (OPT_MORE_CONSERVATIVE_SUB_TEST == true)
    return ( (this->_worklist.size() == 0 /*&& tt->_worklist.size() == 0*/) ? SubsumedType::YES : (are_source_symbols_same ? SubsumedType::YES : SubsumedType::NOT));
    // Happy reading ^^                   ^---- Fixme: maybe this is incorrect?
#   else
    return SubsumedType::YES;
#   endif
#   endif
}

/**
 * Tests the subsumption over the list of terms
 *
 * @param[in] fixpoint:     list of terms contained as fixpoint
 */
SubsumedType TermEmpty::IsSubsumedBy(TermFixpoint* fixpoint, Term*& biggerTerm, SubsumedByParams params) {
    // Empty term is subsumed by everything
    // Fixme: Complemented fixpoint should subsume everything right?
    return ( ( (fixpoint->_fixpoint.size() == 1 && fixpoint->_fixpoint.front().term == nullptr) || GET_IN_COMPLEMENT(this)) ? SubsumedType::NOT : SubsumedType::YES);
}

/*
 * @brief Removes queued pairs, that corresponds to the @p item
 *
 * Removes every computation that will subtract some symbol from the @p item from the worklist
 *
 * @param[in]  worklist  list of (term, symbol) pairs queued for computation
 * @param[in]  item  item we are pruning away from the worklist
 */
void prune_worklist(WorklistType& worklist, Term*& item) {
    for(auto it = worklist.begin(); it != worklist.end();) {
        if(it->term == item) {
            it = worklist.erase(it);
        } else {
            ++it;
        }
    }
}

void switch_in_worklist(WorklistType& worklist, Term*& item, Term*& new_item) {
    for(auto it = worklist.begin(); it != worklist.end(); ++it) {
        if(it->term == item) {
            it->term = new_item;
        }
    }
}

/**
 * @brief Tests whether the product is subsumed by @p fixpoint
 *
 * Tests whether the product (either binary, ternary or nary) is subsumed by the fixpoint.
 * Further this prunes the conversly subsumed pairs as well. Moreover this also takes in
 * account the partial subsumption, by returning the new terms that are created by
 * difference logic during the subsumption, i.e. terms are broken into smaller terms.
 *
 * @param[in,out]  fixpoint  list of terms representing the fixpoint
 * @param[in,out]  worklist  queued pairs of (term, symbol)
 * @param[out]  biggerTerm  additional output of the procedured, mainly for the partial subsumption with
 *   symetric differential semantics
 * @param[in]  no_prune  whether the subsumption should prune the conversely subsumed items in fixpoint
 * @return  the subsumption relation for the term and fixpoint
 */
template<class ProductType>
SubsumedType Term::_ProductIsSubsumedBy(TermFixpoint* fixpoint, Term*& biggerTerm, SubsumedByParams params) {
    assert(this->type == TermType::PRODUCT || this->type == TermType::NARY_PRODUCT || this->type == TermType::TERNARY_PRODUCT);
    bool no_prune = params.no_prune;

    if(this->IsEmpty()) {
        return SubsumedType::YES;
#   if (OPT_ENUMERATED_SUBSUMPTION_TESTING == true)
    } else {
    bool isEmpty = true;
    for (auto &item : fixpoint->_fixpoint) {
        if (item.first == nullptr || !item.second)
            continue;
        isEmpty = false;
        break;
    }
    if (isEmpty)
        return SubsumedType::NOT;
#   endif
    }

    // For each item in fixpoint
    Term* tested_term = this;
    Term* new_term = nullptr;

    for(FixpointMember& item : fixpoint->_fixpoint) {
        // Nullptr is skipped
        if(item.term == nullptr || !item.isValid || item.level != params.level) {
            continue;
        }

        if(item.level > params.level) {
            SubsumedType inner_result;
            if( (inner_result = item.term->IsSubsumed(this, nullptr, SubsumptionTestParams(OPT_PARTIALLY_LIMITED_SUBSUMPTION, params.level))) == SubsumedType::YES) {
#               if (OPT_PRUNE_WORKLIST == true)
                prune_worklist(fixpoint->_worklist, item.term);
#               endif
                item.isValid = false;
                assert(fixpoint->_validCount);
                --fixpoint->_validCount;
            }
            continue;
        }

        // Test the subsumption
        SubsumedType result;
        if(fixpoint->_validCount > 1 && OPT_SUBSUMPTION_INTERSECTION == true) {
            result = tested_term->IsSubsumed(item.term, &new_term, SubsumptionTestParams(OPT_PARTIALLY_LIMITED_SUBSUMPTION, params.level));
        } else {
            result = tested_term->IsSubsumed(item.term, nullptr, SubsumptionTestParams(OPT_PARTIALLY_LIMITED_SUBSUMPTION, params.level));
        }

        if(result == SubsumedType::YES) {
            return SubsumedType::YES;
        } else if(result == SubsumedType::PARTIALLY) {
            assert(new_term != tested_term);
            assert(new_term != nullptr);
            tested_term = new_term;
        }

        if(!no_prune && result != SubsumedType::PARTIALLY) {
            SubsumedType inner_result;
            new_term == nullptr;
#           if (OPT_PARTIAL_PRUNE_FIXPOINTS == true)
            if( (inner_result = item.term->IsSubsumed(this, &new_term, SubsumptionTestParams(OPT_PARTIALLY_LIMITED_SUBSUMPTION, params.level))) == SubsumedType::YES) {
#           else
            if( (inner_result = item.term->IsSubsumed(this, nullptr, SubsumptionTestParams(OPT_PARTIALLY_LIMITED_SUBSUMPTION, params.level))) == SubsumedType::YES) {
#           endif
                assert(!(fixpoint->_validCount == 1 && result == SubsumedType::PARTIALLY));
#               if (OPT_PRUNE_WORKLIST == true)
                prune_worklist(fixpoint->_worklist, item.term);
#               endif
                item.isValid = false;
                assert(fixpoint->_validCount);
                --fixpoint->_validCount;
#           if (OPT_PARTIAL_PRUNE_FIXPOINTS == true)
            } else if(inner_result == SubsumedType::PARTIALLY) {
                assert(new_term != nullptr);
                assert(new_term->type != TermType::EMPTY);
                assert(new_term != item.first);

                switch_in_worklist(fixpoint->_worklist, item.first, new_term);
                item.first = new_term;
#           endif
            }
        }
    }

    assert(!tested_term->IsEmpty());
#   if (DEBUG_TERM_SUBSUMED_BY == true)
    this->dump();
    std::cout << ( !(tested_term == this || no_prune) ? " [\u2286] " : " [\u2288] ");
    std::cout << "{";
    for(auto& item : fixpoint->_fixpoint) {
        if(item.first == nullptr || !item.second) continue;
        item.first->dump();
        std::cout << ",";
    }
    std::cout << "}";
    std::cout << "\n";
#   endif

#   if (OPT_ENUMERATED_SUBSUMPTION_TESTING == true)
    TermEnumerator* enumerator = static_cast<ProductType*>(tested_term)->enumerator;
    enumerator->FullReset();
    assert(!enumerator->IsNull());
    if(fixpoint->_validCount > 1) {
        bool is_subsumed_after_enum = true;
        while (enumerator->IsNull() == false) {
            bool subsumed = false;
            for (auto &item : fixpoint->_fixpoint) {
                if (item.first == nullptr || !item.second) continue;

                if (item.first->Subsumes(enumerator) != SubsumedType::NOT) {
                    enumerator->Next();
                    subsumed = true;
                    break;
                }
            }

            if (!subsumed) {
                is_subsumed_after_enum = false;
                break;
            }
        }
        if (is_subsumed_after_enum) {
            return SubsumedType::YES;
        }
    }
#   endif
    if(tested_term == this || no_prune) {
        assert(tested_term->type != TermType::EMPTY);
        return SubsumedType::NOT;
    } else {
        assert(tested_term != nullptr);
        assert(!tested_term->IsEmpty());
        biggerTerm = tested_term;
        return SubsumedType::PARTIALLY;
    }
}

SubsumedType TermProduct::IsSubsumedBy(TermFixpoint* fixpoint, Term*& biggerTerm, SubsumedByParams params) {
    return this->_ProductIsSubsumedBy<TermProduct>(fixpoint, biggerTerm, params);
}

SubsumedType TermTernaryProduct::IsSubsumedBy(TermFixpoint* fixpoint, Term *& biggerTerm, SubsumedByParams params) {
    return this->_ProductIsSubsumedBy<TermTernaryProduct>(fixpoint, biggerTerm, params);
}

SubsumedType TermNaryProduct::IsSubsumedBy(TermFixpoint* fixpoint, Term *& biggerTerm, SubsumedByParams params) {
    return this->_ProductIsSubsumedBy<TermNaryProduct>(fixpoint, biggerTerm, params);
}

SubsumedType TermBaseSet::IsSubsumedBy(TermFixpoint* fixpoint, Term*& biggerTerm, SubsumedByParams params) {
    bool no_prune = params.no_prune;

    if(this->IsEmpty()) {
        return SubsumedType::YES;
    }
    // For each item in fixpoint
    Term* tested_term = this;
    Term* new_term = nullptr;

    for(FixpointMember& item : fixpoint->_fixpoint) {
        // Nullptr is skipped
        if(item.term == nullptr || !item.isValid || item.level != params.level) continue;

        // Test the subsumption
        SubsumedType result;
        if(!no_prune && fixpoint->_validCount > 1) {
            result = tested_term->IsSubsumed(item.term, &new_term, SubsumptionTestParams(OPT_PARTIALLY_LIMITED_SUBSUMPTION, params.level));
        } else {
            result = tested_term->IsSubsumed(item.term, nullptr, SubsumptionTestParams(OPT_PARTIALLY_LIMITED_SUBSUMPTION, params.level));
        }
        switch(result) {
            case SubsumedType::YES:
                return SubsumedType::YES;
            case SubsumedType::NOT:
                break;
            case SubsumedType::PARTIALLY:
                assert(new_term != nullptr);
                assert(!no_prune);
                tested_term = new_term;
                break;
            default:
                assert(false && "Unsupported subsumption returned");
        }

        if(!no_prune) {
            if (item.term->IsSubsumed(tested_term, nullptr, SubsumptionTestParams(OPT_PARTIALLY_LIMITED_SUBSUMPTION, params.level)) == SubsumedType::YES) {
#               if (OPT_PRUNE_WORKLIST == true)
                prune_worklist(fixpoint->_worklist, item.term);
#               endif
                item.isValid = false;
                assert(fixpoint->_validCount);
                --fixpoint->_validCount;
            }
        }
    }

    if(tested_term == this || no_prune) {
        return SubsumedType::NOT;
    } else {
        assert(tested_term != nullptr);
        assert(!tested_term->IsEmpty());
        biggerTerm = tested_term;
        return SubsumedType::PARTIALLY;
    }
}

SubsumedType TermContinuation::IsSubsumedBy(TermFixpoint* fixpoint, Term*& biggerTerm, SubsumedByParams params) {
    assert(false && "TermContSubset.IsSubsumedBy() is impossible to happen~!");
}

SubsumedType TermList::IsSubsumedBy(TermFixpoint* fixpoint, Term*& biggerTerm, SubsumedByParams params) {
    bool no_prune = params.no_prune;

    if(this->IsEmpty()) {
        return SubsumedType::YES;
    }
    // For each item in fixpoint
    for(FixpointMember& item : fixpoint->_fixpoint) {
        // Nullptr is skipped
        if(item.term == nullptr || !item.isValid || item.level != params.level) continue;

        if (this->IsSubsumed(item.term, nullptr, SubsumptionTestParams(OPT_PARTIALLY_LIMITED_SUBSUMPTION, params.level)) == SubsumedType::YES) {
            return SubsumedType::YES;
        }

        if(!no_prune) {
            if (item.term->IsSubsumed(this, nullptr, SubsumptionTestParams(OPT_PARTIALLY_LIMITED_SUBSUMPTION, params.level)) == SubsumedType::YES) {
#               if (OPT_PRUNE_WORKLIST == true)
                prune_worklist(fixpoint->_worklist, item.term);
#               endif
                item.isValid = false;
                assert(fixpoint->_validCount);
                --fixpoint->_validCount;
            }
        }
    }

    return SubsumedType::NOT;
}

SubsumedType TermFixpoint::IsSubsumedBy(TermFixpoint* fixpoint, Term*& biggerTerm, SubsumedByParams params) {
    auto result = SubsumedType::NOT;
    bool no_prune = params.no_prune;
    // Component-wise comparison
    for(FixpointMember& item : fixpoint->_fixpoint) {
        if(item.term == nullptr || !item.isValid || item.level != params.level) {
            continue;
        }

        if(item.level > params.level) {
            SubsumedType inner_result;
            if( (inner_result = item.term->IsSubsumed(this, nullptr, SubsumptionTestParams(OPT_PARTIALLY_LIMITED_SUBSUMPTION, params.level))) == SubsumedType::YES) {
#               if (OPT_PRUNE_WORKLIST == true)
                prune_worklist(fixpoint->_worklist, item.term);
#               endif
                item.isValid = false;
                assert(fixpoint->_validCount);
                --fixpoint->_validCount;
            }
            continue;
        }

        if (this->IsSubsumed(item.term, nullptr, SubsumptionTestParams(OPT_PARTIALLY_LIMITED_SUBSUMPTION, params.level)) == SubsumedType::YES) {
            result = SubsumedType::YES;
            break;
        }

        if(!no_prune) {
            if (item.term->IsSubsumed(this, nullptr, SubsumptionTestParams(OPT_PARTIALLY_LIMITED_SUBSUMPTION, params.level)) == SubsumedType::YES) {
#               if (OPT_PRUNE_WORKLIST == true)
                prune_worklist(fixpoint->_worklist, item.term);
#               endif
                item.isValid = false;
                assert(fixpoint->_validCount);
                --fixpoint->_validCount;
            }
        }

    }
#if (DEBUG_TERM_SUBSUMED_BY == true)
    this->dump();
    std::cout << (result == SubsumedType::YES ? " [\u2286] " : " [\u2288] ");
    std::cout << "{";
    for(auto& item : fixpoint->_fixpoint) {
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
SubsumedType Term::Subsumes(TermEnumerator* enumerator) {
    if(this->type == TermType::EMPTY) {
        return SubsumedType::NOT;
    }

    SubsumedType result = SubsumedType::NOT;
#   if (OPT_ENUMERATED_SUBSUMPTION_TESTING == true)
    if(!this->_subsumesCache.retrieveFromCache(enumerator, result)) {
        result = this->_SubsumesCore(enumerator);
        if(result != SubsumedType::NOT)
            this->_subsumesCache.StoreIn(enumerator, result);
    }
#   endif
    return result;
}

SubsumedType Term::_SubsumesCore(TermEnumerator* enumerator) {
    assert(enumerator->type == EnumeratorType::GENERIC);
    GenericEnumerator* genericEnumerator = static_cast<GenericEnumerator*>(enumerator);
    // Fixme: There's missing level info
    auto result = genericEnumerator->GetItem()->IsSubsumed(this, nullptr, SubsumptionTestParams(OPT_PARTIALLY_LIMITED_SUBSUMPTION, 0));
    return result;
}

SubsumedType TermProduct::_SubsumesCore(TermEnumerator* enumerator) {
    // TODO: Missing partial subsumption
    assert(enumerator->type == EnumeratorType::PRODUCT);
    ProductEnumerator* productEnumerator = static_cast<ProductEnumerator*>(enumerator);
    if(this->left->stateSpaceApprox <= this->right->stateSpaceApprox) {
        return (this->left->Subsumes(productEnumerator->GetLeft()) != SubsumedType::NOT &&
                this->right->Subsumes(productEnumerator->GetRight()) != SubsumedType::NOT) ? SubsumedType::YES : SubsumedType::NOT;
    } else {
        return (this->right->Subsumes(productEnumerator->GetRight()) != SubsumedType::NOT &&
                this->left->Subsumes(productEnumerator->GetLeft()) != SubsumedType::NOT) ? SubsumedType::YES : SubsumedType::NOT;
    }
}

SubsumedType TermBaseSet::_SubsumesCore(TermEnumerator* enumerator) {
    assert(enumerator->type == EnumeratorType::BASE);
    BaseEnumerator* baseEnumerator = static_cast<BaseEnumerator*>(enumerator);
    auto item = baseEnumerator->GetItem();

    for(auto state : this->states) {
        if(state == item) {
            return SubsumedType::YES;
        } else if(state > item) {
            return SubsumedType::NOT;
        }
    }

    return SubsumedType::NOT;
}

SubsumedType TermTernaryProduct::_SubsumesCore(TermEnumerator* enumerator) {
    assert(enumerator->type == EnumeratorType::TERNARY);
    TernaryProductEnumerator* ternaryEnumerator = static_cast<TernaryProductEnumerator*>(enumerator);
    return (this->left->Subsumes(ternaryEnumerator->GetLeft()) != SubsumedType::NOT &&
            this->middle->Subsumes(ternaryEnumerator->GetMiddle()) != SubsumedType::NOT &&
            this->right->Subsumes(ternaryEnumerator->GetRight()) != SubsumedType::NOT) ? SubsumedType::YES : SubsumedType::NOT;
}

SubsumedType TermNaryProduct::_SubsumesCore(TermEnumerator* enumerator) {
    assert(enumerator->type == EnumeratorType::NARY);
    NaryProductEnumerator* naryEnumerator = static_cast<NaryProductEnumerator*>(enumerator);
    for(size_t i = 0; i < this->arity; ++i) {
        if(this->terms[i]->Subsumes(naryEnumerator->GetEnum(i)) == SubsumedType::NOT) {
            return SubsumedType::NOT;
        }
    }
    return SubsumedType::YES;
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
 * @brief Tests if the term is semantically valid
 *
 * Checks if the restrictions hold for this term, i.e. it is semantically valid
 *
 * @return  true  if the term is semantically valid
 */
bool TermEmpty::IsSemanticallyValid() {
    // Fixme: This stinks, but whatever. I'll do what I want, mom!
    return true;
}

bool TermProduct::IsSemanticallyValid() {
    return this->left->IsSemanticallyValid() && this->right->IsSemanticallyValid();
}

bool TermTernaryProduct::IsSemanticallyValid() {
    return this->left->IsSemanticallyValid() && this->middle->IsSemanticallyValid() && this->right->IsSemanticallyValid();
}

bool TermNaryProduct::IsSemanticallyValid() {
    for(size_t i = 0; i < this->arity; ++i) {
        if(!this->terms[i]->IsSemanticallyValid()) {
            return false;
        }
    }
    return true;
}

bool TermBaseSet::IsSemanticallyValid() {
    if(this->_aut->IsRestriction()) {
        return this->IsAccepting();
    } else {
        return true;
    }
}

bool TermContinuation::IsSemanticallyValid() {
    assert(false && "Unsupported TermType 'CONTINUATION' for IsSemanticallyValid()");
}

bool TermList::IsSemanticallyValid() {
    return true;
}

bool TermFixpoint::IsSemanticallyValid() {
    return true;
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
    for(FixpointMember& item : this->_fixpoint) {
        if(item.term == nullptr || !item.isValid) {
            continue;
        }
        count += (item.term)->MeasureStateSpace();
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

    void dumpResultLevelKey(std::tuple<Term_ptr, Symbol_ptr, size_t, char> const&s) {
        assert(std::get<0>(s) != nullptr);
        std::cout << "<" << (*std::get<0>(s)) << ", " << (*std::get<1>(s)) << ", " << std::get<2>(s);
        std::cout << std::get<3>(s) << ">";
    }

    void dumpResultData(std::pair<Term_ptr, bool> &s) {
        std::cout << "<" << (*s.first) << ", " << (s.second ? "True" : "False") << ">";
    }

    void dumpSubsumptionKey(std::pair<Term_ptr, Term_ptr> const& s) {
        assert(s.first != nullptr);
        assert(s.second != nullptr);

        std::cout << "<" << (*s.first) << ", " << (*s.second) << ">";
    }

    void dumpSubsumptionPairData(std::pair<SubsumedType, Term_ptr> &s) {
        switch(s.first) {
            case SubsumedType::YES:
                std::cout << "True";
                break;
            case SubsumedType::NOT:
                std::cout << "False";
                break;
            case SubsumedType::PARTIALLY:
                std::cout << "Partially";
                break;
        }
        if(s.second) {
            std::cout << ", ";
            s.second->dump();
        }
    }

    void dumpSubsumptionData(SubsumedType &s) {
        std::cout << (s != SubsumedType::NOT ? "True" : "False");
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
    if(this->IsIntermediate()) {
        std::cout << "'";
    }
    if(!this->IsSemanticallyValid()) {
        std::cout << "!";
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

size_t base_state_to_stream(TermBaseSet* base, size_t state) {
    if(base->IsIntermediate()) {
        return state >> 3;
    } else {
        return state;
    }
}

std::string intermediate_to_stream(TermBaseSet* base, size_t state) {
    if(base->IsIntermediate()) {
        return std::string("[") + std::to_string(state) + std::string("]");
    } else {
        return "";
    }
}

void base_to_stream(TermBaseSet* base, std::ostream& os) {
    size_t start = 0, end = 0;
    bool first = true;
    for (auto state : base->states) {
        if(start != 0 && end == state - 1) {
            end = state;
            continue;
        } else if(start != 0) {
            if(first) {
                first = false;
            } else {
                os << ", ";
            }

            if (start == end) {
                os << base_state_to_stream(base, start) << intermediate_to_stream(base, start);
            } else {
                os << base_state_to_stream(base, start) << intermediate_to_stream(base, start) << ".." << base_state_to_stream(base, end) << intermediate_to_stream(base, end);
            }
        }
        start = state;
        end = state;
    }

    if(!first) {
        os << ", ";
    }
    if (start == end) {
        os << base_state_to_stream(base, start) << intermediate_to_stream(base, start);
    } else {
        os << base_state_to_stream(base, start) << intermediate_to_stream(base, start) << ".." << base_state_to_stream(base, end) << intermediate_to_stream(base, end);
    }
}

void TermBaseSet::_dumpCore(unsigned indent) {
    std::cout << "\033[1;35m{";
    base_to_stream(this, std::cout);
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
    std::cout << "[" << this << "]";
    std::cout << "\033[1;34m{\033[0m" << "\n";
    for(FixpointMember& item : this->_fixpoint) {
        if(item.term == nullptr) {
            std::cout << std::string(indent+2, ' ');
            std::cout << "NULL";
            std::cout << "@" << item.level;
            std::cout << "\033[1;34m,\033[0m";
            std::cout << "\n";
        }
        if(item.term == nullptr || !(item.isValid)) {
            continue;
        }
        std::cout << std::string(indent+2, ' ');
        item.term->dump(indent + 2);
        std::cout << "@" << item.level;
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
    for(WorklistItem& workItem : this->_worklist) {
        std::cout << *(workItem.term) << " + " << *(workItem.symbol) << ", ";
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

/**
 * @brief Checks if this set intersect the initial states of the automaton
 *
 * Checks if this states is accepting, i.e. it intersects the initial states
 * of the _aut.
 *
 * @return  true if the set is accepting
 */
bool TermBaseSet::IsAccepting() {
    // Fixme: This could be further micro-optimized
    TermBaseSet* initialStates = static_cast<TermBaseSet*>(this->_aut->GetInitialStates());
    assert(initialStates->states.size() == 1 && "We assume there is only one initial states");
    size_t initialState = initialStates->states.ToVector().front();

    for (auto lhs_state : this->states) {
        if(lhs_state == initialState) {
            return true;
        } else if(lhs_state > initialState) {
            break;
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
    SubsumedType result;
    // Todo: this could be softened to iterators
    // Fixme: There's missing info about level
    if( (result = postponedTerm->IsSubsumed(postponedFixTerm, nullptr, SubsumptionTestParams(OPT_PARTIALLY_LIMITED_SUBSUMPTION, 0, true))) == SubsumedType::NOT) {
        // Push new term to fixpoint
        // Fixme: But there is probably something other that could subsume this crap
        for(auto item : this->_fixpoint) {
            if(item.first == nullptr)
                continue;
            if((result = postponedTerm->IsSubsumed(item.first, SubsumptionTestParams(OPT_PARTIALLY_LIMITED_SUBSUMPTION, 0, true))) != SubsumedType::NOT) {
                assert(result != SubsumedType::PARTIALLY);
                return false;
            }
        }

        this->_fixpoint.push_back(std::make_pair(postponedTerm, true));
        // Push new symbols from _symList, if we are in Fixpoint semantics
        if (this->GetSemantics() == E_FIXTERM_FIXPOINT) {
            for (auto &symbol : this->_symList) {
                assert(!OPT_INCREMENTAL_LEVEL_PRE && "This does not work with IncrementalPre");
                this->_worklist.emplace_front(postponedTerm, symbol, 0, 'X');
            }
        }
        #if (MEASURE_POSTPONED == TRUE)
        ++TermFixpoint::postponedProcessed;
        #endif
        return true;
    } else {
        assert(result != SubsumedType::PARTIALLY);
        return false;
    }
#   else
    return false;
#   endif
}

std::pair<SubsumedType, Term_ptr> TermFixpoint::_fixpointTest(Term_ptr const &term, SubsumedByParams params) {
    if(this->_searchType == WorklistSearchType::UNGROUND_ROOT) {
        // Fixme: Not sure if this is really correct, but somehow I still feel that the Root search is special and
        //   subsumption is maybe not enough? But maybe this simply does not work for fixpoints of negated thing.
        if(this->_unsatTerm == nullptr && this->_satTerm == nullptr) {
            return this->_testIfIn(term, params);
        }

        auto key = term;
        SubsumedType result;
        if(this->_subsumedByCache.retrieveFromCache(key, result)) {
            assert(result == SubsumedType::YES);
            return std::make_pair(result, term);
#       if (MEASURE_SUBSUMEDBY_HITS == true)
        } else {
            ++TermFixpoint::subsumedByHits;
#       endif
        }
        std::pair<SubsumedType, Term_ptr> inner_results;
        if(this->_satTerm == nullptr) {
            inner_results = this->_testIfBiggerExists(term, params);
        } else {
            assert(this->_satTerm != nullptr);
            assert(this->_unsatTerm == nullptr);
            inner_results = this->_testIfSmallerExists(term, params);
        }

        if(inner_results.first == SubsumedType::YES) {
            this->_subsumedByCache.StoreIn(key, SubsumedType::YES);
#           if (OPT_PUMP_SUBSUMED_BY_CACHE == true)
            this->_PumpSubsumedByCache(key);
#           endif
        }

        return inner_results;
    } else {
        return this->_testIfSubsumes(term, params);
    }
}

std::pair<SubsumedType, Term_ptr >TermFixpoint::_testIfBiggerExists(Term_ptr const &term, SubsumedByParams params) {
    return (std::find_if(this->_fixpoint.begin(), this->_fixpoint.end(), [this, &term, &params](FixpointMember& member) {
        if (member.isValid && member.level == params.level && term == member.term) {
            return true;
        } else if(!member.isValid || member.term == nullptr || member.level != params.level || (member.level == 0 && !member.term->IsSemanticallyValid())) {
            return false;
        } else {
            if(member.term != term && member.term->IsSubsumed(term, nullptr, SubsumptionTestParams(OPT_PARTIALLY_LIMITED_SUBSUMPTION, params.level)) != SubsumedType::NOT) {
                member.isValid = false;
                assert(this->_validCount);
                --this->_validCount;
#               if (OPT_PRUNE_WORKLIST == true)
                prune_worklist(this->_worklist, member.term);
#               endif
                return false;
            } else {
                return term->IsSubsumed(member.term, nullptr, SubsumptionTestParams(OPT_PARTIALLY_LIMITED_SUBSUMPTION, params.level)) != SubsumedType::NOT;
            }
        }
    }) == this->_fixpoint.end() ? std::make_pair(SubsumedType::NOT, term) : std::make_pair(SubsumedType::YES, term));
}

std::pair<SubsumedType, Term_ptr> TermFixpoint::_testIfSmallerExists(Term_ptr const &term, SubsumedByParams params) {
    return (std::find_if(this->_fixpoint.begin(), this->_fixpoint.end(), [this, &term, &params](FixpointMember& member) {
        if (member.isValid && term == member.term && member.level == params.level) {
            return true;
        } else if(!member.isValid || member.term == nullptr || member.level != params.level || (member.level == 0 && !member.term->IsSemanticallyValid())) {
            return false;
        } else {
            if(member.term != term && term->IsSubsumed(member.term, nullptr, SubsumptionTestParams(OPT_PARTIALLY_LIMITED_SUBSUMPTION, params.level)) != SubsumedType::NOT) {
                member.isValid = false;
                assert(this->_validCount);
                --this->_validCount;
#               if (OPT_PRUNE_WORKLIST == true)
                prune_worklist(this->_worklist, member.term);
#               endif
                return false;
            } else {
                return member.term->IsSubsumed(term, nullptr, SubsumptionTestParams(OPT_PARTIALLY_LIMITED_SUBSUMPTION, params.level)) != SubsumedType::NOT;
            }
        }
    }) == this->_fixpoint.end() ? std::make_pair(SubsumedType::NOT, term) : std::make_pair(SubsumedType::YES, term));
}

std::pair<SubsumedType, Term_ptr> TermFixpoint::_testIfIn(Term_ptr const &term, SubsumedByParams params) {
    return (std::find_if(this->_fixpoint.begin(), this->_fixpoint.end(), [&term, &params](FixpointMember const& member){
        if(member.isValid && member.level == params.level) {
            return member.term == term;
        } else {
            return false;
        }
    }) == this->_fixpoint.end() ? std::make_pair(SubsumedType::NOT, term) : std::make_pair(SubsumedType::YES, term));
}

/**
 * Tests if term is subsumed by fixpoint, either it is already computed in cache
 * or we have to compute the subsumption testing for each of the fixpoint members
 * and finally store the results in the cache.
 *
 * @param[in] term:     term we are testing subsumption for
 * @return:             true if term is subsumed by fixpoint
 */
std::pair<SubsumedType, Term_ptr> TermFixpoint::_testIfSubsumes(Term_ptr const& term, SubsumedByParams params) {
    Term* subsumedByTerm = nullptr;
    #if (OPT_CACHE_SUBSUMED_BY == true)
    SubsumedType result;
    Term* key = term;
    // Fixme: There should be something different
    if(term->IsIntermediate() || !this->_subsumedByCache.retrieveFromCache(key, result)) {
        // True/Partial results are stored in cache
        if((result = term->IsSubsumedBy(static_cast<TermFixpoint*>(this), subsumedByTerm, params)) != SubsumedType::NOT && !term->IsIntermediate()) {
            // SubsumedType::PARTIALLY is considered as SubsumedType::YES, as it was partitioned already
            this->_subsumedByCache.StoreIn(key, SubsumedType::YES);
#           if (OPT_PUMP_SUBSUMED_BY_CACHE == true)
            this->_PumpSubsumedByCache(key);
#           endif
        }

        if(result == SubsumedType::PARTIALLY) {
            assert(subsumedByTerm != nullptr);
            assert(subsumedByTerm->type != TermType::EMPTY);
#           if (OPT_SUBSUMPTION_INTERSECTION == true)
            return std::make_pair(SubsumedType::NOT, subsumedByTerm);
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
    //assert(!(term->type == TermType::EMPTY && result != SubsumedType::YES));
    return std::make_pair(result, term);
    #else
    return std::make_pair(term->IsSubsumedBy(this->_fixpoint, this->_worklist, subsumedByTerm, params), term);
    #endif
}

/**
 * @brief Pops enqueued item from worklist according to the worklist search type
 *
 * @return  enqueued worklist item
 */
WorklistItemType TermFixpoint::_popFromWorklist() {
    assert(_worklist.size() > 0);
    if(this->_searchType != WorklistSearchType::BFS) {
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
 * @brief Enqueues tripple (term, var, value) if the fixpoint guide allows us to do so
 *
 * Asks the Fixpoint Guide what to do for the tripple (@p term, @p var, @p value). If we
 * should not throw it away, it is enqueued in the front of the worklist.
 *
 * @param[in]  term  enqueued term
 * @param[in]  var  variable level we are subtracting
 * @param[in]  value  value we are subtracting on level @p var
 */
void TermFixpoint::_EnqueueSingleLevelInWorklist(Term_ptr term, size_t var, char value) {
    if(this->_guide->GiveTipForIncremental(term, var, value) != GuideTip::G_THROW) {
        _worklist.emplace_front(term, this->_projectedSymbol, var, value);
    }
}

/**
 * @brief Enqueues @p term in fixpoint, driven by fixpoint guide
 *
 * Enqueues @p term in worklist, according to the the fixpoint guide, and various heuristics.
 *
 * @param[in]  term  enqued term
 * @param[in]  params  params for the enqueued term
 * @param[in]  enqueueNext  true if next war should be computed
 */
void TermFixpoint::_EnqueueInWorklist(Term_ptr term, IntersectNonEmptyParams& params, bool enqueueNext) {
#   if (OPT_WORKLIST_DRIVEN_BY_RESTRICTIONS == true)
    GuideTip tip;
    if (!OPT_INCREMENTAL_LEVEL_PRE && (this->_guide == nullptr || (tip = this->_guide->GiveTip(term)) == GuideTip::G_PROJECT)) {
#   endif
        for (auto &symbol : _symList) {
#           if (OPT_WORKLIST_DRIVEN_BY_RESTRICTIONS == true)
            if (this->_guide != nullptr) {
                bool break_from_for = false;
                switch (this->_guide->GiveTip(term, symbol)) {
                    case GuideTip::G_FRONT:
                        _worklist.emplace_front(term, symbol, 0, 'X');
                        break;
                    case GuideTip::G_BACK:
                        _worklist.emplace_back(term, symbol, 0, 'X');
                        break;
                    case GuideTip::G_THROW:
                        break;
                    case GuideTip::G_PROJECT:
                        _worklist.emplace_front(term, symbol, 0, 'X');
                        break;
                    default:
                        assert(false && "Unsupported guide tip");
                }
            } else {
                _worklist.emplace_front(term, symbol, 0, 'X');
            }
#           elif (OPT_FIXPOINT_BFS_SEARCH == true)
            _worklist.emplace_back(term, symbol, 0, 'X');
#           else
            _worklist.emplace_back(term, symbol, 0, 'X');
#           endif
        }
#   if (OPT_WORKLIST_DRIVEN_BY_RESTRICTIONS == true)
    } else if(params.limitPre) {
        size_t nextVar = enqueueNext ? ((varMap.TrackLength() - 1 + params.variableLevel) % varMap.TrackLength()) : params.variableLevel;
        char value;
        if(this->GetSemantics() == FixpointSemanticType::FIXPOINT) {
            value = this->_projectedSymbol->GetSymbolAt(nextVar);
        } else {
            value = params.variableValue;
        }
        switch(value) {
            case '1':
            case '0':
                // Queue single shit
                this->_EnqueueSingleLevelInWorklist(term, nextVar, value);
                break;
            case 'X':
                // Queue multiple shits
                this->_EnqueueSingleLevelInWorklist(term, nextVar, '0');
                this->_EnqueueSingleLevelInWorklist(term, nextVar, '1');
                break;
            default:
                assert(false && "Unsupported stuff");
                break;
        }
    } else {
        // FIXME: TODO: Enqueueing of stuff
        _worklist.emplace_front(term, this->_projectedSymbol, 0, 'X');
    }
#   endif
}

bool is_skippable(size_t looked_up_var, VarList* vars) {
    for(auto var = vars->begin(); var != vars->end(); ++var) {
        if(*var == looked_up_var) {
            ++var;
            if(*var == looked_up_var + 1) {
                return true;
            } else {
                return false;
            }
        } else if(*var > looked_up_var) {
            break;
        }
    }

    return false;
}

void TermFixpoint::_ProcessComputedResult(std::pair<Term_ptr, bool>& result, bool isBaseFixpoint, IntersectNonEmptyParams& params) {
    // If it is subsumed by fixpoint, we don't add it
    auto fix_result = this->_fixpointTest(result.first, SubsumedByParams(false, params.variableLevel));
    if(fix_result.first != SubsumedType::NOT) {
        assert(fix_result.first != SubsumedType::PARTIALLY);
#       if (MEASURE_PROJECTION == true)
        if(isBaseFixpoint && _worklist.empty() && !this->_fullyComputed) {
            this->_fullyComputed = true;
            ++TermFixpoint::fullyComputedFixpoints;
        }
#       endif
        return;
    }

    // Update examples
    this->_updateExamples(result);

    // Push new term to fixpoint
    _fixpoint.emplace_back(fix_result.second, true, params.variableLevel);
    ++this->_validCount;

    _updated = true;
    // Aggregate the result of the fixpoint computation
    if(!fix_result.second->IsIntermediate())
        _bValue = this->_AggregateResult(_bValue,result.second);
    if(isBaseFixpoint) {
        // Push new symbols from _symList
        this->_EnqueueInWorklist(fix_result.second, params);
    }

    // Update stats
    if(this->_aut->stats.max_symbol_path_len < fix_result.second->link->len) {
        this->_aut->stats.max_symbol_path_len = fix_result.second->link->len;
    }
}

/**
 * @brief Ascends the fixpoint by one level, and push the subtracted value to the pattern list
 *
 * Pushes the currently subtracted (var+value) to the list of patterns. Then every
 * term from fixpoint is enqued, the fixpoint is reduced in sized and then, the
 * whole level of variables is processed.
 */
void TermFixpoint::PushAndCompute(IntersectNonEmptyParams& params) {
    assert(false && "[[Deprecated function]]");
}

/**
 * @brief Computes the next term in fixpoint
 *
 * Dequeues an item from worklist and computes its pre with epsilon test.
 * If the computed term is not subsumed by already computed fixpoint terms,
 *   we add it to the fixpoint and enuques additional computations.
 *
 * @param[in]  isBaseFixpoint  true if the fixpoint is base computation
 */
void TermFixpoint::ComputeNextMember(bool isBaseFixpoint) {
    if(_worklist.empty())
        return;

    // Pop the front item from worklist
    WorklistItemType item = this->_popFromWorklist();
    // Compute the results
    IntersectNonEmptyParams params(GET_NON_MEMBERSHIP_TESTING(this));
    ResultType result;

#   if (OPT_INCREMENTAL_LEVEL_PRE == true)
    params.limitPre = true;
    params.variableLevel = item.level;
    params.variableValue = item.value;
    result = this->_baseAut->IntersectNonEmpty(item.symbol, item.term, params);
    this->_ProcessComputedResult(result, isBaseFixpoint, params);
#   else
    result = this->_baseAut->IntersectNonEmpty(item.symbol, item.term, params);
    this->_ProcessComputedResult(result, isBaseFixpoint, params);
#   endif
}

/**
 * @brief Forcefully takes all the intermediate stuff from worklist, and computes them
 *
 * Dequeues all items from worklist, that are intermediate, computes them and put into fixpoint.
 *
 * @param[in]  isBaseFixpoint  true if the computation is base or not
 */
void TermFixpoint::ForcefullyComputeIntermediate(bool isBaseFixpoint) {
    assert(OPT_INCREMENTAL_LEVEL_PRE);
    assert(false && "[[Deprecated function]]");
    if(_worklist.empty())
        return;

    auto it = this->_worklist.begin();
    auto end = this->_worklist.end();
    IntersectNonEmptyParams params(GET_NON_MEMBERSHIP_TESTING(this));
    params.limitPre = true;
    ResultType result;

    while(it != end) {
        WorklistItem item = *it;
        if (item.level != 0) {
            it = this->_worklist.erase(it);
        } else {
            ++it;
            continue;
        }

        params.variableLevel = item.level;
        params.variableValue = item.value;
        result = this->_baseAut->IntersectNonEmpty(item.symbol, item.term, params);
        size_t size_before = _worklist.size();
        this->_ProcessComputedResult(result, isBaseFixpoint, params);
        if (size_before != _worklist.size()) {
            // We added something to worklist, so we have to go from start
            it = this->_worklist.begin();
        }
    }

    this->RemoveIntermediate();
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
void TermFixpoint::_InitializeSymbols(Workshops::SymbolWorkshop* workshop, Gaston::VarList* nonOccuringVars, IdentList* vars, Symbol *startingSymbol) {
    // The input symbol is first trimmed, then if the AllPosition Variable exist, we generate only the trimmed stuff
    // TODO: Maybe for Fixpoint Pre this should be done? But nevertheless this will happen at topmost
    // Reserve symbols for at least 2^|freeVars|
    this->_symList.reserve(2 << vars->size());
    Symbol* trimmed = workshop->CreateTrimmedSymbol(startingSymbol, nonOccuringVars);
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
        if(*var == allPosVar)
            continue;
        this->_projectedSymbol = workshop->CreateSymbol(this->_projectedSymbol, varMap[(*var)], 'X');
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
 * @brief Initializes @p startingSymbol for multiple pushing
 *
 * @param[in]  workshop  pointer to symbol workshop
 * @param[in]  nonOccuringVars  list of variables that have no occurence in formula
 * @param[in]  vars  list of projected vars
 * @param[in]  startingSymbol  starting symbol we are preparing
 */
void TermFixpoint::_InitializeProjectedSymbol(Workshops::SymbolWorkshop* workshop, Gaston::VarList* nonOccuringVars, IdentList* vars, Symbol *startingSymbol) {
    Symbol* trimmed = workshop->CreateTrimmedSymbol(startingSymbol, nonOccuringVars);
    if (allPosVar != -1) {
        trimmed = workshop->CreateSymbol(trimmed, varMap[allPosVar], '1');
    }

    this->_projectedSymbol = trimmed;
    for(auto var = vars->begin(); var != vars->end(); ++var) {
        if(*var == allPosVar)
            continue;
        this->_projectedSymbol = workshop->CreateSymbol(this->_projectedSymbol, varMap[(*var)], 'X');
    }

#   if (DEBUG_FIXPOINT_SYMBOLS_INIT == true)
    std::cout << "[!] Initialized fixpoint with symbol '" << (*this->_projectedSymbol) << "'\n";
#   endif
}

/**
 * @return: result of fixpoint
 */
bool TermFixpoint::GetResult() {
    return this->_bValue;
}

void TermFixpoint::_updateExamples(ResultType& result) {
    if(this->_searchType == WorklistSearchType::UNGROUND_ROOT) {
        if (result.first->IsIntermediate()) {
            return;
        }
#   if (ALT_SKIP_EMPTY_UNIVERSE == true)
        if (result.first->link->symbol == nullptr)
            return;
#   endif
        if (result.second) {
            if (this->_satTerm == nullptr && this->_baseAut->WasLastExampleValid()) {
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
            if ((*it).term == nullptr) {
                ++it;
                continue;
            }
            if (!(*it).isValid) {
                it = this->_fixpoint.erase(it);
                continue;
            }
            ++it;
        }
    }
}

void TermFixpoint::RemoveIntermediate() {
    for(FixpointMember& member : this->_fixpoint) {
        if(member.level != 0) {
            member.isValid = false;
#           if (OPT_PRUNE_WORKLIST == true)
            prune_worklist(this->_worklist, member.term);
#           endif
            assert(this->_validCount);
            --this->_validCount;
        }
    }
}

/**
 * Returns whether we are computing the Pre of the already finished fixpoint,
 * or if we are computing the fixpoint from scratch
 *
 * @return: semantics of the fixpoint
 */
FixpointSemanticType TermFixpoint::GetSemantics() const {
    return (nullptr == _sourceTerm) ? FixpointSemanticType::FIXPOINT : FixpointSemanticType::PRE;
}

// <<< ADDITIONAL TERMCONTINUATION FUNCTIONS >>>
Term* TermContinuation::unfoldContinuation(UnfoldedIn t) {
    if(this->_unfoldedTerm == nullptr) {
        if(lazyEval) {
            assert(this->aut->aut->type == AutType::INTERSECTION || this->aut->aut->type == AutType::UNION);
            assert(this->initAut != nullptr);
            BinaryOpAutomaton* boAutomaton = static_cast<BinaryOpAutomaton*>(this->initAut);
            std::tie(this->aut, this->term) = boAutomaton->LazyInit(this->term);
            lazyEval = false;
        }

        this->_unfoldedTerm = (this->aut->aut->IntersectNonEmpty(
                (this->symbol == nullptr ? nullptr : this->aut->ReMapSymbol(this->symbol)), this->term, IntersectNonEmptyParams(underComplement))).first;
        #if (MEASURE_CONTINUATION_EVALUATION == true)
        switch(t){
            case UnfoldedIn::SUBSUMPTION:
                ++TermContinuation::unfoldInSubsumption;
                break;
            case UnfoldedIn::ISECT_NONEMPTY:
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
        case TermType::EMPTY:
        case TermType::BASE:
            ++TermBaseSet::comparisonsBySamePtr;
            break;
        case TermType::LIST:
            ++TermList::comparisonsBySamePtr;
            break;
        case TermType::PRODUCT:
            ++TermProduct::comparisonsBySamePtr;
            break;
        case TermType::FIXPOINT:
            ++TermFixpoint::comparisonsBySamePtr;
            break;
        case TermType::CONTINUATION:
            ++TermContinuation::comparisonsBySamePtr;
            break;
        default:
            assert(false);
    }
}

void Term::comparedByDifferentType(TermType t) {
    ++Term::comparisonsByDiffType;
    switch(t) {
        case TermType::EMPTY:
        case TermType::BASE:
            ++TermBaseSet::comparisonsByDiffType;
            break;
        case TermType::LIST:
            ++TermList::comparisonsByDiffType;
            break;
        case TermType::PRODUCT:
            ++TermProduct::comparisonsByDiffType;
            break;
        case TermType::FIXPOINT:
            ++TermFixpoint::comparisonsByDiffType;
            break;
        case TermType::CONTINUATION:
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
        case TermType::EMPTY:
        case TermType::BASE:
            if(res) {
                ++TermBaseSet::comparisonsByStructureTrue;
            } else {
                ++TermBaseSet::comparisonsByStructureFalse;
            }
            break;
        case TermType::LIST:
            if(res) {
                ++TermList::comparisonsByStructureTrue;
            } else {
                ++TermList::comparisonsByStructureFalse;
            }
            break;
        case TermType::PRODUCT:
            if(res) {
                ++TermProduct::comparisonsByStructureTrue;
            } else {
                ++TermProduct::comparisonsByStructureFalse;
            }
            break;
        case TermType::FIXPOINT:
            if(res) {
                ++TermFixpoint::comparisonsByStructureTrue;
            } else {
                ++TermFixpoint::comparisonsByStructureFalse;
            }
            break;
        case TermType::CONTINUATION:
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
    if(tt->type == TermType::CONTINUATION) {
        TermContinuation* ttCont = static_cast<TermContinuation*>(tt);
        tt = ttCont->unfoldContinuation(UnfoldedIn::COMPARISON);
    }
    if(this->type == TermType::CONTINUATION) {
        TermContinuation* thisCont = static_cast<TermContinuation*>(this);
        tthis = thisCont->unfoldContinuation(UnfoldedIn::COMPARISON);
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
    assert(t.type == TermType::EMPTY && "Testing equality of different term types");
    return true;
}

bool TermProduct::_eqCore(const Term &t) {
    assert(t.type == TermType::PRODUCT && "Testing equality of different term types");

    #if (OPT_EQ_THROUGH_POINTERS == true)
    assert(this != &t);
    return false;
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
    assert(t.type == TermType::TERNARY_PRODUCT && "Testing equality of different term types");

#   if (OPT_EQ_THROUGH_POINTERS == true)
    assert(this != &t);
    return false;
#   else
    const TermTernaryProduct &tProduct = static_cast<const TermTernaryProduct&>(t);
    return (*tProduct.left == *this->left) && (*tProduct.middle == *this->middle) && (*tProduct.right == *this->right);
#   endif
}

bool TermNaryProduct::_eqCore(const Term &t) {
    assert(t.type == TermType::NARY_PRODUCT && "Testing equality of different term types");

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
    assert(t.type == TermType::BASE && "Testing equality of different term types");

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
    assert(t.type == TermType::CONTINUATION && "Testing equality of different term types");

    const TermContinuation &tCont = static_cast<const TermContinuation&>(t);
    if(this->_unfoldedTerm == nullptr) {
        return (this->symbol == tCont.symbol) && (this->term == tCont.term);
    } else {
        return this->_unfoldedTerm == tCont._unfoldedTerm;
    }
}

bool TermList::_eqCore(const Term &t) {
    assert(t.type == TermType::LIST && "Testing equality of different term types");
    G_NOT_IMPLEMENTED_YET("TermList::_eqCore");
}

unsigned int TermFixpoint::ValidMemberSize() const {
    unsigned int members = 0;
    for(const FixpointMember& item : this->_fixpoint) {
        members += (item.isValid && item.term != nullptr);
    }
    return members;
}

bool TermFixpoint::_eqCore(const Term &t) {
    assert(t.type == TermType::FIXPOINT && "Testing equality of different term types");

    const TermFixpoint &tFix = static_cast<const TermFixpoint&>(t);
    if(this->_bValue != tFix._bValue) {
        // If the values are different, we can automatically assume that there is some difference
        return false;
    }

    if(this->_validCount != tFix._validCount){
        return false;
    }

    // Fixme: I wonder if this is correct?
    bool are_symbols_the_same = TermFixpoint::_compareSymbols(*this, tFix);
    if(!are_symbols_the_same && (this->_worklist.size() != 0 || tFix._worklist.size() != 0)) {
        return false;
    }

    for(FixpointMember& item : this->_fixpoint) {
        if(item.term == nullptr || !item.isValid)
            continue;
        bool found = false;
        for(const FixpointMember& titem : tFix._fixpoint) {
            if(titem.term == nullptr || !titem.isValid || item.level != titem.level)
                continue;
            if(*item.term == *titem.term) {
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
    if(lhs._symbolPart.value != rhs._symbolPart.value || lhs._symbolPart.level != rhs._symbolPart.value) {
        return false;
    }

#   if (OPT_INCREMENTAL_LEVEL_PRE == false)
    for(auto symbol : lhs._symList) {
        if(std::find_if(rhs._symList.begin(), rhs._symList.end(), [&symbol](Symbol_ptr s) { return s == symbol;}) == rhs._symList.end()) {
            return false;
        }
    }
#   else
    assert(lhs._symList.size() == 0 && rhs._symList.size() == 0);
#   endif

    return true;
}

/**
 * Dumping functions to .dot
 */
std::string TermEmpty::DumpToDot(std::ostream& dot_out) {
    static size_t term_no = 0;
    std::string term_name = std::string("te") + std::to_string(term_no++);

    dot_out << "\t" << term_name << "[label=\"\u2205\"];\n";

    return term_name;
}

std::string TermProduct::DumpToDot(std::ostream& dot_out) {
    static size_t term_no = 0;
    std::string term_name = std::string("tp") + std::to_string(term_no++);
    dot_out << "\t" << term_name << "[label=\"" << ProductTypeToTermSymbol(GET_PRODUCT_SUBTYPE(this)) << "\"];\n";

    std::string left_mem = this->left->DumpToDot(dot_out);
    std::string right_mem = this->right->DumpToDot(dot_out);

    dot_out << "\t" << term_name << " -- " << left_mem << " [label=\"lhs\"];\n";
    dot_out << "\t" << term_name << " -- " << right_mem << " [label=\"rhs\"];\n";

    return term_name;
}

std::string TermTernaryProduct::DumpToDot(std::ostream& dot_out) {
    static size_t term_no = 0;
    std::string term_name = std::string("ttp") + std::to_string(term_no++);
    dot_out << "\t" << term_name << " [label=\"" << ProductTypeToTermSymbol(GET_PRODUCT_SUBTYPE(this)) << "\"];\n";

    std::string left_mem = this->left->DumpToDot(dot_out);
    std::string middle_mem = this->middle->DumpToDot(dot_out);
    std::string right_mem = this->right->DumpToDot(dot_out);

    dot_out << "\t" << term_name << " -- " << left_mem << " [label=\"lhs\"];\n";
    dot_out << "\t" << term_name << " -- " << middle_mem << " [label=\"mhs\"];\n";
    dot_out << "\t" << term_name << " -- " << right_mem << " [label=\"rhs\"];\n";

    return term_name;
}

std::string TermNaryProduct::DumpToDot(std::ostream& dot_out) {
    static size_t term_no = 0;
    std::string term_name = std::string("tnp") + std::to_string(term_no++);
    dot_out << "\t" << term_name << " [label=\"" << ProductTypeToTermSymbol(GET_PRODUCT_SUBTYPE(this)) << "\"];\n";

    for(size_t i = 0; i < this->arity; ++i) {
        std::string member = this->terms[i]->DumpToDot(dot_out);
        dot_out << "\t" << term_name << " -- " << member << " [label=\"" << std::to_string(i) << "hs\"];\n";
    }

    return term_name;
}

std::string TermBaseSet::DumpToDot(std::ostream& dot_out) {
    static size_t term_no = 0;
    std::string term_name = std::string("tbs") + std::to_string(term_no++);

    // Fixme: Better output
    std::ostringstream oss;
    base_to_stream(this, oss);
    dot_out << "\t" << term_name << " [label=\"" << (this->InComplement() ? "~" : "") << "{" << oss.str() << "}\"];\n";

    return term_name;
}

std::string TermContinuation::DumpToDot(std::ostream& dot_out) {
    static size_t term_no = 0;

    assert(false && "Missing implementation of 'DumpToDot' for TermContinuation");
    return std::string("tc") + std::to_string(term_no++);
}

std::string TermList::DumpToDot(std::ostream& dot_out) {
    static size_t term_no = 0;
    std::string term_name = std::string("tl") + std::to_string(term_no++);

    dot_out << "\t" << term_name << " [label=\"" << (this->InComplement() ? "~" : "") << "L\"];\n";
    for(Term* item : this->list) {
        std::string member = item->DumpToDot(dot_out);
        dot_out << "\t" << term_name << " -- " << member << "\n";
    }

    return term_name;
}

std::string TermFixpoint::DumpToDot(std::ostream& dot_out) {
    static size_t term_no = 0;
    std::string term_name = std::string("tf") + std::to_string(term_no++);

    dot_out << "\t" << term_name << " [label=\"" << (this->InComplement() ? "~" : "") << "F\"];\n";
    for(FixpointMember& item : this->_fixpoint) {
        if(item.term == nullptr || !item.isValid)
            continue;
        std::string member = item.term->DumpToDot(dot_out);
        dot_out << "\t" << term_name << " -- " << member << "\n";
    }

#   if (DEBUG_NO_DOT_WORKLIST == false)
    for(WorklistItem& item : this->_worklist) {
        std::string worklisted = item.term->DumpToDot(dot_out);
        // Fixme: Add symbol output
        dot_out << "\t" << term_name << " -- " << worklisted << " [style=\"dashed\"];\n";
    }
#   endif

    return term_name;
}

void Term::ToDot(Term* term, std::ostream& stream) {
    stream << "strict graph aut {\n";
    term->DumpToDot(stream);
    stream << "}\n";
}

std::ostream &operator<<(std::ostream &out, const FixpointMember &rhs) {
    out << (*rhs.term) << (rhs.isValid ? "" : "!") << "@" << rhs.level;
}

void Term::_IncreaseIntermediateInstances() {
    if(!this->IsIntermediate()) {
        switch(this->type) {
            case TermType::BASE:
                ++TermBaseSet::intermediateInstances;
                break;
            case TermType::FIXPOINT:
                ++TermFixpoint::intermediateInstances;
                break;
            case TermType::PRODUCT:
                ++TermProduct::intermediateInstances;
                break;
            case TermType::NARY_PRODUCT:
                ++TermNaryProduct::intermediateInstances;
                break;
            case TermType::TERNARY_PRODUCT:
                ++TermTernaryProduct::intermediateInstances;
                break;
            case TermType::LIST:
                ++TermList::intermediateInstances;
                break;
        }
    }
}

void Term::_DecreaseIntermediateInstances() {
    if(this->IsIntermediate()) {
        switch(this->type) {
            case TermType::BASE:
                --TermBaseSet::intermediateInstances;
                break;
            case TermType::FIXPOINT:
                --TermFixpoint::intermediateInstances;
                break;
            case TermType::PRODUCT:
                --TermProduct::intermediateInstances;
                break;
            case TermType::NARY_PRODUCT:
                --TermNaryProduct::intermediateInstances;
                break;
            case TermType::TERNARY_PRODUCT:
                --TermTernaryProduct::intermediateInstances;
                break;
            case TermType::LIST:
                --TermList::intermediateInstances;
                break;
        }
    }
}

/**
 * @brief Pumps the subsumed by cache by transitive closure
 *
 * Takes all "smaller" terms from subsumed, transitive aware cache and pumps them to subsumed by cache.
 *
 * @param[in]  term  pumping term
 */
void TermFixpoint::_PumpSubsumedByCache(Term_ptr term) {
    if (term->node != nullptr) {
        for (Node *succ : term->node->_succs) {
            this->_subsumedByCache.StoreIn(succ->_term, SubsumedType::YES);
        }
    }
}