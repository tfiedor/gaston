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
// TODO: Subsumption: We can maybe exploit something about the leafstates

#ifndef WSKS_TERM_H
#define WSKS_TERM_H

#include <vector>
#include <list>
#include <algorithm>
#include "../utils/Symbol.h"
#include "../mtbdd/ondriks_mtbdd.hh"
#include "../containers/SymbolicAutomata.h"
#include "../containers/FixpointGuide.h"
#include "../containers/TermEnumerator.h"
#include "../environment.hh"
#include "../containers/Workshops.h"

// <<< MACROS >>>

#define TERM_TYPELIST(code, var) \
    code(Term, var)              \
    code(TermEmpty, var)        \
	code(TermProduct, var)		\
    code(TermTernaryProduct, var) \
    code(TermNaryProduct, var)  \
	code(TermBaseSet, var)		\
	code(TermList, var)			\
	code(TermFixpoint, var)		\
	code(TermContinuation, var)

#define TERM_MEASURELIST(code) \
    code(instances)             \
    code(comparisonsBySamePtr)  \
    code(comparisonsByDiffType) \
    code(comparisonsByStructureTrue) \
    code(prunable)                  \
    code(comparisonsByStructureFalse)

#define DEFINE_STATIC_MEASURE(measure) \
    static size_t measure;

// <<< FORWARD CLASS DECLARATION >>>
class SymbolicAutomaton;
struct SymLink;

// TODO: Move away the usings
using Term_ptr          = Term*;
using TermProductStates = std::pair<Term_ptr, Term_ptr>;
using TermListStates    = std::vector<Term_ptr>;
using BaseState         = size_t;
//using TermBaseSetStates = std::vector<BaseState>;
using TermBaseSetStates = BaseAutomatonStateSet;
using ResultType        = std::pair<Term_ptr, bool>;
using ExamplePair       = std::pair<Term_ptr, Term_ptr>;
using SymbolType        = ZeroSymbol;

using FixpointMember = std::pair<Term_ptr, bool>;
using FixpointType = std::list<FixpointMember>;
using TermListType = std::list<std::pair<Term_ptr, Term_ptr>>;
using Aut_ptr = SymbolicAutomaton*;

using WorklistItemType = std::pair<Term_ptr, SymbolType*>;
using WorklistType = std::list<WorklistItemType>;
using Symbols = std::list<SymbolType*>;


class Term {
    friend class Workshops::TermWorkshop;

    // <<< MEMBERS >>>
protected:
    TermCache _isSubsumedCache;         // [36B] << Cache for results of subsumption
    EnumSubsumesCache _subsumesCache;   // [36B] << Cache for results of subsumes
public:
    struct {                        // [12B] << Link for counterexamples
        Term* succ;
        Symbol* symbol;
        size_t len;
        std::vector<Symbol*> history;
    } link, last_link;
public:
    size_t stateSpace = 0;          // [4-8B] << Exact size of the state space, 0 if unknown
    size_t stateSpaceApprox = 0;    // [4-8B] << Approximation of the state space, used for heuristics
    TermType type;                  // [4B] << Type of the term
protected:
    bool _nonMembershipTesting;     // [1B] << We are testing the nonmembership for this term
    bool _inComplement;             // [1B] << Term is complemented
public:

    NEVER_INLINE Term();
    virtual NEVER_INLINE ~Term();

    // See #L29
    TERM_MEASURELIST(DEFINE_STATIC_MEASURE)

public:
    // <<< PUBLIC API >>>
    virtual SubsumptionResult IsSubsumedBy(FixpointType& fixpoint, WorklistType& worklist, Term*&, bool no_prune = false) = 0;
    virtual SubsumptionResult IsSubsumed(Term* t, int limit, bool b = false);
    virtual SubsumptionResult Subsumes(TermEnumerator*);
    virtual bool IsEmpty() = 0;
    virtual void Complement();
    virtual bool InComplement() {return this->_inComplement;}
    bool operator==(const Term &t);
    bool IsNotComputed();
    void SetSuccessor(Term*, Symbol*);

    // <<< MEASURING FUNCTIONS >>>
    virtual unsigned int MeasureStateSpace();

    #if (MEASURE_COMPARISONS == true)
    static void comparedBySamePtr(TermType);
    static void comparedByDifferentType(TermType);
    static void comparedByStructure(TermType, bool);
    #endif

    // <<< DUMPING FUNCTIONS >>>
    virtual void dump(unsigned indent = 0);
protected:
    // <<< PRIVATE FUNCTIONS >>>
    virtual unsigned int _MeasureStateSpaceCore() = 0;
    virtual SubsumptionResult _IsSubsumedCore(Term* t, int limit, bool b = false) = 0;
    virtual void _dumpCore(unsigned indent = 0) = 0;
    virtual bool _eqCore(const Term&) = 0;
    virtual SubsumptionResult _SubsumesCore(TermEnumerator*);

    friend size_t hash_value(Term* s);
    friend std::ostream& operator <<(std::ostream& osObject, Term& z);
};

/**
 * Class that represents the empty term. Should be unique through the computation
 */
class TermEmpty : public Term {
public:
    // See #L29
    TERM_MEASURELIST(DEFINE_STATIC_MEASURE)

    // <<< CONSTRUCTORS >>>
    explicit NEVER_INLINE TermEmpty(bool inComplement=false);
    NEVER_INLINE ~TermEmpty() {}

    // <<< PUBLIC API >>>
    SubsumptionResult IsSubsumedBy(FixpointType& fixpoint, WorklistType& worklist, Term*&, bool no_prune = false);
    bool IsEmpty();

    // <<< DUMPING FUNCTIONS >>>
private:
    void _dumpCore(unsigned indent = 0);
    bool _eqCore(const Term&);

    // <<< PRIVATE FUNCTIONS >>>
    unsigned int _MeasureStateSpaceCore();
    SubsumptionResult _IsSubsumedCore(Term* t, int limit, bool b = false);
};

/**
 * Class that represents the product of two terms, product can be either Intersection or Union. For example
 * {1, 2} x {4, 5}, which can also be implemented as set of pairs
 */
class TermProduct : public Term {
public:
    // <<< PUBLIC MEMBERS >>>
    Term_ptr left;                              // [4B] << Left member of the product
    Term_ptr right;                             // [4B] << Right member of the product
    ProductType subtype;                        // [4B] << Product type (Union, Intersection)
    ProductEnumerator* enumerator = nullptr;    // [4B] << Enumerator through the terms

    // See #L29
    TERM_MEASURELIST(DEFINE_STATIC_MEASURE)

    // <<< CONSTRUCTORS >>>
    NEVER_INLINE TermProduct(Term_ptr lhs, Term_ptr rhs, ProductType subtype);
    NEVER_INLINE ~TermProduct();

    // <<< PUBLIC API >>>
    SubsumptionResult IsSubsumedBy(FixpointType& fixpoint, WorklistType& worklist, Term*&, bool no_prune = false);
    bool IsEmpty();

    // <<< DUMPING FUNCTIONS >>>
private:
    void _dumpCore(unsigned indent = 0);
    bool _eqCore(const Term&);

private:
    // <<< PRIVATE FUNCTIONS >>>
    unsigned int _MeasureStateSpaceCore();
    SubsumptionResult _IsSubsumedCore(Term* t, int limit, bool b = false);
    SubsumptionResult _SubsumesCore(TermEnumerator*);
};

class TermTernaryProduct : public Term {
public:
    /// <<< PUBLIC MEMBERS >>>
    Term_ptr left;
    Term_ptr middle;
    Term_ptr right;
    ProductType subtype;
    // Fixme: add enumerator

    // See #L29
    TERM_MEASURELIST(DEFINE_STATIC_MEASURE)

    // <<< CONSTRUCTORS >>>
    NEVER_INLINE TermTernaryProduct(Term_ptr, Term_ptr, Term_ptr, ProductType);
    NEVER_INLINE ~TermTernaryProduct();

    // <<< PUBLIC API >>>
    SubsumptionResult IsSubsumedBy(FixpointType&, WorklistType& worklist, Term*&, bool no_prune = false);
    bool IsEmpty();

    // <<< DUMPING FUNCTIONS >>>
private:
    void _dumpCore(unsigned indent = 0);
    bool _eqCore(const Term&);

    // <<< PRIVATE FUNCTIONS >>>
    unsigned int _MeasureStateSpaceCore();
    SubsumptionResult _IsSubsumedCore(Term* t, int limit, bool b = false);
};

class TermNaryProduct : public Term {
public:
    // <<< PUBLIC MEMBERS >>>
    size_t arity;
    ProductType subtype;
    Term_ptr* terms;
    size_t* access_vector;
    // Fixme: add iterator

    // See #L29
    TERM_MEASURELIST(DEFINE_STATIC_MEASURE)

    // <<< CONSTRUCTORS >>>
    NEVER_INLINE TermNaryProduct(Term_ptr*, ProductType, size_t);
    NEVER_INLINE TermNaryProduct(SymLink*, StatesSetType, ProductType, size_t); // NaryBase
    NEVER_INLINE TermNaryProduct(Term_ptr, Symbol_ptr, ProductType, size_t); // NaryPre
    NEVER_INLINE ~TermNaryProduct();

    // <<< PUBLIC API >>>
    SubsumptionResult IsSubsumedBy(FixpointType&, WorklistType& worklist, Term*&, bool no_prune = false);
    bool IsEmpty();
    Term_ptr operator[](size_t);

    // <<< DUMPING FUNCTIONS >>>
private:
    void _InitNaryProduct(ProductType, size_t);
    void _dumpCore(unsigned indent = 0);
    bool _eqCore(const Term&);

    // <<< PRIVATE FUNCTIONS >>>
    unsigned int _MeasureStateSpaceCore();
    SubsumptionResult _IsSubsumedCore(Term* t, int limit, bool b = false);
};

/**
 * Class that represents the base states that occurs on the leaf level of the computation
 */
class TermBaseSet : public Term {
public:
    // <<< PUBLIC MEMBERS >>>
    TermBaseSetStates states;       // [12B] << Linear Structure with Atomic States
    // See #L29
    TERM_MEASURELIST(DEFINE_STATIC_MEASURE)
#   if (MEASURE_BASE_SIZE == true)
    static size_t maxBaseSize;
#   endif

    // <<< CONSTRUCTORS >>>
    NEVER_INLINE TermBaseSet(VATA::Util::OrdVector<size_t> &&, unsigned int, unsigned int);
    NEVER_INLINE ~TermBaseSet();

    // <<< PUBLIC API >>>
    bool Intersects(TermBaseSet* rhs);
    SubsumptionResult IsSubsumedBy(FixpointType& fixpoint, WorklistType& worklist, Term*&, bool no_prune = false);
    bool IsEmpty();

    // <<< DUMPING FUNCTIONS >>>
private:
    void _dumpCore(unsigned indent = 0);
    bool _eqCore(const Term&);

private:
    // <<< PRIVATE FUNCTIONS >>>
    unsigned int _MeasureStateSpaceCore();
    SubsumptionResult _IsSubsumedCore(Term* t, int limit, bool b = false);
    SubsumptionResult _SubsumesCore(TermEnumerator*);
};

/**
 * Class that represents the postponed computation of the (non)membership testing
 */
class TermContinuation : public Term {
protected:
    Term* _unfoldedTerm = nullptr;      // [4B] << Unfolded term for optimizations
public:
    // <<< PUBLIC MEMBERS >>>
    SymLink* aut;                       // [4B] << Link to the automaton for computation
    SymbolicAutomaton* initAut;         // [4B] << Automaton for lazy initialization
    Term* term;                         // [4B] << Term we postponed the evaluation on
    SymbolType* symbol;                 // [4B] << Symbol we were subtracting from the term
    bool lazyEval = false;              // [1B] << The automaton will be lazily constructed
    bool underComplement;               // [1B] << Whether we were doing the membership or nonmembership
    // TODO: ^-- This is maybe redundant with _nonmembershipTesting??

    // See #L29
    TERM_MEASURELIST(DEFINE_STATIC_MEASURE)
    static size_t continuationUnfolding;
    static size_t unfoldInSubsumption;
    static size_t unfoldInIsectNonempty;

    // <<< CONSTRUCTORS >>>
    NEVER_INLINE TermContinuation(SymLink*, SymbolicAutomaton*, Term*, SymbolType*, bool, bool lazy = false);

    // <<< PUBLIC API >>>
    SubsumptionResult IsSubsumedBy(FixpointType& fixpoint, WorklistType& worklist, Term*&, bool no_prune = false);
    bool IsUnfolded() {return this->_unfoldedTerm != nullptr;}
    bool IsEmpty();
    Term* GetUnfoldedTerm() {return this->_unfoldedTerm; }
    Term* unfoldContinuation(UnfoldedInType);

protected:
    // <<< DUMPING FUNCTIONS >>>
    void _dumpCore(unsigned indent = 0);

    // <<< PRIVATE FUNCTIONS >>>
    unsigned int _MeasureStateSpaceCore();
    bool _eqCore(const Term&);
    SubsumptionResult _IsSubsumedCore(Term* t, int limit, bool b = false);
};

/**
 * Class that represents the list
 */
class TermList : public Term {
public:
    // <<< PUBLIC MEMBERS >>>
    // See #L29
    TERM_MEASURELIST(DEFINE_STATIC_MEASURE)

    TermListStates list;

    // <<< CONSTRUCTORS >>>
    NEVER_INLINE TermList(Term_ptr first, bool isCompl);

    // <<< PUBLIC API >>>
    SubsumptionResult IsSubsumedBy(FixpointType& fixpoint, WorklistType& worklist, Term*&, bool no_prune = false);
    bool IsEmpty();

    // <<< DUMPING FUNCTIONS >>>
private:
    void _dumpCore(unsigned indent = 0);

private:
    // <<< PRIVATE FUNCTIONS >>>
    unsigned int _MeasureStateSpaceCore();
    SubsumptionResult _IsSubsumedCore(Term* t, int limit, bool b = false);
    bool _eqCore(const Term&);
};

/**
 * Class representing the fixpoint computation (either classic or pre fixpoint computation)
 */
class TermFixpoint : public Term {
    friend class Workshops::TermWorkshop;
    // <<< MEMBERS >>>
public:
    struct iterator {
    private:
        TermFixpoint &_termFixpoint;
        FixpointType::const_iterator _it;

        Term_ptr _Invalidate() {
            ++_it;
            --_termFixpoint._iteratorNumber;
#           if (OPT_REDUCE_FIXPOINT_EVERYTIME == true)
            _termFixpoint.RemoveSubsumed();
#           endif
            return nullptr;
        }

    public:
        Term_ptr GetNext() {
            assert(!_termFixpoint._fixpoint.empty());
            if(_termFixpoint._fixpoint.cend() == _it) {
                return this->_Invalidate();
            }
            assert(_termFixpoint._fixpoint.cend() != _it);

            FixpointType::const_iterator succIt = _it;
            ++succIt;

            if (_termFixpoint._fixpoint.cend() != succIt) {
                // if we can traverse
                if((*succIt).second) {
                    // fixpoint member is valid
                    return (*++_it).first;
                }  else {
                    ++_it;
                    return this->GetNext();
                }

            } else {
                // we need to refine the fixpoint
                if (E_FIXTERM_FIXPOINT == _termFixpoint.GetSemantics()) {
                    // we need to unfold the fixpoint
                    if (_termFixpoint._worklist.empty()) {
                        // nothing to fold?
                        if(_termFixpoint._postponed.empty()) {
                            // Nothing postponed, we are done
                            return this->_Invalidate();
                        } else {
                            // Take something from the postponed shit
                            if(this->_termFixpoint._processOnePostponed()) {
                                return this->GetNext();
                            } else {
                                return this->_Invalidate();
                            }
                        }
                    } else {
                        _termFixpoint.ComputeNextFixpoint();
                        return this->GetNext();
                    }
                } else {
                    // we need to compute pre of another guy
                    assert(E_FIXTERM_PRE == _termFixpoint.GetSemantics());

                    if (_termFixpoint._worklist.empty()) {
                        Term_ptr term = nullptr;
                        assert(_termFixpoint._sourceIt.get() != nullptr);
                        if ((term = _termFixpoint._sourceIt->GetNext()) != nullptr) {
                            // if more are to be processed
#                           if (DEBUG_RESTRICTION_DRIVEN_FIX == true)
                            std::cout << "ComputeNextPre()\n";
#                           endif
                            for (auto symbol : _termFixpoint._symList) {
#                               if (OPT_WORKLIST_DRIVEN_BY_RESTRICTIONS == true)
#                               if (DEBUG_RESTRICTION_DRIVEN_FIX == true)
                                term->dump(); std::cout << " + " << (*symbol) << " -> ";
#                               endif
                                if(_termFixpoint._guide != nullptr) {
                                    switch(_termFixpoint._guide->GiveTip(term, symbol)) {
                                        case GuideTip::G_FRONT:
#                                           if (DEBUG_RESTRICTION_DRIVEN_FIX == true)
                                            std::cout << "G_FRONT\n";
#                                           endif
                                            _termFixpoint._worklist.insert(_termFixpoint._worklist.cbegin(), std::make_pair(term, symbol));
                                            break;
                                        case GuideTip::G_BACK:
#                                           if (DEBUG_RESTRICTION_DRIVEN_FIX == true)
                                            std::cout << "G_BACK\n";
#                                           endif
                                            _termFixpoint._worklist.push_back(std::make_pair(term, symbol));
                                            break;
                                        case GuideTip::G_THROW:
#                                           if (DEBUG_RESTRICTION_DRIVEN_FIX == true)
                                            std::cout << "G_THROW\n";
#                                           endif
                                            break;
                                        case GuideTip::G_PROJECT:
                                            _termFixpoint._worklist.insert(_termFixpoint._worklist.cbegin(), std::make_pair(term, symbol));
                                            break;
                                        default:
                                            assert(false && "Unsupported guide tip");
                                    }
                                } else {
                                    _termFixpoint._worklist.insert(_termFixpoint._worklist.cbegin(), std::make_pair(term, symbol));
                                }
#                               elif (OPT_FIXPOINT_BFS_SEARCH == true)
                                _termFixpoint._worklist.push_back(std::make_pair(term, symbol));
#                               else
                                _termFixpoint._worklist.insert(_termFixpoint._worklist.cbegin(), std::make_pair(term, symbol));
#                               endif
                            }
                            _termFixpoint.ComputeNextPre();
                            return this->GetNext();
                        } else {
                            // we are complete?
                            if(_termFixpoint._postponed.empty()) {
                                return this->_Invalidate();
                            } else {
                                if(this->_termFixpoint._processOnePostponed()) {
                                    return this->GetNext();
                                } else {
                                    return this->_Invalidate();
                                }
                            }
                        }
                    } else {
                        _termFixpoint.ComputeNextPre();
                        return this->GetNext();
                    }
                }
            }
        }

        explicit iterator(TermFixpoint &termFixpoint) : _termFixpoint(termFixpoint), _it(_termFixpoint._fixpoint.begin()) {
            assert(nullptr != &termFixpoint);
            assert(!_termFixpoint._fixpoint.empty());

            ++_termFixpoint._iteratorNumber;
        }
    };

    // Only for the pre-semantics to link into the source of the pre
protected:
    TermCache _subsumedByCache;             // [36B] << Caching of the subsumption testing
    std::shared_ptr<iterator> _sourceIt;    // [8B] << Source iterator of the pre fixpoint
    FixpointType _fixpoint;                 // [8B] << Fixpoint structure of terms
    TermListType _postponed;                // [8B] << Worklist with postponed terms
    WorklistType _worklist;                 // [8B] << Worklist of the fixpoint
    Symbols _symList;                       // [8B] << List of symbols
    size_t _iteratorNumber = 0;             // [4-8B] << How many iterators are pointing to fixpoint
    Aut_ptr _aut;                           // [4B] << Source automaton
    Term_ptr _sourceTerm;                   // [4B] << Source term of the fixpoint
    Symbol_ptr _sourceSymbol;               // [4B] << Source symbol before breaking to little symboiles
    Term_ptr _satTerm = nullptr;            // [4B] << Satisfiable term of the fixpoint computation
    Term_ptr _unsatTerm = nullptr;          // [4B] << Unsatisfiable term of the fixpoint computation
    bool (*_aggregate_result)(bool, bool);  // [4B] << Agregation function for fixpoint boolean results
    WorklistSearchType _searchType;         // [4B] << Search type for Worklist
    FixpointGuide* _guide = nullptr;        // [4B] << Guide for fixpoints
    bool _bValue;                           // [1B] << Boolean value of the fixpoint testing
    bool _updated = false;                  // [1B] << Flag if the fixpoint was updated during the last unique check
    bool _shortBoolValue;                   // [1B] << Value that leads to early termination fo the fixpoint

public:
    // << STATIC MEASURES >>
    // See #L29
    TERM_MEASURELIST(DEFINE_STATIC_MEASURE)
    static size_t subsumedByHits;
    static size_t preInstances;
    static size_t isNotShared;
    static size_t postponedTerms;
    static size_t postponedProcessed;

    // <<< CONSTRUCTORS >>>
    NEVER_INLINE TermFixpoint(Aut_ptr aut, Term_ptr startingTerm, Symbol* startingSymbol, bool inComplement, bool initbValue, WorklistSearchType search);
    NEVER_INLINE TermFixpoint(Aut_ptr aut, Term_ptr sourceTerm, Symbol* startingSymbol, bool inComplement);
    NEVER_INLINE ~TermFixpoint();

    // <<< PUBLIC API >>>
    FixpointTermSem GetSemantics() const;
    bool IsEmpty();
    SubsumptionResult IsSubsumedBy(FixpointType& fixpoint, WorklistType& worklist, Term*&, bool no_prune = false);
    bool GetResult();
    ExamplePair GetFixpointExamples();
    bool IsFullyComputed() const;
    bool IsShared();
    unsigned int ValidMemberSize() const;
    bool HasEmptyWorklist() { return this->_worklist.empty();}
    void RemoveSubsumed();
    bool TestAndSetUpdate() {
        bool updated = this->_updated;
        this->_updated = false;
        return updated;
    }

    iterator GetIterator() { return iterator(*this); }
    iterator* GetIteratorDynamic() { return new iterator(*this); }

    // <<< DUMPING FUNCTIONS >>>
protected:
    void _dumpCore(unsigned indent = 0);
    static bool _compareSymbols(const TermFixpoint&, const TermFixpoint&);

protected:
    // <<< PRIVATE FUNCTIONS >>>
    void ComputeNextFixpoint();
    void ComputeNextPre();
    bool _processOnePostponed();
    void _updateExamples(ResultType&);
    void _InitializeAggregateFunction(bool inComplement);
    void _InitializeSymbols(Workshops::SymbolWorkshop* form, Gaston::VarList*, IdentList*, Symbol*);
    SubsumptionResult _IsSubsumedCore(Term* t, int limit, bool b = false);
    SubsumptionResult _fixpointTest(Term_ptr const& term);
    SubsumptionResult _testIfSubsumes(Term_ptr const& term);
    SubsumptionResult _testIfIn(Term_ptr const& term);
    SubsumptionResult _testIfBiggerExists(Term_ptr const& term);
    SubsumptionResult _testIfSmallerExists(Term_ptr const& term);
    bool _eqCore(const Term&);
    unsigned int _MeasureStateSpaceCore();
    WorklistItemType _popFromWorklist();
};

#undef DEFINE_STATIC_MEASURE
#endif //WSKS_TERM_H
