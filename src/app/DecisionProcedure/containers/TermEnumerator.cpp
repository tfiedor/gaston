//
// Created by Raph on 03/05/2016.
//

#include <iostream>
#include "TermEnumerator.h"
#include "Term.h"

// Fixme: Fix leaks

/**
 * @brief constructs the enumerator for the given @p term
 *
 * According to the type of the @p term, the function constructs the appropriate enumerator.
 * Note that for other terms than bases and products (binary, ternary, nary) the generic
 * enumerator is created that treats the term as atomic unit, that cannot be further enumerated.
 *
 * @param[in]  term  term we want to enumerate
 * @return  enumerator for the @p term
 */
TermEnumerator* TermEnumerator::ConstructEnumerator(Term* term) {
    switch(term->type) {
        case TermType::TERM_BASE:
            return new BaseEnumerator(static_cast<TermBaseSet*>(term));
        case TermType::TERM_PRODUCT:
            return new ProductEnumerator(static_cast<TermProduct*>(term));
        case TermType::TERM_TERNARY_PRODUCT:
            return new TernaryProductEnumerator(static_cast<TermTernaryProduct*>(term));
        case TermType::TERM_NARY_PRODUCT:
            return new NaryProductEnumerator(static_cast<TermNaryProduct*>(term));
        default:
            return new GenericEnumerator(term);
    }
}

/**
 * @brief Outputs the enumerator to the stream
 *
 * @param[in,out]  out  output stream
 * @param[in]  rhs  enumerator we are outputing out
 * @return  output stream
 */
std::ostream &operator<<(std::ostream &out, const TermEnumerator& rhs) {
    switch(rhs.type) {
        case EnumeratorType::BASE:
            out << static_cast<const BaseEnumerator&>(rhs);
            break;
        case EnumeratorType::PRODUCT:
            out << static_cast<const ProductEnumerator&>(rhs);
            break;
        case EnumeratorType::GENERIC:
            out << static_cast<const GenericEnumerator&>(rhs);
            break;
        case EnumeratorType::TERNARY:
            out << static_cast<const TernaryProductEnumerator&>(rhs);
            break;
        case EnumeratorType::NARY:
            out << static_cast<const NaryProductEnumerator&>(rhs);
            break;
        default:
            assert(false && "Unsupported Enumerator for operator<<\n");
    }
    return out;
}

/**
 * @brief outputs the product enumerator as pair of enumerators
 *
 * @param[in,out]  out  stream we are outputing to
 * @param[in]  rhs  product enumerator
 * @return  output stream
 */
std::ostream &operator<<(std::ostream &out, const ProductEnumerator& rhs) {
    out << "(" << (*rhs.GetLeft()) << ", " << (*rhs.GetRight()) << ")";
    return out;
}

/**
 * @brief outputs the base enumerator as atomic state (size_t type)
 *
 * @param[in,out]  out  stream we are outputing to
 * @param[in]  rhs  base enumerator
 * @return  output stream
 */
std::ostream &operator<<(std::ostream &out, const BaseEnumerator& rhs) {
    out << rhs.GetItem();
    return out;
}

/**
 * @brief outputs the generic enumerator as atomic term
 *
 * @param[in,out]  out  stream we are outputing to
 * @param[in]  rhs  generic enumerator
 * @return  output stream
 */
std::ostream &operator<<(std::ostream &out, const GenericEnumerator& rhs) {
    out << rhs.GetItem();
    return out;
}

/**
 * @brief outputs the ternary enumerator as triple of units
 *
 * @param[in,out]  out  stream we are outputing to
 * @param[in]  rhs  ternary enumerator
 * @return  output stream
 */
std::ostream &operator<<(std::ostream &out, const TernaryProductEnumerator& rhs) {
    out << "(" << (*rhs.GetLeft()) << ", " << (*rhs.GetMiddle()) << ", " << (*rhs.GetRight()) << ")";
    return out;
}

/**
 * @biref outputs the nary enumerator as ntuple of enumerators
 *
 * @param[in,out]  out  stream we are outputing to
 * @param[in]  rhs  nary enumerator
 * @return  output stream
 */
std::ostream &operator<<(std::ostream &out, const NaryProductEnumerator& rhs) {
    out << "(";
    for(size_t i = 0; i < rhs._arity; ++i) {
        if(i != 0) {
            out << ", ";
        }
        out << (*rhs.GetEnum(i));
    }
    out << ")";
}

/**
 * @brief constructs the base enumerator
 *
 * Base enumerator contains the iterator to the atomic (type size_t) states of the base term
 *
 * @param[in]  base  base state we are enumerating, i.e. breaking to atomic units
 */
BaseEnumerator::BaseEnumerator(TermBaseSet* base)
        : _base(base),
          _iteratorPosition(0),
          _spaceSize(base->states.size()) {
    assert(base->type == TermType::TERM_BASE);
    assert(base->states.size() > 0);
    this->type = EnumeratorType::BASE;
    this->_iterator = this->_base->states.begin();
    assert(this->_iterator != this->_base->states.end());
}

/**
 * @brief moves the enumerator by one unit
 */
void BaseEnumerator::Next() {
    assert(this->_iteratorPosition != this->_spaceSize);
    ++this->_iterator;
    ++this->_iteratorPosition;
}

/**
 * @brief returns the item that the enumerator currently points to
 *
 * @return  item that the enumerator points to
 */
BaseItem BaseEnumerator::GetItem() const {
    assert(this->_iteratorPosition != this->_spaceSize);
    return *this->_iterator;
}

/**
 * @brief returns whether the enumerator is at the end of the base states
 *
 * @return  true if all atomic states of base state were enumerated
 */
bool BaseEnumerator::IsNull() {
    return this->_iteratorPosition == this->_spaceSize;
}

/**
 * @brief resets the enumerator to the beginning of the atomic states
 *
 * Note: this is used for nested enumerators that enumerates the base states multiple time
 */
void BaseEnumerator::Reset() {
    this->_iteratorPosition = 0;
    this->_iterator = this->_base->states.begin();
}

/**
 * @brief constructs the product enumerator
 *
 * Product enumerator contains two nested iterators that are iterated in product manner,
 * i.e. the pairs of atomic units are constructed
 *
 * @param[in]  product  product term we are enumerating
 */
ProductEnumerator::ProductEnumerator(TermProduct* product) {
    this->type = EnumeratorType::PRODUCT;
    this->_lhs_enum = TermEnumerator::ConstructEnumerator(product->left);
    this->_rhs_enum = TermEnumerator::ConstructEnumerator(product->right);
}

/**
 * @brief destructor of product enumerator destroys nested enumerators
 */
ProductEnumerator::~ProductEnumerator() {
    if (this->_lhs_enum)
        delete this->_lhs_enum;
    if (this->_rhs_enum)
        delete this->_rhs_enum;
}

/**
 * @brief moves the enumerator by one unit
 *
 * Right operand of the enumerator is moved, if is moved to the end, we reset the right
 * enumerator and progress in the left operand.
 *
 * (1, 2, 3) x (a, b)  =>  (1, 2, 3) x (a, b)  => (3, a) is next pair
 *     ^            ^          ---^    -^   -
 */
void ProductEnumerator::Next() {
    this->_rhs_enum->Next();

    // If everything in the rightmost enumerator was progressed
    if(this->_rhs_enum->IsNull()) {
        this->_lhs_enum->Next();
        this->_rhs_enum->Reset();
    }
}

/**
 * @brief returns whether all pairs of the enumerator were enumerated
 *
 * Note: it is sufficient to test whether the left enumerator is out of the stuff
 * @return  true if everything was enumerated
 */
bool ProductEnumerator::IsNull() {
    return this->_lhs_enum->IsNull();
}

/**
 * @brief resets the enumerator to the original state
 */
void ProductEnumerator::Reset() {
    this->_lhs_enum->Reset();
    this->_rhs_enum->Reset();
}

/**
 * @brief constructs the ternary product enumerator
 *
 * Ternary product enumerator contains two nested iterators that are iterated in product manner,
 * i.e. as triples of atomic units
 *
 * @param[in]  ternary  ternary product term we are enumerating
 */
TernaryProductEnumerator::TernaryProductEnumerator(TermTernaryProduct* ternary) {
    this->type = EnumeratorType::TERNARY;

    this->_lhs_enum = TermEnumerator::ConstructEnumerator(ternary->left);
    this->_mhs_enum = TermEnumerator::ConstructEnumerator(ternary->middle);
    this->_rhs_enum = TermEnumerator::ConstructEnumerator(ternary->right);
}

/**
 * @brief destructs the ternary product enumerator and its nested iterators
 */
TernaryProductEnumerator::~TernaryProductEnumerator() {
    if(this->_lhs_enum)
        delete this->_lhs_enum;
    if(this->_mhs_enum)
        delete this->_mhs_enum;
    if(this->_rhs_enum)
        delete this->_rhs_enum;
}

/**
 * @brief moves the enumerator by one unit
 *
 * Right operand of the enumerator is moved, if it is moved to the end, we reset the right
 * enumerator and progress the left one.
 */
void TernaryProductEnumerator::Next() {
    this->_rhs_enum->Next();

    // If the right enum is at the end, reset it and progress the middle enum
    if(this->_rhs_enum->IsNull()) {
        this->_mhs_enum->Next();
        this->_rhs_enum->Reset();
    }

    // If the middle enum is at the end, reset it and progress the leftmost enum
    if(this->_mhs_enum->IsNull()) {
        this->_lhs_enum->Next();
        this->_mhs_enum->Reset();
    }
}

/**
 * @brief returns whether all triples of the enumerator were enumerated
 *
 * Note: it is sufficient to test whether the left enumerator is out of the stuff
 * @return  true if everything was enumerated
 */
bool TernaryProductEnumerator::IsNull() {
    return this->_lhs_enum->IsNull();
}

/**
 * @brief resets the enumerator to the original state
 */
void TernaryProductEnumerator::Reset() {
    this->_lhs_enum->Reset();
    this->_mhs_enum->Reset();
    this->_rhs_enum->Reset();
}

/**
 * @brief constructs the nary product enumerator
 *
 * Nary product enumerator contains n nested enumerators that are enumerated in product manner,
 * i.e. as n-tuples of atomic units
 *
 * @param[in]  nary  nary product term we are enumerating
 * @param[in]  arity  arity of the product
 */
NaryProductEnumerator::NaryProductEnumerator(TermNaryProduct* nary) : _arity(nary->arity) {
    assert(nary->arity > 0);
    assert(this->_arity > 0);
    assert(nary->type == TERM_NARY_PRODUCT);
    this->type = EnumeratorType::NARY;

    this->_enums = new TermEnumerator*[this->_arity];
    for(size_t i = 0; i < this->_arity; ++i) {
        this->_enums[i] = TermEnumerator::ConstructEnumerator(nary->terms[i]);
        assert(!(this->_enums[i]->IsNull() && this->_enums[i]->type == EnumeratorType::BASE));
        assert(this->_enums[i] != nullptr);
    }
}

/**
 * @brief destructs the nary product enumerator and its nested iterators
 */
NaryProductEnumerator::~NaryProductEnumerator() {
    assert(this->_enums != nullptr);

    for(size_t i = 0; i < this->_arity; ++i) {
        delete this->_enums[i];
    }

    delete this->_enums;
}

/**
 * @brief moves the enumerator by one unit
 *
 * Operands are progressed from the rightmost enumerator, if any of the enumerator is null, it is
 * reseted and the progression is transferred to the left neighbour
 */
void NaryProductEnumerator::Next() {
    size_t i = this->_arity - 1;
    this->_enums[i]->Next();
    while(i != 0) {
        // If we are at the end of this level of enumeration, we reset and progress the left neighbour
        if(this->_enums[i]->IsNull()) {
            this->_enums[i]->Reset();
            this->_enums[i-1]->Next();
            --i;
        } else {
            break;
        }
    }
}

/**
 * @brief returns whether all tuples of the enumerator were enumerated
 *
 * Note: it is again sufficientto test only the leftmost enumerator
 * @return  true if everything was enumerated
 */
bool NaryProductEnumerator::IsNull() {
    assert(this->_arity > 0);
    assert(this->_enums[0] != nullptr);

    return this->_enums[0]->IsNull();
}

/**
 * @brief resets the enumerator to the original state
 */
void NaryProductEnumerator::Reset() {
    for(size_t i = 0; i < this->_arity; ++i) {
        this->_enums[i]->Reset();
    }
}

/**
 * @brief constructs generic enumerator
 *
 * Generic enumerator simply returns the wrapped term and treats it as an atomic value
 *
 * @param[in]  term  term we are enumerating
 */
GenericEnumerator::GenericEnumerator(Term* term) : _term(term) {
    this->type = EnumeratorType::GENERIC;
}

/**
 * @brief returns the item that generic enumerator currently points to
 */
Term* GenericEnumerator::GetItem() const {
    assert(!this->_invalidated);
    return this->_term;
}

/**
 * @brief generic enumerator is always at the end
 */
bool GenericEnumerator::IsNull() {
    return this->_invalidated;
}

/**
 * Fixme: I have no idea what the fuck does this do
 */
void ProductEnumerator::FullReset() {
    this->_lhs_enum->FullReset();
    this->_rhs_enum->FullReset();
}

void TernaryProductEnumerator::FullReset() {
    this->_lhs_enum->FullReset();
    this->_mhs_enum->FullReset();
    this->_rhs_enum->FullReset();
}

void NaryProductEnumerator::FullReset() {
    for(size_t i = 0; i < this->_arity; ++i) {
        this->_enums[i]->FullReset();
    }
}

void BaseEnumerator::FullReset() {
    this->Reset();
}

void GenericEnumerator::FullReset() {
    this->Reset();
}

