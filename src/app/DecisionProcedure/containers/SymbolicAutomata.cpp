/*****************************************************************************
 *  gaston - no real logic behind the name, we simply liked the poor seal gaston. R.I.P. brave soldier.
 *
 *  Copyright (c) 2015  Tomas Fiedor <ifiedortom@fit.vutbr.cz>
 *      Notable mentions: Ondrej Lengal <ondra.lengal@gmail.com>
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

using namespace Gaston;

/**
 * Transforms @p symbols according to the bound variable in @p vars, by pumping
 *  0 and 1 on the tracks
 *
 * @param[in,out] symbols:  list of symbols, that will be transformed
 * @param[in] vars:         list of used vars, that are projected
 */
void initialize_symbols(SymbolList &symbols, IdentList* vars) {
    // TODO: Optimize, this sucks
    unsigned int symNum = 1;
    for(auto var = vars->begin(); var != vars->end(); ++var) {
        // Pop symbol;
        for(auto i = symNum; i != 0; --i) {
            Symbol symF = symbols.front();
            symbols.pop_front();
            Symbol zero(symF.GetTrack(), varMap[(*var)], '0');
            Symbol one(symF.GetTrack(), varMap[(*var)], '1');
            symbols.push_back(zero);
            symbols.push_back(one);
        }

        symNum <<= 1;// times 2
    }
}

/**
 * @param[in] symbol:               symbol we are minusing away
 * @param[in] stateApproximation:   approximation of final states
 * @param[in] underComplement:      true, if we are under the complement
 * @return:                         (fixpoint, true if nonemptyintersect)
 */
ResultType SymbolicAutomaton::IntersectNonEmpty(Symbol_ptr symbol, Term_ptr stateApproximation, bool underComplement) {
    assert(this->type != AutType::SYMBOLIC_BASE);
    ResultType result;

    #if (DEBUG_INTERSECT_NON_EMPTY == true)
    std::cout << "\nIntersectNonEmpty(";
    if(symbol != nullptr) {
        std::cout << (*symbol);
    } else {
        std::cout << "''";
    }
    std::cout << ",";
    if(stateApproximation == nullptr) {
        std::cout << "nullptr";
    } else {
        stateApproximation->dump();
    }
    std::cout << ", " << (underComplement ? "True" : "False");
    std::cout << ")\n";
    #endif

    if(symbol != nullptr) {
        symbol = new Symbol(symbol->GetTrack());

        auto it = this->_freeVars.begin();
        auto end = this->_freeVars.end();
        for(size_t var = 0; var < varMap.TrackLength(); ++var) {
            if (it != end && var == *it) {
                ++it;
            } else {
                symbol->ProjectVar(var);
            }
        }
    }

    #if (OPT_CACHE_RESULTS == true)
    std::shared_ptr<Symbol> symbolKey = nullptr;
    if(symbol != nullptr) {
        symbolKey = std::shared_ptr<Symbol>(new Symbol(symbol->GetTrack()));
    }
    auto key = std::make_pair(stateApproximation, symbolKey);
    if(this->_resCache.retrieveFromCache(key, result)) {
        return result;
    }
    #endif

    if(stateApproximation != nullptr && stateApproximation->type == TERM_CONT_ISECT) {
        TermContProduct* cont = reinterpret_cast<TermContProduct*>(stateApproximation.get());
        stateApproximation = (cont->aut->IntersectNonEmpty((cont->symbol == nullptr ? nullptr : cont->symbol.get()), cont->term, false)).first;
        //                                                                                                           ^--- is this ok?
    }

    if(stateApproximation != nullptr && stateApproximation->type == TERM_CONT_SUBSET) {
        TermContSubset* contS = reinterpret_cast<TermContSubset*>(stateApproximation.get());
        stateApproximation = (contS->aut->IntersectNonEmpty((contS->symbol == nullptr ? nullptr : contS->symbol.get()), contS->term, true)).first;
        //                                                                                                               ^--- is this ok?
    }

    result = this->_IntersectNonEmptyCore(symbol, stateApproximation, underComplement);

    #if (OPT_CACHE_RESULTS == true)
    if(!this->_resCache.inCache(key)) {
        this->_resCache.StoreIn(key, result);
    }
    #endif

    #if (DEBUG_INTERSECT_NON_EMPTY == true)
    std::cout << "\nIntersectNonEmpty(";
    if(symbol != nullptr) {
        std::cout << (*symbol);
    } else {
        std::cout << "''";
    }
    std::cout << ",";
    if(stateApproximation == nullptr) {
        std::cout << "nullptr";
    } else {
        stateApproximation->dump();
    }
    std::cout << ") = " << (result.second ? "True" : "False") << "\n";
    #endif
    return result;
}

/**
 * Lazy evaluation of final states. If states are not initialized, they are recreated and returned
 *
 * @return: Final states of automaton as Term
 */
Term_ptr SymbolicAutomaton::GetFinalStates() {
    if(this->_finalStates == nullptr) {
        this->_InitializeFinalStates();
    }

    return this->_finalStates;
}

/**
 * Lazy evaluation of initial states. If state are not initialized, they are recreated and returned
 *
 * @return: Initial states of automaton as Term
 */
Term_ptr SymbolicAutomaton::GetInitialStates() {
    if(this->_initialStates == nullptr) {
        this->_InitializeInitialStates();
    }

    return this->_initialStates;
}

/**
 * Initialization of Base Automaton. First we rename the states according
 * to the shared class counter, so every base automaton is disjunctive.
 *
 * Then we initialize Initial and Base states.
 */
void BaseAutomaton::_InitializeAutomaton() {
    // TODO: Maybe this could be done only, if we are dumping the automaton?
    this->_RenameStates();
    this->_InitializeInitialStates();
    this->_InitializeFinalStates();
}

/**
 * Initialization of initial states for automata wrt. the structure of the symbolic automaton
 */
void BinaryOpAutomaton::_InitializeInitialStates() {
    this->_initialStates = std::shared_ptr<Term>(new TermProduct(this->_lhs_aut->GetInitialStates(), this->_rhs_aut->GetInitialStates()));
}

void ComplementAutomaton::_InitializeInitialStates() {
    this->_finalStates = std::shared_ptr<Term>(new TermList(this->_aut->GetFinalStates(), true));
}

void ProjectionAutomaton::_InitializeInitialStates() {
    this->_initialStates = std::shared_ptr<Term>(new TermList(this->_aut->GetInitialStates(), false));
}

// TODO: Optimize so this uses boost::dynamic_bitset instead
void BaseAutomaton::_InitializeInitialStates() {
    // NOTE: The automaton is constructed backwards, so final states are initial
    assert(this->_initialStates == nullptr);

    TermBaseSet* temp = new TermBaseSet();
    for(auto state : this->_base_automaton->GetFinalStates()) {
        temp->states.push_back(state);
    }

    this->_initialStates = std::shared_ptr<Term>(temp);
}

/**
 * Initialization of final states for automata wrt. the structure of the symbolic automaton
 */
void BinaryOpAutomaton::_InitializeFinalStates() {
    this->_finalStates = std::shared_ptr<Term>(new TermProduct(this->_lhs_aut->GetFinalStates(), this->_rhs_aut->GetFinalStates()));
}

void ComplementAutomaton::_InitializeFinalStates() {
    this->_initialStates = std::shared_ptr<Term>(new TermList(this->_aut->GetInitialStates(), true));
}

void ProjectionAutomaton::_InitializeFinalStates() {
    this->_finalStates = std::shared_ptr<Term>(new TermList(this->_aut->GetFinalStates(), false));
}

// TODO: Refactor a little
void BaseAutomaton::_InitializeFinalStates() {
    // NOTE: The automaton is constructed backwards, so initial states are finals
    assert(this->_finalStates == nullptr);

    BaseAutomatonStates finalStates;
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

Term_ptr BinaryOpAutomaton::Pre(Symbol_ptr symbol, Term_ptr finalApproximation, bool underComplement) {
    assert(false && "Doing Pre on BinaryOp Automaton!");
}

Term_ptr ComplementAutomaton::Pre(Symbol_ptr symbol, Term_ptr finalApproximation, bool underComplement) {
    assert(false && "Doing Pre on Complement Automaton!");
}

Term_ptr ProjectionAutomaton::Pre(Symbol_ptr symbol, Term_ptr finalApproximation, bool underComplement) {
    assert(false && "Doing Pre on Projection Automaton!");
}

Term_ptr BaseAutomaton::Pre(Symbol_ptr symbol, Term_ptr finalApproximation, bool underComplement) {
    assert(symbol != nullptr);
    // TODO: Cache MTBDD for pre?
    // TODO: Consult the correctness of cpre/pre computation

    TermBaseSet* base = reinterpret_cast<TermBaseSet*>(finalApproximation.get());
    BaseAutomatonStates states;

    for(auto state : base->states) {
        // Get MTBDD for Pre of states @p state
        BaseAut_MTBDD* preState = getMTBDDForStateTuple(*this->_base_automaton, StateTuple({state}));

        // Create the masker functor, that will mask the states away
        MaskerFunctor masker;
        const BaseAut_MTBDD &temp = masker(*preState, *(symbol->GetMTBDD()));

        // Collect the states and set the flag that we already got some states
        BaseCollectorFunctor collector(states, underComplement);
        collector(temp);
        collector._isFirst = false;
    }

    return std::shared_ptr<Term>(new TermBaseSet(states));
}

ResultType BinaryOpAutomaton::_IntersectNonEmptyCore(Symbol_ptr symbol, Term_ptr final, bool underComplement) {
    assert(final != nullptr);
    assert(final->type == TERM_PRODUCT || final->type == TERM_UNION);
    TermProduct* finalApprox = reinterpret_cast<TermProduct*>(final.get());

    ResultType lhs_result = this->_lhs_aut->IntersectNonEmpty(symbol, finalApprox->left, underComplement);
    #if (OPT_EARLY_EVALUATION == true)
    if(this->_eval_early(lhs_result.second, underComplement)) {
        std::shared_ptr<Symbol> contSymbol = (symbol == nullptr) ? nullptr : std::shared_ptr<Symbol>(new ZeroSymbol(symbol->GetTrack()));
        if(!underComplement) {
            TermContProduct *rhsCont = new TermContProduct(this->_rhs_aut, finalApprox->right, contSymbol);
            Term_ptr leftCombined = std::shared_ptr<Term>(
                    new TermProduct(lhs_result.first, std::shared_ptr<Term>(rhsCont)));
            return std::make_pair(leftCombined, this->_early_val(underComplement));
        } else {
            TermContSubset *rhsContSub = new TermContSubset(this->_rhs_aut, finalApprox->right, contSymbol);
            Term_ptr leftCombinedSub = std::shared_ptr<Term>(
                    new TermProduct(lhs_result.first, std::shared_ptr<Term>(rhsContSub)));
            return std::make_pair(leftCombinedSub, this->_early_val(underComplement));
        }
    }
    #endif

    ResultType rhs_result = this->_rhs_aut->IntersectNonEmpty(symbol, finalApprox->right, underComplement);
    Term_ptr combined = std::shared_ptr<Term>(new TermProduct(lhs_result.first, rhs_result.first));
    return std::make_pair(combined, this->_eval_result(lhs_result.second, rhs_result.second, underComplement));
}

ResultType ComplementAutomaton::_IntersectNonEmptyCore(Symbol_ptr symbol, Term_ptr final, bool underComplement) {
    // TODO: Is this ok? Can there be something more
    assert(final->type == TERM_LIST);

    TermList* approx = reinterpret_cast<TermList*>(final.get());
    assert(approx->list.size() == 1);

    ResultType result = this->_aut->IntersectNonEmpty(symbol, approx->list[0], !underComplement);
    // TODO: We need some information, that this is negation. Is this ok?
    TermList* newResult = new TermList(result.first, true);
    return std::make_pair(std::shared_ptr<Term>(newResult), result.second);
}

ResultType ProjectionAutomaton::_IntersectNonEmptyCore(Symbol_ptr symbol, Term_ptr final, bool underComplement) {
    assert(final != nullptr);
    // TODO: Can there be Continuation?
    assert(final->type == TERM_LIST || final->type == TERM_FIXPOINT);

    if(symbol == nullptr) {
        TermList* finalApprox = reinterpret_cast<TermList*>(final.get());
        assert(finalApprox->list.size() == 1);

        // Evaluate the zero unfoldings
        ResultType result = this->_aut->IntersectNonEmpty(symbol, finalApprox->list[0], underComplement);

        std::list<Symbol> symbols;
        symbol = new ZeroSymbol();
        symbols.push_back(*symbol);
        // Transform the symbols
        ASTForm_uvf* form = reinterpret_cast<ASTForm_uvf*>(this->_form);
        initialize_symbols(symbols, form->vl);

        TermFixpointStates* fixpoint = new TermFixpointStates(this->_aut, result.first, symbols, underComplement, result.second);
        TermFixpointStates::iterator it = fixpoint->GetIterator();
        Term_ptr term;

#if (DEBUG_COMPUTE_FULL_FIXPOINT == true)
        while((term = it.GetNext()) != nullptr) {}
        #else
        if(result.second == !underComplement) {
            return std::make_pair(std::shared_ptr<Term>(fixpoint), result.second);
        }

        while( ((term = it.GetNext()) != nullptr) && (underComplement == fixpoint->GetResult())) {}
        //                                            ^--- is this right?
#endif

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

ResultType BaseAutomaton::_IntersectNonEmptyCore(Symbol_ptr symbol, Term_ptr approx, bool underComplement) {
    // initState = {init}
    ResultType tmp;
    TermBaseSet* initial = reinterpret_cast<TermBaseSet*>(this->_initialStates.get());
    TermBaseSet* final = reinterpret_cast<TermBaseSet*>(this->_finalStates.get());

    // Computing Intersection for epsilon symbol, i.e. only whether it intersects the initial states
    if(symbol == nullptr) {
        //assert(initial->states.size() == 1 && "There should be only one initial state"); // TODO: FUCK, there can be more
        return std::make_pair(this->_finalStates, initial->Intersects(final) != underComplement);
        // Doing Pre(final), i.e. one step back from final states
    } else {
        Term_ptr preSet = this->Pre(symbol, approx, underComplement);
        TermBaseSet* preFinal = reinterpret_cast<TermBaseSet*>(preSet.get());
        return std::make_pair(preSet, initial->Intersects(preFinal) != underComplement);
    }
}

void BinaryOpAutomaton::dump() {
    std::cout << "(";
    _lhs_aut->dump();
    std::cout << " x ";
    _rhs_aut->dump();
    std::cout << ")";
}

void ComplementAutomaton::dump() {
    std::cout << "compl(";
    this->_aut->dump();
    std::cout << ")";
}

void ProjectionAutomaton::dump() {
    std::cout << "ex2(";
    this->_aut->dump();
    std::cout << ")";
}

void BaseAutomaton::_RenameStates() {
    StateToStateMap translMap;
    StateToStateTranslator stateTransl(translMap,
                                       [](const StateType &) { return SymbolicAutomaton::stateCnt++; });
    this->_base_automaton.reset(new BaseAutomatonType(this->_base_automaton->ReindexStates(stateTransl)));
}

void BaseAutomaton::baseAutDump() {
    std::cout << "\n[----------------------->]\n";
    std::cout << "[!] Base VATA Automaton\n";
    VATA::Serialization::AbstrSerializer *serializer = new VATA::Serialization::TimbukSerializer();
    std::cout << this->_base_automaton->DumpToString(*serializer, "symbolic") << "\n";
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