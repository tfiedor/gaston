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
    std::pair<std::shared_ptr<MacroStateSet>, bool> res = aut.IntersectNonEmpty(nullptr, nullptr);
    #if (DEBUG_FIXPOINT == true)
    std::cout << "[!] Finished deciding WS1S formula with following fixpoint:\n";
    res.first->dump();
    std::cout << "\n";
    #endif
    if(res.second == true) {
        return VALID;
    } else {
        return UNSATISFIABLE;
    }
};