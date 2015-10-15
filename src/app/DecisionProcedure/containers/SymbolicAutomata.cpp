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
#include "../containers/VarToTrackMap.hh"

extern VarToTrackMap varMap;

StateType SymbolicAutomaton::stateCnt = 0;



SymbolicAutomaton::ISect_Type SymbolicAutomaton::IntersectNonEmpty(::SymbolicAutomaton::Symbol* symbol, StateSet approx, bool underComplement) {
    assert(this->type != AutType::SYMBOLIC_BASE);

    #if (DEBUG_INTERSECT_NON_EMPTY == true)
    std::cout << "\nIntersectNonEmpty(";
    if(symbol != nullptr) {
        std::cout << (*symbol);
    } else {
        std::cout << "''";
    }
    std::cout << ",";
    if(approx == nullptr) {
        std::cout << "nullptr";
    } else {
        approx->dump();
    }
    std::cout << ", " << (underComplement ? "True" : "False");
    std::cout << ")\n";
    #endif
    // TODO: trimmedSymbol = symbol.keepOnly(aut.freeVars)
    // TODO: if((cRes = aut.cache_find(finalStateApprox, trimmedSymbol)) != _|_) { return res;}

    // TODO: UNCOMMENT
    if(approx != nullptr && approx->type == TERM_CONT_ISECT) {
        TermContProduct* cont = reinterpret_cast<TermContProduct*>(approx.get());
        approx = (cont->aut->IntersectNonEmpty((cont->symbol == nullptr ? nullptr : cont->symbol.get()), cont->term, false)).first;
        //                                                                                                           ^--- is this ok?
    }

    if(approx != nullptr && approx->type == TERM_CONT_SUBSET) {
        TermContSubset* contS = reinterpret_cast<TermContSubset*>(approx.get());
        approx = (contS->aut->IntersectNonEmpty((contS->symbol == nullptr ? nullptr : contS->symbol.get()), contS->term, true)).first;
        //                                                                                                               ^--- is this ok?
    }

    ISect_Type result = this->_IntersectNonEmptyCore(symbol, approx, underComplement);
    // TODO: aut.cache_insert(approx, symbol, result);
#if (DEBUG_INTERSECT_NON_EMPTY == true)
    std::cout << "\nIntersectNonEmpty(";
    if(symbol != nullptr) {
        std::cout << (*symbol);
    } else {
        std::cout << "''";
    }
    std::cout << ",";
    if(approx == nullptr) {
        std::cout << "nullptr";
    } else {
        approx->dump();
    }
    std::cout << ") = " << (result.second ? "True" : "False") << "\n";
    #endif
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
SymbolicAutomaton::StateSet BinaryOpAutomaton::Pre(SymbolicAutomaton::Symbol* symbol, SymbolicAutomaton::StateSet states, bool underComplement) {
    assert(false && "Doing Pre on BinaryOp Automaton!");
}

SymbolicAutomaton::ISect_Type BinaryOpAutomaton::_IntersectNonEmptyCore(SymbolicAutomaton::Symbol* symbol, SymbolicAutomaton::StateSet final, bool underComplement) {
    assert(final != nullptr);
    assert(final->type == TERM_PRODUCT || final->type == TERM_UNION);
    TermProduct* finalApprox = reinterpret_cast<TermProduct*>(final.get());

    ISect_Type lhs_result = this->lhs_aut->IntersectNonEmpty(symbol, finalApprox->left, underComplement);
    #if (OPT_EARLY_EVALUATION == true)
    if(this->_eval_early(lhs_result.second, underComplement)) {
        std::shared_ptr<Symbol> contSymbol = (symbol == nullptr) ? nullptr : std::shared_ptr<Symbol>(new ZeroSymbol(symbol->GetTrack()));
        if(!underComplement) {
            TermContProduct *rhsCont = new TermContProduct(this->rhs_aut, finalApprox->right, contSymbol);
            Term_ptr leftCombined = std::shared_ptr<Term>(
                    new TermProduct(lhs_result.first, std::shared_ptr<Term>(rhsCont)));
            return std::make_pair(leftCombined, this->_early_val(underComplement));
        } else {
            TermContSubset *rhsContSub = new TermContSubset(this->rhs_aut, finalApprox->right, contSymbol);
            Term_ptr leftCombinedSub = std::shared_ptr<Term>(
                    new TermProduct(lhs_result.first, std::shared_ptr<Term>(rhsContSub)));
            return std::make_pair(leftCombinedSub, this->_early_val(underComplement));
        }
    }
    #endif

    ISect_Type rhs_result = this->rhs_aut->IntersectNonEmpty(symbol, finalApprox->right, underComplement);
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
    this->_initialStates = std::shared_ptr<Term>(new TermList(this->_aut->GetInitialStates(), true));
}

void ComplementAutomaton::_InitializeInitialStates() {
    this->_finalStates = std::shared_ptr<Term>(new TermList(this->_aut->GetFinalStates(), true));
}

SymbolicAutomaton::StateSet ComplementAutomaton::Pre(SymbolicAutomaton::Symbol* symbol, SymbolicAutomaton::StateSet states, bool underComplement) {
    assert(false && "Doing Pre on Complement Automaton!");
}

SymbolicAutomaton::ISect_Type ComplementAutomaton::_IntersectNonEmptyCore(ComplementAutomaton::Symbol* symbol,ComplementAutomaton::StateSet final, bool underComplement) {
    // TODO: Is this ok? Can there be something more
    assert(final->type == TERM_LIST);

    TermList* approx = reinterpret_cast<TermList*>(final.get());
    assert(approx->list.size() == 1);

    ISect_Type result = this->_aut->IntersectNonEmpty(symbol, approx->list[0], !underComplement);
    // TODO: We need some information, that this is negation. Is this ok?
    TermList* newResult = new TermList(result.first, true);
    return std::make_pair(std::shared_ptr<Term>(newResult), result.second);
}

void ComplementAutomaton::dump() {
    std::cout << "compl(";
    this->_aut->dump();
    std::cout << ")";
}

// <<<<<<<<<<<<<<<<<<<<<< PROJECTION AUTOMATON >>>>>>>>>>>>>>>>>>>>>>>>>>

void ProjectionAutomaton::_InitializeInitialStates() {
    this->_initialStates = std::shared_ptr<Term>(new TermList(this->_aut->GetInitialStates(), false));
}

void ProjectionAutomaton::_InitializeFinalStates() {
    this->_finalStates = std::shared_ptr<Term>(new TermList(this->_aut->GetFinalStates(), false));
}

SymbolicAutomaton::StateSet ProjectionAutomaton::Pre(ProjectionAutomaton::Symbol* symbol, ProjectionAutomaton::StateSet final, bool underComplement) {
    assert(false && "Doing Pre on Projection Automaton!");
}

void initialize_symbols(std::list<ProjectionAutomaton::Symbol> &symbols, IdentList* vars) {
    // Transform the symbols
    // TODO: Optimize, this sucks
    unsigned int symNum = 1;
    for(auto var = vars->begin(); var != vars->end(); ++var) {
        // Pop symbol;
        for(auto i = symNum; i != 0; --i) {
            ProjectionAutomaton::Symbol symF = symbols.front();
            symbols.pop_front();
            ProjectionAutomaton::Symbol zero(symF.GetTrack(), varMap[(*var)], '0');
            ProjectionAutomaton::Symbol one(symF.GetTrack(), varMap[(*var)], '1');
            symbols.push_back(zero);
            symbols.push_back(one);
        }
        symNum <<= 1;// times 2
    }
}

SymbolicAutomaton::ISect_Type ProjectionAutomaton::_IntersectNonEmptyCore(ProjectionAutomaton::Symbol* symbol, ProjectionAutomaton::StateSet final, bool underComplement) {
    assert(final != nullptr);
    // TODO: Can there be Continuation?
    assert(final->type == TERM_LIST || final->type == TERM_FIXPOINT);

    if(symbol == nullptr) {
        TermList* finalApprox = reinterpret_cast<TermList*>(final.get());
        assert(finalApprox->list.size() == 1);

        // Evaluate the zero unfoldings
        ISect_Type result = this->_aut->IntersectNonEmpty(symbol, finalApprox->list[0], underComplement);

        std::list<Symbol> symbols;
        symbol = new ZeroSymbol();
        symbols.push_back(*symbol);
        // Transform the symbols
        ASTForm_uvf* form = reinterpret_cast<ASTForm_uvf*>(this->_form);
        initialize_symbols(symbols, form->vl);

        TermFixpointStates* fixpoint = new TermFixpointStates(this->_aut, result.first, symbols, underComplement, result.second);
        TermFixpointStates::iterator it = fixpoint->GetIterator();
        Term_ptr term;

        // TODO: underComplement
        if(result.second == !underComplement) {
            return std::make_pair(std::shared_ptr<Term>(fixpoint), result.second);
        }

        while( ((term = it.GetNext()) != nullptr) && (underComplement == fixpoint->GetResult())) {}
        //                                            ^--- is this right?

        return std::make_pair(std::shared_ptr<Term>(fixpoint), fixpoint->GetResult());
    } else {
        // TODO: REFACTOR
        std::list<Symbol> symbols;
        symbols.push_back(*symbol);
        // Transform the symbols
        ASTForm_uvf* form = reinterpret_cast<ASTForm_uvf*>(this->_form);
        initialize_symbols(symbols, form->vl);

        TermFixpointStates* fixpoint = new TermFixpointStates(this->_aut, final, symbols, underComplement);
        TermFixpointStates::iterator it = fixpoint->GetIterator();
        Term_ptr term;
        while( ((term = it.GetNext()) != nullptr) && (underComplement == fixpoint->GetResult())) {}

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

SymbolicAutomaton::ISect_Type BaseAutomaton::_IntersectNonEmptyCore(BaseAutomaton::Symbol* symbol, BaseAutomaton::StateSet approx, bool underComplement) {
    // initState = {init}
    ISect_Type tmp;
    TermBaseSet* initial = reinterpret_cast<TermBaseSet*>(this->_initialStates.get());
    TermBaseSet* final = reinterpret_cast<TermBaseSet*>(this->_finalStates.get());

    // Computing Intersection for epsilon symbol, i.e. only whether it intersects the initial states
    if(symbol == nullptr) {
        //assert(initial->states.size() == 1 && "There should be only one initial state"); // TODO: FUCK, there can be more
        return std::make_pair(this->_finalStates, initial->Intersects(final) != underComplement);
    // Doing Pre(final), i.e. one step back from final states
    } else {
        StateSet preSet = this->Pre(symbol, approx, underComplement);
        TermBaseSet* preFinal = reinterpret_cast<TermBaseSet*>(preSet.get());
        return std::make_pair(preSet, initial->Intersects(preFinal) != underComplement);
    }
}

SymbolicAutomaton::StateSet BaseAutomaton::Pre(SymbolicAutomaton::Symbol* symbol, SymbolicAutomaton::StateSet approx, bool underComplement) {
    assert(symbol != nullptr);
    // We know...
    // TODO: Cache pre?
    // TODO: is the cepre/intersect minus correct?
    TermBaseSet* base = reinterpret_cast<TermBaseSet*>(approx.get());
    BaseAut_States states;

    for(auto state : base->states) {
        BaseAut_MTBDD* preState = getMTBDDForStateTuple(*this->_base_automaton, StateTuple({state}));

        MaskerFunctor masker;
        const BaseAut_MTBDD &temp = masker(*preState, *(symbol->GetMTBDD()));

        BaseCollectorFunctor collector(states, underComplement);
        collector(temp);
        collector._isFirst = false;
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
    std::cerr << "\n[----------------------->]\n";
    std::cerr << "[!] Base VATA Automaton\n";
    VATA::Serialization::AbstrSerializer *serializer = new VATA::Serialization::TimbukSerializer();
    std::cerr << this->_base_automaton->DumpToString(*serializer, "symbolic") << "\n";
    delete serializer;

    std::cerr << "[!] Initial states:\n";
    if(this->_initialStates != nullptr) {
        this->_finalStates->dump();
        std::cerr << "\n";
    } else {
        std::cerr << "-> not initialized\n";
    }

    std::cerr << "[!] Final states:\n";
    if(this->_finalStates != nullptr) {
        this->_initialStates->dump();
        std::cerr << "\n";
    } else {
        std::cerr << "-> not initialized\n";
    }
    std::cerr << "[----------------------->]\n";
}