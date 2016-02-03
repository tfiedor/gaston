//
// Created by Raph on 02/02/2016.
//

#include "SymbolicChecker.h"
#include "../../Frontend/timer.h"
#include "../../DecisionProcedure/decision_procedures.hh"
#include "../environment.hh"

extern Timer timer_conversion, timer_mona, timer_base, timer_automaton;

SymbolicChecker::~SymbolicChecker() {

}

void SymbolicChecker::ConstructAutomaton() {
    assert(this->_monaAST != nullptr);

    timer_automaton.start();
    this->_automaton = (this->_monaAST->formula)->toSymbolicAutomaton(false);
    if(options.printProgress)
        this->_automaton->DumpAutomaton();
    std::cout << "\n";

    timer_automaton.stop();
    if (options.dump) {
        std::cout << "\n[*] Formula translation to Automaton [DONE]\n";
        std::cout << "[*] Elapsed time: ";
        timer_automaton.print();
        std::cout << "\n";
    }
}

void SymbolicChecker::Decide() {
    assert(this->_automaton != nullptr);

    Timer timer_deciding;
    int decided;
    try {
        // Deciding WS1S formula
        timer_deciding.start();
        if (options.mode != TREE) {
            decided = ws1s_symbolic_decision_procedure(this->_automaton);
            delete this->_automaton;
            this->_automaton = nullptr;
        } else {
            throw NotImplementedException();
        }
        timer_deciding.stop();

        // Outing the results of decision procedure
        std::cout << "[!] Formula is ";
        switch(decided) {
            case SATISFIABLE:
                std::cout << "\033[1;34m'SATISFIABLE'\033[0m";
                break;
            case UNSATISFIABLE:
                std::cout << "\033[1;31m'UNSATISFIABLE'\033[0m";
                break;
            case VALID:
                std::cout << "\033[1;32m'VALID'\033[0m";
                break;
            default:
                std::cout << "undecided due to an error.\n";
                break;
        }

        std::cout << "\n[*] Decision procedure: ";
        timer_deciding.print();
        std::cout << "[*] DFA creation:       ";
        timer_mona.print();
        std::cout << "[*] MONA <-> VATA:      ";
        timer_conversion.print();
        std::cout << "[*] Bases creation:     ";
        timer_automaton.print();
        // Something that was used is not supported by dWiNA
    } catch (NotImplementedException& e) {
        std::cerr << e.what() << std::endl;
    }
}