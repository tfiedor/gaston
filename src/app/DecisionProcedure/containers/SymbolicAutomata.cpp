/*****************************************************************************
 *  gaston - We pay homage to Gaston, an Africa-born brown fur seal who
 *    escaped the Prague Zoo during the floods in 2002 and made a heroic
 *    journey for freedom of over 300km all the way to Dresden. There he
 *    was caught and subsequently died due to exhaustion and infection.
 *    Rest In Piece, brave soldier.
 *
 *  Copyright (c) 2015  Tomas Fiedor <ifiedortom@fit.vutbr.cz>
 *      Notable mentions: Ondrej Lengal <ondra.lengal@gmail.com>
 *
 *  Description:
 *      Symbolic Automata representing the formulae. The computation is
 *      done on this representation according to the latest paper.
 *****************************************************************************/

#include <list>
#include "SymbolicAutomata.h"
#include "Term.h"
#include "../environment.hh"
#include "../decision_procedures.hh"
#include "../containers/VarToTrackMap.hh"
#include "../../Frontend/symboltable.h"

extern VarToTrackMap varMap;
extern SymbolTable symbolTable;

StateType SymbolicAutomaton::stateCnt = 0;

using namespace Gaston;

// <<< CONSTRUCTORS >>>
SymbolicAutomaton::SymbolicAutomaton(Formula_ptr form) : _form(form) {
    type = AutType::SYMBOLIC_BASE;

    IdentList free, bound;
    this->_form->freeVars(&free, &bound);
    IdentList* allVars;
    allVars = ident_union(&free, &bound);

    for(auto it = allVars->begin(); it != allVars->end(); ++it) {
        _freeVars.insert(varMap[(*it)]);
    }

    delete allVars;
}

BinaryOpAutomaton::BinaryOpAutomaton(SymbolicAutomaton_raw lhs, SymbolicAutomaton_raw rhs, Formula_ptr form)
        : SymbolicAutomaton(form), _lhs_aut(lhs), _rhs_aut(rhs) {
    type = AutType::BINARY;
    this->_InitializeAutomaton();
}

ComplementAutomaton::ComplementAutomaton(SymbolicAutomaton *aut, Formula_ptr form)
        : SymbolicAutomaton(form), _aut(aut) {
    this->_InitializeAutomaton();
    type = AutType::COMPLEMENT;
}

ProjectionAutomaton::ProjectionAutomaton(SymbolicAutomaton_raw aut, Formula_ptr form)
        : SymbolicAutomaton(form), _aut(aut) {
    this->_InitializeAutomaton();
    type = AutType::PROJECTION;
}

// Derive of BinaryOpAutomaton
IntersectionAutomaton::IntersectionAutomaton(SymbolicAutomaton_raw lhs, SymbolicAutomaton_raw rhs, Formula_ptr form)
        : BinaryOpAutomaton(lhs, rhs, form) {
    this->_InitializeAutomaton();
    this->type = AutType::INTERSECTION;
    this->_productType = E_INTERSECTION;
    this->_eval_result = [](bool a, bool b, bool underC) {
        // e in A cap B == e in A && e in B
        if(!underC) {return a && b;}
        // e notin A cap B == e notin A || e notin B
        else {return a || b;}
    };
    this->_eval_early = [](bool a, bool underC) {
        // e in A && e in B => False
        // e notin A || e notin B => True
        return (a == underC);
    };
    this->_early_val = [](bool underC) {
        return underC;
    };
}

// Derive of BinaryOpAutomaton
UnionAutomaton::UnionAutomaton(SymbolicAutomaton_raw lhs, SymbolicAutomaton_raw rhs, Formula_ptr form)
        : BinaryOpAutomaton(lhs, rhs, form) {
    this->_InitializeAutomaton();
    this->type = AutType::UNION;
    this->_productType = E_UNION;
    this->_eval_result = [](bool a, bool b, bool underC) {
        // e in A cup B == e in A || e in B
        if(!underC) {return a || b;}
        // e notin A cup B == e notin A && e notin B
        else { return a && b;}
    };
    this->_eval_early = [](bool a, bool underC) {
        // e in A || e in B => True
        // e notin A && e notin B => False
        return (a != underC);
    };
    this->_early_val = [](bool underC) {
        return !underC;
    };
}

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

    // Trim the variables that are not occuring in the formula away
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
    // Create a new symbol for cache
    std::shared_ptr<Symbol> symbolKey = nullptr;
    if(symbol != nullptr) {
        symbolKey = std::shared_ptr<Symbol>(new Symbol(symbol->GetTrack()));
    }

    // Look up in cache, if in cache, return the result
    auto key = std::make_pair(stateApproximation.get(), symbolKey);
    if(this->_resCache.retrieveFromCache(key, result)) {
        return result;
    }
    #endif

    // If we have continuation, we have to unwind it
    if(stateApproximation != nullptr && stateApproximation->type == TERM_CONTINUATION) {
        #if (MEASURE_CONTINUATION_EVALUATION == true || MEASURE_ALL == true)
        ++this->_contUnfoldingCounter;
        #endif
        TermContinuation* continuation = reinterpret_cast<TermContinuation*>(stateApproximation.get());
        stateApproximation = (continuation->aut->IntersectNonEmpty((continuation->symbol == nullptr ? nullptr : continuation->symbol.get()), continuation->term, continuation->underComplement)).first;
        //                                                                                                           ^--- is this ok?
    }
    assert(stateApproximation->type != TERM_CONTINUATION);

    // Call the core function
    result = this->_IntersectNonEmptyCore(symbol, stateApproximation, underComplement);
    #if (MEASURE_RESULT_HITS == true || MEASURE_ALL == true)
    if(result.second) {
        ++this->_trueCounter;
    } else {
        ++this->_falseCounter;
    }
    #endif

    // Cache Results
    #if (OPT_CACHE_RESULTS == true)
    if(!this->_resCache.inCache(key)) {
        this->_resCache.StoreIn(key, result);
    }
    #endif

    #if (DEBUG_INTERSECT_NON_EMPTY == true)
    std::cout << "Computed for (";
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
    std::cout << ") = <" << (result.second ? "True" : "False") << ","; result.first->dump(); std::cout << ">\n";
    #endif

    // Return results
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

void BinaryOpAutomaton::_InitializeAutomaton() {
    this->_InitializeInitialStates();
    this->_InitializeFinalStates();
}

void ComplementAutomaton::_InitializeAutomaton() {
    this->_InitializeInitialStates();
    this->_InitializeFinalStates();
}

void ProjectionAutomaton::_InitializeAutomaton() {
    this->_InitializeInitialStates();
    this->_InitializeFinalStates();
    this->_projected_vars = static_cast<ASTForm_uvf*>(this->_form)->vl;
}

/**
 * Initialization of initial states for automata wrt. the structure of the symbolic automaton
 */
void BinaryOpAutomaton::_InitializeInitialStates() {
    this->_initialStates = std::shared_ptr<Term>(new TermProduct(this->_lhs_aut->GetInitialStates(), this->_rhs_aut->GetInitialStates(), this->_productType));
}

void ComplementAutomaton::_InitializeInitialStates() {
    this->_initialStates = this->_aut->GetInitialStates();
}

void ProjectionAutomaton::_InitializeInitialStates() {
    this->_initialStates = std::shared_ptr<Term>(new TermList(this->_aut->GetInitialStates(), false));
}

// TODO: Optimize so this uses boost::dynamic_bitset instead
void BaseAutomaton::_InitializeInitialStates() {
    // NOTE: The automaton is constructed backwards, so final states are initial
    assert(this->_initialStates == nullptr);
    assert(this->_stateSpace != 0);

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
    this->_finalStates = std::shared_ptr<Term>(new TermProduct(this->_lhs_aut->GetFinalStates(), this->_rhs_aut->GetFinalStates(), this->_productType));
}

void ComplementAutomaton::_InitializeFinalStates() {
    this->_finalStates = this->_aut->GetFinalStates();
}

void ProjectionAutomaton::_InitializeFinalStates() {
    this->_finalStates = std::shared_ptr<Term>(new TermList(this->_aut->GetFinalStates(), false));
}

// TODO: Refactor a little
void BaseAutomaton::_InitializeFinalStates() {
    // NOTE: The automaton is constructed backwards, so initial states are finals
    assert(this->_finalStates == nullptr);
    assert(this->_stateSpace != 0);

    // Obtain the MTBDD for Initial states
    BaseAutomatonStateSet finalStates;
    BaseAut_MTBDD* initBDD = getMTBDDForStateTuple(*this->_base_automaton, Automaton::StateTuple());

    // Collect the states on leaves
    StateCollectorFunctor collector(finalStates);
    collector(*initBDD);

    // Push states to new Base Set
    TermBaseSet* finalStateSet = new TermBaseSet();
    for(auto state : finalStates) {
        finalStateSet->states.push_back(state);
    }

    // Return new state set
    this->_finalStates = std::shared_ptr<Term>(finalStateSet);
}

/**
 * Computes the Predecessors of @p finalApproximation through the @p symbol.
 * Right now, we only do the Pre on the base automata and leave the higher
 * levels unkept, and fire assertion error.
 *
 * @param[in] symbol:               symbol for which we are doing Pre on @p finalApproximation
 * @param[in] finalApproximation:   approximation of states that we are computing Pre for
 * @param[in] underComplement:      true, if we are under complement
 */
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

    // Reinterpret the approximation as base states
    TermBaseSet* baseSet = reinterpret_cast<TermBaseSet*>(finalApproximation.get());
    BaseAutomatonStateSet states;

    for(auto state : baseSet->states) {
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

    return std::shared_ptr<Term>(new TermBaseSet(states, this->_stateOffset, this->_stateSpace));
}

/**
 * Tests if Initial states intersects the Final states. Returns the pair of
 * computed fixpoint representation and true/false according to the symbolic
 * automaton type.
 *
 * @param[in] symbol:               symbol we are minusing away
 * @param[in] finalApproximation:   approximation of states that were computed above
 * @param[in] underComplement:      true, if we are computing interesction under complement
 * @return (fixpoint, bool)
 */
ResultType BinaryOpAutomaton::_IntersectNonEmptyCore(Symbol_ptr symbol, Term_ptr finalApproximation, bool underComplement) {
    // TODO: Add counter of continuations per node
    assert(finalApproximation != nullptr);
    assert(finalApproximation->type == TERM_PRODUCT);

    // Retype the approximation to TermProduct type
    TermProduct* productStateApproximation = reinterpret_cast<TermProduct*>(finalApproximation.get());

    // Checks if left automaton's initial states intersects the final states
    ResultType lhs_result = this->_lhs_aut->IntersectNonEmpty(symbol, productStateApproximation->left, underComplement);

    // We can prune the state if left side was evaluated as Empty term
    // TODO: This is different for Unionmat!
    if(false && lhs_result.first->type == TERM_EMPTY && this->_productType == ProductType::E_INTERSECTION) {
        return std::make_pair(lhs_result.first, underComplement);
    }

    #if (OPT_EARLY_EVALUATION == true)
    // Sometimes we can evaluate the experession early and return the continuation.
    // For intersection of automata we can return early, if left term was evaluated
    // as false, whereas for union of automata we can return early if left term
    // was true.
    if(this->_eval_early(lhs_result.second, underComplement)) {
        // Construct the pointer for symbol (either symbol or epsilon---nullptr)
        std::shared_ptr<Symbol> suspendedSymbol = (symbol == nullptr) ? nullptr : std::shared_ptr<Symbol>(new ZeroSymbol(symbol->GetTrack()));

        #if (MEASURE_CONTINUATION_CREATION == true || MEASURE_ALL == true)
        ++this->_contCreationCounter;
        #endif
        TermContinuation *continuation = new TermContinuation(this->_rhs_aut, productStateApproximation->right, suspendedSymbol, underComplement);
        Term_ptr leftCombined = std::shared_ptr<Term>(new TermProduct(lhs_result.first, std::shared_ptr<Term>(continuation), this->_productType));
        return std::make_pair(leftCombined, this->_early_val(underComplement));
    }
    #endif

    // Otherwise compute the right side and return full fixpoint
    ResultType rhs_result = this->_rhs_aut->IntersectNonEmpty(symbol, productStateApproximation->right, underComplement);
    // We can prune the state if right side was evaluated as Empty term
    // TODO: This is different for Unionmat!
    if(false && rhs_result.first->type == TERM_EMPTY && this->_productType == ProductType::E_INTERSECTION) {
        return std::make_pair(rhs_result.first, underComplement);
    }

    Term_ptr combined = std::shared_ptr<Term>(new TermProduct(lhs_result.first, rhs_result.first, this->_productType));
    return std::make_pair(combined, this->_eval_result(lhs_result.second, rhs_result.second, underComplement));
}

ResultType ComplementAutomaton::_IntersectNonEmptyCore(Symbol_ptr symbol, Term_ptr finalApproximaton, bool underComplement) {
    // Compute the result of nested automaton with switched complement
    ResultType result = this->_aut->IntersectNonEmpty(symbol, finalApproximaton, !underComplement);
    result.first->Complement();

    return result;
}

ResultType ProjectionAutomaton::_IntersectNonEmptyCore(Symbol_ptr symbol, Term_ptr finalApproximation, bool underComplement) {
    // TODO: There can be continutation probably
    assert(finalApproximation != nullptr);
    assert(finalApproximation->type == TERM_LIST || finalApproximation->type == TERM_FIXPOINT);

    if(symbol == nullptr) {
        // We are doing the initial step by evaluating the epsilon
        TermList* projectionApproximation = reinterpret_cast<TermList*>(finalApproximation.get());
        assert(projectionApproximation->list.size() == 1);

        // Evaluate the initial unfolding of epsilon
        ResultType result = this->_aut->IntersectNonEmpty(symbol, projectionApproximation->list[0], underComplement);

        // Create the new Zero symbol and initialize the symbol list with all projections
        SymbolList symbols;
        symbol = new ZeroSymbol();
        symbols.push_back(*symbol);
        // Transform the symbols
        ASTForm_uvf* form = reinterpret_cast<ASTForm_uvf*>(this->_form);
        initialize_symbols(symbols, form->vl);

        // Create a new fixpoint term and iterator on it
        TermFixpoint* fixpoint = new TermFixpoint(this->_aut, result.first, symbols, underComplement, result.second);
        TermFixpoint::iterator it = fixpoint->GetIterator();
        Term_ptr fixpointTerm;

        #if (DEBUG_COMPUTE_FULL_FIXPOINT == true)
        // Computes the whole fixpoint, withouth early evaluation
        while((fixpointTerm = it.GetNext()) != nullptr) {}
        #else
        // Early evaluation of fixpoint
        if(result.second == !underComplement) {
            return std::make_pair(std::shared_ptr<Term>(fixpoint), result.second);
        }

        // While the fixpoint is not fully unfolded and while we cannot evaluate early
        while( ((fixpointTerm = it.GetNext()) != nullptr) && (underComplement == fixpoint->GetResult())) {}
        //                                                    ^--- is this right?
        #endif

        // Return (fixpoint, bool)
        return std::make_pair(std::shared_ptr<Term>(fixpoint), fixpoint->GetResult());
    } else {
        // Project the quantified symbols and push them to symbol list
        SymbolList symbols;
        symbols.push_back(*symbol);
        // Transform the symbols
        ASTForm_uvf* form = reinterpret_cast<ASTForm_uvf*>(this->_form);
        initialize_symbols(symbols, form->vl);

        // Create a new fixpoint term and iterator on it
        TermFixpoint* fixpoint = new TermFixpoint(this->_aut, finalApproximation, symbols, underComplement);
        TermFixpoint::iterator it = fixpoint->GetIterator();
        Term_ptr fixpointTerm;

        // Compute the Pre of the fixpoint
        while( ((fixpointTerm = it.GetNext()) != nullptr) && (underComplement == fixpoint->GetResult())) {}

        // Return (fixpoint, bool)
        return std::make_pair(std::shared_ptr<Term>(fixpoint), fixpoint->GetResult());
    }
}

ResultType BaseAutomaton::_IntersectNonEmptyCore(Symbol_ptr symbol, Term_ptr approximation, bool underComplement) {
    // Reinterpret the initial and final states
    TermBaseSet* initial = reinterpret_cast<TermBaseSet*>(this->_initialStates.get());
    TermBaseSet* final = reinterpret_cast<TermBaseSet*>(this->_finalStates.get());

    if(symbol == nullptr) {
        // Testing if epsilon is in language, i.e. testing if final states intersect initial ones
        return std::make_pair(this->_finalStates, initial->Intersects(final) != underComplement);
    } else if(approximation->type == TERM_EMPTY) {
        // Empty set has no Pre
        return std::make_pair(approximation, underComplement);
    } else {
        // First do the pre of the approximation
        Term_ptr preSet = this->Pre(symbol, approximation, underComplement);
        TermBaseSet* preFinal = reinterpret_cast<TermBaseSet*>(preSet.get());

        // Return the pre and true if it intersects the initial states
        if(preFinal->IsEmpty()) {
            return std::make_pair(std::shared_ptr<Term>(new TermEmpty()), underComplement);
        } else {
            return std::make_pair(preSet, initial->Intersects(preFinal) != underComplement);
        }
    }
}

void BinaryOpAutomaton::DumpAutomaton() {
    if(this->type == AutType::INTERSECTION) {
        std::cout << "\033[1;32m";
    } else {
        std::cout << "\033[1;33m";
    }
    std::cout << "(\033[0m";
    _lhs_aut->DumpAutomaton();
    if(this->type == AutType::INTERSECTION) {
        std::cout << "\033[1;32m \u2229 \033[0m";
    } else {
        //std::cout << " \u22C3 ";
        std::cout << "\033[1;33m \u222A \033[0m";
    };
    _rhs_aut->DumpAutomaton();
    if(this->type == AutType::INTERSECTION) {
        std::cout << "\033[1;32m";
    } else {
        std::cout << "\033[1;33m";
    }
    std::cout << ")\033[0m";
}

void ComplementAutomaton::DumpAutomaton() {
    std::cout << "\033[1;31m\u2201(\033[0m";
    this->_aut->DumpAutomaton();
    std::cout << "\033[1;31m)\033[0m";
}

void ProjectionAutomaton::DumpAutomaton() {
    std::cout << "\033[1;34m";
    std::cout << "\u2203";
    for(auto it = this->_projected_vars->begin(); it != this->_projected_vars->end(); ++it) {
        std::cout << symbolTable.lookupSymbol(*it);
        if((it + 1) != this->_projected_vars->end()) {
            std::cout << ", ";
        }
    }
    std::cout << "(\033[0m";
    this->_aut->DumpAutomaton();
    std::cout << "\033[1;34m)\033[0m";
}

void GenericBaseAutomaton::DumpAutomaton() { std::cout << "<Aut>";
    #if (DEBUG_BASE_AUTOMATA == true)
    this->BaseAutDump();
    #endif
}

void SubAutomaton::DumpAutomaton() {
    this->_form->dump();
    #if (DEBUG_BASE_AUTOMATA == true)
    this->BaseAutDump();
    #endif
}

void TrueAutomaton::DumpAutomaton() {
    this->_form->dump();
    #if (DEBUG_BASE_AUTOMATA == true)
    this->BaseAutDump();
    #endif
}

void FalseAutomaton::DumpAutomaton() {
    this->_form->dump();
    #if (DEBUG_BASE_AUTOMATA == true)
    this->BaseAutDump();
    #endif
}

void InAutomaton::DumpAutomaton() {
    this->_form->dump();
    #if (DEBUG_BASE_AUTOMATA == true)
    this->BaseAutDump();
    #endif
}

void FirstOrderAutomaton::DumpAutomaton() {
    this->_form->dump();
    #if (DEBUG_BASE_AUTOMATA == true)
    this->BaseAutDump();
    #endif
}

void EqualFirstAutomaton::DumpAutomaton() {
    this->_form->dump();
    #if (DEBUG_BASE_AUTOMATA == true)
    this->BaseAutDump();
    #endif
}

void EqualSecondAutomaton::DumpAutomaton() {
    this->_form->dump();
    #if (DEBUG_BASE_AUTOMATA == true)
    this->BaseAutDump();
    #endif
}

void LessAutomaton::DumpAutomaton() {
    this->_form->dump();
    #if (DEBUG_BASE_AUTOMATA == true)
    this->BaseAutDump();
    #endif
}

void LessEqAutomaton::DumpAutomaton() {
    this->_form->dump();
    #if (DEBUG_BASE_AUTOMATA == true)
    this->BaseAutDump();
    #endif
}

/**
 * Renames the states according to the translation function so we get unique states.
 */
void BaseAutomaton::_RenameStates() {
    this->_stateOffset = SymbolicAutomaton::stateCnt;
    StateToStateMap translMap;
    StateToStateTranslator stateTransl(translMap,
                                       [](const StateType &) { return SymbolicAutomaton::stateCnt++; });
    this->_base_automaton.reset(new BaseAutomatonType(this->_base_automaton->ReindexStates(stateTransl)));
    this->_stateSpace = SymbolicAutomaton::stateCnt - this->_stateOffset;
}

void BaseAutomaton::BaseAutDump() {
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

/**
 * Dump Cache stats of automaton only
 */
void BinaryOpAutomaton::DumpCacheStats() {
    this->_form->dump();
    this->_resCache.dumpStats();
    this->_lhs_aut->DumpCacheStats();
    this->_rhs_aut->DumpCacheStats();
}

void ComplementAutomaton::DumpCacheStats() {
    this->_form->dump();
    this->_resCache.dumpStats();
    this->_aut->DumpCacheStats();
}

void ProjectionAutomaton::DumpCacheStats() {
    this->_form->dump();
    this->_resCache.dumpStats();
    this->_aut->DumpCacheStats();
}

void BaseAutomaton::DumpCacheStats() {
    this->_form->dump();
    this->_resCache.dumpStats();
}

/**
 * Dumps stats for automata
 *
 * 1) True/False hits
 * 2) Cache hit/miss
 * 3) Number of iterations in projection
 * 4) Number of symbols evaluated in projection
 * 5) Number of evaluated continuations
 * 6) Number of created continuation
 */
void print_stat(std::string statName, unsigned int stat) {
    if(stat != 0) {
        std::cout << "  \u2218 " << statName << " -> " << stat << "\n";
    }
}

void BinaryOpAutomaton::DumpStats() {
    this->_form->dump();
    std::cout << "\n";
    std::cout << "  \u2218 Cache stats -> ";
    this->_resCache.dumpStats();
    print_stat("True Hits", this->_trueCounter);
    print_stat("False Hits", this->_falseCounter);
    print_stat("Continuation Generation", this->_contCreationCounter);
    print_stat("Continuation Evaluation", this->_contUnfoldingCounter);

    this->_lhs_aut->DumpStats();
    this->_rhs_aut->DumpStats();
}

void ProjectionAutomaton::DumpStats() {
    this->_form->dump();
    std::cout << "\n";
    std::cout << "  \u2218 Cache stats -> ";
    this->_resCache.dumpStats();
    print_stat("True Hits", this->_trueCounter);
    print_stat("False Hits", this->_falseCounter);
    print_stat("Continuation Evaluation", this->_contUnfoldingCounter);

    this->_aut->DumpStats();
}

void ComplementAutomaton::DumpStats() {
    this->_form->dump();
    std::cout << "\n";
    std::cout << "  \u2218 Cache stats -> ";
    this->_resCache.dumpStats();
    print_stat("True Hits", this->_trueCounter);
    print_stat("False Hits", this->_falseCounter);
    print_stat("Continuation Evaluation", this->_contUnfoldingCounter);

    this->_aut->DumpStats();
}

void BaseAutomaton::DumpStats() {
    this->_form->dump();
    std::cout << "\n";
    std::cout << "  \u2218 Cache stats -> ";
    this->_resCache.dumpStats();
    print_stat("True Hits", this->_trueCounter);
    print_stat("False Hits", this->_falseCounter);
    print_stat("Continuation Evaluation", this->_contUnfoldingCounter);
}