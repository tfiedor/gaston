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

SymbolicAutomaton::ISect_Type SymbolicAutomaton::IntersectNonEmpty(::SymbolicAutomaton::Symbol* symbol, StateSet approx) {
    // TODO: trimmedSymbol = symbol.keepOnly(aut.freeVars);
    // TODO: Cache: if((cRse = aut.cache_find(finalStateApprox, trimmedSymbol)) != _) { return cRes; }

    ISect_Type result = this->_IntersectNonEmptyCore(symbol, approx);
    return result;
}

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

SymbolicAutomaton::ISect_Type BinaryOpAutomaton::_IntersectNonEmptyCore(SymbolicAutomaton::Symbol* symbol, SymbolicAutomaton::StateSet final) {

    // Explore the left automaton
    ISect_Type lhsResult = this->lhs_aut->IntersectNonEmpty(symbol, final);
    // Explore the right automaton
    ISect_Type rhsResult = this->rhs_aut->IntersectNonEmpty(symbol, final);
    //ISect_Type combined = (lhsResult rhsResult, this->_eval_result(lhsBool, rhsBool));
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

SymbolicAutomaton::ISect_Type ComplementAutomaton::_IntersectNonEmptyCore(ComplementAutomaton::Symbol* symbol,ComplementAutomaton::StateSet final) {
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
    // TODO: Maybe not needed?
}

void ProjectionAutomaton::_InitializeFinalStates() {
    // TODO: Maybe not needed?
}

SymbolicAutomaton::StateSet ProjectionAutomaton::Pre(ProjectionAutomaton::Symbol* symbol, ProjectionAutomaton::StateSet final) {
    // Todo may be optimization
}

SymbolicAutomaton::ISect_Type ProjectionAutomaton::_IntersectNonEmptyCore(ProjectionAutomaton::Symbol* symbol, ProjectionAutomaton::StateSet final) {
    ISect_Type projectionFixPoint;
    WorkListSet worklist;

    // First iteration of fixpoint
    if(symbol == nullptr) {
        projectionFixPoint = this->_aut->IntersectNonEmpty(symbol, final);
        symbol = new ZeroSymbol();

        worklist.push_back(projectionFixPoint.first);

        // push projetionFixPoint to worklist?
        // TODO: Do projection for more variables, not just one...
        assert(this->_projected_vars->size() == 1);
        auto var = this->_projected_vars->begin();
        ProjectionAutomaton::Symbol *symbolZero = new ZeroSymbol(symbol->GetTrack(), (*var), '0');
        ProjectionAutomaton::Symbol *symbolOne = new ZeroSymbol(symbol->GetTrack(), (*var), '1');

        while (!worklist.empty()) {
            WorkListTerm_ptr term = worklist.back();
            worklist.pop_back();

            // TODO: Push more symbols;
            ISect_Type resultZero = this->_aut->IntersectNonEmpty(symbolZero, term.get());
            ISect_Type resultOne = this->_aut->IntersectNonEmpty(symbolOne, term.get());

            bool result = resultOne.second || resultZero.second;
            //combine (new, newbool) = (STListFin [resultZero, resultOne], boolZero || boolOne)
            std::cout << "combined:\n";

            // trulyNew = new.removeSubsumedBy(projectionFixpoint);
            // worklist.insertAll(trulyNew);
            // projectionFixPoint = (STListFin removeSubsumed((fst fixpoint) ++ trulyNew), (scd fixpoint) || newBool)
        }

        // TODO: With continuation we may kill ourselves here somehow
        delete symbolZero;
        delete symbolOne;

        return projectionFixPoint;
    } else {
        // TermSet fixpoint = (STListFin [], bool);

        assert(this->_projected_vars->size() == 1);
        auto var = this->_projected_vars->begin();
        ProjectionAutomaton::Symbol *symbolZero = new ZeroSymbol(symbol->GetTrack(), (*var), '0');
        ProjectionAutomaton::Symbol *symbolOne = new ZeroSymbol(symbol->GetTrack(), (*var), '1');

        // for (term in approx) {
        //      (resultZero, boolZero) = aut->IntersectNonEmpty(symbolZero, term);
        //      (resultOne, boolOne    = aut->IntersectNonEmpty(symbolOne, term);
        //      (new, newBool) = (STList [resultZero, resultOne], boolZero || boolOne)

        //      trulyNew =  new.removeSubsumedBy(fixpoint);
        //      projectionFixPoint = (STListFin remove_subsumed((fixpoint.first) ++ trulyNew, fixpoint.second || newBool

        return projectionFixPoint;
    }
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

SymbolicAutomaton::ISect_Type BaseAutomaton::_IntersectNonEmptyCore(BaseAutomaton::Symbol* symbol, BaseAutomaton::StateSet approx) {
    // initState = {init}
    ISect_Type tmp;
    TermBaseSet* initial = reinterpret_cast<TermBaseSet*>(this->_initialStates.get());
    TermBaseSet* final = reinterpret_cast<TermBaseSet*>(this->_finalStates.get());

    // Computing Intersection for epsilon symbol, i.e. only whether it intersects the initial states
    if(symbol == nullptr) {
        assert(initial->states.size() == 1 && "There should be only one initial state");
        tmp = std::make_pair(this->_finalStates, initial->Intersects(final));
    // Doing Pre(final), i.e. one step back from final states
    } else {
        TermBaseSet* preFinal = reinterpret_cast<TermBaseSet*>(this->Pre(symbol, approx));
        tmp = std::make_pair(std::shared_ptr<Term>(preFinal), initial->Intersects(preFinal));
    }

    return tmp;
}

SymbolicAutomaton::StateSet BaseAutomaton::Pre(SymbolicAutomaton::Symbol* symbol, SymbolicAutomaton::StateSet approx) {
    // We know...
    TermBaseSet* base = reinterpret_cast<TermBaseSet*>(approx);
    BaseAut_States states;
    for(auto state : base->states) {
        BaseAut_MTBDD* preState = getMTBDDForStateTuple(*this->_base_automaton, StateTuple({state}));

        MaskerFunctor masker;
        const BaseAut_MTBDD &temp = masker(*preState, *(symbol->GetMTBDD()));
        std::cout << BaseAut_MTBDD::DumpToDot({&temp}) << "\n";

        BaseCollectorFunctor collector(states);
        collector(temp);
    }

    return new TermBaseSet(states);
}

void BaseAutomaton::_InitializeInitialStates() {
    // NOTE: The automaton is constructed backwards, so final states are initial
    assert(this->_initialStates == nullptr);
    TermBaseSet* temp = new TermBaseSet();
    for(auto state : this->_base_automaton->GetFinalStates()) {
       temp->states.push_back(state);
    }

    this->_initialStates = std::shared_ptr<Term>(temp);
}

void BaseAutomaton::_InitializeFinalStates() {
    // NOTE: The automaton is constructed backwards, so initial states are finals
    assert(this->_finalStates == nullptr);
    BaseAut_States finalStates;
    BaseAut_MTBDD* initBDD = getMTBDDForStateTuple(*this->_base_automaton, Automaton::StateTuple());

    StateCollectorFunctor sc_functor(finalStates);
    sc_functor(*initBDD);

    // push states to macrostate
    TermBaseSet* temp = new TermBaseSet();
    for(auto state : finalStates) {
        temp->states.push_back(state);
    }

    this->_finalStates = std::shared_ptr<Term>(temp);
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
    if(this->_initialStates != nullptr) {
        this->_finalStates->dump();
        std::cout << "\n";
    } else {
        std::cout << "-> not initialized\n";
    }

    std::cout << "[!] Final states:\n";
    if(this->_finalStates != nullptr) {
        this->_initialStates->dump();
        std::cout << "\n";
    } else {
        std::cout << "-> not initialized\n";
    }
    std::cout << "[----------------------->]\n";
}