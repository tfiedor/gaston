/*****************************************************************************
 *  gaston - no real logic behind the name, we simply liked the poor seal gaston. R.I.P. brave soldier.
 *
 *  Copyright (c) 2015  Tomas Fiedor <ifiedortom@fit.vutbr.cz>
 *      Notable mentions: Ondrej Lengal <ondra.lengal@gmail.com>
 *          			  Overeating Panda <if-his-simulation-reduction-works>
 *
 *****************************************************************************/

#ifndef WSKS_SYMBOLICAUTOMATA_H
#define WSKS_SYMBOLICAUTOMATA_H

#include "../mtbdd/apply1func.hh"
#include "../mtbdd/apply2func.hh"
#include "../mtbdd/void_apply1func.hh"
#include "../mtbdd/ondriks_mtbdd.hh"
#include "../utils/Symbol.h"
#include "../environment.hh"
#include "../../Frontend/ident.h"
#include "../../Frontend/ast.h"
#include "../containers/VarToTrackMap.hh"
#include "StateSet.hh"
#include <vector>
#include <vata/util/binary_relation.hh>

class Term;

using namespace Gaston;

enum AutType {SYMBOLIC_BASE /*0*/, BINARY /*1*/, INTERSECTION /*2*/, UNION /*3*/, PROJECTION /*4*/, BASE /*5*/, COMPLEMENT /*6*/};
enum AutSubType {FINAL, NONFINAL};
enum AutBaseType {BT_SUB, BT_IN, BT_EQF, BT_EQS, BT_LSS, BT_LEQ, BT_FO, BT_T, BT_F};

extern VarToTrackMap varMap;

/**
 * Base class for symbolic automata
 */
class SymbolicAutomaton {
public:
    static StateType stateCnt;
    AutType type;

protected:
    // < Private Members >
    Formula_ptr _form;
    Term_ptr _initialStates;
    Term_ptr _finalStates;
    ResultCache _resCache;
    SubsumptionCache _subCache;
    VarList _freeVars;

    virtual void _InitializeAutomaton() = 0;
    virtual void _InitializeInitialStates() = 0;
    virtual void _InitializeFinalStates() = 0;
    virtual ResultType _IntersectNonEmptyCore(Symbol*, Term_ptr, bool) = 0;

// < Public API >
public:
    // < Public Constructors >
    SymbolicAutomaton(Formula_ptr form) : _form(form) {
        type = AutType::SYMBOLIC_BASE;

        IdentList free, bound;
        this->_form->freeVars(&free, &bound);
        for(auto it = free.begin(); it != free.end(); ++it) {
            _freeVars.insert(varMap[(*it)]);
        }
    }

    virtual Term_ptr GetInitialStates();
    virtual Term_ptr GetFinalStates();
    virtual Term_ptr Pre(Symbol*, Term_ptr, bool) = 0;
    ResultType IntersectNonEmpty(Symbol*, Term_ptr, bool);

    virtual void dump() = 0;
    virtual void DumpCacheStats() = 0;
};

/**
 *
 */
class BinaryOpAutomaton : public SymbolicAutomaton {
protected:
    std::shared_ptr<SymbolicAutomaton> _lhs_aut;
    std::shared_ptr<SymbolicAutomaton> _rhs_aut;
    bool (*_eval_result)(bool, bool, bool);
    bool (*_eval_early)(bool, bool);
    bool (*_early_val)(bool);
    virtual void _InitializeAutomaton() { this->_InitializeInitialStates(); this->_InitializeFinalStates(); }
    virtual void _InitializeInitialStates();
    virtual void _InitializeFinalStates();
    virtual ResultType _IntersectNonEmptyCore(Symbol*, Term_ptr, bool);

public:
    virtual Term_ptr Pre(Symbol*, Term_ptr, bool);
    BinaryOpAutomaton(SymbolicAutomaton* lhs,
                      SymbolicAutomaton* rhs,
                      Formula_ptr form)
            : SymbolicAutomaton(form), _lhs_aut(lhs), _rhs_aut(rhs) { this->_InitializeAutomaton(); type = AutType::BINARY;}
    virtual void dump();
    virtual void DumpCacheStats() {
        this->_form->dump();
        this->_resCache.dumpStats();
        this->_lhs_aut->DumpCacheStats();
        this->_rhs_aut->DumpCacheStats();
    }
};

/**
 *
 */
class IntersectionAutomaton : public BinaryOpAutomaton {
public:
    IntersectionAutomaton(SymbolicAutomaton* lhs,
                          SymbolicAutomaton* rhs,
                          Formula_ptr form)
            : BinaryOpAutomaton(lhs, rhs, form) {
        this->_InitializeAutomaton();
        this->type = AutType::INTERSECTION;
        this->_eval_result = [](bool a, bool b, bool underC) { if(!underC) {return a && b;} else {return a || b;} };
        this->_eval_early = [](bool a, bool underC) {return (a == underC);};
        this->_early_val = [](bool underC) { return underC;};
    }
};

/**
 *
 */
class UnionAutomaton : public BinaryOpAutomaton {
public:
    UnionAutomaton(SymbolicAutomaton* lhs,
                   SymbolicAutomaton* rhs,
                   Formula_ptr form)
            : BinaryOpAutomaton(lhs, rhs, form) {
        this->_InitializeAutomaton();
        this->type = AutType::UNION;
        this->_eval_result = [](bool a, bool b, bool underC) {if(!underC) {return a || b;} else { return a && b;} };
        this->_eval_early = [](bool a, bool underC) { return (a != underC);};
        this->_early_val = [](bool underC) { return !underC;};
    }
};

/**
 *
 */
class ComplementAutomaton : public SymbolicAutomaton {
protected:
    std::shared_ptr<SymbolicAutomaton> _aut;
    virtual void _InitializeAutomaton() { this->_InitializeInitialStates(); this->_InitializeFinalStates(); }
    virtual void _InitializeInitialStates();
    virtual void _InitializeFinalStates();
    virtual ResultType _IntersectNonEmptyCore(Symbol*, Term_ptr, bool);

public:
    ComplementAutomaton(SymbolicAutomaton *aut, Formula_ptr form) : SymbolicAutomaton(form), _aut(aut) { this->_InitializeAutomaton(); type = AutType::COMPLEMENT;}

    virtual Term_ptr Pre(Symbol*, Term_ptr, bool);
    virtual void dump();
    virtual void DumpCacheStats() {
        this->_form->dump();
        this->_resCache.dumpStats();
        this->_aut->DumpCacheStats();
    }
};

/**
 *
 */
class ProjectionAutomaton : public SymbolicAutomaton {
protected:
    std::shared_ptr<SymbolicAutomaton> _aut;
    IdentList* _projected_vars;

    virtual void _InitializeAutomaton() {
        this->_InitializeInitialStates();
        this->_InitializeFinalStates();
        this->_projected_vars = static_cast<ASTForm_uvf*>(this->_form)->vl;
    }
    virtual void _InitializeInitialStates();
    virtual void _InitializeFinalStates();
    virtual ResultType _IntersectNonEmptyCore(Symbol*, Term_ptr, bool);

public:
    ProjectionAutomaton(SymbolicAutomaton* aut, Formula_ptr form) : SymbolicAutomaton(form), _aut(aut) { this->_InitializeAutomaton(); type = AutType::PROJECTION;}

    virtual Term_ptr Pre(Symbol*, Term_ptr, bool);
    virtual void dump();
    virtual void DumpCacheStats() {
        this->_form->dump();
        this->_resCache.dumpStats();
        this->_aut->DumpCacheStats();
    }
};

/**
 *
 */
class BaseAutomaton : public SymbolicAutomaton {
protected:
    virtual void _InitializeAutomaton();
    virtual void _InitializeInitialStates();
    virtual void _InitializeFinalStates();
    virtual ResultType _IntersectNonEmptyCore(Symbol*, Term_ptr, bool);

    std::shared_ptr<BaseAutomatonType> _base_automaton;
    void _RenameStates();

public:
    BaseAutomaton(BaseAutomatonType* aut, Formula_ptr form) : SymbolicAutomaton(form), _base_automaton(aut){ this->_InitializeAutomaton();type = AutType::BASE; }
    virtual Term_ptr Pre(Symbol*, Term_ptr, bool);
    virtual void baseAutDump();
    virtual void DumpCacheStats() {
        this->_form->dump();
        this->_resCache.dumpStats();
    }
};

class GenericBaseAutomaton : public BaseAutomaton {
public:
    GenericBaseAutomaton(BaseAutomatonType* aut, Formula_ptr form) : BaseAutomaton(aut, form) { }
    virtual void dump() { std::cout << "<Aut>";
#if (DEBUG_BASE_AUTOMATA == true)
        this->baseAutDump();
        #endif
    }
};

class SubAutomaton : public BaseAutomaton {
public:
    SubAutomaton(BaseAutomatonType* aut, Formula_ptr form) : BaseAutomaton(aut, form) { }
    virtual void dump() { std::cout << "Sub";
        #if (DEBUG_BASE_AUTOMATA == true)
        this->baseAutDump();
        #endif
    }
};

class TrueAutomaton : public BaseAutomaton {
public:
    TrueAutomaton(BaseAutomatonType* aut, Formula_ptr form) : BaseAutomaton(aut, form) {}
    virtual void dump() { std::cout << "True";
        #if (DEBUG_BASE_AUTOMATA == true)
        this->baseAutDump();
        #endif
    }
};

class FalseAutomaton : public BaseAutomaton {
public:
    FalseAutomaton(BaseAutomatonType* aut, Formula_ptr form) : BaseAutomaton(aut, form) {}
    virtual void dump() { std::cout << "False";
        #if (DEBUG_BASE_AUTOMATA == true)
        this->baseAutDump();
        #endif
    }
};

class InAutomaton : public BaseAutomaton {
public:
    InAutomaton(BaseAutomatonType* aut, Formula_ptr form) : BaseAutomaton(aut, form) {}
    virtual void dump() { std::cout << "In";
        #if (DEBUG_BASE_AUTOMATA == true)
        this->baseAutDump();
        #endif
    }
};

class FirstOrderAutomaton : public BaseAutomaton {
public:
    FirstOrderAutomaton(BaseAutomatonType* aut, Formula_ptr form) : BaseAutomaton(aut, form) {}
    virtual void dump() { std::cout << "FirstOrder";
        #if (DEBUG_BASE_AUTOMATA == true)
        this->baseAutDump();
        #endif
    }
};

class EqualFirstAutomaton : public BaseAutomaton {
public:
    EqualFirstAutomaton(BaseAutomatonType* aut, Formula_ptr form) : BaseAutomaton(aut, form) {}
    virtual void dump() { std::cout << "Equal1";
        #if (DEBUG_BASE_AUTOMATA == true)
        this->baseAutDump();
        #endif
    }
};

class EqualSecondAutomaton : public BaseAutomaton {
public:
    EqualSecondAutomaton(BaseAutomatonType* aut, Formula_ptr form) : BaseAutomaton(aut, form) {}
    virtual void dump() { std::cout << "Equal2";
        #if (DEBUG_BASE_AUTOMATA == true)
        this->baseAutDump();
        #endif
    }
};

class LessAutomaton : public BaseAutomaton {
public:
    LessAutomaton(BaseAutomatonType* aut, Formula_ptr form) : BaseAutomaton(aut, form) {}
    virtual void dump() { std::cout << "Less";
        #if (DEBUG_BASE_AUTOMATA == true)
        this->baseAutDump();
        #endif
    }
};

class LessEqAutomaton : public BaseAutomaton {
public:
    LessEqAutomaton(BaseAutomatonType* aut, Formula_ptr form) : BaseAutomaton(aut, form) {}
    virtual void dump() { std::cout << "LessEq";
        #if (DEBUG_BASE_AUTOMATA == true)
        this->baseAutDump();
        #endif
    }
};

// <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<< FUNCTORS >>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>>

GCC_DIAG_OFF(effc++)
class BaseCollectorFunctor : public VATA::MTBDDPkg::VoidApply1Functor<BaseCollectorFunctor, BaseAutomatonStates> {
    GCC_DIAG_ON(effc++)
private:
    BaseAutomatonStates& collected;
    bool _minusInteresect;

public:
    bool _isFirst;
    // < Public Constructors >
    BaseCollectorFunctor(BaseAutomatonStates& l, bool minusIntersect) : collected(l), _minusInteresect(minusIntersect), _isFirst(true) {}

    // < Public Methods >
    /**
     * @param lhs: operand of apply
     */
    inline void ApplyOperation(BaseAutomatonStates rhs) {
        if(_minusInteresect) {
            if(_isFirst) {
                collected.insert(rhs);
                return;
            }
            auto itLhs = collected.begin();
            auto itRhs = rhs.begin();
            BaseAutomatonStates intersection;

            while ((itLhs != collected.end()) || (itRhs != rhs.end()))
            {	// until we drop out of the array (or find a common element)
                if (*itLhs == *itRhs)
                {	// in case there exists a common element
                    intersection.insert(*itLhs);
                    ++itLhs;
                    ++itRhs;
                }
                else if (*itLhs < *itRhs)
                {	// in case the element in lhs is smaller
                    ++itLhs;
                }
                else
                {	// in case the element in rhs is smaller
                    assert(*itLhs > *itRhs);
                    ++itRhs;
                }
            }

            collected.clear();
            collected.insert(intersection);
        } else {
            collected.insert(rhs);
        }
    }
};

GCC_DIAG_OFF(effc++)
class MaskerFunctor : public VATA::MTBDDPkg::Apply2Functor<MaskerFunctor, BaseAutomatonStates, BaseAutomatonStates, BaseAutomatonStates> {
    GCC_DIAG_ON(effc++)
public:
    // < Public Methods >
    /**
     * @param lhs: left operand
     * @param rhs: right operand
     * @return union of leaf operands
     */
    inline BaseAutomatonStates ApplyOperation(BaseAutomatonStates lhs, BaseAutomatonStates rhs) {
        if(rhs.size() == 0) {
            return rhs;
        } else {
            return lhs;
        }
    }
};


#endif //WSKS_SYMBOLICAUTOMATA_H
