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
    void GenerateProofFormulaeFor(Term_ptr example, ExampleType);
    static std::string GetProofFormulaFilename(ExampleType);
protected:
    SymbolicAutomaton* _automaton;
    std::vector<std::string> _satInterpretation;
    std::vector<std::string> _unsatInterpretation;
    VerificationResult VerifyProofFormula(std::string);
    void _ProcessExample(Term_ptr, ExampleType);

    // <<< PRIVATE METHODS >>>
    Decision _DecideCore(bool);
};


#endif //WSKS_SYMBOLICCHECKER_H
