#include "Workshops.h"
#include "Term.h"

TermBaseSet* TermWorkshop::CreateBaseSet(VATA::Util::OrdVector<unsigned int>& states, unsigned int offset, unsigned int stateno) {
    std::cout << "TermWorkshop::";
    return new TermBaseSet(states, offset, stateno);
}