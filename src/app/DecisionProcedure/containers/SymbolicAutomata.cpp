/*****************************************************************************
 *  gaston - no real logic behind the name, we simply liked the poor seal gaston. R.I.P. brave soldier.
 *
 *  Copyright (c) 2015  Tomas Fiedor <ifiedortom@fit.vutbr.cz>
 *      Notable mentions: Ondrej Lengal <ondra.lengal@gmail.com>
 *          			  Overeating Panda <if-his-simulation-reduction-works>
 *
 *****************************************************************************/

#include <list>
#include "SymbolicAutomata.h"
#include "Term.h"
#include "../environment.hh"
#include "../decision_procedures.hh"

StateType SymbolicAutomaton::stateCnt = 0;

SymbolicAutomaton::ISect_Type SymbolicAutomaton::IntersectNonEmpty(::SymbolicAutomaton::Symbol* symbol, StateSet approx) {
    // TODO: trimmedSymbol = symbol.keepOnly(aut.freeVars)
    // TODO: if((cRes = aut.cache_find(finalStateApprox, trimmedSymbol)) != _|_) { return res;}

    if(approx != nullptr && approx->type == TERM_CONT_ISECT) {
        TermContProduct* cont = reinterpret_cast<TermContProduct*>(approx.get());
        approx = (cont->aut->IntersectNonEmpty(&cont->symbol, cont->term)).first;
    }

    if(approx != nullptr && approx->type == TERM_CONT_SUBSET) {
        TermContSubset* contS = reinterpret_cast<TermContSubset*>(approx.get());
        approx = (contS->aut->IntersectNonEmpty(&contS->symbol, contS->term)).first;
    }

    ISect_Type result = this->_IntersectNonEmptyCore(symbol, approx);
    // aut.cache_insert(approx, symbol, result);
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
    this->_finalStates = std::shared_ptr<Term>(new TermProduct(this->lhs_aut->GetFinalStates(), this->rhs_aut->GetFinalStates()));
}

void BinaryOpAutomaton::_InitializeInitialStates() {
    this->_initialStates = std::shared_ptr<Term>(new TermProduct(this->lhs_aut->GetInitialStates(), this->rhs_aut->GetInitialStates()));
}

/**
 * Does the pre on the symbolic automaton through the @p symbol from @p states
 */
SymbolicAutomaton::StateSet BinaryOpAutomaton::Pre(SymbolicAutomaton::Symbol* symbol, SymbolicAutomaton::StateSet states) {
    assert(false && "Doing Pre on BinaryOp Automaton!");
}

SymbolicAutomaton::ISect_Type BinaryOpAutomaton::_IntersectNonEmptyCore(SymbolicAutomaton::Symbol* symbol, SymbolicAutomaton::StateSet final) {
    assert(final != nullptr);
    assert(final->type == TERM_PRODUCT || final->type == TERM_UNION);
    // TODO: final to (STISectFin finalLhs finalRhs)

    ISect_Type lhs_result = this->lhs_aut->IntersectNonEmpty(symbol, final);
    if(this->_eval_early(lhs_result.second)) {
        // return std::pair
        // return (STISect lhsResult (TermContIsect lhs_aut finalRhs, symbol);
    }

    ISect_Type rhs_result = this->rhs_aut->IntersectNonEmpty(symbol, final);
    Term_ptr combined = std::shared_ptr<Term>(new TermProduct(lhs_result.first, rhs_result.first));
    return std::make_pair(combined, this->_eval_result(lhs_result.second, rhs_result.second));
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
    this->_initialStates = std::shared_ptr<Term>(new TermList(this->_aut->GetInitialStates()));
}

void ComplementAutomaton::_InitializeInitialStates() {
    this->_finalStates = std::shared_ptr<Term>(new TermList(this->_aut->GetFinalStates()));
}

SymbolicAutomaton::StateSet ComplementAutomaton::Pre(SymbolicAutomaton::Symbol* symbol, SymbolicAutomaton::StateSet states) {
    assert(false && "Doing Pre on Complement Automaton!");
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
    this->_initialStates = std::shared_ptr<Term>(new TermList(this->_aut->GetInitialStates()));
}

void ProjectionAutomaton::_InitializeFinalStates() {
    this->_finalStates = std::shared_ptr<Term>(new TermList(this->_aut->GetFinalStates()));
}

SymbolicAutomaton::StateSet ProjectionAutomaton::Pre(ProjectionAutomaton::Symbol* symbol, ProjectionAutomaton::StateSet final) {
    assert(false && "Doing Pre on Projection Automaton!");
}

void initialize_symbols(std::list<ProjectionAutomaton::Symbol> &symbols, IdentList* vars) {
    // Transform the symbols
    unsigned int symNum = 1;
    for(auto var = vars->begin(); var != vars->end(); ++var) {
        // Pop symbol;
        for(auto i = symNum; i != 0; --i) {
            ProjectionAutomaton::Symbol symF = symbols.front();
            symbols.pop_front();
            ProjectionAutomaton::Symbol zero(symF.GetTrack(), (*var), '0');
            ProjectionAutomaton::Symbol one(symF.GetTrack(), (*var), '1');
            symbols.push_back(zero);
            symbols.push_back(one);
        }
        symNum <<= 1;// times 2
    }
}

SymbolicAutomaton::ISect_Type ProjectionAutomaton::_IntersectNonEmptyCore(ProjectionAutomaton::Symbol* symbol, ProjectionAutomaton::StateSet final) {
    assert(final != nullptr);
    assert(final->type == TERM_LIST);


    if(symbol == nullptr) {
        // Evaluate the zero unfoldings
        ISect_Type result = this->_aut->IntersectNonEmpty(symbol, nullptr);

        // TODO: REFACTOR -> This is not nice at all
        std::list<Symbol> symbols;
        symbol = new ZeroSymbol();
        symbols.push_back(*symbol);
        // Transform the symbols
        ASTForm_uvf* form = reinterpret_cast<ASTForm_uvf*>(this->_form);
        initialize_symbols(symbols, form->vl);

        TermFixpointStates* fixpoint = new TermFixpointStates(this->_aut.get(), result.first, symbols, result.second);
        TermFixpointStates::iterator it = fixpoint->GetIterator();
        Term_ptr term;

        // TODO: There is some issue here
        while( ((term = it.GetNext()) != nullptr)) { }
        //while( ((term = it.GetNext()) != nullptr) && (!fixpoint->GetResult())) {}
        //                                            ^--- is this right?

        return std::make_pair(std::shared_ptr<Term>(fixpoint), fixpoint->GetResult());
    } else {
        // TODO: REFACTOR
        std::list<Symbol> symbols;
        symbols.push_back(*symbol);
        // Transform the symbols
        ASTForm_uvf* form = reinterpret_cast<ASTForm_uvf*>(this->_form);
        initialize_symbols(symbols, form->vl);

        TermFixpointStates* fixpoint = new TermFixpointStates(this->_aut.get(), final, symbols);
        TermFixpointStates::iterator it = fixpoint->GetIterator();
        Term_ptr term;
        while( ((term = it.GetNext()) != nullptr) && (!fixpoint->GetResult())) {}
        //                                            ^--- is this right?

        return std::make_pair(std::shared_ptr<Term>(fixpoint), fixpoint->GetResult());
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
        return std::make_pair(this->_finalStates, initial->Intersects(final));
    // Doing Pre(final), i.e. one step back from final states
    } else {
        StateSet preSet = this->Pre(symbol, approx);
        TermBaseSet* preFinal = reinterpret_cast<TermBaseSet*>(preSet.get());
        return std::make_pair(preSet, initial->Intersects(preFinal));
    }
}

SymbolicAutomaton::StateSet BaseAutomaton::Pre(SymbolicAutomaton::Symbol* symbol, SymbolicAutomaton::StateSet approx) {
    // We know...
    TermBaseSet* base = reinterpret_cast<TermBaseSet*>(approx.get());
    BaseAut_States states;

    for(auto state : base->states) {
        BaseAut_MTBDD* preState = getMTBDDForStateTuple(*this->_base_automaton, StateTuple({state}));

        MaskerFunctor masker;
        const BaseAut_MTBDD &temp = masker(*preState, *(symbol->GetMTBDD()));

        BaseCollectorFunctor collector(states);
        collector(temp);
    }

    return std::shared_ptr<Term>(new TermBaseSet(states));
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