//
// Created by Raph on 03/05/2016.
//

#include <iostream>
#include "TermEnumerator.h"
#include "Term.h"

// Fixme: Fix leaks

TermEnumerator* TermEnumerator::ConstructEnumerator(Term* term) {
    switch(term->type) {
        case TermType::TERM_BASE:
            return new BaseEnumerator(static_cast<TermBaseSet*>(term));
        case TermType::TERM_PRODUCT:
            return new ProductEnumerator(static_cast<TermProduct*>(term));
        default:
            return new GenericEnumerator(term);
    }
}

std::ostream &operator<<(std::ostream &out, const TermEnumerator& rhs) {
    switch(rhs.type) {
        case ENUM_BASE:
            out << static_cast<const BaseEnumerator&>(rhs);
            break;
        case ENUM_PRODUCT:
            out << static_cast<const ProductEnumerator&>(rhs);
            break;
        case ENUM_GENERIC:
            out << static_cast<const GenericEnumerator&>(rhs);
            break;
        default:
            assert(false && "Unsupported Enumerator for operator<<\n");
    }
    return out;
}

std::ostream &operator<<(std::ostream &out, const ProductEnumerator& rhs) {
    out << "(" << (*rhs.GetLeft()) << ", " << (*rhs.GetRight()) << ")";
    return out;
}

std::ostream &operator<<(std::ostream &out, const BaseEnumerator& rhs) {
    out << rhs.GetItem();
    return out;
}

std::ostream &operator<<(std::ostream &out, const GenericEnumerator& rhs) {
    out << rhs.GetItem();
    return out;
}

BaseEnumerator::BaseEnumerator(TermBaseSet* base) : _base(base) {
    this->type = ENUM_BASE;
    this->_iterator = this->_base->states.begin();
}

void BaseEnumerator::Next() {
    assert(this->_iterator != this->_base->states.end());
    ++this->_iterator;
}

BaseItem BaseEnumerator::GetItem() const {
    assert(this->_iterator != this->_base->states.end());
    return *this->_iterator;
}

bool BaseEnumerator::IsNull() {
    return this->_iterator == this->_base->states.end();
}

void BaseEnumerator::Reset() {
    this->_iterator = this->_base->states.begin();
}

ProductEnumerator::ProductEnumerator(TermProduct* product) {
    this->type = ENUM_PRODUCT;
    this->_lhs_enum = TermEnumerator::ConstructEnumerator(product->left);
    this->_rhs_enum = TermEnumerator::ConstructEnumerator(product->right);
}

void ProductEnumerator::Next() {
    this->_rhs_enum->Next();
    if(this->_rhs_enum->IsNull()) {
        this->_lhs_enum->Next();
        this->_rhs_enum->Reset();
    }
}

bool ProductEnumerator::IsNull() {
    return this->_lhs_enum->IsNull();
}

void ProductEnumerator::Reset() {
    this->_lhs_enum->Reset();
    this->_rhs_enum->Reset();
}

GenericEnumerator::GenericEnumerator(Term* term) : _term(term) {
    this->type = ENUM_GENERIC;
}

void GenericEnumerator::Next() {

}

Term* GenericEnumerator::GetItem() const {
    return this->_term;
}

void GenericEnumerator::Reset() {

}

bool GenericEnumerator::IsNull() {
    return true;
}