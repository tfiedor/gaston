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

class SymbolicChecker;
struct SymLink;

/**
 * Base class for Symbolic Automata. Each symbolic automaton contains the
 * pointer to the formula it corresponds to (for further informations),
 * its initial and final states, free variables and caches.
 */
class SymbolicAutomaton {
public:
    friend class SymbolicChecker;
    friend struct SymLink;

    // <<< PUBLIC MEMBERS >>>
    static StateType stateCnt;
    AutType type;
    using TermWorkshop  = Workshops::TermWorkshop;
    SymbolWorkshop* symbolFactory;
    static DagNodeCache* dagNodeCache;
    Formula_ptr _form;
protected:
    // <<< PRIVATE MEMBERS >>>
    Term_ptr _initialStates = nullptr;
    Term_ptr _finalStates = nullptr;
    TermWorkshop _factory;          // Creates terms
    ResultCache _resCache;          // Caches (states, symbol) = (fixpoint, bool)
    SubsumptionCache _subCache;     // Caches (term, term) = bool
    VarList _freeVars;
    Term_ptr _satExample = nullptr;
    Term_ptr _unsatExample = nullptr;
    size_t _refs;
    bool marked = false;

    // <<< PRIVATE FUNCTIONS >>>
    virtual void _InitializeAutomaton() = 0;
    virtual void _InitializeInitialStates() = 0;
    virtual void _InitializeFinalStates() = 0;
    virtual ResultType _IntersectNonEmptyCore(Symbol*, Term*, bool) = 0;
    virtual void _DumpExampleCore(ExampleType) = 0;

    // <<< MEASURES >>>
    unsigned int _falseCounter = 0;
    unsigned int _trueCounter = 0;
    unsigned int _contUnfoldingCounter = 0;
    unsigned int _contCreationCounter = 0;
    unsigned int _projectIterationCounter = 0;
    unsigned int _projectSymbolEvaluationCounter = 0;

public:
    // <<< CONSTRUCTORS >>>
    NEVER_INLINE explicit SymbolicAutomaton(Formula_ptr form);

    // <<< PUBLIC API >>>
    void IncReferences() {++this->_refs;}
    void DecReferences() {assert(this->_refs > 0); --this->_refs; if(this->_refs < 1) delete this;}
    void InitializeStates();
    virtual Term_ptr GetInitialStates();
    virtual Term_ptr GetFinalStates();
    Gaston::VarList* GetFreeVars() { return &this->_freeVars;}
    virtual Term* Pre(Symbol*, Term*, bool) = 0;
    virtual ResultType IntersectNonEmpty(Symbol*, Term*, bool);
    void SetSatisfiableExample(Term*);
    void SetUnsatisfiableExample(Term*);

    // <<< DUMPING FUNCTIONS >>>
    virtual void DumpAutomaton() = 0;
    virtual void DumpCacheStats() = 0;
    virtual void DumpExample(ExampleType);
    virtual void DumpStats() = 0;
    virtual unsigned int CountNodes() = 0;
    virtual void DumpToDot(std::ofstream&, bool) = 0;
    static void AutomatonToDot(std::string, SymbolicAutomaton*, bool);
protected:
    NEVER_INLINE virtual ~SymbolicAutomaton();
};

struct SymLink {
    SymbolicAutomaton* aut;
    bool remap;
    std::map<unsigned int, unsigned int>* varRemap;

    SymLink() : aut(nullptr), remap(false), varRemap(nullptr) {}
    explicit SymLink(SymbolicAutomaton* s) : aut(s), remap(false), varRemap(nullptr) {}
    ~SymLink() {
        if(varRemap != nullptr) {
            delete varRemap;
        }
    }

    void InitializeSymLink(ASTForm*);
    ZeroSymbol* ReMapSymbol(ZeroSymbol*);
};

/**
 * BinaryOpAutomaton corresponds to Binary Operations of Intersection and
 * Union of subautomata. It further contains the links to left and right
 * operands and some additional functions for evaluation of results
 */
class BinaryOpAutomaton : public SymbolicAutomaton {
protected:
    // <<< PRIVATE MEMBERS >>>
    SymLink _lhs_aut;
    SymLink _rhs_aut;
    ProductType _productType;
    bool (*_eval_result)(bool, bool, bool);     // Boolean function for evaluation of left and right results
    bool (*_eval_early)(bool, bool);            // Boolean function for evaluating early evaluation
    bool (*_early_val)(bool);                   // Boolean value of early result

    // <<< PRIVATE FUNCTIONS >>>
    virtual void _InitializeAutomaton();
    virtual void _InitializeInitialStates();
    virtual void _InitializeFinalStates();
    virtual ResultType _IntersectNonEmptyCore(Symbol*, Term*, bool);
    virtual void _DumpExampleCore(ExampleType);

public:
    NEVER_INLINE BinaryOpAutomaton(SymbolicAutomaton_raw lhs, SymbolicAutomaton_raw rhs, Formula_ptr form);

    // <<< PUBLIC API >>>
    virtual Term* Pre(Symbol*, Term*, bool);

    // <<< DUMPING FUNCTIONS >>>
    virtual void DumpAutomaton();
    virtual void DumpToDot(std::ofstream&, bool);
    virtual void DumpStats();
    virtual void DumpCacheStats();
    virtual unsigned int CountNodes();
protected:
    NEVER_INLINE virtual ~BinaryOpAutomaton();
};

/**
 * Automaton corresponding to the formula: phi and psi
 */
class IntersectionAutomaton : public BinaryOpAutomaton {
public:
    NEVER_INLINE IntersectionAutomaton(SymbolicAutomaton* lhs, SymbolicAutomaton* rhs, Formula_ptr form);
};

/**
 * Automaton corresponding to the formula: phi or psi
 */
class UnionAutomaton : public BinaryOpAutomaton {
public:
    NEVER_INLINE UnionAutomaton(SymbolicAutomaton* lhs, SymbolicAutomaton* rhs, Formula_ptr form);
};

/**
 * Automaton corresponding to the formulae: not phi
 */
class ComplementAutomaton : public SymbolicAutomaton {
protected:
    // <<< PRIVATE MEMBERS >>>
    SymLink _aut;

    // <<< PRIVATE FUNCTIONS >>>
    virtual void _InitializeAutomaton();
    virtual void _InitializeInitialStates();
    virtual void _InitializeFinalStates();
    virtual ResultType _IntersectNonEmptyCore(Symbol*, Term*, bool);
    virtual void _DumpExampleCore(ExampleType);

public:
    // <<< CONSTRUCTORS >>>
    NEVER_INLINE ComplementAutomaton(SymbolicAutomaton *aut, Formula_ptr form);

    // <<< PUBLIC API >>>
    virtual Term* Pre(Symbol*, Term*, bool);

    // <<< DUMPING FUNCTIONS >>>
    virtual void DumpAutomaton();
    virtual void DumpToDot(std::ofstream&, bool);
    virtual void DumpStats();
    virtual void DumpCacheStats();
    virtual unsigned int CountNodes();
protected:
    NEVER_INLINE virtual ~ComplementAutomaton();
};

/**
 * Automaton corresponding to the formulae: Exists X. phi
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
    SymLink _aut;
    bool _isRoot;

    // <<< PRIVATE FUNCTIONS >>>
    virtual void _InitializeAutomaton();
    virtual void _InitializeInitialStates();
    virtual void _InitializeFinalStates();
    virtual ResultType _IntersectNonEmptyCore(Symbol*, Term*, bool);
    virtual void _DumpExampleCore(ExampleType);

public:
    /// <<< CONSTRUCTORS >>>
    NEVER_INLINE ProjectionAutomaton(SymbolicAutomaton* aut, Formula_ptr form, bool isRoot = false);

    // <<< PUBLIC API >>>
    virtual Term* Pre(Symbol*, Term*, bool);
    SymbolicAutomaton* GetBase() { return this->_aut.aut;}
    bool IsRoot() { return this-> _isRoot; }

    // <<< DUMPING FUNCTIONS >>>
    virtual void DumpAutomaton();
    virtual void DumpToDot(std::ofstream&, bool);
    virtual void DumpStats();
    virtual void DumpCacheStats();
    virtual unsigned int CountNodes();
protected:
    NEVER_INLINE virtual ~ProjectionAutomaton();
};

/**
 * Automaton for the topmost fixpoint computation for unground formulae.
 *
 * Note this is optimization in order to omit the double computation of the
 * fixpoints, by testing the satisfiability and unsatisfiability.
 */
class RootProjectionAutomaton : public ProjectionAutomaton {
protected:
    // <<< PRIVATE MEMBERS >>>
public:
    // <<< CONSTRUCTORS >>>
    NEVER_INLINE RootProjectionAutomaton(SymbolicAutomaton*, Formula_ptr);

    // <<< PUBLIC API >>>
    virtual ResultType IntersectNonEmpty(Symbol*, Term*, bool);
};

/**
 * Symbolic automaton corresponding to base automata
 */
class BaseAutomaton : public SymbolicAutomaton {
protected:
    /// <<< PRIVATE MEMBERS >>>
    MonaWrapper<size_t> _autWrapper;                // Wrapper of mona automaton
    unsigned int _stateSpace = 0;               // Number of states in automaton
    unsigned int _stateOffset = 0;              // Offset of states used for mask
    PreCache _preCache;

    /// <<< PRIVATE FUNCTIONS >>>
    virtual void _InitializeAutomaton();
    virtual void _InitializeInitialStates();
    virtual void _InitializeFinalStates();
    virtual ResultType _IntersectNonEmptyCore(Symbol*, Term*, bool);
    void _RenameStates();
    virtual void _DumpExampleCore(ExampleType) {}
public:
    // <<< CONSTRUCTORS >>>
    NEVER_INLINE BaseAutomaton(BaseAutomatonType* aut, size_t vars, Formula_ptr form, bool emptyTracks);

    // <<< PUBLIC API >>>
    virtual Term* Pre(Symbol*, Term*, bool);

    // <<< DUMPING FUNCTIONS >>>
    virtual void DumpToDot(std::ofstream&, bool);
    virtual void BaseAutDump();
    virtual void DumpStats();
    virtual void DumpCacheStats();
    virtual unsigned int CountNodes();

protected:
    NEVER_INLINE ~BaseAutomaton();
};

class GenericBaseAutomaton : public BaseAutomaton {
public:
    NEVER_INLINE GenericBaseAutomaton(BaseAutomatonType* aut, size_t vars, Formula_ptr form, bool emptyTracks) : BaseAutomaton(aut, vars, form, emptyTracks) { }
    virtual void DumpAutomaton();
};

class SubAutomaton : public BaseAutomaton {
public:
    SubAutomaton(BaseAutomatonType* aut, size_t vars, Formula_ptr form, bool emptyTracks = false) : BaseAutomaton(aut, vars, form, emptyTracks) { }
    virtual void DumpAutomaton();
};

class TrueAutomaton : public BaseAutomaton {
public:
    TrueAutomaton(BaseAutomatonType* aut, size_t vars, Formula_ptr form, bool emptyTracks = false) : BaseAutomaton(aut, vars, form, emptyTracks) { }
    virtual void DumpAutomaton();
};

class FalseAutomaton : public BaseAutomaton {
public:
    FalseAutomaton(BaseAutomatonType* aut, size_t vars, Formula_ptr form, bool emptyTracks = false) : BaseAutomaton(aut, vars, form, emptyTracks) { }
    virtual void DumpAutomaton();
};

class InAutomaton : public BaseAutomaton {
public:
    InAutomaton(BaseAutomatonType* aut, size_t vars, Formula_ptr form, bool emptyTracks = false) : BaseAutomaton(aut, vars, form, emptyTracks) { }
    virtual void DumpAutomaton();
};

class FirstOrderAutomaton : public BaseAutomaton {
public:
    FirstOrderAutomaton(BaseAutomatonType* aut, size_t vars, Formula_ptr form, bool emptyTracks = false) : BaseAutomaton(aut, vars, form, emptyTracks) { }
    virtual void DumpAutomaton();
};

class EqualFirstAutomaton : public BaseAutomaton {
public:
    EqualFirstAutomaton(BaseAutomatonType* aut, size_t vars, Formula_ptr form, bool emptyTracks = false) : BaseAutomaton(aut, vars, form, emptyTracks) { }
    virtual void DumpAutomaton();
};

class EqualSecondAutomaton : public BaseAutomaton {
public:
    EqualSecondAutomaton(BaseAutomatonType* aut, size_t vars, Formula_ptr form, bool emptyTracks = false) : BaseAutomaton(aut, vars, form, emptyTracks) { }
    virtual void DumpAutomaton();
};

class LessAutomaton : public BaseAutomaton {
public:
    LessAutomaton(BaseAutomatonType* aut, size_t vars, Formula_ptr form, bool emptyTracks = false) : BaseAutomaton(aut, vars, form, emptyTracks) { }
    virtual void DumpAutomaton();
};

class LessEqAutomaton : public BaseAutomaton {
public:
    LessEqAutomaton(BaseAutomatonType* aut, size_t vars, Formula_ptr form, bool emptyTracks = false) : BaseAutomaton(aut, vars, form, emptyTracks) { }
    virtual void DumpAutomaton();
};

#endif //WSKS_SYMBOLICAUTOMATA_H
