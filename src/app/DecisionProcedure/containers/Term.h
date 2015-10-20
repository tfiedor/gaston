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

// <<< FORWARD CLASS DECLARATION >>>
class SymbolicAutomaton;

// TODO: Move away the usings
using Term_ptr          = std::shared_ptr<Term>;
using TermProductStates = std::pair<Term_ptr, Term_ptr>;
using TermListStates    = std::vector<Term_ptr>;
using BaseState         = size_t;
using TermBaseSetStates = std::vector<BaseState>;
using ResultType        = std::pair<Term_ptr, bool>;
using SymbolType        = ZeroSymbol;

class Term {
public:
    TermType type;

    // <<< PUBLIC API >>>
    virtual bool IsSubsumedBy(std::list<Term_ptr>& fixpoint) = 0;
    virtual bool IsSubsumed(Term* t) = 0;
    virtual bool IsEmpty() = 0;

    // <<< MEASURING FUNCTIONS >>>
    virtual unsigned int MeasureStateSpace() = 0;

    // <<< DUMPING FUNCTIONS >>>
    virtual void dump() = 0;
};

class TermProduct : public Term {
public:
    // <<< PUBLIC MEMBERS >>>
    static int instances;
    Term_ptr left;
    Term_ptr right;

    // <<< CONSTRUCTORS >>>
    TermProduct(Term_ptr lhs, Term_ptr rhs);
    TermProduct(Term_ptr lhs, Term_ptr rhs, TermType t);

    // <<< PUBLIC API >>>
    bool IsSubsumed(Term* t);
    bool IsSubsumedBy(std::list<Term_ptr>& fixpoint);
    bool IsEmpty();

    // <<< MEASURING FUNCTIONS >>>
    unsigned int MeasureStateSpace();

    // <<< DUMPING FUNCTIONS >>>
    void dump();
};

class TermBaseSet : public Term {
public:
    // <<< PUBLIC MEMBERS >>>
    static int instances;
    TermBaseSetStates states;

    // <<< CONSTRUCTORS >>>
    TermBaseSet();
    TermBaseSet(TermBaseSetStates& s);
    TermBaseSet(VATA::Util::OrdVector<unsigned int>& s);

    // <<< PUBLIC API >>>
    bool Intersects(TermBaseSet* rhs);
    bool IsSubsumedBy(std::list<Term_ptr>& fixpoint);
    bool IsEmpty();
    bool IsSubsumed(Term* term);

    // <<< MEASURING FUNCTIONS >>>
    unsigned int MeasureStateSpace();

    // <<< DUMPING FUNCTIONS >>>
    void dump();
};

class TermContProduct : public Term {
public:
    // <<< PUBLIC MEMBERS >>>
    static int instances;

    std::shared_ptr<SymbolicAutomaton> aut;
    Term_ptr term;
    std::shared_ptr<SymbolType> symbol;

    // <<< CONSTRUCTORS >>>
    TermContProduct(std::shared_ptr<SymbolicAutomaton> a, Term_ptr t, std::shared_ptr<SymbolType> s);

    // <<< PUBLIC API >>>
    bool IsSubsumedBy(std::list<Term_ptr>& fixpoint);
    bool IsSubsumed(Term *t);
    bool IsEmpty();

    // <<< MEASURING FUNCTIONS >>>
    unsigned int MeasureStateSpace();

    // <<< DUMPING FUNCTIONS >>>
    void dump();
};

class TermContSubset : public Term {
public:
    // <<< PUBLIC MEMBERS >>>
    static int instances;

    std::shared_ptr<SymbolicAutomaton> aut;
    Term_ptr term;
    std::shared_ptr<SymbolType> symbol;

    // <<< CONSTRUCTORS >>>
    TermContSubset(std::shared_ptr<SymbolicAutomaton> a, Term_ptr t, std::shared_ptr<SymbolType> s);

    // <<< PUBLIC API >>>
    bool IsSubsumedBy(std::list<Term_ptr>& fixpoint);
    bool IsSubsumed(Term *t);
    bool IsEmpty();

    // <<< MEASURING FUNCTIONS >>>
    unsigned int MeasureStateSpace();

    // <<< DUMPING FUNCTIONS >>>
    void dump();
};

class TermList : public Term {
public:
    // <<< PUBLIC MEMBERS >>>
    static int instances;

    TermListStates list;
    bool isComplement;

    // <<< CONSTRUCTORS >>>
    TermList();
    TermList(Term_ptr first, bool isCompl);
    TermList(Term_ptr f, Term_ptr s, bool isCompl);

    // <<< PUBLIC API >>>
    bool IsSubsumedBy(std::list<Term_ptr>& fixpoint);
    bool IsSubsumed(Term* t);
    bool IsEmpty();

    // <<< MEASURING FUNCTIONS >>>
    unsigned int MeasureStateSpace();

    // <<< DUMPING FUNCTIONS >>>
    void dump();
};

class TermFixpoint : public Term {
public:
    using FixpointType = std::list<Term_ptr>;
    using Aut_ptr = std::shared_ptr<SymbolicAutomaton>;

    using WorklistItemType = std::pair<Term_ptr, SymbolType>;
    using WorklistType = std::list<WorklistItemType>;
    using Symbols = std::list<SymbolType>;

    // <<< PUBLIC MEMBERS >>>
    static int instances;
    struct iterator {
    private:
        TermFixpoint &_termFixpoint;
        FixpointType::const_iterator _it;

    public:
        Term_ptr GetNext() {
            assert(!_termFixpoint._fixpoint.empty());
            // TODO: Not sure if this is valid
            if(_termFixpoint._fixpoint.cend() == _it) {
                return nullptr;
            }
            assert(_termFixpoint._fixpoint.cend() != _it);

            FixpointType::const_iterator succIt = _it;
            ++succIt;

            if (_termFixpoint._fixpoint.cend() != succIt) {
                // if we can traverse
                return *(++_it);
            } else {
                // we need to refine the fixpoint
                if (E_FIXTERM_FIXPOINT == _termFixpoint.GetSemantics()) {
                    // we need to unfold the fixpoint
                    if (_termFixpoint._worklist.empty()) {
                        ++_it;

                        return nullptr;
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
                            // we are complete;
                            ++_it;
                            // TODO: kill something and make it behave like a fixpoint semantics

                            return nullptr;
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
        }
    };

    // Only for the pre-semantics to link into the source of the pre
    Term_ptr _sourceTerm;
    std::shared_ptr<iterator> _sourceIt;

    Aut_ptr _aut;
    FixpointType _fixpoint;
    WorklistType _worklist;
    Symbols _symList;
    bool _bValue;
    bool _inComplement;
    bool (*_aggregate_result)(bool, bool);

    // <<< CONSTRUCTORS >>>
    TermFixpoint(std::shared_ptr<SymbolicAutomaton> aut,Term_ptr startingTerm, Symbols symList, bool inComplement, bool initbValue);
    TermFixpoint(std::shared_ptr<SymbolicAutomaton> aut, Term_ptr sourceTerm, Symbols symList, bool inComplement);

    // <<< PUBLIC API >>>
    FixpointTermSem GetSemantics() const;
    bool IsEmpty();
    bool IsSubsumedBy(std::list<Term_ptr>& fixpoint);
    bool IsSubsumed(Term* t);
    bool GetResult();

    iterator GetIterator() { return iterator(*this); }
    iterator* GetIteratorDynamic() { return new iterator(*this); }

    // <<< MEASURING FUNCTIONS >>>
    unsigned int MeasureStateSpace();

    // <<< DUMPING FUNCTIONS >>>
    void dump();

private:
    // <<< PRIVATE FUNCTIONS >>>
    void ComputeNextFixpoint();
    void ComputeNextPre();
    void _InitializeAggregateFunction(bool inComplement);
};
#endif //WSKS_TERM_H
