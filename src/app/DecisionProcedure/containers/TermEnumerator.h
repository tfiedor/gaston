//
// Created by Raph on 03/05/2016.
//

#ifndef WSKS_TERMENUMERATOR_H
#define WSKS_TERMENUMERATOR_H

#include <cstdlib>
#include <vector>

class Term;
class TermProduct;
class TermBaseSet;
class TermFixpoint;
using BaseVector = std::vector<long unsigned int>;
using BaseItem = long unsigned int;

class TermEnumerator {
public:
    virtual void Reset() = 0;
    virtual void Next() = 0;
    virtual bool IsNull() = 0;

    static TermEnumerator* ConstructEnumerator(Term*);
};

class FixpointEnumerator : public TermEnumerator {
    TermFixpoint* _fixpoint;
public:
    FixpointEnumerator(TermFixpoint*);

    void Reset();
    void Next();
    bool IsNull();
    Term* GetItem();
};

class ProductEnumerator : public TermEnumerator {
    TermEnumerator* _lhs_enum;
    TermEnumerator* _rhs_enum;
public:
    ProductEnumerator(TermProduct*);

    void Reset();
    void Next();
    bool IsNull();
    TermEnumerator* GetLeft() {return _lhs_enum;};
    TermEnumerator* GetRight() {return _rhs_enum;};
};

class BaseEnumerator : public TermEnumerator {
    TermBaseSet* _base;
    BaseVector::iterator _iterator;
public:
    BaseEnumerator(TermBaseSet*);

    void Reset();
    void Next();
    bool IsNull();
    BaseItem GetItem();
};

#endif //WSKS_TERMENUMERATOR_H
