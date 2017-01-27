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
#include "../utils/Timer.h"
#include "../containers/SymbolicCache.hh"
#include "../containers/VarToTrackMap.hh"
#include "../containers/Workshops.h"
#include "../containers/FixpointGuide.h"
#include "../containers/TransitiveCache.h"
#include "../../Frontend/ident.h"
#include "../../Frontend/ast.h"
#include "../environment.hh"
#include <vector>
#include <vata/util/binary_relation.hh>
#include <string>
#include <iostream>
#include <fstream>

// <<< FORWARD CLASS DECLARATIONS >>>
class Term;
class FixpointGuide;
extern VarToTrackMap varMap;

using namespace Gaston;
using SymbolWorkshop    = Workshops::SymbolWorkshop;

class SymbolicChecker;
struct SymLink;

struct IntersectNonEmptyParams {
    bool underComplement;
    bool limitPre;        // < Whether the Pre should be limited
    size_t variableLevel; // < On which level we are doing the pre currently
    char variableValue;   // < Which value we are limiting the Pre to, i.e. '0', '1' or 'X'

    IntersectNonEmptyParams(bool uC) : underComplement(uC), limitPre(false), variableLevel(0) {}
    IntersectNonEmptyParams(bool uC, bool lP, size_t level, char value)
            : underComplement(uC), limitPre(lP), variableLevel(level), variableValue(value) {}
};

/**
 * Base class for Symbolic Automata. Each symbolic automaton contains the
 * pointer to the formula it corresponds to (for further informations),
 * its initial and final states, free variables and caches.
 */
class SymbolicAutomaton {
public:
    friend class SymbolicChecker;
    friend struct SymLink;

    using TermWorkshop  = Workshops::TermWorkshop;

    // <<< PUBLIC MEMBERS >>>
    static StateType stateCnt;
    static DagNodeCache* dagNodeCache;
    static DagNodeCache* dagNegNodeCache;

protected:
    // <<< PRIVATE MEMBERS >>>
    ResultCache _resCache;          // Caches (states, symbol) = (fixpoint, bool)
    ResultLevelCache _resLevelCache;// Caches (states, level, char) = (fixpoint, bool)
    VarList _freeVars;              // Fixme: This is not really free variables...
    VarList _nonOccuringVars;       // Variables that are not occuring in formula, used for trimming

public:
    TermWorkshop _factory;          // Creates terms
    TransitiveCache _subCache;     // Caches (term, term) = bool
    SymbolWorkshop symbolFactory;
    struct SymbolicAutomatonStats {
        unsigned int height = 0;
        unsigned int fixpoint_computations = 0;
        unsigned int max_fixpoint_nesting = 0;
        unsigned int max_fixpoint_width = 0;
        double avg_fixpoint_width = 0;
        unsigned int nodes = 0;
        unsigned int real_nodes = 0;
        unsigned int max_refs;
        unsigned int max_symbol_path_len = 0;
    } stats;

    Formula_ptr _form;
protected:
    Term_ptr _satExample = nullptr;
    Term_ptr _unsatExample = nullptr;
    Term_ptr _initialStates = nullptr;
    Term_ptr _finalStates = nullptr;

#   if (MEASURE_SUBAUTOMATA_TIMING == true)
    ChronoTimer timer;
#   endif
    unsigned int _refs;

    // <<< MEASURES >>>
    unsigned int _falseCounter = 0;
    unsigned int _trueCounter = 0;
    unsigned int _contUnfoldingCounter = 0;
    unsigned int _contCreationCounter = 0;
    unsigned int _projectIterationCounter = 0;
    unsigned int _projectSymbolEvaluationCounter = 0;

public:
    AutType type;

protected:
    bool marked = false;
    bool _isRestriction = false;
    bool _lastResult;

    // <<< PRIVATE FUNCTIONS >>>
    void _InitializeOccuringVars();
    void _InitializeNonOccuring();
    virtual void _InitializeAutomaton() = 0;
    virtual void _InitializeInitialStates() = 0;
    virtual void _InitializeFinalStates() = 0;
    virtual ResultType _IntersectNonEmptyCore(Symbol*, Term*, IntersectNonEmptyParams) = 0;
    virtual void _DumpExampleCore(std::ostream&, ExampleType, InterpretationType&) = 0;

public:
    // <<< CONSTRUCTORS >>>
    NEVER_INLINE explicit SymbolicAutomaton(Formula_ptr form);

    // <<< PUBLIC API >>>
    void MarkAsRestriction() { this->_isRestriction = true; }
    bool IsRestriction() { return this->_isRestriction; };
    void IncReferences() {++this->_refs;}
    void DecReferences() {assert(this->_refs > 0); --this->_refs; if(this->_refs < 1) delete this;}
    void InitializeStates();
    virtual Term_ptr GetInitialStates();
    virtual Term_ptr GetFinalStates();
    Gaston::VarList* GetFreeVars() { return &this->_freeVars;}
    Gaston::VarList* GetNonOccuringVars() { return &this->_nonOccuringVars; }
    virtual Term* Pre(Symbol*, Term*, IntersectNonEmptyParams) = 0;
    virtual ResultType IntersectNonEmpty(Symbol*, Term*, IntersectNonEmptyParams);
    void SetSatisfiableExample(Term*);
    void SetUnsatisfiableExample(Term*);
    virtual bool WasLastExampleValid() = 0;

    // <<< DUMPING FUNCTIONS >>>
    void DumpAutomatonMetrics();
    virtual void DumpAutomaton() = 0;
    virtual void DumpExample(std::ostream&, ExampleType, InterpretationType&);
    virtual void DumpComputationStats() = 0;
    virtual void FillStats() = 0;
    virtual void DumpProductHeader(std::ofstream&, bool, ProductType);
    virtual void DumpToDot(std::ofstream&, bool) = 0;
    static void AutomatonToDot(std::string, SymbolicAutomaton*, bool);
    static void GastonInfoToDot(std::ofstream&);
protected:
    NEVER_INLINE virtual ~SymbolicAutomaton();
};

struct SymLink {
    SymbolicAutomaton* aut;
    bool remap;
    std::map<unsigned int, unsigned int>* varRemap;
    size_t remap_tag;
    static size_t remap_number;

    SymLink() : aut(nullptr), remap(false), varRemap(nullptr), remap_tag(0) {}
    explicit SymLink(SymbolicAutomaton* s) : aut(s), remap(false), varRemap(nullptr), remap_tag(0) {}
    ~SymLink() {
        if(varRemap != nullptr) {
            delete varRemap;
        }
    }

    void InitializeSymLink(ASTForm*);
    ZeroSymbol* ReMapSymbol(ZeroSymbol*);
    unsigned int ReMapVariable(unsigned int var) {
        if(remap) {
            assert(this->varRemap != nullptr);
            return varMap.inverseGet((*this->varRemap)[varMap[var]]);
        }
    }
};

/**
 * BinaryOpAutomaton corresponds to Binary Operations of Intersection and
 * Union of subautomata. It further contains the links to left and right
 * operands and some additional functions for evaluation of results.
 *
 * The TernaryOpAutomaton and NaryOpAutomaton are the extensions of this
 * approach for optimization purposes.
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
    virtual ResultType _IntersectNonEmptyCore(Symbol*, Term*, IntersectNonEmptyParams);
    virtual void _DumpExampleCore(std::ostream&, ExampleType, InterpretationType&);

public:
    NEVER_INLINE BinaryOpAutomaton(SymbolicAutomaton_raw lhs, SymbolicAutomaton_raw rhs, Formula_ptr form);

    // <<< PUBLIC API >>>
    SymLink* GetLeft() { return &this->_lhs_aut;}
    virtual Term* Pre(Symbol*, Term*, IntersectNonEmptyParams);
    virtual bool WasLastExampleValid();
    std::pair<SymLink*, Term_ptr> LazyInit(Term_ptr);

    // <<< DUMPING FUNCTIONS >>>
    virtual void DumpAutomaton();
    virtual void DumpToDot(std::ofstream&, bool);
    virtual void DumpComputationStats();
    virtual void FillStats();
protected:
    NEVER_INLINE virtual ~BinaryOpAutomaton();
};

class TernaryOpAutomaton : public SymbolicAutomaton {
protected:
    // <<< PRIVATE MEMBERS >>>
    SymLink _lhs_aut;
    SymLink _mhs_aut;
    SymLink _rhs_aut;
    ProductType _productType;
    bool (*_eval_result)(bool, bool, bool, bool);   // Boolean function for evaluation of left, middle and right results
    bool (*_eval_early)(bool, bool, bool);          // Boolean function for early evaluation
    bool (*_early_val)(bool);

    // <<< PRIVATE FUNCTIONS >>>
    virtual void _InitializeAutomaton();
    virtual void _InitializeInitialStates();
    virtual void _InitializeFinalStates();
    virtual ResultType _IntersectNonEmptyCore(Symbol*, Term*, IntersectNonEmptyParams);
    virtual void _DumpExampleCore(std::ostream&, ExampleType, InterpretationType&);

public:
    NEVER_INLINE TernaryOpAutomaton(SymbolicAutomaton_raw lhs, SymbolicAutomaton_raw mhs, SymbolicAutomaton_raw rhs, Formula_ptr form);

    // <<< PUBLIC API >>>
    SymLink* GetLeft() { return &this->_lhs_aut; }
    virtual Term* Pre(Symbol*, Term*, IntersectNonEmptyParams);
    virtual bool WasLastExampleValid();
    std::pair<SymLink*, Term_ptr> LazyInit(Term_ptr); // Fixme: This maybe will need change of implementation

    // <<< DUMPING FUNCTIONS >>>
    virtual void DumpAutomaton();
    virtual void DumpToDot(std::ofstream&, bool);
    virtual void DumpComputationStats();
    virtual void FillStats();

protected:
    NEVER_INLINE virtual ~TernaryOpAutomaton();
};

class NaryOpAutomaton : public SymbolicAutomaton {
protected:
    // <<< PRIVATE MEMBERS >>>
    SymLink* _auts;
    std::vector<ASTForm*> _leaves;
    size_t _arity;
    ProductType _productType;
    bool (*_eval_result)(bool, bool, bool);
    bool (*_eval_early)(bool, bool);
    bool (*_early_val)(bool);

    // <<< PRIVATE FUNCTIONS >>>
    virtual void _InitializeAutomaton();
    virtual void _InitializeInitialStates();
    virtual void _InitializeFinalStates();
    virtual ResultType _IntersectNonEmptyCore(Symbol*, Term*, IntersectNonEmptyParams);
    virtual void _DumpExampleCore(std::ostream&, ExampleType, InterpretationType&);

public:
    NEVER_INLINE NaryOpAutomaton(Formula_ptr form, bool doComplement);

    // <<< PUBLIC API >>>
    SymLink* GetLeft() { return &this->_auts[0]; }
    virtual Term* Pre(Symbol*, Term*, IntersectNonEmptyParams);
    virtual bool WasLastExampleValid();
    // Fixme: Lazy init?

    // <<< DUMPING FUNCTIONS >>>
    virtual void DumpAutomaton();
    virtual void DumpToDot(std::ofstream&, bool);
    virtual void DumpComputationStats();
    virtual void FillStats();

protected:
    NEVER_INLINE virtual ~NaryOpAutomaton();
};

/**
 * Automaton corresponding to the formula: phi and psi
 */
class IntersectionAutomaton : public BinaryOpAutomaton {
public:
    NEVER_INLINE IntersectionAutomaton(SymbolicAutomaton* lhs, SymbolicAutomaton* rhs, Formula_ptr form);
};

class TernaryIntersectionAutomaton : public TernaryOpAutomaton {
public:
    NEVER_INLINE TernaryIntersectionAutomaton(SymbolicAutomaton_raw lhs, SymbolicAutomaton_raw mhs, SymbolicAutomaton_raw rhs, Formula_ptr form);
};

class NaryIntersectionAutomaton : public NaryOpAutomaton {
public:
    NEVER_INLINE NaryIntersectionAutomaton(Formula_ptr form, bool doComplement);
};

/**
 * Automaton corresponding to the formula: phi or psi
 */
class UnionAutomaton : public BinaryOpAutomaton {
public:
    NEVER_INLINE UnionAutomaton(SymbolicAutomaton* lhs, SymbolicAutomaton* rhs, Formula_ptr form);
};

class TernaryUnionAutomaton : public TernaryOpAutomaton {
public:
    NEVER_INLINE TernaryUnionAutomaton(SymbolicAutomaton_raw lhs, SymbolicAutomaton_raw mhs, SymbolicAutomaton_raw rhs, Formula_ptr form);
};

class NaryUnionAutomaton : public NaryOpAutomaton {
public:
    NEVER_INLINE NaryUnionAutomaton(Formula_ptr form, bool doComplement);
};

/**
 * Automaton corresponding to the formula: phi => psi
 */
class ImplicationAutomaton : public BinaryOpAutomaton {
public:
    NEVER_INLINE ImplicationAutomaton(SymbolicAutomaton_raw, SymbolicAutomaton_raw, Formula_ptr);
};

class TernaryImplicationAutomaton : public TernaryOpAutomaton {
public:
    NEVER_INLINE TernaryImplicationAutomaton(SymbolicAutomaton_raw, SymbolicAutomaton_raw, SymbolicAutomaton_raw, Formula_ptr);
};

class NaryImplicationAutomaton : public NaryOpAutomaton {
public:
    NEVER_INLINE NaryImplicationAutomaton(Formula_ptr, bool);
};

/**
 * Automaton corresponding to the formula: phi <=> pis
 */
class BiimplicationAutomaton : public BinaryOpAutomaton {
public:
    NEVER_INLINE BiimplicationAutomaton(SymbolicAutomaton_raw, SymbolicAutomaton_raw, Formula_ptr);
};

class TernaryBiimplicationAutomaton : public TernaryOpAutomaton {
public:
    NEVER_INLINE TernaryBiimplicationAutomaton(SymbolicAutomaton_raw, SymbolicAutomaton_raw, SymbolicAutomaton_raw, Formula_ptr);
};

class NaryBiimplicationAutomaton : public NaryOpAutomaton {
public:
    NEVER_INLINE NaryBiimplicationAutomaton(Formula_ptr, bool);
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
    virtual ResultType _IntersectNonEmptyCore(Symbol*, Term*, IntersectNonEmptyParams);
    virtual void _DumpExampleCore(std::ostream&, ExampleType, InterpretationType&);

public:
    // <<< CONSTRUCTORS >>>
    NEVER_INLINE ComplementAutomaton(SymbolicAutomaton *aut, Formula_ptr form);

    // <<< PUBLIC API >>>
    virtual Term* Pre(Symbol*, Term*, IntersectNonEmptyParams);
    virtual bool WasLastExampleValid();

    // <<< DUMPING FUNCTIONS >>>
    virtual void DumpAutomaton();
    virtual void DumpToDot(std::ofstream&, bool);
    virtual void DumpComputationStats();
    virtual void FillStats();
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
    unsigned int fixpointNext = 0;
    unsigned int fixpointPreNext = 0;
    std::string fixpointRes = "";
    std::string fixpointPreRes = "";
    #endif

protected:
    // <<< PRIVATE MEMBERS >>>
    SymLink _aut;
    FixpointGuide* _guide = nullptr;
    bool _isRoot;

    // <<< PRIVATE FUNCTIONS >>>
    virtual void _InitializeAutomaton();
    virtual void _InitializeInitialStates();
    virtual void _InitializeFinalStates();
    virtual ResultType _IntersectNonEmptyCore(Symbol*, Term*, IntersectNonEmptyParams);
    virtual void _DumpExampleCore(std::ostream&, ExampleType, InterpretationType&);

public:
    /// <<< CONSTRUCTORS >>>
    NEVER_INLINE ProjectionAutomaton(SymbolicAutomaton* aut, Formula_ptr form, bool isRoot = false);
    NEVER_INLINE ProjectionAutomaton(Formula_ptr form, SymbolicAutomaton* aut)
            : SymbolicAutomaton(form), _aut(aut), _isRoot(false) {
        type = AutType::PROJECTION;

        ASTForm_uvf* uvf_form = static_cast<ASTForm_uvf*>(this->_form);
        this->_guide = new FixpointGuide(uvf_form->vl);
    }
    // FIXME: Some day in future, I will curse myself for this Constructor ---^

    // <<< PUBLIC API >>>
    virtual Term* Pre(Symbol*, Term*, IntersectNonEmptyParams);
    SymbolicAutomaton* GetBase() { return this->_aut.aut;}
    bool IsRoot() { return this-> _isRoot; }
    FixpointGuide* GetGuide() { return this->_guide; }
    virtual bool WasLastExampleValid();
    char ProjectSymbol(size_t, char);

    // <<< DUMPING FUNCTIONS >>>
    virtual void DumpAutomaton();
    virtual void DumpToDot(std::ofstream&, bool);
    virtual void DumpComputationStats();
    virtual void FillStats();
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
public:
    // <<< CONSTRUCTORS >>>
    NEVER_INLINE RootProjectionAutomaton(SymbolicAutomaton*, Formula_ptr);

    // <<< PUBLIC API >>>
    virtual ResultType IntersectNonEmpty(Symbol*, Term*, IntersectNonEmptyParams);
};

class BaseProjectionAutomaton : public ProjectionAutomaton {
protected:
    // <<< PRIVATE MEMBERS >>>
    void _InitializeInitialStates();
    void _InitializeFinalStates();
    ResultType _IntersectNonEmptyCore(Symbol*, Term*, IntersectNonEmptyParams);

public:
    // <<< CONSTRUCTORS >>>
    NEVER_INLINE BaseProjectionAutomaton(SymbolicAutomaton*, Formula_ptr);
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
#   if (OPT_USE_SET_PRE == true)
    SetPreCache _setCache;
#   else
    PreCache _preCache;
#   endif

    /// <<< PRIVATE FUNCTIONS >>>
    virtual void _InitializeAutomaton();
    virtual void _InitializeInitialStates();
    virtual void _InitializeFinalStates();
    virtual ResultType _IntersectNonEmptyCore(Symbol*, Term*, IntersectNonEmptyParams);
    void _RenameStates();
    virtual void _DumpExampleCore(std::ostream&, ExampleType, InterpretationType&) {}
public:
    // <<< CONSTRUCTORS >>>
    NEVER_INLINE BaseAutomaton(BaseAutomatonType* aut, size_t vars, Formula_ptr form, bool emptyTracks);

    // <<< PUBLIC API >>>
    virtual Term* Pre(Symbol*, Term*, IntersectNonEmptyParams);
    virtual bool WasLastExampleValid();

    // <<< DUMPING FUNCTIONS >>>
    virtual void DumpToDot(std::ofstream&, bool);
    virtual void BaseAutDump();
    virtual void DumpComputationStats();
    virtual void FillStats();

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
