//
// Created by Raph on 03/05/2016.
//

#ifndef WSKS_TERMENUMERATOR_H
#define WSKS_TERMENUMERATOR_H

#include <cstdlib>
#include <vector>
#include "../environment.hh"

class Term;
class TermProduct;
class TermTernaryProduct;
class TermNaryProduct;
class TermBaseSet;
class TermFixpoint;
using BaseVector = std::vector<size_t>;
using BaseItem = long unsigned int;

enum class EnumeratorType {GENERIC, PRODUCT, BASE, TERNARY, NARY};

class TermEnumerator {
public:
    NEVER_INLINE virtual ~TermEnumerator() {}
    EnumeratorType type;

    virtual void Reset() = 0;
    virtual void FullReset() = 0;
    virtual void Next() = 0;
    virtual bool IsNull() = 0;

    static TermEnumerator* ConstructEnumerator(Term*);
    friend std::ostream &operator<<(std::ostream &stream, const TermEnumerator&);
};

class GenericEnumerator : public TermEnumerator {
    Term* _term;
    bool _invalidated = false;
public:
    GenericEnumerator(Term*);

    void Reset() { this->_invalidated = false;};
    void FullReset();
    void Next() { this->_invalidated = true;}
    bool IsNull();
    Term* GetItem() const;

    friend std::ostream &operator<<(std::ostream &stream, const GenericEnumerator&);
};

class ProductEnumerator : public TermEnumerator {
    TermEnumerator* _lhs_enum = nullptr;
    TermEnumerator* _rhs_enum = nullptr;
public:
    NEVER_INLINE ProductEnumerator(TermProduct*);
    NEVER_INLINE ~ProductEnumerator();

    void Reset();
    void FullReset();
    void Next();
    bool IsNull();
    TermEnumerator* GetLeft() const {return _lhs_enum;};
    TermEnumerator* GetRight() const {return _rhs_enum;};

    friend std::ostream &operator<<(std::ostream &stream, const ProductEnumerator&);
};

class TernaryProductEnumerator : public TermEnumerator { 
    TermEnumerator* _lhs_enum = nullptr;
    TermEnumerator* _mhs_enum = nullptr;
    TermEnumerator* _rhs_enum = nullptr;
    
public:
    NEVER_INLINE TernaryProductEnumerator(TermTernaryProduct*);
    NEVER_INLINE ~TernaryProductEnumerator();
    
    void Reset();
    void FullReset();
    void Next();
    bool IsNull();
    TermEnumerator *GetLeft() const { return _lhs_enum;}
    TermEnumerator *GetMiddle() const { return _mhs_enum;}
    TermEnumerator *GetRight() const { return _rhs_enum;}

    friend std::ostream &operator<<(std::ostream &stream, const TernaryProductEnumerator&);
};

class NaryProductEnumerator : public TermEnumerator {
    size_t _arity;
    TermEnumerator** _enums = nullptr;

public:
    NEVER_INLINE NaryProductEnumerator(TermNaryProduct*);
    NEVER_INLINE ~NaryProductEnumerator();

    void Reset();
    void FullReset();
    void Next();
    bool IsNull();
    TermEnumerator *GetEnum(size_t i) const { assert(i < this->_arity); return this->_enums[i]; }

    friend std::ostream &operator<<(std::ostream &stream, const NaryProductEnumerator&);
};

class BaseEnumerator : public TermEnumerator {
    TermBaseSet* _base;
    BaseVector::const_iterator _iterator;
public:
    BaseEnumerator(TermBaseSet*);

    void Reset();
    void FullReset();
    void Next();
    bool IsNull();
    BaseItem GetItem() const;

    friend std::ostream &operator<<(std::ostream &stream, const BaseEnumerator&);
};

#endif //WSKS_TERMENUMERATOR_H
