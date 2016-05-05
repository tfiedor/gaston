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

enum EnumeratorType {ENUM_GENERIC, ENUM_PRODUCT, ENUM_BASE};

class TermEnumerator {
public:
    EnumeratorType type;

    virtual void Reset() = 0;
    virtual void Next() = 0;
    virtual bool IsNull() = 0;

    static TermEnumerator* ConstructEnumerator(Term*);
    friend std::ostream &operator<<(std::ostream &stream, const TermEnumerator&);
};

class GenericEnumerator : public TermEnumerator {
    Term* _term;
public:
    GenericEnumerator(Term*);

    void Reset();
    void Next();
    bool IsNull();
    Term* GetItem() const;

    friend std::ostream &operator<<(std::ostream &stream, const GenericEnumerator&);
};

class ProductEnumerator : public TermEnumerator {
    TermEnumerator* _lhs_enum;
    TermEnumerator* _rhs_enum;
public:
    ProductEnumerator(TermProduct*);

    void Reset();
    void Next();
    bool IsNull();
    TermEnumerator* GetLeft() const {return _lhs_enum;};
    TermEnumerator* GetRight() const {return _rhs_enum;};

    friend std::ostream &operator<<(std::ostream &stream, const ProductEnumerator&);
};

class BaseEnumerator : public TermEnumerator {
    TermBaseSet* _base;
    BaseVector::iterator _iterator;
public:
    BaseEnumerator(TermBaseSet*);

    void Reset();
    void Next();
    bool IsNull();
    BaseItem GetItem() const;

    friend std::ostream &operator<<(std::ostream &stream, const BaseEnumerator&);
};

#endif //WSKS_TERMENUMERATOR_H
