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
#include <stdint.h>
#include "SymbolicAutomata.h"
#include "Term.h"
#include "../environment.hh"
#include "../containers/VarToTrackMap.hh"
#include "../containers/Workshops.h"
#include "../../Frontend/dfa.h"
#include "../../Frontend/symboltable.h"
#include "../../Frontend/timer.h"

extern VarToTrackMap varMap;
extern SymbolTable symbolTable;
extern Ident lastPosVar, allPosVar;

StateType SymbolicAutomaton::stateCnt = 0;
// Fixme: Delete this shit
DagNodeCache* SymbolicAutomaton::dagNodeCache = new DagNodeCache();
DagNodeCache* SymbolicAutomaton::dagNegNodeCache = new DagNodeCache();

using namespace Gaston;

// <<< SYMLINK FUNCTIONS >>>
void SymLink::InitializeSymLink(ASTForm* form) {
    if(this->aut->_form != form) {
        // Construct the mapping
        this->varRemap = new std::map<unsigned int, unsigned int>();
        this->remap = true;
        form->ConstructMapping(this->aut->_form, *this->varRemap);
#       if (DEBUG_DAG_REMAPPING == true)
        std::cout << "[!] Mapping for: "; form->dump(); std::cout << "\n";
        for(auto it = this->varRemap->begin(); it != this->varRemap->end(); ++it) {
            std::cout << (it->first) << " -> " << (it->second) << "\n";
        }
#       endif
    }
}

ZeroSymbol* SymLink::ReMapSymbol(ZeroSymbol* symbol) {
    if(this->remap && symbol != nullptr) {
        return this->aut->symbolFactory->CreateRemappedSymbol(symbol, this->varRemap);
    } else {
        return symbol;
    }
}

// <<< CONSTRUCTORS >>>
SymbolicAutomaton::SymbolicAutomaton(Formula_ptr form) :
        _form(form), _factory(this), _initialStates(nullptr), _finalStates(nullptr), _satExample(nullptr),
        _unsatExample(nullptr), _refs(0) {
    type = AutType::SYMBOLIC_BASE;

    this->symbolFactory = new Workshops::SymbolWorkshop();
    IdentList free, bound;
    this->_form->freeVars(&free, &bound);
    IdentList* allVars;
    allVars = ident_union(&free, &bound);
    if(allVars != nullptr) {
        for(auto it = allVars->begin(); it != allVars->end(); ++it) {
            if(varMap.IsIn(*it))
                _freeVars.insert(varMap[(*it)]);
        }

        delete allVars;
    }
}

SymbolicAutomaton::~SymbolicAutomaton() {
    delete this->symbolFactory;
}

BinaryOpAutomaton::BinaryOpAutomaton(SymbolicAutomaton_raw lhs, SymbolicAutomaton_raw rhs, Formula_ptr form)
        : SymbolicAutomaton(form), _lhs_aut(lhs), _rhs_aut(rhs) {
    type = AutType::BINARY;
    lhs->IncReferences();
    rhs->IncReferences();
    this->_lhs_aut.InitializeSymLink(reinterpret_cast<ASTForm_ff*>(this->_form)->f1);
    this->_rhs_aut.InitializeSymLink(reinterpret_cast<ASTForm_ff*>(this->_form)->f2);
}

BinaryOpAutomaton::~BinaryOpAutomaton() {
    this->_lhs_aut.aut->DecReferences();
    this->_rhs_aut.aut->DecReferences();
}

ComplementAutomaton::ComplementAutomaton(SymbolicAutomaton *aut, Formula_ptr form)
        : SymbolicAutomaton(form), _aut(aut) {
    type = AutType::COMPLEMENT;
    this->_InitializeAutomaton();
    aut->IncReferences();
    this->_aut.InitializeSymLink(reinterpret_cast<ASTForm_Not*>(this->_form)->f);
}

ComplementAutomaton::~ComplementAutomaton() {
    this->_aut.aut->DecReferences();
}

ProjectionAutomaton::ProjectionAutomaton(SymbolicAutomaton_raw aut, Formula_ptr form, bool isRoot)
        : SymbolicAutomaton(form), _aut(aut) {
    type = AutType::PROJECTION;
    this->_isRoot = isRoot;
    this->_InitializeAutomaton();
    aut->IncReferences();
    this->_aut.InitializeSymLink(reinterpret_cast<ASTForm_q*>(this->_form)->f);
}

ProjectionAutomaton::~ProjectionAutomaton() {
    this->_aut.aut->DecReferences();
}

RootProjectionAutomaton::RootProjectionAutomaton(SymbolicAutomaton* aut, Formula_ptr form)
        : ProjectionAutomaton(aut, form, true) {}

BaseAutomaton::BaseAutomaton(BaseAutomatonType* aut, size_t vars, Formula_ptr form, bool emptyTracks) : SymbolicAutomaton(form), _autWrapper(dfaCopy(aut), emptyTracks, vars) {
    type = AutType::BASE;
    this->_InitializeAutomaton();
    this->_stateSpace = vars;
}

BaseAutomaton::~BaseAutomaton() {

}

// Derive of BinaryOpAutomaton
IntersectionAutomaton::IntersectionAutomaton(SymbolicAutomaton_raw lhs, SymbolicAutomaton_raw rhs, Formula_ptr form)
        : BinaryOpAutomaton(lhs, rhs, form) {
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

    this->_InitializeAutomaton();
}

// Derive of BinaryOpAutomaton
UnionAutomaton::UnionAutomaton(SymbolicAutomaton_raw lhs, SymbolicAutomaton_raw rhs, Formula_ptr form)
        : BinaryOpAutomaton(lhs, rhs, form) {
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

    this->_InitializeAutomaton();
}

/**
 * @param[in] symbol:               symbol we are minusing away
 * @param[in] stateApproximation:   approximation of final states
 * @param[in] underComplement:      true, if we are under the complement
 * @return:                         (fixpoint, true if nonemptyintersect)
 */
ResultType SymbolicAutomaton::IntersectNonEmpty(Symbol* symbol, Term* stateApproximation, bool underComplement) {
    assert(this->type != AutType::SYMBOLIC_BASE);
    assert(stateApproximation != nullptr);
    ResultType result;

    // Empty set needs not to be computed
    if(stateApproximation->type == TERM_EMPTY) {
        return std::make_pair(stateApproximation, underComplement != stateApproximation->InComplement());
    }

#   if (DEBUG_INTERSECT_NON_EMPTY == true)
    std::cout << "\nIntersectNonEmpty(";
    if(symbol != nullptr) {
        std::cout << (*symbol);
    } else {
        std::cout << "''";
    }
    std::cout << ",";
    stateApproximation->dump();
    std::cout << ", " << (underComplement ? "True" : "False");
    std::cout << ")\n";
#   endif

    // Trim the variables that are not occuring in the formula away
    if(symbol != nullptr) {
        symbol = this->symbolFactory->CreateTrimmedSymbol(symbol, &this->_freeVars);
    }

    // If we have continuation, we have to unwind it
    if(stateApproximation->type == TERM_CONTINUATION) {
#       if (MEASURE_CONTINUATION_EVALUATION == true || MEASURE_ALL == true)
        ++this->_contUnfoldingCounter;
#       endif
        TermContinuation* continuation = reinterpret_cast<TermContinuation*>(stateApproximation);
        stateApproximation = continuation->unfoldContinuation(UnfoldedInType::E_IN_ISECT_NONEMPTY);
    }
    assert(stateApproximation != nullptr);
    assert(stateApproximation->type != TERM_CONTINUATION);

    // Empty set needs not to be computed
    if(stateApproximation->type == TERM_EMPTY) {
        return std::make_pair(stateApproximation, underComplement != stateApproximation->InComplement());
    }

#   if (OPT_CACHE_RESULTS == true)
    // Look up in cache, if in cache, return the result
    bool inCache = true;
    auto key = std::make_pair(stateApproximation, symbol);
#       if (OPT_DONT_CACHE_CONT == true)
        bool dontSearchTheCache = (stateApproximation->type == TERM_PRODUCT && stateApproximation->IsNotComputed());
        if (!dontSearchTheCache && (inCache = this->_resCache.retrieveFromCache(key, result))) {
#       else
        if (inCache = this->_resCache.retrieveFromCache(key, result)) {
#       endif
        assert(result.first != nullptr);
        return result;
    }
    //}
#   endif

    // Call the core function
    result = this->_IntersectNonEmptyCore(symbol, stateApproximation, underComplement); // TODO: Memory consumption
#   if (MEASURE_RESULT_HITS == true || MEASURE_ALL == true)
    (result.second ? ++this->_trueCounter : ++this->_falseCounter);
#   endif

    // Cache Results
#   if (OPT_CACHE_RESULTS == true)
#       if (OPT_DONT_CACHE_CONT == true)
        if(stateApproximation->type == TERM_PRODUCT) {
            // If either side is not fully computed, we do not cache it
            TermProduct* tProduct = reinterpret_cast<TermProduct*>(stateApproximation);
            inCache = tProduct->left->IsNotComputed() || tProduct->right->IsNotComputed();
        }
#       endif
#       if (OPT_DONT_CACHE_UNFULL_FIXPOINTS == true)
        if(result.first->type == TERM_FIXPOINT) {
            // If it is not fully computed, we do not cache it
            TermFixpoint* tFix = reinterpret_cast<TermFixpoint*>(stateApproximation);
            inCache |= !tFix->IsFullyComputed();
        }
#       endif
    if(!inCache) {
        this->_resCache.StoreIn(key, result);
    }
#   endif

#   if (DEBUG_INTERSECT_NON_EMPTY == true)
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
#   endif

    if(symbol != nullptr) {
        result.first->SetSuccessor(stateApproximation, symbol);
    }

    // Return results
    assert(result.first != nullptr);
    return result;
}

ResultType RootProjectionAutomaton::IntersectNonEmpty(Symbol* symbol, Term* finalApproximation, bool underComplement) {
    assert(this->_unsatExample == nullptr && this->_satExample == nullptr);

    // We are doing the initial step by evaluating the epsilon
    TermList* projectionApproximation = reinterpret_cast<TermList*>(finalApproximation);
    assert(projectionApproximation->list.size() == 1);

    // Evaluate the initial unfolding of epsilon
    ResultType result = this->_aut.aut->IntersectNonEmpty(this->_aut.ReMapSymbol(symbol), projectionApproximation->list[0], underComplement);

    // Create a new fixpoint term and iterator on it
    TermFixpoint* fixpoint = this->_factory.CreateFixpoint(result.first, SymbolWorkshop::CreateZeroSymbol(), underComplement, result.second, WorklistSearchType::E_UNGROUND_ROOT);
    TermFixpoint::iterator it = fixpoint->GetIterator();
    Term_ptr fixpointTerm = nullptr;

#   if (DEBUG_EXAMPLE_PATHS == true)
    size_t maxPath = 0;
    Timer timer_paths;
    timer_paths.start();
#   endif
    // While the fixpoint is not fully unfolded and while we cannot evaluate early
    while((this->_satExample == nullptr || this->_unsatExample == nullptr) && ((fixpointTerm = it.GetNext()) != nullptr)) {
        fixpoint->RemoveSubsumed();
#       if (DEBUG_EXAMPLE_PATHS == true)
        if(fixpointTerm != nullptr && fixpointTerm->link.len > maxPath) {
            std::cout << "[*] Finished exploring examples of length '" << maxPath << "': ";
            timer_paths.stop();
            timer_paths.print();
            timer_paths.start();
            maxPath = fixpointTerm->link.len;
#           if (DEBUG_MAX_SEARCH_PATH > 0)
            if(maxPath > DEBUG_MAX_SEARCH_PATH) {
                std::cout << "[!] Maximal search depth reached!\n";
                break;
            }
#           endif
        }
#       endif
#       if (DEBUG_ROOT_AUTOMATON == true)
        std::cout << "[!] Fixpoint = "; fixpoint->dump(); std::cout << "\n";
        std::cout << "[!] Explored: ";
        if(fixpointTerm != nullptr)
            std::cout << fixpointTerm;
        else
            std::cout << "nullptr";
        std::cout << " + ";
        if(fixpointTerm != nullptr && fixpointTerm->link.symbol != nullptr) {
            std::cout << (*fixpointTerm->link.symbol);
        } else {
            std::cout << "''";
        }
        std::cout << "\n";
#       endif
        ExamplePair examples = fixpoint->GetFixpointExamples();
#       if (DEBUG_ROOT_AUTOMATON == true)
        std::cout << "[!] Satisfiable example: ";
        if(examples.first != nullptr)
            examples.first->dump();
        std::cout << "\n";
        std::cout << "[!] Unsatisfiable example: ";
        if(examples.second != nullptr)
            examples.second->dump();
        std::cout << "\n";
#       endif
        if(this->_satExample == nullptr && examples.first != nullptr) {
            std::cout << "[*] Found satisfying example\n";
            this->_satExample = examples.first;
        }
        if(this->_unsatExample == nullptr && examples.second != nullptr) {
            std::cout << "[*] Found unsatisfying counter-example\n";
            this->_unsatExample = examples.second;
        }
    }
#   if (DEBUG_EXAMPLE_PATHS == true)
    timer_paths.stop();
#   endif

    return std::make_pair(fixpoint, fixpoint->GetResult());
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

void SymbolicAutomaton::SetSatisfiableExample(Term* satExample) {
    if(this->_satExample == nullptr) {
        this->_satExample = satExample;
    }
}

void SymbolicAutomaton::SetUnsatisfiableExample(Term* unsatExample) {
    if(this->_unsatExample == nullptr) {
        this->_unsatExample = unsatExample;
    }
}

/**
 * Initialization of Base Automaton. First we rename the states according
 * to the shared class counter, so every base automaton is disjunctive.
 *
 * Then we initialize Initial and Base states.
 */
void SymbolicAutomaton::InitializeStates() {
    assert(this->_initialStates == nullptr);
    assert(this->_finalStates == nullptr);
    this->_InitializeAutomaton();
}

void BaseAutomaton::_InitializeAutomaton() {
    // TODO: Maybe this could be done only, if we are dumping the automaton?
    this->_factory.InitializeWorkshop();
    this->_InitializeInitialStates();
    this->_InitializeFinalStates();
}

void BinaryOpAutomaton::_InitializeAutomaton() {
    this->_factory.InitializeWorkshop();
    this->_InitializeInitialStates();
    this->_InitializeFinalStates();
}

void ComplementAutomaton::_InitializeAutomaton() {
    this->_factory.InitializeWorkshop();
    this->_InitializeInitialStates();
    this->_InitializeFinalStates();
}

void ProjectionAutomaton::_InitializeAutomaton() {
    this->_factory.InitializeWorkshop();
    this->_InitializeInitialStates();
    this->_InitializeFinalStates();
    this->projectedVars = static_cast<ASTForm_uvf*>(this->_form)->vl;
}

/**
 * Initialization of initial states for automata wrt. the structure of the symbolic automaton
 */
void BinaryOpAutomaton::_InitializeInitialStates() {
    // #TERM_CREATION
    #if (DEBUG_NO_WORKSHOPS)
    this->_initialStates = new TermProduct(this->_lhs_aut.aut->GetInitialStates(), this->_rhs_aut.aut->GetInitialStates(), this->_productType);
    #else
    this->_initialStates = this->_factory.CreateProduct(this->_lhs_aut.aut->GetInitialStates(), this->_rhs_aut.aut->GetInitialStates(), this->_productType);
    #endif
}

void ComplementAutomaton::_InitializeInitialStates() {
    this->_initialStates = this->_aut.aut->GetInitialStates();
}

void ProjectionAutomaton::_InitializeInitialStates() {
    #if (DEBUG_NO_WORKSHOPS == true)
    this->_initialStates = new TermList(this->_aut.aut->GetInitialStates(), false);
    #else
    this->_initialStates = this->_factory.CreateList(this->_aut.aut->GetInitialStates(), false);
    #endif
}

void BaseAutomaton::_InitializeInitialStates() {
    // NOTE: The automaton is constructed backwards, so final states are initial
    assert(this->_initialStates == nullptr);

    BaseAutomatonStateSet initialStates;
    initialStates.insert(this->_autWrapper.GetInitialState());

    this->_initialStates = this->_factory.CreateBaseSet(initialStates, this->_stateOffset, this->_stateSpace);
}

/**
 * Initialization of final states for automata wrt. the structure of the symbolic automaton
 */
void BinaryOpAutomaton::_InitializeFinalStates() {
    // #TERM_CREATION
    #if (DEBUG_NO_WORKSHOPS == true)
    this->_finalStates = new TermProduct(this->_lhs_aut.aut->GetFinalStates(), this->_rhs_aut.aut->GetFinalStates(), this->_productType);
    #else
    this->_finalStates = this->_factory.CreateProduct(this->_lhs_aut.aut->GetFinalStates(), this->_rhs_aut.aut->GetFinalStates(), this->_productType);
    #endif
}

void ComplementAutomaton::_InitializeFinalStates() {
    this->_finalStates = this->_aut.aut->GetFinalStates();
    assert(this->_finalStates->type != TERM_EMPTY);
    this->_finalStates->Complement();
}

void ProjectionAutomaton::_InitializeFinalStates() {
#   if (DEBUG_NO_WORKSHOPS == true)
    this->_finalStates = new TermList(this->_aut.aut->GetFinalStates(), false);
#   else
    this->_finalStates = this->_factory.CreateList(this->_aut.aut->GetFinalStates(), false);
#   endif
}

void BaseAutomaton::_InitializeFinalStates() {
    // NOTE: The automaton is constructed backwards, so initial states are finals
    assert(this->_finalStates == nullptr);

    // Obtain the MTBDD for Initial states
    BaseAutomatonStateSet finalStates;
    this->_autWrapper.GetFinalStates(finalStates);

    // Push states to new Base Set
    // #TERM_CREATION
    this->_finalStates = reinterpret_cast<Term*>(this->_factory.CreateBaseSet(finalStates, this->_stateOffset, this->_stateSpace));
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
Term* BinaryOpAutomaton::Pre(Symbol* symbol, Term* finalApproximation, bool underComplement) {
    assert(false && "Doing Pre on BinaryOp Automaton!");
}

Term* ComplementAutomaton::Pre(Symbol* symbol, Term* finalApproximation, bool underComplement) {
    assert(false && "Doing Pre on Complement Automaton!");
}

Term* ProjectionAutomaton::Pre(Symbol* symbol, Term* finalApproximation, bool underComplement) {
    assert(false && "Doing Pre on Projection Automaton!");
}

Term* BaseAutomaton::Pre(Symbol* symbol, Term* finalApproximation, bool underComplement) {
    assert(symbol != nullptr);
    // TODO: Implement the -minus

    // Reinterpret the approximation as base states
    TermBaseSet* baseSet = reinterpret_cast<TermBaseSet*>(finalApproximation);
    BaseAutomatonStateSet states;
    BaseAutomatonStateSet preStates;

    #if (DEBUG_PRE == true)
    std::cout << "[!] ";
    this->_form->dump();
    std::cout << "\nComputing: ";
    finalApproximation->dump();
    std::cout << " \u2212\u222A ";
    std::cout << (*symbol) << "\n";
    #endif

    BaseAutomatonStateSet null;

    for(auto state : baseSet->states) {
        // Get MTBDD for Pre of states @p state
        auto key = std::make_pair(state, symbol);
        preStates.clear();
        if(!this->_preCache.retrieveFromCache(key, preStates)) {
            // TODO: there is probably useless copying --^
            preStates = this->_autWrapper.Pre(state, symbol->GetTrackMask());
            this->_preCache.StoreIn(key, preStates);
        }
        states.insert(preStates);
        #if (DEBUG_PRE == true)
        std::cout << "{" << state << "} \u2212 " << (*symbol) << " = " << preStates << "\n";
        #endif
    }

    #if (DEBUG_PRE == true)
    std::cout << "= " << states << "\n";
    #endif

    return this->_factory.CreateBaseSet(states, this->_stateOffset, this->_stateSpace);
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
ResultType BinaryOpAutomaton::_IntersectNonEmptyCore(Symbol* symbol, Term* finalApproximation, bool underComplement) {
    // TODO: Add counter of continuations per node
    assert(finalApproximation != nullptr);
    assert(finalApproximation->type == TERM_PRODUCT);

    // Retype the approximation to TermProduct type
    TermProduct* productStateApproximation = reinterpret_cast<TermProduct*>(finalApproximation);

    // Checks if left automaton's initial states intersects the final states
    ResultType lhs_result = this->_lhs_aut.aut->IntersectNonEmpty(this->_lhs_aut.ReMapSymbol(symbol), productStateApproximation->left, underComplement); // TODO: another memory consumption

    // We can prune the state if left side was evaluated as Empty term
    // TODO: This is different for Unionmat!
#   if (OPT_PRUNE_EMPTY == true)
    if(lhs_result.first->type == TERM_EMPTY && !lhs_result.first->InComplement() && this->_productType == ProductType::E_INTERSECTION) {
        return std::make_pair(lhs_result.first, underComplement);
    }
#   endif

#   if (OPT_EARLY_EVALUATION == true && MONA_FAIR_MODE == false)
    // Sometimes we can evaluate the experession early and return the continuation.
    // For intersection of automata we can return early, if left term was evaluated
    // as false, whereas for union of automata we can return early if left term
    // was true.
#   if (OPT_CONT_ONLY_WHILE_UNSAT == true)
    bool canGenerateContinuations = (this->_productType == ProductType::E_INTERSECTION ?
                                     (this->_trueCounter == 0 && this->_falseCounter >= 0) :
                                     (this->_falseCounter == 0 && this->_trueCounter >= 0) );
    if(canGenerateContinuations && this->_eval_early(lhs_result.second, underComplement)) {
#   else
    if(this->_eval_early(lhs_result.second, underComplement)) {
#   endif
        // Construct the pointer for symbol (either symbol or epsilon---nullptr)
#       if (MEASURE_CONTINUATION_CREATION == true || MEASURE_ALL == true)
        ++this->_contCreationCounter;
#       endif
#       if (DEBUG_NO_WORKSHOPS == true)
        TermContinuation *continuation = new TermContinuation(this->_rhs_aut.aut, productStateApproximation->right, symbol, underComplement);
        Term_ptr leftCombined = new TermProduct(lhs_result.first, continuation, this->_productType);
#       else
        Term* continuation = this->_factory.CreateContinuation(this->_rhs_aut.aut, productStateApproximation->right, symbol, underComplement);
        Term_ptr leftCombined = this->_factory.CreateProduct(lhs_result.first, continuation, this->_productType);
#       endif
        return std::make_pair(leftCombined, this->_early_val(underComplement));
    }
#   endif

    // Otherwise compute the right side and return full fixpoint
    ResultType rhs_result = this->_rhs_aut.aut->IntersectNonEmpty(this->_rhs_aut.ReMapSymbol(symbol), productStateApproximation->right, underComplement);
    // We can prune the state if right side was evaluated as Empty term
    // TODO: This is different for Unionmat!
#   if (OPT_PRUNE_EMPTY == true)
    if(rhs_result.first->type == TERM_EMPTY && !rhs_result.first->InComplement() && this->_productType == ProductType::E_INTERSECTION) {
        return std::make_pair(rhs_result.first, underComplement);
    }
#   endif

    // TODO: #TERM_CREATION
#   if (DEBUG_NO_WORKSHOPS == true)
    Term_ptr combined = new TermProduct(lhs_result.first, rhs_result.first, this->_productType);
#   else
    Term_ptr combined = this->_factory.CreateProduct(lhs_result.first, rhs_result.first, this->_productType);
#   endif
    return std::make_pair(combined, this->_eval_result(lhs_result.second, rhs_result.second, underComplement));
}

ResultType ComplementAutomaton::_IntersectNonEmptyCore(Symbol* symbol, Term* finalApproximaton, bool underComplement) {
    // Compute the result of nested automaton with switched complement
    ResultType result = this->_aut.aut->IntersectNonEmpty(this->_aut.ReMapSymbol(symbol), finalApproximaton, !underComplement);
    // TODO: fix, because there may be falsely complemented things
    if(finalApproximaton->InComplement() != result.first->InComplement()) {
        if(result.first->type == TERM_EMPTY) {
            if(result.first->InComplement()) {
                result.first = Workshops::TermWorkshop::CreateEmpty();
            } else {
                result.first = Workshops::TermWorkshop::CreateComplementedEmpty();
            }
        } else {
            result.first->Complement();
        }
    }

    return result;
}

ResultType ProjectionAutomaton::_IntersectNonEmptyCore(Symbol* symbol, Term* finalApproximation, bool underComplement) {
    // TODO: There can be continutation probably
    assert(finalApproximation != nullptr);
    assert(finalApproximation->type == TERM_LIST || finalApproximation->type == TERM_FIXPOINT);

    if(symbol == nullptr) {
        // We are doing the initial step by evaluating the epsilon
        TermList* projectionApproximation = reinterpret_cast<TermList*>(finalApproximation);
        assert(projectionApproximation->list.size() == 1);

        // Evaluate the initial unfolding of epsilon
        ResultType result = this->_aut.aut->IntersectNonEmpty(this->_aut.ReMapSymbol(symbol), projectionApproximation->list[0], underComplement);

        // Create a new fixpoint term and iterator on it
        #if (DEBUG_NO_WORKSHOPS == true)
        TermFixpoint* fixpoint = new TermFixpoint(this->aut, result.first, SymbolWorkshop::CreateZeroSymbol(), underComplement, result.second);
        #else
        TermFixpoint* fixpoint = this->_factory.CreateFixpoint(result.first, SymbolWorkshop::CreateZeroSymbol(), underComplement, result.second);
        #endif
        TermFixpoint::iterator it = fixpoint->GetIterator();
        Term_ptr fixpointTerm;

        #if (DEBUG_COMPUTE_FULL_FIXPOINT == true || MONA_FAIR_MODE == true)
        // Computes the whole fixpoint, withouth early evaluation
        while((fixpointTerm = it.GetNext()) != nullptr) {
            #if (MEASURE_PROJECTION == true)
            ++this->fixpointNext;
            #endif
        }
        #else
#       if (OPT_NO_SATURATION_FOR_M2L == true)
        // We will not saturate the fixpoint computation when computing the M2L(str) logic
        if(allPosVar != -1) {
#           if (OPT_REDUCE_FULL_FIXPOINT == true)
            fixpoint->RemoveSubsumed();
#           endif
            return std::make_pair(fixpoint, result.second);
        }
#       endif

        // Early evaluation of fixpoint
        if(result.second == !underComplement) {
            #if (OPT_REDUCE_FULL_FIXPOINT == true)
            fixpoint->RemoveSubsumed();
            #endif
            return std::make_pair(fixpoint, result.second);
        }

        // While the fixpoint is not fully unfolded and while we cannot evaluate early
        while( ((fixpointTerm = it.GetNext()) != nullptr) && (underComplement == fixpoint->GetResult())) {
            //                                                ^--- is this right?
            #if (MEASURE_PROJECTION == true)
            ++this->fixpointNext;
            #endif
        }
        #endif

        // Return (fixpoint, bool)
        #if (OPT_REDUCE_FULL_FIXPOINT == true)
        fixpoint->RemoveSubsumed();
        #endif
        return std::make_pair(fixpoint, fixpoint->GetResult());
    } else {
        // Create a new fixpoint term and iterator on it
        #if (DEBUG_NO_WORKSHOPS == true)
        TermFixpoint* fixpoint = new TermFixpoint(this, finalApproximation, symbol, underComplement);
        #else
        TermFixpoint* fixpoint = this->_factory.CreateFixpointPre(finalApproximation, symbol, underComplement);
        #endif
        TermFixpoint::iterator it = fixpoint->GetIterator();
        Term_ptr fixpointTerm;

        // Compute the Pre of the fixpoint
        #if (DEBUG_COMPUTE_FULL_FIXPOINT == true || MONA_FAIR_MODE == true)
            while((fixpointTerm = it.GetNext()) != nullptr) {
                #if (MEASURE_PROJECTION == true)
                ++this->fixpointPreNext;
                #endif
            };
        #else
            while( ((fixpointTerm = it.GetNext()) != nullptr) && (underComplement == fixpoint->GetResult())) {
                #if (MEASURE_PROJECTION == true)
                ++this->fixpointPreNext;
                #endif
            }
        #endif

        // TODO: Fixpoint cache should probably be here!
        #if (OPT_REDUCE_PREFIXPOINT == true)
        fixpoint->RemoveSubsumed();
        #endif
        #if (OPT_GENERATE_UNIQUE_TERMS == true && UNIQUE_FIXPOINTS == true)
        fixpoint = this->_factory.GetUniqueFixpoint(fixpoint);
        #endif
        return std::make_pair(fixpoint, fixpoint->GetResult());
    }
}

ResultType BaseAutomaton::_IntersectNonEmptyCore(Symbol* symbol, Term* approximation, bool underComplement) {
    // Reinterpret the initial and final states
    TermBaseSet* initial = reinterpret_cast<TermBaseSet*>(this->_initialStates);
    TermBaseSet* final = reinterpret_cast<TermBaseSet*>(this->_finalStates);

    if(symbol == nullptr) {
        // Testing if epsilon is in language, i.e. testing if final states intersect initial ones
        return std::make_pair(this->_finalStates, initial->Intersects(final) != underComplement);
    } else if(approximation->type == TERM_EMPTY) {
        // Empty set has no Pre
        return std::make_pair(approximation, underComplement);
    } else {
        // First do the pre of the approximation
        TermBaseSet* preFinal = reinterpret_cast<TermBaseSet*>(this->Pre(symbol, approximation, underComplement));

        // Return the pre and true if it intersects the initial states
        if(preFinal->IsEmpty()) {
            return std::make_pair(Workshops::TermWorkshop::CreateEmpty(), underComplement);
        } else {
            return std::make_pair(preFinal, initial->Intersects(preFinal) != underComplement);
        }
    }
}

void SymbolicAutomaton::DumpExample(ExampleType e) {
    switch(e) {
        case ExampleType::SATISFYING:
            if(this->_satExample != nullptr) {
                this->_DumpExampleCore(e);
            }
            break;
        case ExampleType::UNSATISFYING:
            if(this->_unsatExample != nullptr) {
                this->_DumpExampleCore(e);
            }
            break;
        default:
            assert(false && "Something impossible has happened");
    }
}

void BinaryOpAutomaton::_DumpExampleCore(ExampleType e) {
    assert(false && "BinaryOpAutomata cannot have examples yet!");
}

void ComplementAutomaton::_DumpExampleCore(ExampleType e) {
    assert(false && "ComplementAutomata cannot have examples yet!");
}

std::string interpretModel(std::string& str, bool isFirstOrder) {
    size_t idx = 0;
    if(isFirstOrder) {
        // Interpret the first one
        while(idx < str.length() && str[idx] != '1') {++idx;}
        if(idx == str.length()) {
            return std::string("");
        }
        return std::string(std::to_string(idx));
    } else {
        if(str.empty()) {
            return std::string("{}");
        }

        std::string result;
        bool isFirst = true;
        result += "{";
        while(idx != str.size()) {
            if(str[idx] == '1') {
                if(!isFirst) {
                    result += ", ";
                } else {
                    isFirst = false;
                }
                result += std::to_string(idx);
            }
            ++idx;
        }
        result += "}";
        return result;
    }
}

void ProjectionAutomaton::_DumpExampleCore(ExampleType e) {
    Term* example = (e == ExampleType::SATISFYING ? this->_satExample : this->_unsatExample);

    // Print the bounded part
    auto varNo = this->projectedVars->size();
    std::string* examples = new std::string[varNo];

    while(example != nullptr && example->link.succ != nullptr && example != example->link.succ) {
    //                                                           ^--- not sure this is right
        for(size_t i = 0; i < varNo; ++i) {
            examples[i] += example->link.symbol->GetSymbolAt(varMap[this->projectedVars->get(i)]);
        }
        example = example->link.succ;
    }

    for(size_t i = 0; i < varNo; ++i) {
        std::cout << (symbolTable.lookupSymbol(this->projectedVars->get(i))) << ": " << examples[i] << "\n";
    }

    for(size_t i = 0; i < varNo; ++i) {
        bool isFirstOrder = (symbolTable.lookupType(this->projectedVars->get(i)) == MonaTypeTag::Varname1);
        std::cout << (symbolTable.lookupSymbol(this->projectedVars->get(i))) << " = " << interpretModel(examples[i], isFirstOrder) << "\n";
    }

    delete[] examples;
}

void BinaryOpAutomaton::DumpAutomaton() {
    #if (DEBUG_AUTOMATA_ADDRESSES == true)
        std::cout << "[" << this << "]";
    #endif
    if(this->type == AutType::INTERSECTION) {
        std::cout << "\033[1;32m";
    } else {
        std::cout << "\033[1;33m";
    }
    std::cout << "(\033[0m";
    this->_lhs_aut.aut->DumpAutomaton();
    if(this->type == AutType::INTERSECTION) {
        std::cout << "\033[1;32m \u2229 \033[0m";
    } else {
        std::cout << "\033[1;33m \u222A \033[0m";
    };
    this->_rhs_aut.aut->DumpAutomaton();
    if(this->type == AutType::INTERSECTION) {
        std::cout << "\033[1;32m";
    } else {
        std::cout << "\033[1;33m";
    }
    std::cout << ")\033[0m";
}

void ComplementAutomaton::DumpAutomaton() {
    #if (DEBUG_AUTOMATA_ADDRESSES == true)
        std::cout << "[" << this << "]";
    #endif
    std::cout << "\033[1;31m\u2201(\033[0m";
    this->_aut.aut->DumpAutomaton();
    std::cout << "\033[1;31m)\033[0m";
}

void ProjectionAutomaton::DumpAutomaton() {
    #if (DEBUG_AUTOMATA_ADDRESSES == true)
        std::cout << "[" << this << "]";
    #endif
    std::cout << "\033[1;34m";
    std::cout << "\u2203";
    for(auto it = this->projectedVars->begin(); it != this->projectedVars->end(); ++it) {
        std::cout << symbolTable.lookupSymbol(*it);
        if((it + 1) != this->projectedVars->end()) {
            std::cout << ", ";
        }
    }
    std::cout << "(\033[0m";
    this->_aut.aut->DumpAutomaton();
    std::cout << "\033[1;34m)\033[0m";
}

void GenericBaseAutomaton::DumpAutomaton() {
    #if (DEBUG_AUTOMATA_ADDRESSES == true)
        std::cout << "[" << this << "]";
    #endif
    std::cout << "Automaton";
    _form->dump();
    #if (DEBUG_BASE_AUTOMATA == true)
    this->BaseAutDump();
    #endif
}

void SubAutomaton::DumpAutomaton() {
    #if (DEBUG_AUTOMATA_ADDRESSES == true)
        std::cout << "[" << this << "]";
    #endif
    this->_form->dump();
    #if (DEBUG_BASE_AUTOMATA == true)
    this->BaseAutDump();
    #endif
}

void TrueAutomaton::DumpAutomaton() {
    #if (DEBUG_AUTOMATA_ADDRESSES == true)
        std::cout << "[" << this << "]";
    #endif
    this->_form->dump();
    #if (DEBUG_BASE_AUTOMATA == true)
    this->BaseAutDump();
    #endif
}

void FalseAutomaton::DumpAutomaton() {
    #if (DEBUG_AUTOMATA_ADDRESSES == true)
        std::cout << "[" << this << "]";
    #endif
    this->_form->dump();
    #if (DEBUG_BASE_AUTOMATA == true)
    this->BaseAutDump();
    #endif
}

void InAutomaton::DumpAutomaton() {
    #if (DEBUG_AUTOMATA_ADDRESSES == true)
        std::cout << "[" << this << "]";
    #endif
    this->_form->dump();
    #if (DEBUG_BASE_AUTOMATA == true)
    this->BaseAutDump();
    #endif
}

void FirstOrderAutomaton::DumpAutomaton() {
    #if (DEBUG_AUTOMATA_ADDRESSES == true)
        std::cout << "[" << this << "]";
    #endif
    this->_form->dump();
    #if (DEBUG_BASE_AUTOMATA == true)
    this->BaseAutDump();
    #endif
}

void EqualFirstAutomaton::DumpAutomaton() {
    #if (DEBUG_AUTOMATA_ADDRESSES == true)
        std::cout << "[" << this << "]";
    #endif
    this->_form->dump();
    #if (DEBUG_BASE_AUTOMATA == true)
    this->BaseAutDump();
    #endif
}

void EqualSecondAutomaton::DumpAutomaton() {
    #if (DEBUG_AUTOMATA_ADDRESSES == true)
        std::cout << "[" << this << "]";
    #endif
    this->_form->dump();
    #if (DEBUG_BASE_AUTOMATA == true)
    this->BaseAutDump();
    #endif
}

void LessAutomaton::DumpAutomaton() {
    #if (DEBUG_AUTOMATA_ADDRESSES == true)
        std::cout << "[" << this << "]";
    #endif
    this->_form->dump();
    #if (DEBUG_BASE_AUTOMATA == true)
    this->BaseAutDump();
    #endif
}

void LessEqAutomaton::DumpAutomaton() {
    #if (DEBUG_AUTOMATA_ADDRESSES == true)
        std::cout << "[" << this << "]";
    #endif
    this->_form->dump();
    #if (DEBUG_BASE_AUTOMATA == true)
    this->BaseAutDump();
    #endif
}

void SymbolicAutomaton::AutomatonToDot(std::string filename, SymbolicAutomaton *aut, bool inComplement) {
    std::ofstream os;
    os.open(filename);
    // TODO: Add exception handling
    os << "strict graph aut {\n";
    aut->DumpToDot(os, inComplement);
    os << "}\n";
    os.close();
}

void BinaryOpAutomaton::DumpToDot(std::ofstream & os, bool inComplement) {
    os << "\t" << (uintptr_t) &*this << "[label=\"";
    os << this->_form->dag_height << "\\n";
    os << "\u03B5 " << (inComplement ? "\u2209 " : "\u2208 ");
    if(this->_productType == ProductType::E_INTERSECTION) {
        os << "\u2229";
    } else {
        os << "\u222A";
    }
    os << "\\n(" << this->_trueCounter << "\u22A8, " << this->_falseCounter << "\u22AD)\"";
    if(this->_trueCounter == 0 && this->_falseCounter != 0) {
        os << ",style=filled, fillcolor=red";
    }
    os << "];\n";
    os << "\t" << (uintptr_t) &*this << " -- " << (uintptr_t) (this->_lhs_aut.aut) << ";\n";
    os << "\t" << (uintptr_t) &*this << " -- " << (uintptr_t) (this->_rhs_aut.aut) << ";\n";
    this->_lhs_aut.aut->DumpToDot(os, inComplement);
    this->_rhs_aut.aut->DumpToDot(os, inComplement);
}

void ComplementAutomaton::DumpToDot(std::ofstream & os, bool inComplement) {
    os << "\t" << (uintptr_t) &*this << "[label=\"";
    os << this->_form->dag_height << "\\n";
    os << "\u03B5 " << (inComplement ? "\u2209 " : "\u2208 ");
    os << "\u00AC\\n(" << this->_trueCounter << "\u22A8, " << this->_falseCounter << "\u22AD)\"";
    if(this->_trueCounter == 0 && this->_falseCounter != 0) {
        os << ",style=filled, fillcolor=red";
    }
    os << "];\n";
    os << "\t" << (uintptr_t) &*this << " -- " << (uintptr_t) (this->_aut.aut) << ";\n";
    this->_aut.aut->DumpToDot(os, !inComplement);
}

void ProjectionAutomaton::DumpToDot(std::ofstream & os, bool inComplement) {
    os << "\t" << (uintptr_t) &*this << "[label=\"";
    os << this->_form->dag_height << "\\n";
    os << "\u03B5 " << (inComplement ? "\u2209 " : "\u2208 ");
    os << "\u2203";
    for(auto id = this->projectedVars->begin(); id != this->projectedVars->end(); ++id) {
        os << symbolTable.lookupSymbol(*id) << ",";
    }
    os << "\\n(" << this->_trueCounter << "\u22A8, " << this->_falseCounter << "\u22AD)\"";
    if(this->_trueCounter == 0 && this->_falseCounter != 0) {
        os << ",style=filled, fillcolor=red";
    }
    os << "];\n";
    os << "\t" << (uintptr_t) &*this << " -- " << (uintptr_t) (this->_aut.aut) << ";\n";
    this->_aut.aut->DumpToDot(os, inComplement);
}

void BaseAutomaton::DumpToDot(std::ofstream & os, bool inComplement) {
    os << "\t" << (uintptr_t) &*this << "[label=\"";
    os << this->_form->dag_height << "\\n";
    os << "\u03B5 " << (inComplement ? "\u2209 " : "\u2208 ") << this->_form->ToString();
    os << "\\n(" << this->_trueCounter << "\u22A8, " << this->_falseCounter << "\u22AD)\"";
    if(this->_trueCounter == 0 && this->_falseCounter != 0) {
        os << ",style=filled, fillcolor=red";
    }
    os << "];\n";

}

/**
 * Renames the states according to the translation function so we get unique states.
 */
void BaseAutomaton::_RenameStates() {
    assert(false && "Obsolete function called\n");
}

void BaseAutomaton::BaseAutDump() {
    std::cout << "\n[----------------------->]\n";
    std::cout << "[!] Initial states:\n";
    if(this->_initialStates != nullptr) {
        this->_initialStates->dump();
        std::cout << "\n";
    } else {
        std::cout << "-> not initialized\n";
    }

    std::cout << "[!] Final states:\n";
    if(this->_finalStates != nullptr) {
        this->_finalStates->dump();
        std::cout << "\n";
    } else {
        std::cout << "-> not initialized\n";
    }
    this->_autWrapper.DumpDFA();
    std::cout << "[----------------------->]\n";
}

/**
 * Dump Cache stats of automaton only
 */
void BinaryOpAutomaton::DumpCacheStats() {
    this->_form->dump();
    this->_resCache.dumpStats();
    this->_lhs_aut.aut->DumpCacheStats();
    this->_rhs_aut.aut->DumpCacheStats();
}

void ComplementAutomaton::DumpCacheStats() {
    this->_form->dump();
    this->_resCache.dumpStats();
    this->_aut.aut->DumpCacheStats();
}

void ProjectionAutomaton::DumpCacheStats() {
    this->_form->dump();
    this->_resCache.dumpStats();
    this->_aut.aut->DumpCacheStats();
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

void print_stat(std::string statName, std::string stat) {
    if(stat != "") {
        std::cout << "  \u2218 " << statName << " -> " << stat << "\n";
    }
}

void print_stat(std::string statName, double stat) {
    std::cout << "  \u2218 " << statName << " -> " << std::fixed << std::setprecision(2) << stat << "\n";
}

void BinaryOpAutomaton::DumpComputationStats() {
    if(this->marked) {
        return;
    }
    this->marked = true;
#   if (PRINT_STATS_PRODUCT == true)
    this->_form->dump();
    std::cout << "\n";
    print_stat("Refs", this->_refs);
    std::cout << "  \u2218 Cache stats -> ";
#       if (MEASURE_CACHE_HITS == true)
        this->_resCache.dumpStats();
#       endif
#       if (DEBUG_WORKSHOPS == true)
        this->_factory.Dump();
#       endif
#       if (DEBUG_SYMBOL_CREATION == true)
        this->symbolFactory->Dump();
#       endif
        print_stat("True Hits", this->_trueCounter);
        print_stat("False Hits", this->_falseCounter);
        print_stat("Continuation Generation", this->_contCreationCounter);
        print_stat("Continuation Evaluation", this->_contUnfoldingCounter);
#   endif
    std::cout << "\n";
    this->_lhs_aut.aut->DumpComputationStats();
    this->_rhs_aut.aut->DumpComputationStats();
}

void ProjectionAutomaton::DumpComputationStats() {
    if(this->marked) {
        return;
    }
    this->marked = true;
#   if (PRINT_STATS_PROJECTION == true)
    this->_form->dump();
    std::cout << "\n";
    print_stat("Refs", this->_refs);
    std::cout << "  \u2218 Cache stats -> ";
#       if (MEASURE_CACHE_HITS == true)
        this->_resCache.dumpStats();
#       endif
#       if (DEBUG_WORKSHOPS)
        this->_factory.Dump();
#       endif
#       if (DEBUG_SYMBOL_CREATION == true)
        this->symbolFactory->Dump();
#       endif
#       if (MEASURE_PROJECTION == true)
        print_stat("Fixpoint Nexts", this->fixpointNext);
        print_stat("Fixpoint Results", this->fixpointRes);
        print_stat("FixpointPre Nexts", this->fixpointPreNext);
        print_stat("FixpointPre Results", this->fixpointPreRes);
#       endif
        print_stat("True Hits", this->_trueCounter);
        print_stat("False Hits", this->_falseCounter);
        print_stat("Continuation Evaluation", this->_contUnfoldingCounter);
#   endif
    std::cout << "\n";
    this->_aut.aut->DumpComputationStats();
}

void ComplementAutomaton::DumpComputationStats() {
    if(this->marked) {
        return;
    }
    this->marked = true;
#   if (PRINT_STATS_NEGATION == true)
    this->_form->dump();
    std::cout << "\n";
    print_stat("Refs", this->_refs);
    std::cout << "  \u2218 Cache stats -> ";
#       if (MEASURE_CACHE_HITS == true)
        this->_resCache.dumpStats();
#       endif
#       if (DEBUG_WORKSHOPS)
        this->_factory.Dump();
#       endif
#       if (DEBUG_SYMBOL_CREATION == true)
        this->symbolFactory->Dump();
#       endif
        print_stat("True Hits", this->_trueCounter);
        print_stat("False Hits", this->_falseCounter);
        print_stat("Continuation Evaluation", this->_contUnfoldingCounter);
#   endif
    std::cout << "\n";
    this->_aut.aut->DumpComputationStats();
}

void BaseAutomaton::DumpComputationStats() {
    if(this->marked) {
        return;
    }
    this->marked = true;
#   if (PRINT_STATS_BASE == true)
    this->_form->dump();
    std::cout << "\n";
    print_stat("Refs", this->_refs);
    std::cout << "  \u2218 Cache stats -> ";
#       if (MEASURE_CACHE_HITS == true)
        this->_resCache.dumpStats();
#       endif
#       if (DEBUG_WORKSHOPS)
        this->_factory.Dump();
#       endif
#       if (DEBUG_SYMBOL_CREATION == true)
        this->symbolFactory->Dump();
#       endif
        print_stat("True Hits", this->_trueCounter);
        print_stat("False Hits", this->_falseCounter);
        print_stat("Continuation Evaluation", this->_contUnfoldingCounter);
#   endif
    std::cout << "\n";
}

void SymbolicAutomaton::DumpAutomatonMetrics() {
    this->FillStats();

    std::cout << "\u2218 Overall SymbolicAutomaton Metrics:\n";
    print_stat("Nodes", this->stats.nodes);
    print_stat("Real nodes", this->stats.real_nodes);
    print_stat("DAG gain", ((double)this->stats.nodes / this->stats.real_nodes));
    print_stat("Fixpoint Computations", this->stats.fixpoint_computations);
    print_stat("Maximal Fixpoint Nesting", this->stats.max_fixpoint_nesting);
    print_stat("Automaton Height", this->stats.height);
    print_stat("Maximal References", this->stats.max_refs);
}

void ProjectionAutomaton::FillStats() {
    bool count_inner = !this->_aut.remap;
    if(count_inner) {
        this->_aut.aut->FillStats();
    }

    this->stats.fixpoint_computations = (count_inner ? this->_aut.aut->stats.fixpoint_computations : 0) + 1;
    this->stats.height = this->_aut.aut->stats.height + 1;
    this->stats.nodes = 1 + (count_inner ? this->_aut.aut->stats.nodes : 0);
    this->stats.real_nodes = 1 + this->_aut.aut->stats.real_nodes;
    this->stats.max_refs = std::max(this->_refs, this->_aut.aut->stats.max_refs);
    this->stats.max_fixpoint_nesting = this->_aut.aut->stats.max_fixpoint_nesting + 1;
}

void BinaryOpAutomaton::FillStats() {
    bool count_left = !this->_lhs_aut.remap;
    bool count_right = !this->_rhs_aut.remap;

    if(count_left) {
        this->_lhs_aut.aut->FillStats();
    }
    if(count_right) {
        this->_rhs_aut.aut->FillStats();
    }

    this->stats.fixpoint_computations = (count_left ? this->_lhs_aut.aut->stats.fixpoint_computations : 0) + (count_right ? this->_rhs_aut.aut->stats.fixpoint_computations : 0);
    this->stats.height = std::max(this->_lhs_aut.aut->stats.height, this->_rhs_aut.aut->stats.height) + 1;
    this->stats.nodes = (count_left ? this->_lhs_aut.aut->stats.nodes : 0) + (count_right ? this->_rhs_aut.aut->stats.nodes : 0) + 1;
    this->stats.real_nodes = 1 + this->_lhs_aut.aut->stats.real_nodes + this->_rhs_aut.aut->stats.real_nodes;
    this->stats.max_refs = std::max({this->_refs, this->_lhs_aut.aut->stats.max_refs, this->_rhs_aut.aut->stats.max_refs});
    this->stats.max_fixpoint_nesting = std::max(this->_lhs_aut.aut->stats.max_fixpoint_nesting, this->_rhs_aut.aut->stats.max_fixpoint_nesting);
}

void BaseAutomaton::FillStats() {
    this->stats.fixpoint_computations = 0;
    this->stats.height = 1;
    this->stats.nodes = 1;
    this->stats.real_nodes = 1;
    this->stats.max_refs = this->_refs;
    this->stats.max_fixpoint_nesting = 0;
}

void ComplementAutomaton::FillStats() {
    bool count_inner = !this->_aut.remap;
    if(count_inner) {
        this->_aut.aut->FillStats();
    }

    this->stats.fixpoint_computations = (count_inner ? this->_aut.aut->stats.fixpoint_computations : 0);
    this->stats.height = this->_aut.aut->stats.height + 1;
    this->stats.nodes = (count_inner ? this->_aut.aut->stats.nodes : 0) + 1;
    this->stats.real_nodes = 1 + this->_aut.aut->stats.real_nodes;
    this->stats.max_refs = std::max(this->_refs, this->_aut.aut->stats.max_refs);
    this->stats.max_fixpoint_nesting = this->_aut.aut->stats.max_fixpoint_nesting;
}