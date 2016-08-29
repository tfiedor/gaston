//
// Created by Raph on 02/05/2016.
//

#ifndef WSKS_FIXPOINTGUIDE_H
#define WSKS_FIXPOINTGUIDE_H

#include "../utils/Symbol.h"
#include "../environment.hh"
#include "../../Frontend/ident.h"

enum GuideTip {G_FRONT, G_BACK, G_THROW, G_PROJECT, G_PROJECT_ALL};

class SymLink;
class Term;

class FixpointGuide {
    std::vector<size_t> _vars;

    void _InitializeVars(IdentList*);
public:
    NEVER_INLINE FixpointGuide() {}
    NEVER_INLINE explicit FixpointGuide(IdentList*);

    GuideTip GiveTip(Term*, Symbol*);
    GuideTip GiveTip(Term*);

    friend std::ostream &operator<<(std::ostream &stream, const FixpointGuide &);
};


#endif //WSKS_FIXPOINTGUIDE_H
