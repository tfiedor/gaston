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
#include "containers/SymbolicAutomata.h"
#include "containers/Term.h"

// TODO: OPT: Pushing symbols down the formulae
// TODO: OPT: Early Projection
// TODO: OPT: Subformulae to Automaton
// TODO: OPT: Anti-prenexing
// TODO: OPT: Pandareduction
// TODO: OPT: Caching
// TODO: (Counter)Example printing

/**
 *
 */
int decideWS1S_symbolically(SymbolicAutomaton& aut) {
    // TODO: We assume we have ground formulae
    std::cout << "\n[*] Deciding WS1S Symbolically\n";

    Term_ptr finalApprox = aut.GetFinalStates();
    #if (DEBUG_INITIAL_APPROX == true)
    finalApprox->dump();
    std::cout << "\n";
    #endif

    // TODO: Extracttype
    std::pair<std::shared_ptr<Term>, bool> res = aut.IntersectNonEmpty(nullptr, nullptr);
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