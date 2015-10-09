/*****************************************************************************
 *  gaston - no real logic behind the name, we simply liked the poor seal gaston. R.I.P. brave soldier.
 *
 *  Copyright (c) 2015  Tomas Fiedor <ifiedortom@fit.vutbr.cz>
 *      Notable mentions: Ondrej Lengal <ondra.lengal@gmail.com>
 *          			  Overeating Panda <if-his-simulation-reduction-works>
 *
 *****************************************************************************/

#include "environment.hh"
#include "decision_procedures.hh"

/**
 *
 */
int decideWS1S_symbolically(SymbolicAutomaton& aut) {
    std::cout << "[*] Deciding WS1S Symbolically\n";
    // Input: Symbolic automaton

    // finalStateApproximation = autPhi.GetFirstFinStateApprox();


    // aut.IntersectInitialNonEmpty(finalStateApproximation, EmptySymbol)
    bool res = aut.IntersectNonEmpty(nullptr, nullptr);

    return 0;
};