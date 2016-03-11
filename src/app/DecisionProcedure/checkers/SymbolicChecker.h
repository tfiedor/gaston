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
    SymbolicChecker() : _automaton(nullptr), Checker() {}
    ~SymbolicChecker();

    void ConstructAutomaton();
    void Decide();
    bool Run();
protected:
    SymbolicAutomaton* _automaton;

    // <<< PRIVATE METHODS >>>
    int _DecideCore(bool);
};


#endif //WSKS_SYMBOLICCHECKER_H
