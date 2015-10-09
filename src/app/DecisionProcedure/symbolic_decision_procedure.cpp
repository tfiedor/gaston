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
    // TODO: We assume we have ground formulae
    std::cout << "\n[*] Deciding WS1S Symbolically\n";

    // finalStateApproximation = autPhi.GetFirstFinStateApprox();
    // TODO: Do the approximation somehow

    // aut.IntersectInitialNonEmpty(finalStateApproximation, EmptySymbol)
    FixPoint_MTBDD* res = aut.IntersectNonEmpty(nullptr, nullptr);
    std::pair<MacroStateSet*, bool> resValue = res->GetValue(constructUniversalTrack());
    #if (DEBUG_FIXPOINT == true)
    std::cout << "[!] Finished deciding WS1S formula with following fixpoint:\n";
    resValue.first->dump();
    std::cout << "\n";
    #endif
    if(resValue.second == true) {
        return VALID;
    } else {
        return UNSATISFIABLE;
    }
};