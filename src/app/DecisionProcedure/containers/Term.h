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
#include "../environment.hh"
#include "../containers/Workshops.h"

// <<< MACROS >>>

#define TERM_TYPELIST(code, var) \
    code(Term, var)              \
    code(TermEmpty, var)        \
	code(TermProduct, var)		\
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

// TODO: Move away the usings
using Term_ptr          = Term*;
using TermProductStates = std::pair<Term_ptr, Term_ptr>;
using TermListStates    = std::vector<Term_ptr>;
using BaseState         = size_t;
using TermBaseSetStates = std::vector<BaseState>;
using ResultType        = std::pair<Term_ptr, bool>;
using SymbolType        = ZeroSymbol;

//using FixpointMember = std::pair<Term_ptr, bool>;
using FixpointMember = std::pair<Term_ptr, bool>;
using FixpointType = std::list<FixpointMember>;
using TermListType = std::list<std::pair<Term_ptr, Term_ptr>>;
using Aut_ptr = SymbolicAutomaton*;

using WorklistItemType = std::pair<Term_ptr, SymbolType*>;
using WorklistType = std::list<WorklistItemType>;
using Symbols = std::list<SymbolType*>;


class Term {
    friend class Workshops::TermWorkshop;
public:
    TermType type;
    size_t stateSpace = 0;         // << Exact size of the state space, 0 if unknown
    size_t stateSpaceApprox = 0;   // << Approximation of the state space, used for heuristics
    bool valid = true;

    // See #L29
    TERM_MEASURELIST(DEFINE_STATIC_MEASURE)

protected:
    // <<< PRIVATE MEMBERS >>>
    bool _nonMembershipTesting;
    bool _inComplement;
    TermCache _isSubsumedCache;

public:
    // <<< PUBLIC API >>>
    virtual SubsumptionResult IsSubsumedBy(FixpointType& fixpoint, Term*&) = 0;
    virtual SubsumptionResult IsSubsumed(Term* t, bool b = false);
    virtual bool IsEmpty() = 0;
    virtual void Complement();
    virtual bool InComplement() {return this->_inComplement;}
    bool operator==(const Term &t);
    bool IsNotComputed();

    // <<< MEASURING FUNCTIONS >>>
    virtual unsigned int MeasureStateSpace();

    #if (MEASURE_COMPARISONS == true)
    static void comparedBySamePtr(TermType);
    static void comparedByDifferentType(TermType);
    static void comparedByStructure(TermType, bool);
    #endif

    // <<< DUMPING FUNCTIONS >>>
    virtual void dump();
protected:
    // <<< PRIVATE FUNCTIONS >>>
    virtual unsigned int _MeasureStateSpaceCore() = 0;
    virtual SubsumptionResult _IsSubsumedCore(Term* t, bool b = false) = 0;
    virtual void _dumpCore() = 0;
    virtual bool _eqCore(const Term&) = 0;

    friend size_t hash_value(Term* s);
    friend std::ostream& operator <<(std::ostream& osObject, Term& z);
};

class TermEmpty : public Term {
public:
    // See #L29
    TERM_MEASURELIST(DEFINE_STATIC_MEASURE)

    // <<< CONSTRUCTORS >>>
    TermEmpty(bool inComplement=false);

    // <<< PUBLIC API >>>
    SubsumptionResult IsSubsumedBy(FixpointType& fixpoint, Term*&);
    bool IsEmpty();

    // <<< DUMPING FUNCTIONS >>>
private:
    void _dumpCore();
    bool _eqCore(const Term&);

    // <<< PRIVATE FUNCTIONS >>>
    unsigned int _MeasureStateSpaceCore();
    SubsumptionResult _IsSubsumedCore(Term* t, bool b = false);
};

class TermProduct : public Term {
public:
    // <<< PUBLIC MEMBERS >>>
    Term_ptr left;
    size_t leftSubFalse = 0;
    Term_ptr right;
    size_t rightSubFalse = 0;
    ProductType subtype;
    // See #L29
    TERM_MEASURELIST(DEFINE_STATIC_MEASURE)

    // <<< CONSTRUCTORS >>>
    TermProduct(Term_ptr lhs, Term_ptr rhs, ProductType subtype);

    // <<< PUBLIC API >>>
    SubsumptionResult IsSubsumedBy(FixpointType& fixpoint, Term*&);
    bool IsEmpty();

    // <<< DUMPING FUNCTIONS >>>
private:
    void _dumpCore();
    bool _eqCore(const Term&);

private:
    // <<< PRIVATE FUNCTIONS >>>
    unsigned int _MeasureStateSpaceCore();
    SubsumptionResult _IsSubsumedCore(Term* t, bool b = false);
};

class TermBaseSet : public Term {
public:
    // <<< PUBLIC MEMBERS >>>
    TermBaseSetStates states;
    BitMask stateMask;
    // See #L29
    TERM_MEASURELIST(DEFINE_STATIC_MEASURE)

    // <<< CONSTRUCTORS >>>
    TermBaseSet(VATA::Util::OrdVector<unsigned int>&, unsigned int, unsigned int);

    // <<< PUBLIC API >>>
    bool Intersects(TermBaseSet* rhs);
    SubsumptionResult IsSubsumedBy(FixpointType& fixpoint, Term*&);
    bool IsEmpty();

    // <<< DUMPING FUNCTIONS >>>
private:
    void _dumpCore();
    bool _eqCore(const Term&);

private:
    // <<< PRIVATE FUNCTIONS >>>
    unsigned int _MeasureStateSpaceCore();
    SubsumptionResult _IsSubsumedCore(Term* t, bool b = false);
};

class TermContinuation : public Term {
protected:
    Term* _unfoldedTerm = nullptr;

public:
    // <<< PUBLIC MEMBERS >>>
    SymbolicAutomaton* aut;
    Term* term;
    SymbolType* symbol;
    bool underComplement;
    // See #L29
    TERM_MEASURELIST(DEFINE_STATIC_MEASURE)
    static size_t continuationUnfolding;
    static size_t unfoldInSubsumption;
    static size_t unfoldInIsectNonempty;

    // <<< CONSTRUCTORS >>>
    TermContinuation(SymbolicAutomaton*, Term*, SymbolType*, bool);

    // <<< PUBLIC API >>>
    SubsumptionResult IsSubsumedBy(FixpointType& fixpoint, Term*&);
    bool IsUnfolded() {return this->_unfoldedTerm != nullptr;}
    bool IsEmpty();
    Term* GetUnfoldedTerm() {return this->_unfoldedTerm; }
    Term* unfoldContinuation(UnfoldedInType);

protected:
    // <<< DUMPING FUNCTIONS >>>
    void _dumpCore();

    // <<< PRIVATE FUNCTIONS >>>
    unsigned int _MeasureStateSpaceCore();
    bool _eqCore(const Term&);
    SubsumptionResult _IsSubsumedCore(Term* t, bool b = false);
};

class TermList : public Term {
public:
    // <<< PUBLIC MEMBERS >>>
    // See #L29
    TERM_MEASURELIST(DEFINE_STATIC_MEASURE)

    TermListStates list;

    // <<< CONSTRUCTORS >>>
    TermList(Term_ptr first, bool isCompl);

    // <<< PUBLIC API >>>
    SubsumptionResult IsSubsumedBy(FixpointType& fixpoint, Term*&);
    bool IsEmpty();

    // <<< DUMPING FUNCTIONS >>>
private:
    void _dumpCore();

private:
    // <<< PRIVATE FUNCTIONS >>>
    unsigned int _MeasureStateSpaceCore();
    SubsumptionResult _IsSubsumedCore(Term* t, bool b = false);
    bool _eqCore(const Term&);
};

class TermFixpoint : public Term {
    friend class Workshops::TermWorkshop;
public:
    // <<< PUBLIC MEMBERS >>>
    // See #L29
    TERM_MEASURELIST(DEFINE_STATIC_MEASURE)
    static size_t subsumedByHits;
    static size_t preInstances;
    static size_t isNotShared;
    static size_t postponedTerms;
    static size_t postponedProcessed;

    struct iterator {
    private:
        TermFixpoint &_termFixpoint;
        FixpointType::const_iterator _it;

        Term_ptr _Invalidate() {
            ++_it;
            --_termFixpoint._iteratorNumber;
            #if (OPT_REDUCE_FIXPOINT_EVERYTIME == true)
                _termFixpoint.RemoveSubsumed();
            #endif
            return nullptr;
        }

    public:
        Term_ptr GetNext() {
            assert(!_termFixpoint._fixpoint.empty());
            // TODO: Not sure if this is valid
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
                            for (auto symbol : _termFixpoint._symList) {
                                _termFixpoint._worklist.insert(_termFixpoint._worklist.cbegin(), std::make_pair(term, symbol));
                            }
                            _termFixpoint.ComputeNextPre();
                            return this->GetNext();
                        } else {
                            // we are complete?
                            // TODO: Add taking things from postponed
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

        iterator(TermFixpoint &termFixpoint) : _termFixpoint(termFixpoint), _it(_termFixpoint._fixpoint.begin()) {
            assert(nullptr != &termFixpoint);
            assert(!_termFixpoint._fixpoint.empty());

            ++_termFixpoint._iteratorNumber;
        }
    };

    // Only for the pre-semantics to link into the source of the pre
protected:
    Term_ptr _sourceTerm;
    std::shared_ptr<iterator> _sourceIt;
    TermCache _subsumedByCache;

    size_t _iteratorNumber = 0;
    Aut_ptr _aut;
    FixpointType _fixpoint;
    TermListType _postponed;
    WorklistType _worklist;
    Symbols _symList;
    bool _bValue;
    bool (*_aggregate_result)(bool, bool);

public:
    // <<< CONSTRUCTORS >>>
    TermFixpoint(Aut_ptr aut, Term_ptr startingTerm, Symbol* startingSymbol, bool inComplement, bool initbValue);
    TermFixpoint(Aut_ptr aut, Term_ptr sourceTerm, Symbol* startingSymbol, bool inComplement);
    ~TermFixpoint();

    // <<< PUBLIC API >>>
    FixpointTermSem GetSemantics() const;
    bool IsEmpty();
    SubsumptionResult IsSubsumedBy(FixpointType& fixpoint, Term*&);
    bool GetResult();
    bool IsFullyComputed() const;
    bool IsShared();
    void RemoveSubsumed();

    iterator GetIterator() { return iterator(*this); }
    iterator* GetIteratorDynamic() { return new iterator(*this); }

    // <<< DUMPING FUNCTIONS >>>
protected:
    void _dumpCore();

protected:
    // <<< PRIVATE FUNCTIONS >>>
    void ComputeNextFixpoint();
    void ComputeNextPre();
    bool _processOnePostponed();
    void _InitializeAggregateFunction(bool inComplement);
    void _InitializeSymbols(Workshops::SymbolWorkshop* workshop, IdentList*, Symbol*);
    SubsumptionResult _IsSubsumedCore(Term* t, bool b = false);
    SubsumptionResult _testIfSubsumes(Term_ptr const& term);
    bool _eqCore(const Term&);
    unsigned int _MeasureStateSpaceCore();
};

#undef DEFINE_STATIC_MEASURE
#endif //WSKS_TERM_H
