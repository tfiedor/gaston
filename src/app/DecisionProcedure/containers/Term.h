//
// Created by Raph on 10/10/2015.
//

#ifndef WSKS_TERM_H
#define WSKS_TERM_H

#include <vector>
#include "../utils/Symbol.h"

struct Term;

// < Usings >
using Term_ptr          = std::shared_ptr<Term>;
using TermProductStates = std::pair<Term_ptr, Term_ptr>;
using ResultType        = std::pair<Term_ptr, bool>;
using SymbolType        = ZeroSymbol;

struct TermFixpointStates {
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
    TermFixpointStates(Term_ptr approx, Symbols symbols) {
        // ????
    }

    bool empty() {
        return this->_worklist.empty();
    }
};

struct Term {
    enum TermType {TERM_FIXPOINT, TERM_UNION};

    union {
        TermFixpointStates fixpoint;
        TermProductStates product;
        // TERM BASE -> LIST
        // TERM COMPL
    };
};
#endif //WSKS_TERM_H
