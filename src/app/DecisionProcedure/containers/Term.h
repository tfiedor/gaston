//
// Created by Raph on 10/10/2015.
//

#ifndef WSKS_TERM_H
#define WSKS_TERM_H

#include <vector>
#include <algorithm>
#include "../utils/Symbol.h"
#include "../mtbdd/ondriks_mtbdd.hh"

enum TermType {TERM_FIXPOINT, TERM_PRODUCT, TERM_UNION, TERM_BASE, TERM_LIST};

class Term {
public:
    TermType type;
    virtual void dump() = 0;
    virtual bool IsSubsumed(Term* t) = 0;
    virtual bool IsEmpty() = 0;
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

    bool IsEmpty() { return false; };

    TermProduct(Term* lhs, Term* rhs) : left(lhs), right(rhs) { type = TERM_PRODUCT; }
    TermProduct(Term* lhs, Term* rhs, TermType t) : left(lhs), right(rhs) { type = t; }
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

    TermList() {type = TERM_LIST;}

    TermList(Term_ptr first) {
        this->list.push_back(first);
    }

    TermList(Term* first) {
        this->list.clear();
        if(!first->IsEmpty()) {
            this->list.push_back(std::shared_ptr<Term>(first));
        }
    }

    TermList(Term_ptr f, Term_ptr s) {
        this->list.push_back(f);
        this->list.push_back(s);
    }

    TermList(Term* first, Term* second) {
        std::cout << first->type << " and " << second->type << "\n";
        this->list.clear();
        if(!first->IsEmpty()) {
            this->list.push_back(std::shared_ptr<Term>(first));
        }
        if(!second->IsEmpty()) {
            this->list.push_back(std::shared_ptr<Term>(second));
        }
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

    void RemoveSubsumed(TermList* fixpoint) {
        if(this->IsEmpty()) {
            std::cout << "Returning as empty";
            return;
        }
        TermListStates temp;
        temp.clear();
        for(auto state : this->list) {
            bool isSub = false;
            for(auto tstate : fixpoint->list) {
                if(isSub = state->IsSubsumed(tstate.get())) {
                    break;
                }
            }
            if(!isSub) {
                temp.push_back(state);
            }
        }
        // TODO: Very inefficient
        //this->list = temp;
        this->list.clear();
        for(auto item : temp) {
            this->list.push_back(item);
        }
    }

    void Add(TermList* term) {
        for(auto t: term->list) {
            this->list.push_back(t);
        }
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
                if (_termFixpoint.IsEmpty()) {
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
        type = TERM_FIXPOINT;
    }

    TermFixpointStates() {
        type = TERM_FIXPOINT;
    }

    bool IsEmpty() {
        return this->_worklist.empty();
    }

    bool IsSubsumed(Term* t) {
        // TODO:
        return false;
    }
};
#endif //WSKS_TERM_H
