//
// Created by Raph on 03/05/2016.
//

#include "TermEnumerator.h"
#include "Term.h"

// Fixme: Fix leaks

TermEnumerator* TermEnumerator::ConstructEnumerator(Term* term) {
    switch(term->type) {
        case TermType::TERM_BASE:
            return new BaseEnumerator(static_cast<TermBaseSet*>(term));
        case TermType::TERM_FIXPOINT:
            return new FixpointEnumerator(static_cast<TermFixpoint*>(term));
        case TermType::TERM_PRODUCT:
            return new ProductEnumerator(static_cast<TermProduct*>(term));
        default:
            assert(false && "Unsupported term type\n");
    }
}

BaseEnumerator::BaseEnumerator(TermBaseSet* base) : _base(base) {
    this->_iterator = this->_base->states.begin();
}

void BaseEnumerator::Next() {
    assert(this->_iterator != this->_base->states.end());
    ++this->_iterator;
}

BaseItem BaseEnumerator::GetItem() {
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

FixpointEnumerator::FixpointEnumerator(TermFixpoint* fixpoint) : _fixpoint(fixpoint) {

}

void FixpointEnumerator::Next() {

}

Term* FixpointEnumerator::GetItem() {
    return this->_fixpoint;
}

void FixpointEnumerator::Reset() {

}

bool FixpointEnumerator::IsNull() {
    return true;
}