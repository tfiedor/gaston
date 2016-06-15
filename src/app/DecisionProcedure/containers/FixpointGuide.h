//
// Created by Raph on 02/05/2016.
//

#ifndef WSKS_FIXPOINTGUIDE_H
#define WSKS_FIXPOINTGUIDE_H

#include "../utils/Symbol.h"
#include "../environment.hh"

enum GuideTip {G_FRONT, G_BACK, G_THROW, G_PROJECT, G_PROJECT_ALL};

class SymLink;
class Term;

class FixpointGuide {
    SymLink* _link;
    std::vector<size_t> _vars;
    bool _isQuantifierFree;

    void _InitializeVars(ASTForm*);
public:
    NEVER_INLINE FixpointGuide() : _link(nullptr), _isQuantifierFree(false) {}
    NEVER_INLINE explicit FixpointGuide(SymLink* link, bool is_qf);

    GuideTip GiveTip(Term*, Symbol*);
    GuideTip GiveTip(Term*);

    void SetAutomaton(SymLink*);
    friend std::ostream &operator<<(std::ostream &stream, const FixpointGuide &);
};


#endif //WSKS_FIXPOINTGUIDE_H
