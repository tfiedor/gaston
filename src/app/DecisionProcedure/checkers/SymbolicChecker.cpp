//
// Created by Raph on 02/02/2016.
//

#include "SymbolicChecker.h"
#include "../containers/Term.h"
#include "../../Frontend/timer.h"
#include "../../Frontend/env.h"
#include "../environment.hh"

extern Timer timer_conversion, timer_mona, timer_base, timer_automaton, timer_preprocess;
extern Ident lastPosVar, allPosVar;
extern Options options;

SymbolicChecker::~SymbolicChecker() {

}

/**
 * Construction of Automaton for Symbolic Decision procedure
 */
void SymbolicChecker::ConstructAutomaton() {
    assert(this->_monaAST != nullptr);

    timer_automaton.start();
    this->_automaton = (this->_monaAST->formula)->toSymbolicAutomaton(false);

    if(allPosVar != -1) {
        std::cout << "[*] AllPosVar predicate detected. Will use the M2L(str) decision procedure.\n";
    }

    // Formulae with free variable will be decided using the backward search, that is implemented in the RootProjection
    IdentList free, bound;
    this->_monaAST->formula->freeVars(&free, &bound);
    if (!free.empty()) {
        this->_monaAST->formula = new ASTForm_Ex1(nullptr, free.copy(), this->_monaAST->formula, Pos());
        //                        ^---- this is just a placeholding Ex1, semantically it is not First Order
        this->_automaton = new RootProjectionAutomaton(this->_automaton, this->_monaAST->formula);
    }

    if(options.printProgress)
        this->_automaton->DumpAutomaton();
    std::cout << "\n";
#   if (MEASURE_AUTOMATA_METRICS == true)
    this->_automaton->DumpAutomatonMetrics();
#   endif

    timer_automaton.stop();
    if (options.dump) {
        std::cout << "\n[*] Formula translation to Automaton [DONE]\n";
        std::cout << "[*] Elapsed time: ";
        timer_automaton.print();
        std::cout << "\n";
    }
}

/**
 * Core evaluation of decision procedure according to the membership testing of epsilon and/or the found (counter)
 * examples during the backward search
 *
 * @param[in] isValid:  true if the epsilon is in the language of the automaton
 */
int SymbolicChecker::_DecideCore(bool isValid) {
    assert(this->_automaton != nullptr);

    if(this->_isGround) {
        // Ground formula is valid if epsilon is in its language
        if(isValid) {
            return Decision::VALID;
        // Else it is unsatisfiable
        } else {
            return Decision::UNSATISFIABLE;
        }
    } else {
        // Formula is unsatisfiable if there is no satisfying example
        if(this->_automaton->_satExample == nullptr) {
            return Decision::UNSATISFIABLE;
        // Formula is valid if there is satisfying example and no unsatisfying example
        } else if(this->_automaton->_unsatExample == nullptr) {
            return Decision::VALID;
        // Formula is satisfiable if there exists satisfying example
        } else {
            return Decision::SATISFIABLE;
        }
    }
}

/**
 * Core of the Symbolic Decision procedure, Runs the procedure and prints the result according to the call of the
 * core function. Moreover prints various timings.
 */
void SymbolicChecker::Decide() {
    assert(this->_automaton != nullptr);

    try {
        Timer timer_deciding;
        bool decided;
        // Deciding WS1S formula
        timer_deciding.start();
        decided = this->Run();
        timer_deciding.stop();

        // Outing the results of decision procedure
        std::cout << "[!] Formula is ";
        switch(this->_DecideCore(decided)) {
            case Decision::SATISFIABLE:
                std::cout << "\033[1;34m'SATISFIABLE'\033[0m";
                break;
            case Decision::UNSATISFIABLE:
                std::cout << "\033[1;31m'UNSATISFIABLE'\033[0m";
                break;
            case Decision::VALID:
                std::cout << "\033[1;32m'VALID'\033[0m";
                break;
            default:
                std::cout << "undecided due to an error.\n";
                break;
        }

        std::cout << "\n";
        std::cout << "[*] Preprocessing:      ";
        timer_preprocess.print();
        std::cout << "[*] DFA creation:       ";
        timer_mona.print();
        std::cout << "[*] SA creation:        ";
        timer_automaton.print();
        std::cout << "[*] Decision procedure: ";
        timer_deciding.print();
        // Something that was used is not supported by dWiNA

#   if (DEBUG_GENERATE_DOT_AUTOMATON == true)
        if(options.printProgress) {
            SymbolicAutomaton::AutomatonToDot("automaton.dot", this->_automaton, false);
        }
#   endif
        delete this->_automaton;
        this->_automaton = nullptr;
    } catch (NotImplementedException& e) {
        std::cerr << e.what() << std::endl;
    }
}

/**
 * Runs the decision procedure on the constructed automaton
 */
bool SymbolicChecker::Run() {
    assert(this->_automaton != nullptr);
    std::cout << "\n[*] Deciding WS1S Symbolically\n";

    // Construct the initial approximation for final states
    // Note: This only copies the structure of fixpoint term with final
    //      states of base automata on leaves

    Term_ptr finalStatesApproximation = this->_automaton->GetFinalStates();
#   if (DEBUG_INITIAL_APPROX == true)
    finalStatesApproximation->dump();
    std::cout << "\n";
#   endif

    // Checks if Initial States intersect Final states
    std::pair<Term_ptr, bool> result = this->_automaton->IntersectNonEmpty(nullptr, finalStatesApproximation, false);
    Term_ptr fixpoint = result.first;
    bool isValid = result.second;

#   if (DUMP_EXAMPLES == true)
    // TODO: Better output
    if(this->_automaton->_satExample) {
        std::cout << "[*] Printing satisfying example of least length\n";
        this->_automaton->DumpExample(ExampleType::SATISFYING);
        std::cout << "\n";
    }

    if(this->_automaton->_unsatExample) {
        std::cout << "[*] Printing unsatisfying example of least length\n";
        this->_automaton->DumpExample(ExampleType::UNSATISFYING);
        std::cout << "\n";
    }
#   endif

#   if (DEBUG_FIXPOINT == true)
    std::cout << "[!] Finished deciding WS1S formula with following fixpoint:\n";
    fixpoint->dump();
    std::cout << "\n";
#   endif

#   if (MEASURE_STATE_SPACE == true)
#   define OUTPUT_MEASURES(TermType) \
        std::cout << "\t\t\u2218 prunnable: " << TermType::prunable << "\n"; \
        std::cout << "\t\t\u2218 (==) by same ptr: " << TermType::comparisonsBySamePtr << "/" << Term::comparisonsBySamePtr; \
        std::cout << " (" << std::fixed << std::setprecision(2) << (TermType::comparisonsBySamePtr/(double)Term::comparisonsBySamePtr)*100 <<"%)\n"; \
        std::cout << "\t\t\u2218 (==) by diff type: " << TermType::comparisonsByDiffType << "/" << Term::comparisonsByDiffType; \
        std::cout << " (" << std::fixed << std::setprecision(2) << (TermType::comparisonsByDiffType/(double)Term::comparisonsByDiffType)*100 <<"%)\n"; \
        std::cout << "\t\t\u2218 (==) by structure: " << TermType::comparisonsByStructureTrue << " trues /" << TermType::comparisonsByStructureFalse << " falses\n";

    std::cout << "[*] Measured State Space: \n";
    std::cout << "\t\u2218 Symbols: " << ZeroSymbol::instances << "\n";
    std::cout << "\t\u2218 Term Empty: " << TermEmpty::instances << "\n";
    std::cout << "\t\u2218 Term Products: " << TermProduct::instances << "\n";
#   if (MEASURE_COMPARISONS == true)
    OUTPUT_MEASURES(TermProduct)
#   endif
    std::cout << "\t\u2218 Term Bases: " << TermBaseSet::instances << "\n";
#   if (MEASURE_COMPARISONS == true)
    OUTPUT_MEASURES(TermBaseSet)
#   endif
    std::cout << "\t\u2218 Term Fixpoints: " << TermFixpoint::instances << " (" << (TermFixpoint::instances - TermFixpoint::preInstances) << " + " << TermFixpoint::preInstances <<")\n";
#   if (MEASURE_PROJECTION == true)
    std::cout << "\t\t\u2218 is not shared: " << TermFixpoint::isNotShared << "\n";
#   endif
#   if (MEASURE_SUBSUMEDBY_HITS == true)
    std::cout << "\t\t\u2218 subsumedBy hits: " << TermFixpoint::subsumedByHits << "\n";
#   endif
#   if (MEASURE_POSTPONED == true)
    std::cout << "\t\t\u2218 postponed terms: " << TermFixpoint::postponedTerms << " (" << TermFixpoint::postponedProcessed << " evaluated)\n";
#   endif
#   if (MEASURE_COMPARISONS == true)
    OUTPUT_MEASURES(TermFixpoint)
#   endif
    std::cout << "\t\u2218 Term Lists: " << TermList::instances << "\n";
#   if (MEASURE_COMPARISONS == true)
    OUTPUT_MEASURES(TermList)
#   endif
    std::cout << "\t\u2218 Term Continuations: " << (TermContinuation::instances) << "\n";
#   if (MEASURE_CONTINUATION_EVALUATION == true)
    std::cout << "\t\t\u2218 evaluated: " << TermContinuation::continuationUnfolding << "\n";
    std::cout << "\t\t\t\u2218 in subsumption: " << TermContinuation::unfoldInSubsumption << "\n";
    std::cout << "\t\t\t\u2218 in isect nonempty: " << TermContinuation::unfoldInIsectNonempty << "\n";
#   endif
#   if (MEASURE_COMPARISONS == true)
    OUTPUT_MEASURES(TermContinuation)
#   endif
#   undef OUTPUT_MEASURES
    std::cout << "[*] Overall State Space: " << (TermProduct::instances + TermBaseSet::instances + TermFixpoint::instances
                                                 + TermList::instances + TermContinuation::instances) << "\n";
    std::cout << "[*] Explored Fixpoint Space: " << fixpoint->MeasureStateSpace() << "\n";
#   endif

#   if (PRINT_STATS == true)
    std::cout << "[*] Printing Statistics\n";
    this->_automaton->DumpComputationStats();
    std::cout << "\n";
#   endif

    // If Initial States does intersect final ones, the formula is valid, else it is unsatisfiable
    return isValid;
}