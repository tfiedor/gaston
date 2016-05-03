//
// Created by Raph on 02/05/2016.
//

#ifndef WSKS_FIXPOINTGUIDE_H
#define WSKS_FIXPOINTGUIDE_H

#include "../utils/Symbol.h"
#include "../environment.hh"

enum GuideTip {G_FRONT, G_BACK, G_THROW, G_PROJECT};

class SymbolicAutomaton;
class Term;

class FixpointGuide {
    SymbolicAutomaton* _aut;
public:
    NEVER_INLINE FixpointGuide() : _aut(nullptr) {}
    NEVER_INLINE explicit FixpointGuide(SymbolicAutomaton* aut) : _aut(aut) {}

    GuideTip GiveTip(Term*, Symbol*);

    void SetAutomaton(SymbolicAutomaton *aut);
};


#endif //WSKS_FIXPOINTGUIDE_H
