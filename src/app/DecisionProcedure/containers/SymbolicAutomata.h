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


#ifndef WSKS_SYMBOLICAUTOMATA_H
#define WSKS_SYMBOLICAUTOMATA_H

#include "../mtbdd/apply1func.hh"
#include "../mtbdd/apply2func.hh"
#include "../mtbdd/void_apply1func.hh"
#include "../mtbdd/ondriks_mtbdd.hh"
#include "../mtbdd/monawrapper.hh"
#include "../utils/Symbol.h"
#include "../environment.hh"
#include "../containers/SymbolicCache.hh"
#include "../containers/Workshops.h"
#include "../../Frontend/ident.h"
#include "../../Frontend/ast.h"
#include "../containers/VarToTrackMap.hh"
#include <vector>
#include <vata/util/binary_relation.hh>
#include <string>
#include <iostream>
#include <fstream>

// <<< FORWARD CLASS DECLARATIONS >>>
class Term;
extern VarToTrackMap varMap;

using namespace Gaston;
using SymbolWorkshop    = Workshops::SymbolWorkshop;

/**
 * Base class for Symbolic Automata. Each symbolic automaton contains the
 * pointer to the formula it corresponds to (for further informations),
 * its initial and final states, free variables and caches.
 */
class SymbolicAutomaton {
public:
    // <<< PUBLIC MEMBERS >>>
    static StateType stateCnt;
    AutType type;
    using TermWorkshop  = Workshops::TermWorkshop;
    SymbolWorkshop* symbolFactory;

protected:
    // <<< PRIVATE MEMBERS >>>
    Formula_ptr _form;
    Term_ptr _initialStates = nullptr;
    Term_ptr _finalStates = nullptr;
    TermWorkshop _factory;          // Creates terms
    ResultCache _resCache;          // Caches (states, symbol) = (fixpoint, bool)
    SubsumptionCache _subCache;     // Caches (term, term) = bool
    VarList _freeVars;

    // <<< PRIVATE FUNCTIONS >>>
    virtual void _InitializeAutomaton() = 0;
    virtual void _InitializeInitialStates() = 0;
    virtual void _InitializeFinalStates() = 0;
    virtual ResultType _IntersectNonEmptyCore(Symbol*, Term*, bool) = 0;

    // <<< MEASURES >>>
    unsigned int _falseCounter = 0;
    unsigned int _trueCounter = 0;
    unsigned int _contUnfoldingCounter = 0;
    unsigned int _contCreationCounter = 0;
    unsigned int _projectIterationCounter = 0;
    unsigned int _projectSymbolEvaluationCounter = 0;

public:
    // <<< CONSTRUCTORS >>>
    SymbolicAutomaton(Formula_ptr form);
    virtual ~SymbolicAutomaton();

    // <<< PUBLIC API >>>
    void InitializeStates();
    virtual Term_ptr GetInitialStates();
    virtual Term_ptr GetFinalStates();
    virtual Term* Pre(Symbol*, Term*, bool) = 0;
    ResultType IntersectNonEmpty(Symbol*, Term*, bool);

    // <<< DUMPING FUNCTIONS >>>
    virtual void DumpAutomaton() = 0;
    virtual void DumpCacheStats() = 0;
    virtual void DumpStats() = 0;
    virtual void DumpToDot(std::ofstream&, bool) = 0;
    static void AutomatonToDot(std::string, SymbolicAutomaton*, bool);
};

/**
 * BinaryOpAutomaton corresponds to Binary Operations of Intersection and
 * Union of subautomata. It further contains the links to left and right
 * operands and some additional functions for evaluation of results
 */
class BinaryOpAutomaton : public SymbolicAutomaton {
protected:
    // <<< PRIVATE MEMBERS >>>
    SymbolicAutomaton* _lhs_aut;
    SymbolicAutomaton* _rhs_aut;
    ProductType _productType;
    bool (*_eval_result)(bool, bool, bool);     // Boolean function for evaluation of left and right results
    bool (*_eval_early)(bool, bool);            // Boolean function for evaluating early evaluation
    bool (*_early_val)(bool);                   // Boolean value of early result

    // <<< PRIVATE FUNCTIONS >>>
    virtual void _InitializeAutomaton();
    virtual void _InitializeInitialStates();
    virtual void _InitializeFinalStates();
    virtual ResultType _IntersectNonEmptyCore(Symbol*, Term*, bool);

public:
    BinaryOpAutomaton(SymbolicAutomaton_raw lhs, SymbolicAutomaton_raw rhs, Formula_ptr form);
    virtual ~BinaryOpAutomaton();

    // <<< PUBLIC API >>>
    virtual Term* Pre(Symbol*, Term*, bool);

    // <<< DUMPING FUNCTIONS >>>
    virtual void DumpAutomaton();
    virtual void DumpToDot(std::ofstream&, bool);
    virtual void DumpStats();
    virtual void DumpCacheStats();
};

/**
 * Automaton corresponding to the formula: phi and psi
 */
class IntersectionAutomaton : public BinaryOpAutomaton {
public:
    IntersectionAutomaton(SymbolicAutomaton* lhs, SymbolicAutomaton* rhs, Formula_ptr form);
};

/**
 * Automaton corresponding to the formula: phi or psi
 */
class UnionAutomaton : public BinaryOpAutomaton {
public:
    UnionAutomaton(SymbolicAutomaton* lhs, SymbolicAutomaton* rhs, Formula_ptr form);
};

/**
 * Automaton corresponding to the formulae: not phi
 */
class ComplementAutomaton : public SymbolicAutomaton {
protected:
    // <<< PRIVATE MEMBERS >>>
    SymbolicAutomaton* _aut;

    // <<< PRIVATE FUNCTIONS >>>
    virtual void _InitializeAutomaton();
    virtual void _InitializeInitialStates();
    virtual void _InitializeFinalStates();
    virtual ResultType _IntersectNonEmptyCore(Symbol*, Term*, bool);

public:
    // <<< CONSTRUCTORS >>>
    ComplementAutomaton(SymbolicAutomaton *aut, Formula_ptr form);
    virtual ~ComplementAutomaton();

    // <<< PUBLIC API >>>
    virtual Term* Pre(Symbol*, Term*, bool);

    // <<< DUMPING FUNCTIONS >>>
    virtual void DumpAutomaton();
    virtual void DumpToDot(std::ofstream&, bool);
    virtual void DumpStats();
    virtual void DumpCacheStats();
};

/**
 * Automaotn corresponding to the formulae: Exists X. phi
 */
class ProjectionAutomaton : public SymbolicAutomaton {
public:
    // <<< PUBLIC MEMBERS >>>
    IdentList* projectedVars;
    #if (MEASURE_PROJECTION == true)
    size_t fixpointNext = 0;
    size_t fixpointPreNext = 0;
    std::string fixpointRes = "";
    std::string fixpointPreRes = "";
    #endif

protected:
    // <<< PRIVATE MEMBERS >>>
    SymbolicAutomaton* _aut;

    // <<< PRIVATE FUNCTIONS >>>
    virtual void _InitializeAutomaton();
    virtual void _InitializeInitialStates();
    virtual void _InitializeFinalStates();
    virtual ResultType _IntersectNonEmptyCore(Symbol*, Term*, bool);

public:
    /// <<< CONSTRUCTORS >>>
    ProjectionAutomaton(SymbolicAutomaton* aut, Formula_ptr form);
    virtual ~ProjectionAutomaton();

    // <<< PUBLIC API >>>
    virtual Term* Pre(Symbol*, Term*, bool);
    SymbolicAutomaton* GetBase() { return this->_aut;}

    // <<< DUMPING FUNCTIONS >>>
    virtual void DumpAutomaton();
    virtual void DumpToDot(std::ofstream&, bool);
    virtual void DumpStats();
    virtual void DumpCacheStats();
};

/**
 * Symbolic automaton corresponding to base automata
 */
class BaseAutomaton : public SymbolicAutomaton {
protected:
    /// <<< PRIVATE MEMBERS >>>
    MonaWrapper<size_t> _autWrapper;                // Wrapper of mona automaton
    unsigned int _stateSpace;               // Number of states in automaton
    unsigned int _stateOffset;              // Offset of states used for mask
    PreCache _preCache;

    /// <<< PRIVATE FUNCTIONS >>>
    virtual void _InitializeAutomaton();
    virtual void _InitializeInitialStates();
    virtual void _InitializeFinalStates();
    virtual ResultType _IntersectNonEmptyCore(Symbol*, Term*, bool);
    void _RenameStates();

public:
    // <<< CONSTRUCTORS >>>
    BaseAutomaton(BaseAutomatonType* aut, size_t vars, Formula_ptr form);
    ~BaseAutomaton();

    // <<< PUBLIC API >>>
    virtual Term* Pre(Symbol*, Term*, bool);

    // <<< DUMPING FUNCTIONS >>>
    virtual void DumpToDot(std::ofstream&, bool);
    virtual void BaseAutDump();
    virtual void DumpStats();
    virtual void DumpCacheStats();
};

class GenericBaseAutomaton : public BaseAutomaton {
public:
    GenericBaseAutomaton(BaseAutomatonType* aut, size_t vars, Formula_ptr form) : BaseAutomaton(aut, vars, form) { }
    virtual void DumpAutomaton();
};

class SubAutomaton : public BaseAutomaton {
public:
    SubAutomaton(BaseAutomatonType* aut, size_t vars, Formula_ptr form) : BaseAutomaton(aut, vars, form) { }
    virtual void DumpAutomaton();
};

class TrueAutomaton : public BaseAutomaton {
public:
    TrueAutomaton(BaseAutomatonType* aut, size_t vars, Formula_ptr form) : BaseAutomaton(aut, vars, form) { }
    virtual void DumpAutomaton();
};

class FalseAutomaton : public BaseAutomaton {
public:
    FalseAutomaton(BaseAutomatonType* aut, size_t vars, Formula_ptr form) : BaseAutomaton(aut, vars, form) { }
    virtual void DumpAutomaton();
};

class InAutomaton : public BaseAutomaton {
public:
    InAutomaton(BaseAutomatonType* aut, size_t vars, Formula_ptr form) : BaseAutomaton(aut, vars, form) { }
    virtual void DumpAutomaton();
};

class FirstOrderAutomaton : public BaseAutomaton {
public:
    FirstOrderAutomaton(BaseAutomatonType* aut, size_t vars, Formula_ptr form) : BaseAutomaton(aut, vars, form) { }
    virtual void DumpAutomaton();
};

class EqualFirstAutomaton : public BaseAutomaton {
public:
    EqualFirstAutomaton(BaseAutomatonType* aut, size_t vars, Formula_ptr form) : BaseAutomaton(aut, vars, form) { }
    virtual void DumpAutomaton();
};

class EqualSecondAutomaton : public BaseAutomaton {
public:
    EqualSecondAutomaton(BaseAutomatonType* aut, size_t vars, Formula_ptr form) : BaseAutomaton(aut, vars, form) { }
    virtual void DumpAutomaton();
};

class LessAutomaton : public BaseAutomaton {
public:
    LessAutomaton(BaseAutomatonType* aut, size_t vars, Formula_ptr form) : BaseAutomaton(aut, vars, form) { }
    virtual void DumpAutomaton();
};

class LessEqAutomaton : public BaseAutomaton {
public:
    LessEqAutomaton(BaseAutomatonType* aut, size_t vars, Formula_ptr form) : BaseAutomaton(aut, vars, form) { }
    virtual void DumpAutomaton();
};

#endif //WSKS_SYMBOLICAUTOMATA_H
