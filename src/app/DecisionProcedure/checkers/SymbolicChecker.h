//
// Created by Raph on 02/02/2016.
//

#ifndef WSKS_SYMBOLICCHECKER_H
#define WSKS_SYMBOLICCHECKER_H

#include "Checker.h"
#include "../containers/SymbolicAutomata.h"

class SymbolicChecker : public Checker {
public:
    // <<< PUBLIC CONSTRUCTORS >>>
    SymbolicChecker() : Checker() {}
    ~SymbolicChecker();

    void ConstructAutomaton();
    void Decide();

protected:
    SymbolicAutomaton* _automaton;
};


#endif //WSKS_SYMBOLICCHECKER_H
