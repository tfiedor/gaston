/*****************************************************************************
 *  gaston - no real logic behind the name, we simply liked the poor seal gaston. R.I.P. brave soldier.
 *
 *  Copyright (c) 2015  Tomas Fiedor <ifiedortom@fit.vutbr.cz>
 *      Notable mentions: Ondrej Lengal <ondra.lengal@gmail.com>
 *          			  Overeating Panda <if-his-simulation-reduction-works>
 *
 *****************************************************************************/

#include "SymbolicAutomata.h"
#include "../decision_procedures.hh"

StateType SymbolicAutomaton::stateCnt = 0;

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

// <<<<<<<<<<<<<<<<<<<<<< BINARY AUTOMATA >>>>>>>>>>>>>>>>>>>>>>>>>>

void BinaryOpAutomaton::_InitializeFinalStates() {
    // TODO: not implemented
}

void BinaryOpAutomaton::_InitializeInitialStates() {
    // TODO: not implemented
}

/**
 * Does the pre on the symbolic automaton through the @p symbol from @p states
 */
SymbolicAutomaton::StateSet BinaryOpAutomaton::Pre(SymbolicAutomaton::Symbol* symbol, SymbolicAutomaton::StateSet &states) {
    // TODO: not implemented, do we need this?
    // Pre left?
    // Pre right?
    // Combine???
    // Profit!!!
    return nullptr;
}

SymbolicAutomaton::ISect_Type BinaryOpAutomaton::IntersectNonEmpty(SymbolicAutomaton::Symbol* symbol, SymbolicAutomaton::StateSet &final) {
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
void BinaryOpAutomaton::dump() {
    lhs_aut->dump();
    std::cout << " x ";
    rhs_aut->dump();
}

// <<<<<<<<<<<<<<<<<<<<<< COMPLEMENT AUTOMATON >>>>>>>>>>>>>>>>>>>>>>>>>>

void ComplementAutomaton::_InitializeFinalStates() {
    // TODO:
}

void ComplementAutomaton::_InitializeInitialStates() {
    // TODO:
}

SymbolicAutomaton::StateSet ComplementAutomaton::Pre(SymbolicAutomaton::Symbol* symbol, SymbolicAutomaton::StateSet &states) {
    // TODO:
}

SymbolicAutomaton::ISect_Type ComplementAutomaton::IntersectNonEmpty(ComplementAutomaton::Symbol* symbol,ComplementAutomaton::StateSet &final) {
    // TODO: Implement details
    // mtbdd = evalSubset(this->_aut, nonfinNested, symbol)
    // return unaryApply(mtbdd, \(fix, bool) -> (STDownClosed fix, bool) );
}

void ComplementAutomaton::dump() {
    std::cout << "compl(";
    this->_aut->dump();
    std::cout << ")";
}

// <<<<<<<<<<<<<<<<<<<<<< PROJECTION AUTOMATON >>>>>>>>>>>>>>>>>>>>>>>>>>

void ProjectionAutomaton::_InitializeInitialStates() {

}

void ProjectionAutomaton::_InitializeFinalStates() {

}

SymbolicAutomaton::StateSet ProjectionAutomaton::Pre(ProjectionAutomaton::Symbol* symbol, ProjectionAutomaton::StateSet &final) {

}

SymbolicAutomaton::ISect_Type ProjectionAutomaton::IntersectNonEmpty(ProjectionAutomaton::Symbol* symbol, ProjectionAutomaton::StateSet &final) {
    // First iteration of fixpoint
    if(symbol == nullptr) {
        // MTBDD nested Mtbdd = this->
        //bool res = this->_aut->IntersectNonEmpty(symbol, final); // final states should be unnested

        // symbol = new BDD(0000000, true);

        // worklist nestedMtbdd.collectSinks();
    // Next iteration
    } else {
        // worklist = listOfStates (from final)
    }

    // symbol.set(freeVars, X);
    // MTBDD fixpoint = new MTBDD(freeVars, listOfStates);

    // while(!worklist.empty()) {
    //      term = worklist.get_and_destroy();

    //      MTBDD nestedMtbdd = this->_aut->IntersectNonEmpty(symbol, final);
    //      MTBDD projMtbdd = nestedMtbdd.project(vars, (remove_subsumed(\lhs rhs -> {lhs, rhs}, lhs OR rhs)))

    //      fixpoint = apply(fixpoint, projMtbdd, \(oldFix, oldBool) (newFix, newBool) ->
    //          -> {
    //              if(old.contains(term)) {
    //                  trulyNew = new.removeSubsumedBy(old);
    //                  worklist.insertAll(trulyNew);
    //                  return (remove_subsumed(old ++ trulyNew), oldBool OR newBool)
    //              } else {
    //                  return (old);
    //              }
    //             });
    //  }
    //  return fixpoint;
}

void ProjectionAutomaton::dump() {
    std::cout << "ex2(";
    this->_aut->dump();
    std::cout << ")";
}

// <<<<<<<<<<<<<<<<<<<<<< BASE AUTOMATON >>>>>>>>>>>>>>>>>>>>>>>>>>

void BaseAutomaton::_InitializeAutomaton() {
    this->_RenameStates();
    this->_InitializeInitialStates();
    this->_InitializeFinalStates();
}

SymbolicAutomaton::ISect_Type BaseAutomaton::IntersectNonEmpty(BaseAutomaton::Symbol* symbol, BaseAutomaton::StateSet &final) {
    // initState = STSet init
    FixPoint_MTBDD_B* tmp;

    if(symbol == nullptr) {
        // computing epsilon
        tmp = new FixPoint_MTBDD_B(true);
    }

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

SymbolicAutomaton::StateSet BaseAutomaton::Pre(SymbolicAutomaton::Symbol* symbol, SymbolicAutomaton::StateSet &states) {
    // Clean MTBDD
    //FixPoint_MTBDD nullary = new FixPoint_MTBDD(new MacroStateSet());
    for(auto state : states->getMacroStates()) {
        // Pre(state, symbol)
        // union to nullary
    }

    return nullptr;
}

void BaseAutomaton::_InitializeInitialStates() {
    // NOTE: The automaton is constructed backwards, so final states are initial
    assert(this->_initialStates == nullptr);
    this->_initialStates = std::make_shared<MacroStateSet>();
    for(auto state : this->_base_automaton->GetFinalStates()) {
        std::cout << "Adding initial state: " << state << "\n";
        this->_initialStates->addState(new LeafStateSet(state));
    }

}

void BaseAutomaton::_InitializeFinalStates() {
    // NOTE: The automaton is constructed backwards, so initial states are finals
    assert(this->_finalStates == nullptr);
    BaseAut_States finalStates;
    BaseAut_MTBDD* initBDD = getMTBDDForStateTuple(*this->_base_automaton, Automaton::StateTuple());

    StateCollectorFunctor sc_functor(finalStates);
    sc_functor(*initBDD);

    std::cout << BaseAut_MTBDD::DumpToDot({initBDD}) << "\n";

    // push states to macrostate
    this->_finalStates = std::make_shared<MacroStateSet>();
    std::cout << "Dumping collected states:\n";
    std::cout << finalStates << "\n";
    for(auto state : finalStates) {
        std::cout << "Adding final state: " << state << "\n";
        this->_finalStates->addState(new LeafStateSet(state));
        for(auto s : this->_finalStates->getMacroStates()) {
            std::cout << "Ministate: " << s << "\n";
        }
    }

    for(auto s : this->_finalStates->getMacroStates()) {
        std::cout << "Ministate: " << s << "\n";
    }
}

void BaseAutomaton::_RenameStates() {
    StateToStateMap translMap;
    StateToStateTranslator stateTransl(translMap,
                                       [](const StateType &) { return SymbolicAutomaton::stateCnt++; });
    this->_base_automaton.reset(new LeafAutomaton_Type(this->_base_automaton->ReindexStates(stateTransl)));
}

void BaseAutomaton::baseAutDump() {
    std::cout << "[----------------------->]\n";
    std::cout << "[!] Base VATA Automaton\n";
    VATA::Serialization::AbstrSerializer *serializer = new VATA::Serialization::TimbukSerializer();
    std::cerr << this->_base_automaton->DumpToString(*serializer, "symbolic") << "\n";
    delete serializer;

    std::cout << "[!] Initial states:\n";
    if(this->_initialStates) {
        this->_initialStates->dump();
        std::cout << "\n";
    } else {
        std::cout << "-> not initialized\n";
    }

    std::cout << "[!] Final states:\n";
    if(this->_finalStates) {
        this->_finalStates->dump();
        std::cout << "\n";
    } else {
        std::cout << "-> not initialized\n";
    }
    std::cout << "[----------------------->]\n";
}