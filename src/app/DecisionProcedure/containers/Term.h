//
// Created by Raph on 10/10/2015.
//

#ifndef WSKS_TERM_H
#define WSKS_TERM_H

#include <vector>
#include "../utils/Symbol.h"
#include "../mtbdd/ondriks_mtbdd.hh"

class Term {
public:
    virtual void dump() = 0;
};
// Wow such clean!

// < Usings >
using Term_ptr          = std::shared_ptr<Term>;
using TermProductStates = std::pair<Term_ptr, Term_ptr>;
using TermListStates    = std::vector<Term_ptr>;
using BaseState         = size_t;
using TermBaseSetStates = std::vector<BaseState>;
using ResultType        = std::pair<Term_ptr, bool>;
using SymbolType        = ZeroSymbol;

enum TermType {TERM_FIXPOINT, TERM_PRODUCT, TERM_BASE, TERM_LIST};

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
};

class TermBaseSet : public Term {
public:
    TermBaseSetStates states;

    void dump() {
        std::cout << "{";
        for(auto state : this->states) {
            std::cout << (state) << ",";
        }
        std::cout << "}";
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

    TermBaseSet() {}
    TermBaseSet(TermBaseSetStates& states) {
        for(auto state : states) {
            this->states.push_back(state);
        }
    }

    TermBaseSet(VATA::Util::OrdVector<unsigned int>& states) {
        for(auto state : states) {
            this->states.push_back(state);
        }
    }
};

class TermList : public Term {
public:
    TermListStates list;
    void dump() {
        std::cout << "{";
        for(auto state : this->list) {
            state->dump();
            std::cout  << ",";
        }
        std::cout << "}";
    }
};

class TermFixpointStates : public Term {
public:
    using FixpointType = std::vector<Term_ptr>;

    using WorklistItemType = std::pair<Term_ptr, SymbolType>;
    using WorklistType = std::vector<WorklistItemType>;

    using Symbols = std::vector<SymbolType>;

    Term_ptr _approx;
    FixpointType _fixpoint;
    WorklistType _worklist;
    bool _result;
    Symbols _symbols;

    struct iterator {
    private:
        TermFixpointStates& _termFixpoint;
        FixpointType::const_iterator _it;

    public:
        Term_ptr getNext() {
            if(_termFixpoint._fixpoint.cend() != _it) {
                return *(_it++);
            } else {
                // We need to unfold the fixpoint
                if (_termFixpoint.empty()) {
                    // we get the fixpoint
                    return nullptr;
                } else {
                    _termFixpoint.computeNext();
                    return this->getNext();
                }
            }
        }

        iterator(TermFixpointStates& termFixpoint) : _termFixpoint(termFixpoint), _it(_termFixpoint._fixpoint.begin()) {
            assert(nullptr != &termFixpoint);
            assert(!termFixpoint._fixpoint.empty());
        }

        iterator getIterator() {
            return iterator(*this);
        }
    };

private:
    void computeNext() {
        assert(!_worklist.empty());

        // TODO: somewhere we should call getNext() on _approx and it might be interesting to try various strategies DFS/BFS

        WorklistItemType item = _worklist.back();
        _worklist.pop_back();

        ResultType result; // ...compute the pre of term item.first over symbol item.second

        /*if(result.is_subsumed_by(_fixpoint)) {
            return;
        }*/

        _fixpoint.push_back(result.first);
        _result = _result || result.second;
        for(auto symbol : _symbols) {
            _worklist.push_back(std::make_pair(result.first, symbol));
        }
    }

public:
    void dump() {}
    TermFixpointStates(Term_ptr approx, Symbols symbols) {
        // ????
    }

    bool empty() {
        return this->_worklist.empty();
    }
};
#endif //WSKS_TERM_H
