/*****************************************************************************
 *  gaston - We pay homage to Gaston, an Africa-born brown fur seal who
 *    escaped the Prague Zoo during the floods in 2002 and made a heroic
 *    journey for freedom of over 300km all the way to Dresden. There he
 *    was caught and subsequently died due to exhaustion and infection.
 *    Rest In Piece, brave soldier.
 *
 *  Copyright (c) 2015  Tomas Fiedor <ifiedortom@fit.vutbr.cz>
 *      Notable mentions: Ondrej Lengal <ondra.lengal@gmail.com>
 *
 *  Description:
 *      Symbolic Method of deciding the WS1S formulae. The method traverses
 *      the formulae symbolically and tests if initial states of automaton
 *      corresponding to the formulae and final states intersection is
 *      nonempty.
 *****************************************************************************/

#include "containers/SymbolicAutomata.h"
#include "containers/Term.h"
#include "decision_procedures.hh"
#include "environment.hh"

using namespace Gaston;

/**
 * @param[in] symbolicAutomaton: input formula in symbolic automaton representation
 * @return: VALID/UNSATISFIABLE/SATISFIABLE
 */
int ws1s_symbolic_decision_procedure(SymbolicAutomaton_ptr symbolicAutomaton) {
    // TODO: Extend the notion to ground formulae
    std::cout << "\n[*] Deciding WS1S Symbolically\n";

    // Construct the initial approximation for final states
    // Note: This only copies the structure of fixpoint term with final
    //      states of base automata on leaves
    Term_ptr finalStatesApproximation = symbolicAutomaton->GetFinalStates();
    #if (DEBUG_INITIAL_APPROX == true)
    finalStatesApproximation->dump();
    std::cout << "\n";
    #endif

    // Checks if Initial States intersect Final states
    std::pair<Term_ptr, bool> result = symbolicAutomaton->IntersectNonEmpty(nullptr, finalStatesApproximation.get(), false);
    Term_ptr fixpoint = result.first;
    bool isValid = result.second;

    #if (DEBUG_FIXPOINT == true)
    std::cout << "[!] Finished deciding WS1S formula with following fixpoint:\n";
    fixpoint->dump();
    std::cout << "\n";
    #endif

    #if (MEASURE_STATE_SPACE == true)
    #define OUTPUT_MEASURES(TermType) \
        std::cout << "		\u2218 (==) by same ptr: " << TermType::comparisonsBySamePtr << "/" << Term::comparisonsBySamePtr; \
        std::cout << " (" << std::fixed << std::setprecision(2) << (TermType::comparisonsBySamePtr/(double)Term::comparisonsBySamePtr)*100 <<"%)\n"; \
        std::cout << "		\u2218 (==) by diff type: " << TermType::comparisonsByDiffType << "/" << Term::comparisonsByDiffType; \
        std::cout << " (" << std::fixed << std::setprecision(2) << (TermType::comparisonsByDiffType/(double)Term::comparisonsByDiffType)*100 <<"%)\n"; \
        std::cout << "		\u2218 (==) by structure: " << TermType::comparisonsByStructure << "/" << Term::comparisonsByStructure; \
        std::cout << " (" << std::fixed << std::setprecision(2) << (TermType::comparisonsByStructure/(double)Term::comparisonsByStructure)*100 <<"%)\n";

    std::cout << "[*] Measured State Space: \n";
    std::cout << "	\u2218 Term Products: " << TermProduct::instances << "\n";
    #if (MEASURE_COMPARISONS == true)
    OUTPUT_MEASURES(TermProduct)
    #endif
    std::cout << "	\u2218 Term Bases: " << TermBaseSet::instances << "\n";
    #if (MEASURE_COMPARISONS == true)
    OUTPUT_MEASURES(TermBaseSet)
    #endif
    std::cout << "	\u2218 Term Fixpoints: " << TermFixpoint::instances << "\n";
    #if (MEASURE_COMPARISONS == true)
    OUTPUT_MEASURES(TermFixpoint)
    #endif
    std::cout << "	\u2218 Term Lists: " << TermList::instances << "\n";
    #if (MEASURE_COMPARISONS == true)
    OUTPUT_MEASURES(TermList)
    #endif
    std::cout << "	\u2218 Term Continuations: " << (TermContinuation::instances) << "\n";
    #if (MEASURE_COMPARISONS == true)
    OUTPUT_MEASURES(TermContinuation)
    #endif
    #undef OUTPUT_MEASURES
    std::cout << "[*] Overall State Space: " << (TermProduct::instances + TermBaseSet::instances + TermFixpoint::instances
                                            + TermList::instances + TermContinuation::instances) << "\n";
    std::cout << "[*] Explored Fixpoint Space: " << fixpoint->MeasureStateSpace() << "\n";
    #endif

    #if (MEASURE_CACHE_HITS == true)
    std::cout << "[*] Printing Statistics\n";
    symbolicAutomaton->DumpStats();
    std::cout << "\n";
    #endif

    // If Initial States does intersect final ones, the formula is valid, else it is unsatisfiable
    if(isValid) {
        return VALID;
    } else {
        return UNSATISFIABLE;
    }
};