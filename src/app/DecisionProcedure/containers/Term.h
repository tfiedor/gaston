//
// Created by Raph on 10/10/2015.
//

#ifndef WSKS_TERM_H
#define WSKS_TERM_H

#include <vector>
#include <list>
#include <algorithm>
#include "../utils/Symbol.h"
#include "../mtbdd/ondriks_mtbdd.hh"
#include "../containers/SymbolicAutomata.h"

enum TermType {TERM_FIXPOINT, TERM_PRODUCT, TERM_UNION, TERM_BASE, TERM_LIST, TERM_CONT_ISECT, TERM_CONT_SUBSET};

class SymbolicAutomaton;

// < Usings >
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
    virtual void dump() = 0;
    virtual bool IsSubsumed(Term* t) = 0;
    virtual bool IsSubsumedBy(std::list<Term_ptr>& fixpoint) = 0;
    virtual bool IsEmpty() = 0;
};
// Wow such clean!

class TermList : public Term {
public:
    TermListStates list;

    bool IsSubsumedBy(std::list<Term_ptr>& fixpoint) {
        assert(false && "TermList.IsSubsumedBy() is not implemented yet~!");
    }

    void dump() {
        std::cout << "{";
        for(auto state : this->list) {
            state->dump();
            std::cout  << ",";
        }
        std::cout << "}";
    }

    TermList() {type = TERM_LIST;}

    TermList(Term_ptr first) {
        this->list.push_back(first);
    }

    TermList(Term_ptr f, Term_ptr s) {
        this->list.push_back(f);
        this->list.push_back(s);
    }

    bool IsSubsumed(Term* t) {
        // TODO:
        return false;
    }

    bool IsEmpty() {
        return false;
        std::cout << "this->list.size() = " << this->list.size() << "\n";
        return this->list.size() == 0 ||
               this->list.size() == 1 && this->list[0]->IsEmpty();
    }
};

class TermProduct : public Term {
public:
    Term_ptr left;
    Term_ptr right;
    void dump() {
        std::cout << "{";
        left->dump();
        std::cout << ", ";
        right->dump();
        std::cout << "}";
    }

    bool IsSubsumed(Term* t) {
        // TODO:
        return false;
    }

    bool IsSubsumedBy(std::list<Term_ptr>& fixpoint) {
        assert(false && "TermProduct.IsSubsumedBy() is not implemented yet~!");
    }

    bool IsEmpty() { return false; };

    TermProduct(Term_ptr lhs, Term_ptr rhs) : left(lhs), right(rhs) { type = TERM_PRODUCT; }
    TermProduct(Term_ptr lhs, Term_ptr rhs, TermType t) : left(lhs), right(rhs) { type = t; }
};

class TermBaseSet : public Term {
public:
    TermBaseSetStates states;

    void dump() {
        std::cout << this->type;
        std::cout << "{";
        for(auto state : this->states) {
            std::cout << (state) << ",";
        }
        std::cout << "}";
    }

    bool IsSubsumedBy(std::list<Term_ptr>& fixpoint) {
        std::cout << "IsSubsumedBy()? "; this->dump(); std::cout << "->" << this->states.size() << "\n";

        std::cout << "{";
        for(auto item : fixpoint) {
            if(item == nullptr) continue;
            item->dump();
            std::cout << ",";
        }
        std::cout << "}\n";

        if(this->states.size() == 0) {
            std::cout << "Yes it is subsumed\n";
            return true;
        }

        std::cout << "Starting checking\n";
        for(auto item : fixpoint) {
            if(item == nullptr) continue;
            std::cout << " -> "; item->dump();
            if(this->IsSubsumed(item.get())) {
                std::cout << "true\n";
                return true;
            }
            std::cout << "\n";
        }


        return false;
    }

    bool IsEmpty() {
        return this->states.size() == 0;
    }

    bool Intersects(TermBaseSet* rhs) {
        for (auto lhs_state : this->states) {
            for(auto rhs_state : rhs->states) {
                if(lhs_state == rhs_state) {
                    return true;
                }
            }
        }
        return false;
    }

    bool IsSubsumed(Term* term) {
        if(term->type != TERM_BASE) {
            return false;
        } else {
            TermBaseSet *t = reinterpret_cast<TermBaseSet*>(term);
            if(t->states.size() < this->states.size()) {
                return false;
            } else {
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
    }

    TermBaseSet() : states() { type = TERM_BASE; }
    TermBaseSet(TermBaseSetStates& s) : states()  {
        type = TERM_BASE;
        for(auto state : s) {
            this->states.push_back(state);
        }
    }

    TermBaseSet(VATA::Util::OrdVector<unsigned int>& s) : states()  {
        type = TERM_BASE;
        for(auto state : s) {
            this->states.push_back(state);
        }
    }
};

class TermContProduct : public Term {
public:
    std::shared_ptr<SymbolicAutomaton> aut;
    Term_ptr term;
    SymbolType symbol;

    bool IsSubsumedBy(std::list<Term_ptr>& fixpoint) {
        assert(false && "TermContProduct.IsSubsumedBy() is not implemented yet~!");
    }

    TermContProduct(SymbolicAutomaton* a, Term_ptr t, SymbolType s) : aut(a), term(t), symbol(s) {
        this->type = TERM_CONT_ISECT;
    }
};

class TermContSubset : public Term {
public:
    std::shared_ptr<SymbolicAutomaton> aut;
    Term_ptr term;
    SymbolType symbol;

    bool IsSubsumedBy(std::list<Term_ptr>& fixpoint) {
        assert(false && "TermContSubset.IsSubsumedBy() is not implemented yet~!");
    }

    TermContSubset(SymbolicAutomaton* a, Term_ptr t, SymbolType s) : aut(a), term(t), symbol(s) {
        this->type = TERM_CONT_SUBSET;
    }
};

class TermFixpointStates : public Term {
public:
    using FixpointType = std::list<Term_ptr>;
    using Aut_ptr = std::shared_ptr<SymbolicAutomaton>;

    using WorklistItemType = std::pair<Term_ptr, SymbolType>;
    using WorklistType = std::list<WorklistItemType>;
    using Symbols = std::list<SymbolType>;

    enum FixpointTermSem {E_FIXTERM_FIXPOINT, E_FIXTERM_PRE};

    bool IsSubsumedBy(std::list<Term_ptr>& fixpoint) {
        assert(false && "TermFixpointStates.IsSubsumedBy() is not implemented yet~!");
    }

    struct iterator {
    private:
        TermFixpointStates &_termFixpoint;
        FixpointType::const_iterator _it;

    public:
        Term_ptr GetNext() {
            assert(!_termFixpoint._fixpoint.empty());
            assert(_termFixpoint._fixpoint.cend() != _it);

            FixpointType::const_iterator succIt = _it;
            ++succIt;

            if (_termFixpoint._fixpoint.cend() != succIt) {
                // if we can traverse
                //assert(nullptr != *_it);
                return *(++_it);
            } else {
                std::cout << "--";
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
                    }
                }
            }
        }

        iterator(TermFixpointStates &termFixpoint) : _termFixpoint(termFixpoint), _it(_termFixpoint._fixpoint.begin()) {
            assert(nullptr != &termFixpoint);
            assert(!_termFixpoint._fixpoint.empty());
        }
    };

    // Only for the pre-semantics to link into the source of the pre
    Term_ptr _sourceTerm;
    std::auto_ptr<iterator> _sourceIt;

    Aut_ptr _aut;
    FixpointType _fixpoint;
    WorklistType _worklist;
    Symbols _symList;
    bool _bValue;

    iterator GetIterator() {
        return iterator(*this);
    }

private:
    void ComputeNextFixpoint() {
        assert(!_worklist.empty());

        WorklistItemType item = _worklist.front();
        std::cout << "ComputeNextFixpoint->pop_front(): ";
        std::cout << (item.second) << " -> ";
        item.first->dump();
        std::cout << "\n";
        _worklist.pop_front();

        std::cout << "_aut->IntersectNonEmpty()\n";
        ResultType result = _aut->IntersectNonEmpty(&item.second, item.first);
        std::cout << "Returned from nestedAut\n";
        result.first->dump();
        std::cout << " (" << result.second << ")\n";

        if(result.first->IsSubsumedBy(_fixpoint)) {
            std::cout << "Is Subsumed\n";
            return;
        }

        std::cout << "DO PICE UZ:";
        result.first->dump();
        std::cout <<"\n";

        _fixpoint.push_back(result.first);
        _bValue = _bValue || result.second;
        for(auto symbol : _symList) {
            std::cout << "ComputeNextFixpoint()->insert(";
            result.first->dump();
            std::cout << ", " << (symbol) << ")\n";
            _worklist.insert(_worklist.cbegin(), std::make_pair(result.first, symbol));
        }
    }

    void ComputeNextPre() {
        assert(!_worklist.empty());

        WorklistItemType item = _worklist.front();
        _worklist.pop_front();

        ResultType result = _aut->IntersectNonEmpty(&item.second, item.first);

        if(result.first->IsSubsumedBy(_fixpoint)) {
            return;
        }

        _fixpoint.push_back(result.first);
        _bValue = _bValue || result.second;
    }
public:
    TermFixpointStates(
            SymbolicAutomaton* aut,
            Term_ptr startingTerm,
            Symbols symList,
            bool initbValue) : // also differentiates between two constructors
        _sourceTerm(nullptr),
        _sourceIt(nullptr),
        _aut(aut),
        //_fixpoint({nullptr, startingTerm}),
        //_worklist({startingTerm}),
        _bValue(initbValue) {
        this->_fixpoint.push_front(startingTerm);
        this->_fixpoint.push_front(nullptr);
        for(auto symbol : symList) {
            this->_symList.push_back(symbol);
            this->_worklist.insert(this->_worklist.cbegin(), std::make_pair(startingTerm, symbol));
        }
    }

    TermFixpointStates(
            SymbolicAutomaton* aut,
            Term_ptr sourceTerm,
            Symbols symList) :
            _sourceTerm(sourceTerm),
            _fixpoint({nullptr}),
            _aut(aut),
            _worklist(),
            _bValue(false) {
        for(auto symbol : symList) {
            this->_symList.push_back(symbol);
        }
    }

    FixpointTermSem GetSemantics() const {
        return (nullptr == _sourceTerm) ? E_FIXTERM_FIXPOINT : E_FIXTERM_PRE;
    }

    void dump() {
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

    bool IsEmpty() {
        return this->_worklist.empty();
    }

    bool IsSubsumed(Term* t) {
        // TODO:
        return false;
    }

    bool GetResult() {
        return this->_bValue;
    }
};
#endif //WSKS_TERM_H
