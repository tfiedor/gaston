/*****************************************************************************
 *  gaston - no real logic behind the name, we simply liked the poor seal gaston. R.I.P. brave soldier.
 *
 *  Copyright (c) 2015  Tomas Fiedor <ifiedortom@fit.vutbr.cz>
 *      Notable mentions: Ondrej Lengal <ondra.lengal@gmail.com>
 *          			  Overeating Panda <if-his-simulation-reduction-works>
 *
 *****************************************************************************/

#include "SymbolicAutomata.h"
#include "../environment.hh"
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

    return this->_finalStates.get();
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

    return this->_initialStates.get();
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
SymbolicAutomaton::StateSet BinaryOpAutomaton::Pre(SymbolicAutomaton::Symbol* symbol, SymbolicAutomaton::StateSet states) {
    // TODO: not implemented, do we need this?
    // Pre left?
    // Pre right?
    // Combine???
    // Profit!!!
    return nullptr;
}

SymbolicAutomaton::ISect_Type BinaryOpAutomaton::IntersectNonEmpty(SymbolicAutomaton::Symbol* symbol, SymbolicAutomaton::StateSet final) {
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

SymbolicAutomaton::StateSet ComplementAutomaton::Pre(SymbolicAutomaton::Symbol* symbol, SymbolicAutomaton::StateSet states) {
    // TODO:
}

SymbolicAutomaton::ISect_Type ComplementAutomaton::IntersectNonEmpty(ComplementAutomaton::Symbol* symbol,ComplementAutomaton::StateSet final) {
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

SymbolicAutomaton::StateSet ProjectionAutomaton::Pre(ProjectionAutomaton::Symbol* symbol, ProjectionAutomaton::StateSet final) {

}

SymbolicAutomaton::ISect_Type ProjectionAutomaton::IntersectNonEmpty(ProjectionAutomaton::Symbol* symbol, ProjectionAutomaton::StateSet final) {
    ISect_Type projectionFixPoint;
    WorkListSet worklist;

    // First iteration of fixpoint
    if(symbol == nullptr) {
        projectionFixPoint = this->_aut->IntersectNonEmpty(symbol, final);
        symbol = new ZeroSymbol();
    // Next iteration
    } else {
        projectionFixPoint = std::make_pair(std::shared_ptr<MacroStateSet>(final), false);
    }

    // push projetionFixPoint to worklist?
    // Do projection for more variables, not just one...
    assert(this->_projected_vars->size() == 1);
    auto var = this->_projected_vars->begin();
    ProjectionAutomaton::Symbol *symbolZero = new ZeroSymbol(symbol->GetTrack(), (*var), '0');
    ProjectionAutomaton::Symbol *symbolOne = new ZeroSymbol(symbol->GetTrack(), (*var), '1');

    std::cout << (*symbolZero) << "\n";
    std::cout << (*symbolOne) << "\n";
    #if optimisation
       // in some cases, we can go down with symbol and split later
    #endif

    while(!worklist.empty()) {
        WorkListTerm* term = worklist.back().get();
        worklist.pop_back();

        ISect_Type resultZero = this->_aut->IntersectNonEmpty(symbolZero, term);
        ISect_Type resultOne = this->_aut->IntersectNonEmpty(symbolOne, term);
        // combine (new, newbool) = (STListFin [resultZero, resultOne], boolZero || boolOne)

        // trulyNew = new.removeSubsumedBy(projectionFixpoint);
        // worklist.insertAll(trulyNew);
        // projectionFixPoint = (STListFin removeSubsumed((fst fixpoint) ++ trulyNew), (scd fixpoint) || newBool)
    }

    delete symbolZero;
    delete symbolOne;

    return projectionFixPoint;
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

SymbolicAutomaton::ISect_Type BaseAutomaton::IntersectNonEmpty(BaseAutomaton::Symbol* symbol, BaseAutomaton::StateSet final) {
    // initState = {init}
    ISect_Type tmp;

    // Computing Intersection for epsilon symbol, i.e. only whether it intersects the initial states
    if(symbol == nullptr) {
        tmp = std::make_pair(this->_finalStates, this->_finalStates->Intersects(this->_initialStates.get()));
    // Doing Pre(final), i.e. one step back from final states
    } else {
        MacroStateSet* preFinal = this->Pre(symbol, final);
        tmp = std::make_pair(std::shared_ptr<MacroStateSet>(preFinal), preFinal->Intersects(this->_initialStates.get()));
    }

    return tmp;
}

SymbolicAutomaton::StateSet BaseAutomaton::Pre(SymbolicAutomaton::Symbol* symbol, SymbolicAutomaton::StateSet states) {
    // Clean MTBDD
    std::cout << "[*] Computing Pre(" << (*symbol) << ")\n";
    std::cout << "For: ";
    states->dump();
    std::cout << "\n";

    SymbolicAutomaton::StateSet preStates = new MacroStateSet();

    for(auto state : states->getMacroStates()) {
        assert(state->type == STATE);

        BaseAut_MTBDD* preState = getMTBDDForStateTuple(*this->_base_automaton, StateTuple({state->state}));
        //                                                     Are you fucking kidding me -.- ---^

        // There is leak maybe, fuck it though
        MaskerFunctor masker;
        const BaseAut_MTBDD &temp = masker(*preState, *symbol->GetMTBDD());

        // Collect
    }

    return preStates;
}

void BaseAutomaton::_InitializeInitialStates() {
    // NOTE: The automaton is constructed backwards, so final states are initial
    assert(this->_initialStates == nullptr);
    this->_initialStates = std::make_shared<MacroStateSet>();
    for(auto state : this->_base_automaton->GetFinalStates()) {
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

    // push states to macrostate
    this->_finalStates = std::make_shared<MacroStateSet>();
    for(auto state : finalStates) {
        this->_finalStates->addState(new LeafStateSet(state));
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