/*****************************************************************************
 *  gaston - no real logic behind the name, we simply liked the poor seal gaston. R.I.P. brave soldier.
 *
 *  Copyright (c) 2015  Tomas Fiedor <ifiedortom@fit.vutbr.cz>
 *      Notable mentions: Ondrej Lengal <ondra.lengal@gmail.com>
 *          			  Overeating Panda <if-his-simulation-reduction-works>
 *
 *****************************************************************************/

#include "SymbolicAutomata.h"

/**
 * Returns final states with lazy construction
 *
 * @return final states as macrostate
 */
SymbolicAutomaton::StateSet SymbolicAutomaton::GetFinalStates() {
    if(this->_finalStates == nullptr) {
        this->_InitializeFinalStates();
    }

    return this->_finalStates;
}

/**
 * Return initial states with lazy construction
 *
 * @return initial states as macrostate
 */
SymbolicAutomaton::StateSet SymbolicAutomaton::GetInitialStates() {
    if(this->_initialStates == nullptr) {
        this->_InitializeInitialStates();
    }

    return this->_initialStates;
}

// <<< INTERSECTION AUTOMATON >>>

void IntersectionAutomaton::_InitializeFinalStates() {
    // TODO: not implemented
}

void IntersectionAutomaton::_InitializeInitialStates() {
    // TODO: not implemented
}

/**
 * Does the pre on the symbolic automaton through the @p symbol from @p states
 */
SymbolicAutomaton::StateSet IntersectionAutomaton::Pre(SymbolicAutomaton::Symbol &symbol, SymbolicAutomaton::StateSet &states) {
    // TODO: not implemented
    // Pre left?
    // Pre right?
    // Combine???
    // Profit!!!
    return nullptr;
}

SymbolicAutomaton::ISect_Type IntersectionAutomaton::IntersectNonEmpty(SymbolicAutomaton::Symbol &symbol, SymbolicAutomaton::StateSet &final) {
    // Explore the left automaton
    this->lhs_aut->IntersectNonEmpty(symbol, final);
    // Explore the right automaton
    this->rhs_aut->IntersectNonEmpty(symbol, final);
    // combined = apply(lhsmtbdd, rhsmtbdd, \(lfix, \lBool) (rfix, rBool) -> (u lFix rFix, lBool || rBool))
    // return combined
}

/**
 * Dumps the intersection automaton to std::cout
 */
void IntersectionAutomaton::dump() {
    lhs_aut->dump();
    std::cout << " x ";
    rhs_aut->dump();
}

// <<< BASE AUTOMATA >>>
SymbolicAutomaton::ISect_Type BaseAutomaton::IntersectNonEmpty(::BaseAutomaton::Symbol &symbol, ::BaseAutomaton::StateSet &final) {
    // initState = STSet init

    // MTBDD tmp
    // if (symbol == _|_) {
    //    tmp = new MTBDD(X...X, aut.finalStates)
    // } else {
    //    tmp = new MTBDD(pre(symb, fix))
    // }

    // tmp = unaryApply(tmp, \set -> (set, is_isect(init, fix)) );
    // return tmp;
    return false;
}

void SubAutomaton::_InitializeInitialStates() {
    this->_initialStates->
}

void SubAutomaton::_InitializeFinalStates() {

}

SymbolicAutomaton::StateSet SubAutomaton::Pre(SymbolicAutomaton::Symbol& symb, SymbolicAutomaton::StateSet& states) {

}

void SubAutomaton::dump() {
    std::cout << "Sub" << std::endl;
}

