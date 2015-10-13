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

// TODO: Subsumption: We can maybe exploit something about the leafstates

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
    virtual bool IsSubsumedBy(std::list<Term_ptr>& fixpoint) = 0;
    virtual bool IsSubsumed(Term* t) = 0;
    virtual bool IsEmpty() = 0;
};
// Wow such clean!

#define DEBUG_TERM_SUBSUMPTION true

class TermList : public Term {
public:
    TermListStates list;
    bool isComplement;

    bool IsSubsumedBy(std::list<Term_ptr>& fixpoint) {
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

        for(auto item : fixpoint) {
            if(item == nullptr) continue;
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

    void dump() {
        std::cout << "{";
        for(auto state : this->list) {
            state->dump();
            std::cout  << ",";
        }
        std::cout << "}";
    }

    TermList() {type = TERM_LIST;}

    TermList(Term_ptr first, bool isCompl) : isComplement(isCompl) {
        this->type = TERM_LIST;
        this->list.push_back(first);
    }

    TermList(Term_ptr f, Term_ptr s, bool isCompl) : isComplement(isCompl) {
        this->type = TERM_LIST;
        this->list.push_back(f);
        this->list.push_back(s);
    }

    bool IsSubsumed(Term* t) {
        if(t->type != TERM_LIST) {
            std::cerr << "Warning: Testing subsumption of incompatible terms: '";
            this->dump();
            std::cerr << "' <?= '";
            t->dump();
            std::cerr << "'\n";
        }
        TermList* tt = reinterpret_cast<TermList*>(t);
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

    bool IsEmpty() {
        return this->list.size() == 0 ||
                (this->list.size() == 1 && this->list[0]->IsEmpty());
    }
};

class TermProduct : public Term {
public:
    Term_ptr left;
    Term_ptr right;
    void dump() {
        std::cout << "{";
        left->dump();
        std::cout << " x ";
        right->dump();
        std::cout << "}";
    }

    bool IsSubsumed(Term* t) {
        if(t->type != TERM_PRODUCT) {
            // TODO: Maybe assert?
            return false;
        } else {
            TermProduct *rhs = reinterpret_cast<TermProduct*>(t);
            return (this->left->IsSubsumed(rhs->left.get())) && (this->right->IsSubsumed(rhs->right.get()));
        }
    }

    bool IsSubsumedBy(std::list<Term_ptr>& fixpoint) {
        if(this->left->IsEmpty() && this->right->IsEmpty()) {
            return true;
        }

        for(auto item : fixpoint) {
            if(item == nullptr) continue;
            if(this->IsSubsumed(item.get())) {
                return true;
            }
        }

        return false;
    }

    bool IsEmpty() { return this->left->IsEmpty() && this->right->IsEmpty(); };

    TermProduct(Term_ptr lhs, Term_ptr rhs) : left(lhs), right(rhs) {
        type = TERM_PRODUCT; }
    TermProduct(Term_ptr lhs, Term_ptr rhs, TermType t) : left(lhs), right(rhs) {
        type = t; }
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

    bool IsSubsumedBy(std::list<Term_ptr>& fixpoint) {
        if(this->states.size() == 0) {
            return true;
        }

        for(auto item : fixpoint) {
            if(item == nullptr) continue;
            if(this->IsSubsumed(item.get())) {
                return true;
            }
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

    void dump() {
        std::cout << "?!?";
    }

    bool IsSubsumed(Term *t) {
        assert(false && "To do");
    }

    bool IsEmpty() {return false;}
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

    void dump() {
        std::cout << "???";
    }

    bool IsSubsumed(Term *t) {
        assert(false && "To do");
    }

    bool IsEmpty() {return false;}
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
    bool _inComplement;

    iterator GetIterator() {
        return iterator(*this);
    }

private:
    void ComputeNextFixpoint() {
        assert(!_worklist.empty());

        WorklistItemType item = _worklist.front();
        _worklist.pop_front();

        ResultType result = _aut->IntersectNonEmpty(&item.second, item.first, this->_inComplement);


        if(result.first->IsSubsumedBy(_fixpoint)) {
            return;
        }

        _fixpoint.push_back(result.first);
        _bValue = _bValue || result.second;
        for(auto symbol : _symList) {
            _worklist.insert(_worklist.cbegin(), std::make_pair(result.first, symbol));
        }
    }

    void ComputeNextPre() {
        assert(!_worklist.empty());

        WorklistItemType item = _worklist.front();
        _worklist.pop_front();

        ResultType result = _aut->IntersectNonEmpty(&item.second, item.first, this->_inComplement);

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
            bool inComplement,
            bool initbValue) : // also differentiates between two constructors
        _sourceTerm(nullptr),
        _sourceIt(nullptr),
        _aut(aut),
        //_fixpoint({nullptr, startingTerm}),
        //_worklist({startingTerm}),
        _bValue(initbValue),
        _inComplement(inComplement) {
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
            Symbols symList,
            bool inComplement) :
            _sourceTerm(sourceTerm),
            _fixpoint({nullptr}),
            _aut(aut),
            _worklist(),
            _bValue(false),
            _inComplement(inComplement) {
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
