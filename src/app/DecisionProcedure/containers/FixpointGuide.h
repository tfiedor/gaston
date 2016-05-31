//
// Created by Raph on 02/05/2016.
//

#ifndef WSKS_FIXPOINTGUIDE_H
#define WSKS_FIXPOINTGUIDE_H

#include "../utils/Symbol.h"
#include "../environment.hh"

enum GuideTip {G_FRONT, G_BACK, G_THROW, G_PROJECT};

class SymLink;
class Term;

class FixpointGuide {
    SymLink* _link;
    std::vector<size_t> _vars;

    void _InitializeVars(ASTForm*);
public:
    NEVER_INLINE FixpointGuide() : _link(nullptr) {}
    NEVER_INLINE explicit FixpointGuide(SymLink* link);

    GuideTip GiveTip(Term*, Symbol*);

    void SetAutomaton(SymLink*);
    friend std::ostream &operator<<(std::ostream &stream, const FixpointGuide &);
};


#endif //WSKS_FIXPOINTGUIDE_H
