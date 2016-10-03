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
#include "../environment.hh"
#include "../mtbdd/ondriks_mtbdd.hh"
#include "../utils/Symbol.h"
#include "../containers/SymbolicAutomata.h"
#include "../containers/FixpointGuide.h"
#include "../containers/TermEnumerator.h"
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

struct FixpointMember {
    Term_ptr term;
    bool isValid;
    size_t level;

    FixpointMember(Term_ptr t, bool i, size_t l) : term(t), isValid(i), level(l) {}

    friend std::ostream &operator<<(std::ostream &stream, const FixpointMember&);
};

//using FixpointMember = std::pair<Term_ptr, bool>;
using FixpointType = std::list<FixpointMember>;
using TermListType = std::list<std::pair<Term_ptr, Term_ptr>>;
using Aut_ptr = SymbolicAutomaton*;

// Fixme: Refactor this, it is shit

struct WorklistItem {
    Term_ptr term;
    SymbolType* symbol;
    size_t level;
    char value;

    WorklistItem(Term_ptr t, SymbolType* s, size_t l, char c) : term(t), symbol(s), level(l), value(c) {}
};

struct SubsumedByParams {
    bool no_prune;
    size_t level;

    SubsumedByParams(size_t l) : no_prune(false), level(l) {}
    SubsumedByParams(bool np, size_t l) : no_prune(np), level(l) {}
};

using WorklistItemType = WorklistItem;
//using WorklistItemType = std::pair<Term_ptr, SymbolType*>;
using WorklistType = std::list<WorklistItemType>;
using Symbols = std::vector<SymbolType*>;
using SymbolParts = std::unordered_map<size_t, char>;

// <<< MACROS FOR ACCESS OF FLAGS >>>
#   define GET_IN_COMPLEMENT(term) (term->_flags & (1 << 1))
#   define SET_IN_COMPLEMENT(term) (term->_flags |= (1 << 1))
#   define FLIP_IN_COMPLEMENT(term) (term->_flags ^= (1 << 1))
#   define GET_NON_MEMBERSHIP_TESTING(term) (term->_flags & 1)
#   define SET_NON_MEMBERSHIP_TESTING(term) (term->_flags |= 1)
#   define SET_VALUE_NON_MEMBERSHIP_TESTING(term, val) (term->_flags |= val)
#   define FLIP_NON_MEMBERSHIP_TESTING(term) (term->_flags ^= 1)
#   define GET_PRODUCT_SUBTYPE(term) ( (term->_flags & (0b11 << 2)) >> 2)
#   define SET_PRODUCT_SUBTYPE(term, pt) (term->_flags |= (static_cast<int>(pt)) << 2) // Note that we assign only once, so this should be ok
#   define GET_IS_INTERMEDIATE(term)  (term->_flags & (1 << 4))
#   define SET_IS_INTERMEDIATE(term)  (term->_flags |= (1 << 4))
#   define RESET_IS_INTERMEDIATE(term)  (term->_flags &= 0xEF)

class Term {
    friend class Workshops::TermWorkshop;

    // <<< MEMBERS >>>
protected:
    Aut_ptr _aut;                           // [4B] << Source automaton
#   if (OPT_ENUMERATED_SUBSUMPTION_TESTING == true)
    EnumSubsumesCache _subsumesCache;   // [36B] << Cache for results of subsumes
#   endif
public:
    struct link_t {                        // [8B*3] << Link for counterexamples
        Term* succ;
        Symbol* symbol;
        size_t len;

        // For little pre
        size_t var;
        std::string val;

        link_t(Term* s, Symbol* sym, size_t l) : succ(s), symbol(sym), len(l), val("") {}

        friend std::ostream &operator<<(std::ostream &out, const link_t &rhs) {
            if (rhs.succ == nullptr) {
                out << " -|";
            } else if(rhs.val == "") {
                out << " -[" << (*rhs.symbol) << "]-> " << rhs.succ;
            } else {
                out << " -['" << rhs.val << "'(" << rhs.var << "-)]-> " << rhs.succ;
            }
        }
    };

    link_t* link;
    size_t stateSpaceApprox = 0;    // [4-8B] << Approximation of the state space, used for heuristics
    TermType type;                  // [4B] << Type of the term
protected:
    char _flags = 0;                // [1B] << Flags with 0: nonMembership, 1: inComplement, 2-3: subtype. 4: is intermediate
public:
    NEVER_INLINE Term(Aut_ptr);
    virtual NEVER_INLINE ~Term();

    // See #L29
    TERM_MEASURELIST(DEFINE_STATIC_MEASURE)
    static size_t partial_subsumption_hits;

public:
    // <<< PUBLIC API >>>
    virtual SubsumedType IsSubsumedBy(FixpointType& fixpoint, WorklistType& worklist, Term*&, SubsumedByParams) = 0;
    virtual SubsumedType IsSubsumed(Term* t, int limit, Term** new_term = nullptr, bool b = false);
    virtual SubsumedType Subsumes(TermEnumerator*);
    virtual bool IsEmpty() = 0;
    virtual void Complement();
    virtual bool InComplement() {return GET_IN_COMPLEMENT(this);}
    bool IsIntermediate() { return GET_IS_INTERMEDIATE(this);}
    void SetIsIntermediate() { SET_IS_INTERMEDIATE(this);}
    void ResetIsIntermediate() { RESET_IS_INTERMEDIATE(this); }
    virtual bool IsSemanticallyValid() = 0;
    bool operator==(const Term &t);
    bool IsNotComputed();
    void SetSuccessor(Term*, Symbol*, IntersectNonEmptyParams&);
    void SetSameSuccesorAs(Term*);

    // <<< MEASURING FUNCTIONS >>>
    virtual unsigned int MeasureStateSpace();

    #if (MEASURE_COMPARISONS == true)
    static void comparedBySamePtr(TermType);
    static void comparedByDifferentType(TermType);
    static void comparedByStructure(TermType, bool);
    #endif

    // <<< DUMPING FUNCTIONS >>>
    virtual void dump(unsigned indent = 0);
    virtual std::string DumpToDot(std::ostream&) = 0;
    static void ToDot(Term*, std::ostream&);
protected:
    // <<< PRIVATE FUNCTIONS >>>
    template<class ProductType>
    SubsumedType _ProductIsSubsumedBy(FixpointType&, WorklistType&, Term*&, SubsumedByParams);

    virtual unsigned int _MeasureStateSpaceCore() = 0;
    virtual SubsumedType _IsSubsumedCore(Term* t, int limit, Term** new_term = nullptr, bool b = false) = 0;
    virtual void _dumpCore(unsigned indent = 0) = 0;
    virtual bool _eqCore(const Term&) = 0;
    virtual SubsumedType _SubsumesCore(TermEnumerator*);

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
    explicit NEVER_INLINE TermEmpty(Aut_ptr aut, bool inComplement=false);
    NEVER_INLINE ~TermEmpty() {}

    // <<< PUBLIC API >>>
    SubsumedType IsSubsumedBy(FixpointType& fixpoint, WorklistType& worklist, Term*&, SubsumedByParams);
    bool IsEmpty();
    bool IsSemanticallyValid();

    // <<< DUMPING FUNCTIONS >>>
    std::string DumpToDot(std::ostream&);
private:
    void _dumpCore(unsigned indent = 0);
    bool _eqCore(const Term&);

    // <<< PRIVATE FUNCTIONS >>>
    unsigned int _MeasureStateSpaceCore();
    SubsumedType _IsSubsumedCore(Term* t, int limit, Term** new_term = nullptr, bool b = false);
};

/**
 * Class that represents the product of two terms, product can be either Intersection or Union. For example
 * {1, 2} x {4, 5}, which can also be implemented as set of pairs
 */
class TermProduct : public Term {
public:
    // <<< PUBLIC MEMBERS >>>
    Term_ptr left;                              // [8B] << Left member of the product
    Term_ptr right;                             // [8B] << Right member of the product
    ProductEnumerator* enumerator = nullptr;    // [8B] << Enumerator through the terms

    // See #L29
    TERM_MEASURELIST(DEFINE_STATIC_MEASURE)

    // <<< CONSTRUCTORS >>>
    NEVER_INLINE TermProduct(Aut_ptr, Term_ptr lhs, Term_ptr rhs, ProductType subtype);
    NEVER_INLINE TermProduct(Aut_ptr aut, std::pair<Term_ptr, Term_ptr> terms, ProductType subType) :
        TermProduct(aut, terms.first, terms.second, subType) {}
    NEVER_INLINE ~TermProduct();

    // <<< PUBLIC API >>>
    SubsumedType IsSubsumedBy(FixpointType& fixpoint, WorklistType& worklist, Term*&, SubsumedByParams);
    bool IsEmpty();
    bool IsSemanticallyValid();

    // <<< DUMPING FUNCTIONS >>>
    std::string DumpToDot(std::ostream&);
private:
    void _dumpCore(unsigned indent = 0);
    bool _eqCore(const Term&);

private:
    // <<< PRIVATE FUNCTIONS >>>
    unsigned int _MeasureStateSpaceCore();
    SubsumedType _IsSubsumedCore(Term* t, int limit, Term** new_term = nullptr, bool b = false);
    SubsumedType _SubsumesCore(TermEnumerator*);
};

class TermTernaryProduct : public Term {
public:
    /// <<< PUBLIC MEMBERS >>>
    Term_ptr left;
    Term_ptr middle;
    Term_ptr right;
    TernaryProductEnumerator* enumerator = nullptr;

    // See #L29
    TERM_MEASURELIST(DEFINE_STATIC_MEASURE)

    // <<< CONSTRUCTORS >>>
    NEVER_INLINE TermTernaryProduct(Aut_ptr, Term_ptr, Term_ptr, Term_ptr, ProductType);
    NEVER_INLINE TermTernaryProduct(Aut_ptr aut, std::tuple<Term_ptr, Term_ptr, Term_ptr> productTripple, ProductType type) :
        TermTernaryProduct(aut, std::get<0>(productTripple), std::get<1>(productTripple), std::get<2>(productTripple), type)
            {}
    NEVER_INLINE ~TermTernaryProduct();

    // <<< PUBLIC API >>>
    SubsumedType IsSubsumedBy(FixpointType&, WorklistType& worklist, Term*&, SubsumedByParams);
    bool IsEmpty();
    bool IsSemanticallyValid();

    // <<< DUMPING FUNCTIONS >>>
    std::string DumpToDot(std::ostream&);
private:
    void _dumpCore(unsigned indent = 0);
    bool _eqCore(const Term&);

    // <<< PRIVATE FUNCTIONS >>>
    unsigned int _MeasureStateSpaceCore();
    SubsumedType _IsSubsumedCore(Term* t, int limit, Term** new_term = nullptr, bool b = false);
    SubsumedType _SubsumesCore(TermEnumerator*);
};

class TermNaryProduct : public Term {
public:
    // <<< PUBLIC MEMBERS >>>
    size_t arity;
    Term_ptr* terms;
    size_t* access_vector;
    NaryProductEnumerator* enumerator = nullptr;

    // See #L29
    TERM_MEASURELIST(DEFINE_STATIC_MEASURE)

    // <<< CONSTRUCTORS >>>
    NEVER_INLINE TermNaryProduct(Aut_ptr, Term_ptr*, ProductType, size_t);
    NEVER_INLINE TermNaryProduct(Aut_ptr, SymLink*, StatesSetType, ProductType, size_t); // NaryBase
    NEVER_INLINE TermNaryProduct(Aut_ptr, Term_ptr, Symbol_ptr, ProductType, size_t); // NaryPre
    NEVER_INLINE TermNaryProduct(Aut_ptr aut, Term_ptr* links, std::pair<ProductType, size_t> config) :
        TermNaryProduct(aut, links, config.first, config.second) {}
    NEVER_INLINE TermNaryProduct(Aut_ptr aut, SymLink* links, std::tuple<StatesSetType, ProductType, size_t> config) :
        TermNaryProduct(aut, links, std::get<0>(config), std::get<1>(config), std::get<2>(config)) {}
    NEVER_INLINE TermNaryProduct(Aut_ptr aut, std::pair<Term_ptr, Symbol_ptr> base, std::pair<ProductType, size_t> config) :
        TermNaryProduct(aut, base.first, base.second, config.first, config.second) {}
    NEVER_INLINE ~TermNaryProduct();

    // <<< PUBLIC API >>>
    SubsumedType IsSubsumedBy(FixpointType&, WorklistType& worklist, Term*&, SubsumedByParams);
    bool IsEmpty();
    bool IsSemanticallyValid();
    Term_ptr operator[](size_t);

    // <<< DUMPING FUNCTIONS >>>
    std::string DumpToDot(std::ostream&);
private:
    void _InitNaryProduct(ProductType, size_t);
    void _dumpCore(unsigned indent = 0);
    bool _eqCore(const Term&);

    // <<< PRIVATE FUNCTIONS >>>
    unsigned int _MeasureStateSpaceCore();
    SubsumedType _IsSubsumedCore(Term* t, int limit, Term** new_term = nullptr, bool b = false);
    SubsumedType _SubsumesCore(TermEnumerator*);
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
    NEVER_INLINE TermBaseSet(Aut_ptr, VATA::Util::OrdVector<size_t> &&);
    NEVER_INLINE TermBaseSet(Aut_ptr aut, VATA::Util::OrdVector<size_t> &states)
        : TermBaseSet(aut, std::move(states)) {}
    NEVER_INLINE ~TermBaseSet();

    // <<< PUBLIC API >>>
    bool IsAccepting();
    SubsumedType IsSubsumedBy(FixpointType& fixpoint, WorklistType& worklist, Term*&, SubsumedByParams);
    bool IsEmpty();
    bool IsSemanticallyValid();

    // <<< DUMPING FUNCTIONS >>
    std::string DumpToDot(std::ostream&);
private:
    void _dumpCore(unsigned indent = 0);
    bool _eqCore(const Term&);

private:
    // <<< PRIVATE FUNCTIONS >>>
    unsigned int _MeasureStateSpaceCore();
    SubsumedType _IsSubsumedCore(Term* t, int limit, Term** newTerm = nullptr, bool b = false);
    SubsumedType _SubsumesCore(TermEnumerator*);
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
    NEVER_INLINE TermContinuation(Aut_ptr, SymLink*, SymbolicAutomaton*, Term*, SymbolType*, bool, bool lazy = false);

    // <<< PUBLIC API >>>
    SubsumedType IsSubsumedBy(FixpointType& fixpoint, WorklistType& worklist, Term*&, SubsumedByParams);
    bool IsUnfolded() {return this->_unfoldedTerm != nullptr;}
    bool IsEmpty();
    bool IsSemanticallyValid();
    Term* GetUnfoldedTerm() {return this->_unfoldedTerm; }
    Term* unfoldContinuation(UnfoldedIn);

    std::string DumpToDot(std::ostream&);
protected:
    // <<< DUMPING FUNCTIONS >>>
    void _dumpCore(unsigned indent = 0);

    // <<< PRIVATE FUNCTIONS >>>
    unsigned int _MeasureStateSpaceCore();
    bool _eqCore(const Term&);
    SubsumedType _IsSubsumedCore(Term* t, int limit, Term** new_term = nullptr, bool b = false);
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
    NEVER_INLINE TermList(Aut_ptr, Term_ptr first, bool isCompl);

    // <<< PUBLIC API >>>
    SubsumedType IsSubsumedBy(FixpointType& fixpoint, WorklistType& worklist, Term*&, SubsumedByParams);
    bool IsEmpty();
    bool IsSemanticallyValid();

    // <<< DUMPING FUNCTIONS >>>
    std::string DumpToDot(std::ostream&);
private:
    void _dumpCore(unsigned indent = 0);

private:
    // <<< PRIVATE FUNCTIONS >>>
    unsigned int _MeasureStateSpaceCore();
    SubsumedType _IsSubsumedCore(Term* t, int limit, Term** new_term = nullptr, bool b = false);
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
                if((*succIt).isValid && (*succIt).term != nullptr) {
                    // Fixme: Fuck this fucking stinks, if this is base fixpoint, we should not
                    // Fixme: return the intermediate stuff!
                    // fixpoint member is valid
                    if(_termFixpoint.GetSemantics() == FixpointSemanticType::FIXPOINT && (*succIt).level != 0) {
                        ++_it;
                        return this->GetNext();
                    } else {
                        return (*++_it).term;
                    }
                }  else {
                    ++_it;
                    return this->GetNext();
                }

            } else {
                // we need to refine the fixpoint
                if (FixpointSemanticType::FIXPOINT == _termFixpoint.GetSemantics()) {
                    // we need to unfold the fixpoint
                    if (_termFixpoint._worklist.empty()) {
                        // nothing to fold?
#                       if (OPT_EARLY_EVALUATION == true)
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
#                       else
                        return this->_Invalidate();
#                       endif
                    } else {
                        _termFixpoint.ComputeNextMember();
                        return this->GetNext();
                    }
                } else {
                    // we need to compute pre of another guy
                    assert(FixpointSemanticType::PRE == _termFixpoint.GetSemantics());

                    if (_termFixpoint._worklist.empty()) {
                        Term_ptr term = nullptr;
                        assert(_termFixpoint._sourceIt.get() != nullptr);
                        if ((term = _termFixpoint._sourceIt->GetNext()) != nullptr) {
                            // if more are to be processed
                            IntersectNonEmptyParams params(_termFixpoint.InComplement());
                            params.limitPre = OPT_INCREMENTAL_LEVEL_PRE;
#                           if (OPT_INCREMENTAL_LEVEL_PRE == true)
                            params.variableLevel = _termFixpoint._symbolPart.level;
                            assert(params.variableLevel != varMap.TrackLength() - 1 || !term->IsIntermediate());
                            params.variableValue = _termFixpoint._symbolPart.value;
#                           endif
                            _termFixpoint._EnqueueInWorklist(term, params, false);
                            _termFixpoint.ComputeNextMember(false);
                            return this->GetNext();
                        } else {
                            // we are complete?
#                           if (MEASURE_PROJECTION == true)
                            if(!_termFixpoint._fullyComputed) {
                                _termFixpoint._fullyComputed = true;
                                ++TermFixpoint::fullyComputedFixpoints;
                            }
#                           endif
#                           if (OPT_EARLY_EVALUATION == true)
                            if(_termFixpoint._postponed.empty()) {
                                return this->_Invalidate();
                            } else {
                                if(this->_termFixpoint._processOnePostponed()) {
                                    return this->GetNext();
                                } else {
                                    return this->_Invalidate();
                                }
                            }
#                           else
                            return this->_Invalidate();
#                           endif
                        }
                    } else {
                        _termFixpoint.ComputeNextMember(false);
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
#   if (OPT_EARLY_EVALUATION == true)
    TermListType _postponed;                // [8B] << Worklist with postponed terms
#   endif
    WorklistType _worklist;                 // [8B] << Worklist of the fixpoint
    Symbols _symList;                       // [8B] << List of symbols
    size_t _iteratorNumber = 0;             // [4-8B] << How many iterators are pointing to fixpoint
    Symbol_ptr _projectedSymbol;            // [4B] << Source symbol with projected vars
    Aut_ptr _baseAut;
    Term_ptr _sourceTerm;                   // [4B] << Source term of the fixpoint
    Symbol_ptr _sourceSymbol;               // [4B] << Source symbol before breaking to little symboiles
    struct {
        size_t level = 0;
        char value = 'I';
    } _symbolPart;
    Term_ptr _satTerm = nullptr;            // [4B] << Satisfiable term of the fixpoint computation
    Term_ptr _unsatTerm = nullptr;          // [4B] << Unsatisfiable term of the fixpoint computation
    FixpointGuide* _guide = nullptr;        // [4B] << Guide for fixpoints
    WorklistSearchType _searchType;         // [4B] << Search type for Worklist
    bool _bValue;                           // [1B] << Boolean value of the fixpoint testing
    bool _updated = false;                  // [1B] << Flag if the fixpoint was updated during the last unique check
    bool _shortBoolValue;                   // [1B] << Value that leads to early termination fo the fixpoint
    bool _fullyComputed = false;            // [1B] << Whether the fixpoint is fully computed

public:
    // << STATIC MEASURES >>
    // See #L29
    TERM_MEASURELIST(DEFINE_STATIC_MEASURE)
    static size_t subsumedByHits;
    static size_t preInstances;
    static size_t isNotShared;
    static size_t postponedTerms;
    static size_t postponedProcessed;
    static size_t fullyComputedFixpoints;

    // <<< CONSTRUCTORS >>>
    NEVER_INLINE TermFixpoint(Aut_ptr aut, Term_ptr startingTerm, Symbol* startingSymbol, bool inComplement, bool initbValue, WorklistSearchType search);
    NEVER_INLINE TermFixpoint(Aut_ptr aut, Term_ptr sourceTerm, Symbol* startingSymbol, bool inComplement);
    // Fixme: Change to (size_t, value)?
    NEVER_INLINE TermFixpoint(Aut_ptr aut, Term_ptr sourceTerm, Symbol* startingSymbol, size_t level, char value, bool inComplement) :
        TermFixpoint(aut, sourceTerm, startingSymbol, inComplement) {
        this->_symbolPart.level = level;
        this->_symbolPart.value = value;
    }

    NEVER_INLINE TermFixpoint(Aut_ptr aut, std::pair<Term_ptr, Symbol*> startingPair, bool inComplement) :
        TermFixpoint(aut, startingPair.first, startingPair.second, inComplement) {}
    NEVER_INLINE TermFixpoint(Aut_ptr aut, std::tuple<Term_ptr, Symbol*, size_t, char> startingTuple, bool inComplement) :
        TermFixpoint(aut, std::get<0>(startingTuple), std::get<1>(startingTuple), std::get<2>(startingTuple), std::get<3>(startingTuple), inComplement) {}
    NEVER_INLINE TermFixpoint(Aut_ptr aut, std::tuple<Term_ptr, Symbol*, bool, bool, WorklistSearchType> initTuple) :
        TermFixpoint(aut, std::get<0>(initTuple), std::get<1>(initTuple), std::get<2>(initTuple), std::get<3>(initTuple),
            std::get<4>(initTuple)) {}
    NEVER_INLINE ~TermFixpoint();

    // <<< PUBLIC API >>>
    FixpointSemanticType GetSemantics() const;
    bool IsEmpty();
    bool IsSemanticallyValid();
    SubsumedType IsSubsumedBy(FixpointType& fixpoint, WorklistType& worklist, Term*&, SubsumedByParams);
    bool GetResult();
    ExamplePair GetFixpointExamples();
    bool IsFullyComputed() const;
    bool IsShared();
    unsigned int ValidMemberSize() const;
    bool HasEmptyWorklist() { return this->_worklist.empty();}
    void RemoveSubsumed();
    void RemoveIntermediate();
    void PushAndCompute(IntersectNonEmptyParams&);
    void ForcefullyComputeIntermediate(bool isBaseFixpoint);
    bool TestAndSetUpdate() {
        bool updated = this->_updated;
        this->_updated = false;
        return updated;
    }

    iterator GetIterator() { return iterator(*this); }
    iterator* GetIteratorDynamic() { return new iterator(*this); }

    // <<< DUMPING FUNCTIONS >>>
    std::string DumpToDot(std::ostream&);
protected:
    void _dumpCore(unsigned indent = 0);
    static bool _compareSymbols(const TermFixpoint&, const TermFixpoint&);

protected:
    // <<< PRIVATE FUNCTIONS >>>
    void ComputeNextMember(bool isBaseFixpoint = true);
    void _ProcessComputedResult(std::pair<Term_ptr, bool>& result, bool, IntersectNonEmptyParams& params);
    bool _processOnePostponed();
    void _updateExamples(ResultType&);
    void _InitializeAggregateFunction(bool inComplement);
    bool _AggregateResult(bool, bool);
    void _InitializeSymbols(Workshops::SymbolWorkshop* form, Gaston::VarList*, IdentList*, Symbol*);
    void _InitializeProjectedSymbol(Workshops::SymbolWorkshop* form, Gaston::VarList*, IdentList*, Symbol*);
    void _EnqueueInWorklist(Term_ptr, IntersectNonEmptyParams&, bool enqueueNext = true);
    void _EnqueueSingleLevelInWorklist(Term_ptr, size_t, char);
    SubsumedType _IsSubsumedCore(Term* t, int limit, Term** new_term = nullptr, bool b = false);
    std::pair<SubsumedType, Term_ptr> _fixpointTest(Term_ptr const& term, SubsumedByParams);
    std::pair<SubsumedType, Term_ptr> _testIfSubsumes(Term_ptr const& term, SubsumedByParams);
    std::pair<SubsumedType, Term_ptr> _testIfIn(Term_ptr const& term, SubsumedByParams);
    std::pair<SubsumedType, Term_ptr> _testIfBiggerExists(Term_ptr const& term, SubsumedByParams);
    std::pair<SubsumedType, Term_ptr> _testIfSmallerExists(Term_ptr const& term, SubsumedByParams);
    bool _eqCore(const Term&);
    unsigned int _MeasureStateSpaceCore();
    WorklistItemType _popFromWorklist();
};

#undef DEFINE_STATIC_MEASURE
#endif //WSKS_TERM_H
