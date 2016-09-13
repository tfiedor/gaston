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
#include "../checkers/Checker.h"
#include "../containers/VarToTrackMap.hh"
#include "../containers/Workshops.h"
#include "../utils/Timer.h"
#include "../../Frontend/dfa.h"
#include "../../Frontend/env.h"
#include "../../Frontend/symboltable.h"

extern VarToTrackMap varMap;
extern SymbolTable symbolTable;
extern Ident lastPosVar, allPosVar;
extern Options options;
extern char *inputFileName;

StateType SymbolicAutomaton::stateCnt = 0;
DagNodeCache* SymbolicAutomaton::dagNodeCache = new DagNodeCache();
DagNodeCache* SymbolicAutomaton::dagNegNodeCache = new DagNodeCache();

using namespace Gaston;

size_t SymLink::remap_number = 0;

// <<< SYMLINK FUNCTIONS >>>
void SymLink::InitializeSymLink(ASTForm* form) {
    if(this->aut->_form != form) {
        // Construct the mapping
        this->varRemap = new std::map<unsigned int, unsigned int>();
        form->ConstructMapping(this->aut->_form, *this->varRemap);
        bool is_identity = true;
        for(auto it = this->varRemap->begin(); it != this->varRemap->end(); ++it) {
            if(it->first != it->second) {
                is_identity = false;
                break;
            }
        }

        if(is_identity) {
            this->remap = false;
            delete this->varRemap;
            this->varRemap = nullptr;
        } else {
            this->remap = true;
            this->remap_tag = ++SymLink::remap_number;
        }
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
        return this->aut->symbolFactory.CreateRemappedSymbol(symbol, this->varRemap, this->remap_tag);
    } else {
        return symbol;
    }
}

/**
 * @brief Initializes variables that are occuring in the formula corresponding to the automaton
 *
 * Takes the union of free and bound variables, and initializes the occuring variable of the
 * automaton. I.e. those variables that are not don't care
 */
void SymbolicAutomaton::_InitializeOccuringVars() {
    IdentList free, bound;
    this->_form->freeVars(&free, &bound);
    IdentList* allVars;
    allVars = ident_union(&free, &bound);
    size_t varNum = varMap.TrackLength();

    if(allVars != nullptr) {
        for(auto it = allVars->begin(); it != allVars->end(); ++it) {
            if(varMap.IsIn(*it))
                _freeVars.insert(varMap[(*it)]);
        }

        delete allVars;
    }
}

/**
 * @brief Initializes variables that are NOT occuring in the formula corresponding to the automaton
 *
 * Takes the complement of the occuring variables. These variables are further trimmed during the
 * computation from the symbols.
 */
void SymbolicAutomaton::_InitializeNonOccuring() {
    size_t varNum = varMap.TrackLength();
    auto it = this->_freeVars.begin();
    auto end = this->_freeVars.end();

    for(size_t var = 0; var < varNum; ++var) {
        if(it != end && var == *it) {
            ++it;
        } else {
            this->_nonOccuringVars.insert(var);
        }
    }
}

// <<< CONSTRUCTORS >>>
SymbolicAutomaton::SymbolicAutomaton(Formula_ptr form) :
        _form(form), _factory(this), _initialStates(nullptr), _finalStates(nullptr), _satExample(nullptr),
        _unsatExample(nullptr), _refs(0) {
    // Fixme: isn't allVars the same as formula->allVars?
    this->_InitializeOccuringVars();
    this->_InitializeNonOccuring();

}

SymbolicAutomaton::~SymbolicAutomaton() {
}

BinaryOpAutomaton::BinaryOpAutomaton(SymbolicAutomaton_raw lhs, SymbolicAutomaton_raw rhs, Formula_ptr form)
        : SymbolicAutomaton(form), _lhs_aut(lhs), _rhs_aut(rhs) {
    type = AutType::BINARY;
    lhs->IncReferences();
    this->_lhs_aut.InitializeSymLink(reinterpret_cast<ASTForm_ff*>(this->_form)->f1);
#   if (OPT_EARLY_EVALUATION == true)
    // We will do the lazy initialization by ourselves;
    assert(rhs == nullptr);
    this->_rhs_aut.aut = nullptr;
#   else
    rhs->IncReferences();
    this->_rhs_aut.InitializeSymLink(reinterpret_cast<ASTForm_ff*>(this->_form)->f2);
#   endif
}

BinaryOpAutomaton::~BinaryOpAutomaton() {
    this->_lhs_aut.aut->DecReferences();
    if(this->_rhs_aut.aut != nullptr) {
        this->_rhs_aut.aut->DecReferences();
    }
}

TernaryOpAutomaton::TernaryOpAutomaton(SymbolicAutomaton_raw lhs, SymbolicAutomaton_raw mhs, SymbolicAutomaton_raw rhs, Formula_ptr form)
        : SymbolicAutomaton(form), _lhs_aut(lhs), _mhs_aut(mhs), _rhs_aut(rhs) {
    // Fixme: Add lazy init
    type = AutType::TERNARY;

    // Fixme: Refactor: Move to other function?
    ASTForm* left, *middle, *right;
    ASTKind k = form->kind;
    ASTForm_ff* ff_form = static_cast<ASTForm_ff*>(form);
    if(ff_form->f1->kind == k) {
        ASTForm_ff* f1_form = static_cast<ASTForm_ff*>(ff_form->f1);
        left = f1_form->f1;
        middle = f1_form->f2;
        right = ff_form->f2;
    } else {
        ASTForm_ff* f2_form = static_cast<ASTForm_ff*>(ff_form->f2);
        left = ff_form->f1;
        middle = f2_form->f1;
        right = f2_form->f2;
    }

    lhs->IncReferences();
    this->_lhs_aut.InitializeSymLink(left);
    mhs->IncReferences();
    this->_mhs_aut.InitializeSymLink(middle);
    rhs->IncReferences();
    this->_rhs_aut.InitializeSymLink(right);
}

TernaryOpAutomaton::~TernaryOpAutomaton() {
    this->_lhs_aut.aut->DecReferences();
    if(this->_mhs_aut.aut != nullptr) {
        this->_mhs_aut.aut->DecReferences();
    }
    if(this->_rhs_aut.aut != nullptr) {
        this->_rhs_aut.aut->DecReferences();
    }
}

void collect_leaves(ASTKind k, ASTForm* form, std::vector<ASTForm*>& leaves, ASTForm*& qf_free) {
    if(form->kind == k) {
        ASTForm_ff* ff_form = static_cast<ASTForm_ff*>(form);
        collect_leaves(k, ff_form->f1, leaves, qf_free);
        collect_leaves(k, ff_form->f2, leaves, qf_free);
    } else {
        if(form->fixpoint_number > 0) {
            leaves.push_back(form);
        } else {
            // Collect qf_free stuff to the same shit
            if(qf_free == nullptr) {
                qf_free = form;
            } else {
                if(k == aAnd) {
                    qf_free = new ASTForm_And(form, qf_free, Pos());
                } else {
                    assert(k == aOr);
                    qf_free = new ASTForm_Or(form, qf_free, Pos());
                }
            }
            qf_free->fixpoint_number = 0;
            qf_free->tag = 0;
        }
    }
}

NaryOpAutomaton::NaryOpAutomaton(Formula_ptr form, bool doComplement) : SymbolicAutomaton(form) {
    type = AutType::NARY;

    ASTForm* qf_free = nullptr;
    collect_leaves(form->kind, form, this->_leaves, qf_free);
    this->_arity = this->_leaves.size() + (qf_free == nullptr ? 0 : 1);
    this->_auts = new SymLink[this->_arity];
    for (int i = (qf_free == nullptr ? 0 : 1); i < this->_arity; ++i) {
        this->_auts[i].aut = this->_leaves[i - (qf_free == nullptr ? 0 : 1)]->toSymbolicAutomaton(doComplement);
        assert(this->_auts[i].aut != nullptr);
        this->_auts[i].aut->IncReferences();
        this->_auts[i].InitializeSymLink(this->_leaves[i - (qf_free == nullptr ? 0 : 1)]);
    }

    if(qf_free != nullptr) {
        this->_auts[0].aut = qf_free->toSymbolicAutomaton(doComplement);
        this->_auts[0].aut->IncReferences();
        this->_auts[0].InitializeSymLink(qf_free);
    }
}

NaryOpAutomaton::~NaryOpAutomaton() {
    for (int i = 0; i < this->_arity; ++i) {
        if(this->_auts[i].aut != nullptr) {
            this->_auts[i].aut->DecReferences();
        }
    }
    delete[] this->_auts;
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

    // Initialize the guide
    // Fixme: Refactor: This is mess!!!
    ASTForm* innerForm = static_cast<ASTForm_q*>(this->_form)->f;
    ASTForm_uvf* uvf_form = static_cast<ASTForm_uvf*>(this->_form);
    this->_guide = new FixpointGuide(uvf_form->vl);
}

ProjectionAutomaton::~ProjectionAutomaton() {
    if(this->_guide != nullptr) {
        delete _guide;
    }
    this->_aut.aut->DecReferences();
}

RootProjectionAutomaton::RootProjectionAutomaton(SymbolicAutomaton* aut, Formula_ptr form)
        : ProjectionAutomaton(aut, form, true) {}

BaseProjectionAutomaton::BaseProjectionAutomaton(SymbolicAutomaton* aut, Formula_ptr form)
        : ProjectionAutomaton(form, aut) {
    this->_InitializeAutomaton();
    aut->IncReferences();
    this->_aut.InitializeSymLink(reinterpret_cast<ASTForm_q*>(this->_form)->f);

    // Remove bound variables from free and push them to nonoccuring
    ASTForm_uvf* fixpoint_formula = static_cast<ASTForm_uvf*>(this->_form);
    for(auto it = fixpoint_formula->vl->begin(); it != fixpoint_formula->vl->end(); ++it) {
        //Fixme: But freeVars are not really used anywhere, right?
        assert(varMap.IsIn(*it));
        this->_nonOccuringVars.insert(varMap[*it]);
    }
}

BaseAutomaton::BaseAutomaton(BaseAutomatonType* aut, size_t vars, Formula_ptr form, bool emptyTracks) : SymbolicAutomaton(form), _autWrapper(dfaCopy(aut), emptyTracks, form->is_restriction, vars) {
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
    this->_productType = ProductType::INTERSECTION;
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

TernaryIntersectionAutomaton::TernaryIntersectionAutomaton(SymbolicAutomaton_raw lhs, SymbolicAutomaton_raw mhs, SymbolicAutomaton_raw rhs, Formula_ptr form)
        : TernaryOpAutomaton(lhs, mhs, rhs, form) {
    this->type = AutType::TERNARY_INTERSECTION;
    this->_productType = ProductType::INTERSECTION;
    this->_eval_result = [](bool l, bool m, bool r, bool underC) {
        // e in L cap M cap R == e in L && e in M && e in R
        if(!underC) {return l && m && r;}
        // e notin L cap M cap R == e notin L || ...
        else {return l || m || r;}
    };
    this->_eval_early = [](bool l, bool m, bool underC) {
        if(!underC) {
            return !l || !m;
        } else {
            return l || m;
        }
    };
    this->_early_val = [](bool underC) {
        return underC;
    };

    this->_InitializeAutomaton();
}

NaryIntersectionAutomaton::NaryIntersectionAutomaton(Formula_ptr form, bool doComplement) : NaryOpAutomaton(form, doComplement) {
    this->type = AutType::NARY_INTERSECTION;
    this->_productType = ProductType::INTERSECTION;
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
    this->_productType = ProductType::UNION;
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

TernaryUnionAutomaton::TernaryUnionAutomaton(SymbolicAutomaton_raw lhs, SymbolicAutomaton_raw mhs, SymbolicAutomaton_raw rhs, Formula_ptr form)
        : TernaryOpAutomaton(lhs, mhs, rhs, form) {
    this->type = AutType::TERNARY_UNION;
    this->_productType = ProductType::UNION;
    this->_eval_result = [](bool l, bool m, bool r, bool underC) {
        // e in L cup M cup R == e in L || e in M || e in R
        if(!underC) {return l || m || r;}
            // e notin L cup M cup R == e notin L && ...
        else {return l && m && r;}
    };
    this->_eval_early = [](bool l, bool m, bool underC) {
        if(!underC) {
            return l || m;
        } else {
            return !l || !m;
        }
    };
    this->_early_val = [](bool underC) {
        return !underC;
    };

    this->_InitializeAutomaton();
}

NaryUnionAutomaton::NaryUnionAutomaton(Formula_ptr form, bool doComplement) : NaryOpAutomaton(form, doComplement) {
    this->type = AutType::NARY_UNION;
    this->_productType = ProductType::UNION;
    this->_eval_result = [](bool a, bool b, bool underC) {
        // e in A cup B == e in A || e in B
        if(!underC) {return a || b;}
            // e notin A cup B == e notin A && e notin B
        else {return a && b;}
    };
    this->_eval_early = [](bool a, bool underC) {
        return (a != underC);
    };
    this->_early_val = [](bool underC) {
        return !underC;
    };

    this->_InitializeAutomaton();
}

// Derives of Implication
ImplicationAutomaton::ImplicationAutomaton(SymbolicAutomaton_raw lhs, SymbolicAutomaton_raw rhs, Formula_ptr form)
        : BinaryOpAutomaton(lhs, rhs, form) {
    this->type == AutType::IMPLICATION;
    this->_productType = ProductType::IMPLICATION;
    this->_eval_result = [](bool a, bool b, bool underC) {
        // e in ~A cup B == e in A => e in B
        if(!underC) {return !a || b;}
        // e not in ~A cup B == !e notin A && e notin B
        else { return !a && b; }
    };
    this->_eval_early = [](bool a, bool underC) {
        return (a == underC);
    };
    this->_early_val = [](bool underC) {
        return !underC;
    };

    this->_InitializeAutomaton();
}

TernaryImplicationAutomaton::TernaryImplicationAutomaton(SymbolicAutomaton_raw lhs, SymbolicAutomaton_raw mhs, SymbolicAutomaton_raw rhs, Formula_ptr form)
        : TernaryOpAutomaton(lhs, mhs, rhs, form) {
    assert(false && "Ternary Implication is currently disabled as it is unclear how such automata should work");
}

NaryImplicationAutomaton::NaryImplicationAutomaton(Formula_ptr form, bool b)
        : NaryOpAutomaton(form, b) {
    assert(false && "Nary Implication is currently disabled as it is unclear how such automata should work");
}

// Derives of Biimplication
BiimplicationAutomaton::BiimplicationAutomaton(SymbolicAutomaton_raw lhs, SymbolicAutomaton_raw rhs, Formula_ptr form)
        : BinaryOpAutomaton(lhs, rhs, form) {
    this->type = AutType::BIIMPLICATION;
    this->_productType = ProductType::BIIMPLICATION;
    this->_eval_result = [](bool a, bool b, bool underC) {
        if(!underC) { return a == b;}
        else {return (!a) == b;}
    };
    this->_eval_early = [](bool a, bool underC) {
        assert(false && "Biimplication cannot be short circuited");
        return false;
    };
    this->_early_val = [](bool underC) {
        assert(false && "Biimplication does not have short circuit");
        return false;
    };

    this->_InitializeAutomaton();
}

TernaryBiimplicationAutomaton::TernaryBiimplicationAutomaton(SymbolicAutomaton_raw lhs, SymbolicAutomaton_raw mhs, SymbolicAutomaton_raw rhs, Formula_ptr form)
        : TernaryOpAutomaton(lhs, mhs, rhs, form) {
    this->type =  AutType::TERNARY_BIIMPLICATION;
    this->_productType = ProductType::BIIMPLICATION;
    this->_eval_result = [](bool l, bool m, bool r, bool underC) {
        // Fixme: I'm still little bit concerned whether this is true
        if(!underC) { return l == m == r; }
        else {return (l == (!m) == (!r)); }
    };
    this->_eval_early = [](bool l, bool m, bool underC) {
        assert(false && "Biimplication cannot be short circuited");
        return false;
    };
    this->_early_val = [](bool underC) {
        assert(false && "Biimplication does not have short circuit");
        return false;
    };

    this->_InitializeAutomaton();
}

NaryBiimplicationAutomaton::NaryBiimplicationAutomaton(Formula_ptr form, bool underC)
        : NaryOpAutomaton(form, underC) {
    this->type == AutType::NARY_BIIMPLICATION;
    this->_productType = ProductType::BIIMPLICATION;
    this->_eval_result = [](bool a, bool b, bool underC) {
        if(!underC) { return a == b;}
        else {return a == (!b);}
    };
    this->_eval_early = [](bool a, bool underC) {
        assert(false && "Biimplication cannot be short circuited");
        return false;
    };
    this->_early_val = [](bool a) {
        assert(false && "Biimplication does not have short circuit");
        return false;
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
    assert(stateApproximation != nullptr);
#   if (MEASURE_SUBAUTOMATA_TIMING == true)
    this->timer.Start();
#   endif
    ResultType result;

    // Empty set needs not to be computed
    if(stateApproximation->type == TermType::EMPTY) {
        bool res = underComplement != stateApproximation->InComplement();
        this->_lastResult = res;
#       if (MEASURE_SUBAUTOMATA_TIMING == true)
        this->timer.Stop();
#       endif
        return std::make_pair(stateApproximation, res);
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
        symbol = this->symbolFactory.CreateTrimmedSymbol(symbol, &this->_nonOccuringVars);
    }

    // If we have continuation, we have to unwind it
#   if (OPT_EARLY_EVALUATION == true)
    if(stateApproximation->type == TermType::CONTINUATION) {
#       if (MEASURE_CONTINUATION_EVALUATION == true || MEASURE_ALL == true)
        ++this->_contUnfoldingCounter;
#       endif
        TermContinuation* continuation = reinterpret_cast<TermContinuation*>(stateApproximation);
        stateApproximation = continuation->unfoldContinuation(UnfoldedInType::E_IN_ISECT_NONEMPTY);
    }

    assert(stateApproximation != nullptr);
    assert(stateApproximation->type != TermType::CONTINUATION);

    // Empty set needs not to be computed
    if(stateApproximation->type == TermType::EMPTY) {
        bool res = underComplement != stateApproximation->InComplement();
        this->_lastResult = res;
#       if (MEASURE_SUBAUTOMATA_TIMING == true)
        this->timer.Stop();
#       endif
        return std::make_pair(stateApproximation, res);
    }
#   endif

#   if (OPT_CACHE_RESULTS == true)
    // Look up in cache, if in cache, return the result
    bool inCache = true;
    auto key = std::make_pair(stateApproximation, symbol);
#       if (OPT_DONT_CACHE_CONT == true && OPT_EARLY_EVALUATION == true)
    bool dontSearchTheCache = (stateApproximation->type == TermType::PRODUCT && stateApproximation->IsNotComputed());
    if (!dontSearchTheCache && (inCache = this->_resCache.retrieveFromCache(key, result))) {
#       else
    if (inCache = this->_resCache.retrieveFromCache(key, result)) {
#       endif
        assert(result.first != nullptr);
        this->_lastResult = result.second;
#       if (MEASURE_SUBAUTOMATA_TIMING == true)
        this->timer.Stop();
#       endif
        if(symbol != nullptr) {
            result.first->SetSuccessor(stateApproximation, symbol);
        }
        return result;
    }
#   endif

    // Call the core function
    result = this->_IntersectNonEmptyCore(symbol, stateApproximation, underComplement); // TODO: Memory consumption
#   if (MEASURE_RESULT_HITS == true || MEASURE_ALL == true)
    (result.second ? ++this->_trueCounter : ++this->_falseCounter);
#   endif

    // Cache Results
#   if (OPT_CACHE_RESULTS == true)
#       if (OPT_DONT_CACHE_CONT == true && OPT_EARLY_EVALUATION == true)
        if(stateApproximation->type == TermType::PRODUCT) {
            // If either side is not fully computed, we do not cache it
            TermProduct* tProduct = reinterpret_cast<TermProduct*>(stateApproximation);
            inCache = tProduct->left->IsNotComputed() || tProduct->right == nullptr || tProduct->right->IsNotComputed();
        }
#       endif
#       if (OPT_DONT_CACHE_UNFULL_FIXPOINTS == true)
        if(result.first->type == TermType::FIXPOINT) {
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
    this->_lastResult = result.second;
#   if (MEASURE_SUBAUTOMATA_TIMING == true)
    this->timer.Stop();
#   endif
    return result;
}

ResultType RootProjectionAutomaton::IntersectNonEmpty(Symbol* symbol, Term* finalApproximation, bool underComplement) {
    assert(this->_unsatExample == nullptr && this->_satExample == nullptr); // ! Nothing should be found yet
    assert(symbol == nullptr); // ! The search is "manual"
    assert(underComplement == false);

    // We are doing the initial step by evaluating the epsilon
    TermList* projectionApproximation = reinterpret_cast<TermList*>(finalApproximation);
    if(finalApproximation->type == TermType::EMPTY || projectionApproximation->list.size() == 0) {
        return std::make_pair(this->_factory.CreateEmpty(), underComplement);
    }

    // Evaluate the initial unfolding of epsilon
    ResultType result = this->_aut.aut->IntersectNonEmpty(this->_aut.ReMapSymbol(symbol), projectionApproximation->list[0], underComplement);
    if(result.first->type == TermType::EMPTY) {
        return result;
    }
#   if (DEBUG_ROOT_AUTOMATON == true || true)
    std::cout << "Finished computing initial unfolding of epsilon\n";
#   endif

    // Create a new fixpoint term and iterator on it
    TermFixpoint* fixpoint = this->_factory.CreateFixpoint(result.first, SymbolWorkshop::CreateZeroSymbol(), underComplement, result.second, WorklistSearchType::UNGROUND_ROOT);
    TermFixpoint::iterator it = fixpoint->GetIterator();
    Term_ptr fixpointTerm = nullptr;
    assert(allPosVar != -1 || options.test == TestType::EVERYTHING);

#   if (DEBUG_EXAMPLE_PATHS == true)
    size_t maxPath = 0;
    Timer timer_paths;
    timer_paths.start();
#   endif
    std::cout << "Start something bitch\n";
    // While the fixpoint is not fully unfolded and while we cannot evaluate early
    while((this->_satExample == nullptr || this->_unsatExample == nullptr) && ((fixpointTerm = it.GetNext()) != nullptr)) {
        if(allPosVar != -1 && options.test != TestType::EVERYTHING) {
            if (options.test == TestType::VALIDITY && this->_unsatExample != nullptr) {
                break;
            } else if ((options.test == TestType::SATISFIABILITY || options.test == TestType::UNSATISFIABILITY) &&
                       this->_satExample != nullptr) {
                break;
            }
        }
        fixpoint->RemoveSubsumed();
#       if (DEBUG_EXAMPLE_PATHS == true)
        if(fixpointTerm != nullptr && fixpointTerm->link->len > maxPath) {
            std::cout << "[*] Finished exploring examples of length '" << maxPath << "': ";
            timer_paths.stop();
            timer_paths.print();
            timer_paths.start();
            maxPath = fixpointTerm->link->len;
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
        if(fixpointTerm != nullptr && fixpointTerm->link->symbol != nullptr) {
            std::cout << (*fixpointTerm->link->symbol);
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
#           if (DEBUG_ROOT_AUTOMATON == true)
            std::cout << "[*] Found satisfying example\n";
#           endif
            this->_satExample = examples.first;
        }
        if(this->_unsatExample == nullptr && examples.second != nullptr) {
#           if (DEBUG_ROOT_AUTOMATON == true)
            std::cout << "[*] Found unsatisfying counter-example\n";
#           endif
            this->_unsatExample = examples.second;
        }
    }
#   if (DEBUG_EXAMPLE_PATHS == true)
    timer_paths.stop();
#   endif

    ExamplePair examples = fixpoint->GetFixpointExamples();
    this->_satExample = examples.first;
    this->_unsatExample = examples.second;

    return std::make_pair(fixpoint, fixpoint->GetResult());
}

/**
 * Lazy evaluation of final states. If states are not initialized, they are recreated and returned
 *
 * @return: Final states of automaton as Term
 */
Term_ptr SymbolicAutomaton::GetFinalStates() {
    return this->_finalStates;
}

/**
 * Lazy evaluation of initial states. If state are not initialized, they are recreated and returned
 *
 * @return: Initial states of automaton as Term
 */
Term_ptr SymbolicAutomaton::GetInitialStates() {
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

void TernaryOpAutomaton::_InitializeAutomaton() {
    this->_factory.InitializeWorkshop();
    this->_InitializeInitialStates();
    this->_InitializeFinalStates();
}

void NaryOpAutomaton::_InitializeAutomaton() {
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
#   if (OPT_EARLY_EVALUATION == true)
    this->_initialStates = this->_factory.CreateProduct(this->_lhs_aut.aut->GetInitialStates(),
          (this->_rhs_aut.aut == nullptr ? nullptr : this->_rhs_aut.aut->GetInitialStates()), this->_productType);
#   else
    this->_initialStates = this->_factory.CreateProduct(this->_lhs_aut.aut->GetInitialStates(), this->_rhs_aut.aut->GetInitialStates(), this->_productType);
#   endif
    #endif
}

void TernaryOpAutomaton::_InitializeInitialStates() {
    // Fixme: Add lazy initialization + no workshops
    this->_initialStates = this->_factory.CreateTernaryProduct(this->_lhs_aut.aut->GetInitialStates(),
        this->_mhs_aut.aut->GetInitialStates(), this->_rhs_aut.aut->GetInitialStates(), this->_productType);
}

void NaryOpAutomaton::_InitializeInitialStates() {
    // Fixme: Add lazy initialization + no workshops
    this->_initialStates = this->_factory.CreateBaseNaryProduct(this->_auts, this->_arity, StatesSetType::INITIAL, this->_productType);
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

void BaseProjectionAutomaton::_InitializeInitialStates() {
    this->_initialStates = this->_aut.aut->GetInitialStates();
}

void BaseAutomaton::_InitializeInitialStates() {
    // NOTE: The automaton is constructed backwards, so final states are initial
    assert(this->_initialStates == nullptr);

    BaseAutomatonStateSet initialStates;
    initialStates.insert(this->_autWrapper.GetInitialState());

    this->_initialStates = this->_factory.CreateBaseSet(std::move(initialStates));
}

/**
 * Initialization of final states for automata wrt. the structure of the symbolic automaton
 */
void BinaryOpAutomaton::_InitializeFinalStates() {
    // #TERM_CREATION
    #if (DEBUG_NO_WORKSHOPS == true)
    this->_finalStates = new TermProduct(this->_lhs_aut.aut->GetFinalStates(), this->_rhs_aut.aut->GetFinalStates(), this->_productType);
    #else
#   if (OPT_EARLY_EVALUATION == true)
    this->_finalStates = this->_factory.CreateProduct(this->_lhs_aut.aut->GetFinalStates(),
          (this->_rhs_aut.aut == nullptr) ? nullptr : this->_rhs_aut.aut->GetFinalStates(), this->_productType);
#   else
    this->_finalStates = this->_factory.CreateProduct(this->_lhs_aut.aut->GetFinalStates(), this->_rhs_aut.aut->GetFinalStates(), this->_productType);
#   endif
    #endif
}

void TernaryOpAutomaton::_InitializeFinalStates() {
    // Fixme: Add lazy initialization + no workshops
    this->_finalStates = this->_factory.CreateTernaryProduct(this->_lhs_aut.aut->GetFinalStates(),
        this->_mhs_aut.aut->GetFinalStates(), this->_rhs_aut.aut->GetFinalStates(), this->_productType);
}

void NaryOpAutomaton::_InitializeFinalStates() {
    // Fixme: Add lazy initialization + no workshops
    this->_finalStates = this->_factory.CreateBaseNaryProduct(this->_auts, this->_arity, StatesSetType::FINAL, this->_productType);
}

void ComplementAutomaton::_InitializeFinalStates() {
    this->_finalStates = this->_aut.aut->GetFinalStates();
    //Fixme: Should this assert hold? assert(this->_finalStates->type != TermType::EMPTY);
    if(this->_finalStates->type == TermType::EMPTY) {
        if(this->_finalStates->InComplement()) {
            this->_finalStates = this->_factory.CreateEmpty();
        } else {
            this->_finalStates = this->_factory.CreateComplementedEmpty();
        }
    } else {
        this->_finalStates->Complement();
    }
}

void ProjectionAutomaton::_InitializeFinalStates() {
#   if (DEBUG_NO_WORKSHOPS == true)
    this->_finalStates = new TermList(this->_aut.aut->GetFinalStates(), false);
#   else
    this->_finalStates = this->_factory.CreateList(this->_aut.aut->GetFinalStates(), false);
#   endif
}

void BaseProjectionAutomaton::_InitializeFinalStates() {
    this->_finalStates = this->_aut.aut->GetFinalStates();
}

void BaseAutomaton::_InitializeFinalStates() {
    // NOTE: The automaton is constructed backwards, so initial states are finals
    assert(this->_finalStates == nullptr);

    // Obtain the MTBDD for Initial states
    BaseAutomatonStateSet finalStates;
    this->_autWrapper.GetFinalStates(finalStates);

    // Push states to new Base Set
    // #TERM_CREATION
    this->_finalStates = reinterpret_cast<Term*>(this->_factory.CreateBaseSet(std::move(finalStates)));
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

Term* TernaryOpAutomaton::Pre(Symbol* symbol, Term* finalApproximation, bool underComplement) {
    assert(false && "Doing Pre on TernaryOp Automaton!");
}

Term* NaryOpAutomaton::Pre(Symbol* symbol, Term* finalApproximation, bool underComplement) {
    assert(false && "Doing Pre on NaryOpAutomaton!");
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
#   if (OPT_USE_SET_PRE == true)
    Term_ptr accumulatedState = nullptr;
    auto key = std::make_pair(baseSet->states, symbol);
    if(!this->_setCache.retrieveFromCache(key, accumulatedState)) {
        accumulatedState = this->_factory.CreateBaseSet(this->_autWrapper.Pre(baseSet->states, symbol->GetTrackMask()));
        this->_setCache.StoreIn(key, accumulatedState);
    }
    return accumulatedState;
#   else
    Term_ptr preState = nullptr;
    Term_ptr accumulatedState = nullptr;

    #if (DEBUG_PRE == true)
    std::cout << "[!] ";
    this->_form->dump();
    std::cout << "\nComputing: ";
    finalApproximation->dump();
    std::cout << " \u2212\u222A ";
    std::cout << (*symbol) << "\n";
    #endif

    for(auto state : baseSet->states) {
        // Get MTBDD for Pre of states @p state
        auto key = std::make_pair(state, symbol);
        if(!this->_preCache.retrieveFromCache(key, preState)) {
            // TODO: there is probably useless copying --^
            preState = this->_factory.CreateBaseSet(this->_autWrapper.Pre(state, symbol->GetTrackMask()));
            this->_preCache.StoreIn(key, preState);
        }
        accumulatedState = static_cast<TermBaseSet*>(this->_factory.CreateUnionBaseSet(accumulatedState, preState));
        #if (DEBUG_PRE == true)
        std::cout << "{" << state << "} \u2212 " << (*symbol) << " = "; preState->dump(); std::cout << "\n";
        #endif
    }

    #if (DEBUG_PRE == true)
        std::cout << "= "; accumulatedState->dump(); std::cout << "\n";
    #endif

    return accumulatedState;
#   endif
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
    assert(finalApproximation->type == TermType::PRODUCT);

    // Retype the approximation to TermProduct type
    TermProduct* productStateApproximation = reinterpret_cast<TermProduct*>(finalApproximation);

    // Checks if left automaton's initial states intersects the final states
    ResultType lhs_result = this->_lhs_aut.aut->IntersectNonEmpty(this->_lhs_aut.ReMapSymbol(symbol), productStateApproximation->left, underComplement); // TODO: another memory consumption

    // We can prune the state if left side was evaluated as Empty term
    // TODO: This is different for Unionmat!
#   if (OPT_PRUNE_EMPTY == true)
    if(lhs_result.first->type == TermType::EMPTY && !lhs_result.first->InComplement() && this->_productType == ProductType::INTERSECTION) {
        return std::make_pair(lhs_result.first, underComplement);
    }
#   endif

#   if (OPT_EARLY_EVALUATION == true && MONA_FAIR_MODE == false)
    // Sometimes we can evaluate the experession early and return the continuation.
    // For intersection of automata we can return early, if left term was evaluated
    // as false, whereas for union of automata we can return early if left term
    // was true.
    bool canGenerateContinuations = this->_productType == ProductType::INTERSECTION|| this->_productType == ProductType::UNION;
#   if (OPT_CONT_ONLY_WHILE_UNSAT == true)
    canGenerateContinuations = canGenerateContinuations &&
        (this->_productType == ProductType::INTERSECTION ?
            (this->_trueCounter == 0 && this->_falseCounter >= 0) : (this->_falseCounter == 0 && this->_trueCounter >= 0) );
#   endif
#   if (OPT_CONT_ONLY_FOR_NONRESTRICTED == true)
    canGenerateContinuations = canGenerateContinuations && !this->_lhs_aut.aut->IsRestriction();
#   endif
    if(canGenerateContinuations && this->_eval_early(lhs_result.second, underComplement)) {
        // Construct the pointer for symbol (either symbol or epsilon---nullptr)
#       if (MEASURE_CONTINUATION_CREATION == true || MEASURE_ALL == true)
        ++this->_contCreationCounter;
#       endif
#       if (DEBUG_NO_WORKSHOPS == true)
        TermContinuation *continuation = new TermContinuation(this->_rhs_aut.aut, productStateApproximation->right, symbol, underComplement);
        Term_ptr leftCombined = new TermProduct(lhs_result.first, continuation, this->_productType);
#       else
        Term *continuation;
        if(this->_rhs_aut.aut == nullptr) {
            continuation = this->_factory.CreateContinuation(&this->_rhs_aut, this, productStateApproximation->right, symbol, underComplement, true);
        } else {
            continuation = this->_factory.CreateContinuation(&this->_rhs_aut, nullptr, productStateApproximation->right,
                                                                   symbol, underComplement);
        }
        Term_ptr leftCombined = this->_factory.CreateProduct(lhs_result.first, continuation, this->_productType);
#       endif
        return std::make_pair(leftCombined, this->_early_val(underComplement));
    }
#   endif

    // Otherwise compute the right side and return full fixpoint
#   if (OPT_EARLY_EVALUATION == true)
    if(this->_rhs_aut.aut == nullptr) {
        SymLink* temp;
        std::tie(temp, productStateApproximation->right) = this->LazyInit(productStateApproximation->right);
    }
#   endif
    ResultType rhs_result = this->_rhs_aut.aut->IntersectNonEmpty(this->_rhs_aut.ReMapSymbol(symbol), productStateApproximation->right, underComplement);
    // We can prune the state if right side was evaluated as Empty term
    // TODO: This is different for Unionmat!
#   if (OPT_PRUNE_EMPTY == true)
    if(rhs_result.first->type == TermType::EMPTY && !rhs_result.first->InComplement() && this->_productType == ProductType::INTERSECTION) {
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

ResultType TernaryOpAutomaton::_IntersectNonEmptyCore(Symbol* symbol, Term* finalApproximation, bool underComplement) {
    assert(finalApproximation != nullptr);
    assert(finalApproximation->type == TermType::TERNARY_PRODUCT);

    // Retype the approximation to TermProduct type
    TermTernaryProduct* termTernaryProduct = static_cast<TermTernaryProduct*>(finalApproximation);

    // Checks if left automaton's initial states interesct the final states;
    ResultType lhs_result = this->_lhs_aut.aut->IntersectNonEmpty(this->_lhs_aut.ReMapSymbol(symbol), termTernaryProduct->left, underComplement);
#   if (OPT_PRUNE_EMPTY == true)
    if(lhs_result.first->type == TermType::EMPTY && !lhs_result.first->InComplement() && this->_productType == ProductType::INTERSECTION) {
        return std::make_pair(lhs_result.first, underComplement);
    }
#   endif
    // Fixme: Add early evaluation

    ResultType mhs_result = this->_mhs_aut.aut->IntersectNonEmpty(this->_mhs_aut.ReMapSymbol(symbol), termTernaryProduct->middle, underComplement);
#   if (OPT_PRUNE_EMPTY == true)
    if(mhs_result.first->type == TermType::EMPTY && !mhs_result.first->InComplement() && this->_productType == ProductType::INTERSECTION) {
        return std::make_pair(mhs_result.first, underComplement);
    }
#   endif

    ResultType rhs_result = this->_rhs_aut.aut->IntersectNonEmpty(this->_rhs_aut.ReMapSymbol(symbol), termTernaryProduct->right, underComplement);
#   if (OPT_PRUNE_EMPTY == true)
    if(rhs_result.first->type == TermType::EMPTY && !rhs_result.first->InComplement() && this->_productType == ProductType::INTERSECTION) {
        return std::make_pair(rhs_result.first, underComplement);
    }
#   endif

    Term_ptr combined = this->_factory.CreateTernaryProduct(lhs_result.first, mhs_result.first, rhs_result.first, this->_productType);
    return std::make_pair(combined, this->_eval_result(lhs_result.second, mhs_result.second, rhs_result.second, underComplement));
}

ResultType NaryOpAutomaton::_IntersectNonEmptyCore(Symbol* symbol, Term* finalApproximation, bool underComplement) {
    assert(finalApproximation != nullptr);
    assert(finalApproximation->type == TermType::NARY_PRODUCT);

    // Retype the approximation
    TermNaryProduct* termNaryProduct = static_cast<TermNaryProduct*>(finalApproximation);
    assert(this->_arity == termNaryProduct->arity);

    // Fixme: Leak
    ResultType result;
    bool bool_result = !this->_early_val(underComplement);
    Term_ptr* terms = new Term_ptr[this->_arity];
    for(auto i = 0; i < this->_arity; ++i) {
        result = this->_auts[i].aut->IntersectNonEmpty(this->_auts[i].ReMapSymbol(symbol), termNaryProduct->terms[i], underComplement);
#       if (OPT_PRUNE_EMPTY == true)
        if(result.first->type == TermType::EMPTY && !result.first->InComplement() && this->_productType == ProductType::INTERSECTION) {
            delete[] terms;
            return std::make_pair(result.first, underComplement);
        }
#       endif
        bool_result = this->_eval_result(bool_result, result.second, underComplement);
        terms[i] = result.first;
    };

    Term_ptr combined = this->_factory.CreateNaryProduct(terms, this->_arity, this->_productType);
    return std::make_pair(combined, bool_result);
}

ResultType ComplementAutomaton::_IntersectNonEmptyCore(Symbol* symbol, Term* finalApproximaton, bool underComplement) {
    // Compute the result of nested automaton with switched complement
    ResultType result = this->_aut.aut->IntersectNonEmpty(this->_aut.ReMapSymbol(symbol), finalApproximaton, !underComplement);
    // TODO: fix, because there may be falsely complemented things
    if(finalApproximaton->InComplement() != result.first->InComplement()) {
        if(result.first->type == TermType::EMPTY) {
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
    assert(finalApproximation->type == TermType::LIST || finalApproximation->type == TermType::FIXPOINT);

    if(symbol == nullptr) {
        // We are doing the initial step by evaluating the epsilon
        TermList* projectionApproximation = reinterpret_cast<TermList*>(finalApproximation);
        assert(projectionApproximation->list.size() == 1);

        // Evaluate the initial unfolding of epsilon
        ResultType result = this->_aut.aut->IntersectNonEmpty(this->_aut.ReMapSymbol(symbol), projectionApproximation->list[0], underComplement);
        if(result.first->type == TermType::EMPTY) {
            // Prune the empty starting term
            return result;
        }

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
        if(fixpoint->InComplement() != finalApproximation->InComplement()) {
            assert(fixpoint->type != TermType::EMPTY);
            fixpoint->Complement();
        }
        fixpoint = this->_factory.GetUniqueFixpoint(fixpoint);
        #endif
        return std::make_pair(fixpoint, fixpoint->GetResult());
    }
}

ResultType base_pre_core(SymbolicAutomaton* aut, Symbol* symbol, Term* approximation, bool underComplement) {
    TermBaseSet *preFinal = reinterpret_cast<TermBaseSet *>(aut->Pre(symbol, approximation, underComplement));
    TermBaseSet* initial = reinterpret_cast<TermBaseSet*>(aut->GetInitialStates());

    // Return the pre and true if it intersects the initial states
    if (preFinal->IsEmpty()) {
        return std::make_pair(Workshops::TermWorkshop::CreateEmpty(), underComplement);
    } else {
        return std::make_pair(preFinal, initial->Intersects(preFinal) != underComplement);
    }
}

ResultType BaseAutomaton::_IntersectNonEmptyCore(Symbol* symbol, Term* approximation, bool underComplement) {
    // Reinterpret the initial and final states
    TermBaseSet* initial = reinterpret_cast<TermBaseSet*>(this->_initialStates);
    TermBaseSet* final = reinterpret_cast<TermBaseSet*>(this->_finalStates);

    if(symbol == nullptr) {
        // Testing if epsilon is in language, i.e. testing if final states intersect initial ones
        return std::make_pair(this->_finalStates, initial->Intersects(final) != underComplement);
    } else if(approximation->type == TermType::EMPTY) {
        // Empty set has no Pre
        return std::make_pair(approximation, underComplement);
    } else {
        // First do the pre of the approximation
        return base_pre_core(this, symbol, approximation, underComplement);
    }
}

ResultType BaseProjectionAutomaton::_IntersectNonEmptyCore(Symbol* symbol, Term* approximation, bool underComplement) {
    assert(this->_aut.aut->type == AutType::BASE);

    if(symbol == nullptr) {
        // We will pump the final states
        Symbol* zero_symbol = this->symbolFactory.CreateZeroSymbol();
        zero_symbol = this->symbolFactory.CreateTrimmedSymbol(zero_symbol, &this->_nonOccuringVars);
        zero_symbol = this->_aut.ReMapSymbol(zero_symbol);
        Term* pumped_states = approximation;
        Term* previous_iteration = approximation;
#       if (DEBUG_BASE_FIXPOINT_PUMPING == true)
        std::cout << "In: "; this->_form->dump(); std::cout << "Pumping "; pumped_states->dump(); std::cout << " with " << (*zero_symbol) << "\n";
#       endif
        do {
            previous_iteration = pumped_states;
            pumped_states = this->_aut.aut->_factory.CreateUnionBaseSet(pumped_states, this->_aut.aut->Pre(zero_symbol, pumped_states, underComplement));
#           if (DEBUG_BASE_FIXPOINT_PUMPING == true)
            std::cout << "Pumped: "; pumped_states->dump(); std::cout << "\n";
#           endif
        } while(pumped_states != previous_iteration);

#       if (DEBUG_BASE_FIXPOINT_PUMPING == true)
        std::cout << "Resulting states: "; pumped_states->dump(); std::cout << "\n";
#       endif
        TermBaseSet* initial = static_cast<TermBaseSet*>(this->_aut.aut->GetInitialStates());
        return std::make_pair(pumped_states, initial->Intersects(static_cast<TermBaseSet*>(pumped_states)) != underComplement);
    } else if(approximation->type == TermType::EMPTY) {
        return std::make_pair(approximation, underComplement);
    } else {
        // Do the pre of the approximation
        ResultType temp = base_pre_core(this->_aut.aut, this->_aut.ReMapSymbol(symbol), approximation, underComplement);
        return temp;
    }
}

void SymbolicAutomaton::DumpExample(std::ostream& out, ExampleType e, InterpretationType& interpretation) {
    switch(e) {
        case ExampleType::SATISFYING:
            if(this->_satExample != nullptr) {
                this->_DumpExampleCore(out, e, interpretation);
            }
            break;
        case ExampleType::UNSATISFYING:
            if(this->_unsatExample != nullptr) {
                this->_DumpExampleCore(out, e, interpretation);
            }
            break;
        default:
            assert(false && "Something impossible has happened");
    }
}

void BinaryOpAutomaton::_DumpExampleCore(std::ostream& out, ExampleType e, InterpretationType& interpretation) {
    assert(false && "BinaryOpAutomata cannot have examples yet!");
}

void TernaryOpAutomaton::_DumpExampleCore(std::ostream& out, ExampleType e, InterpretationType& interpretation) {
    assert(false && "TernaryOpAutomata cannot have examples yet!");
}

void NaryOpAutomaton::_DumpExampleCore(std::ostream& out, ExampleType e, InterpretationType& interpretation) {
    assert(false && "NaryOpAutomata cannot have examples yet!");
}

void ComplementAutomaton::_DumpExampleCore(std::ostream& out, ExampleType e, InterpretationType& interpretation) {
    assert(false && "ComplementAutomata cannot have examples yet!");
}

std::string interpretModel(std::string& str, MonaTypeTag order) {
    size_t idx = 0;
    if(order == MonaTypeTag::Varname1) {
        // Interpret the first one
        while(idx < str.length() && str[idx] != '1') {++idx;}
        if(idx == str.length()) {
            return std::string("");
        }
        return std::string(std::to_string(idx));
    } else if (order == MonaTypeTag::Varname2) {
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
    } else {
        assert(order == MonaTypeTag::Varname0);
        while(idx != str.size()) {
            if(str[idx++] == '1') {
                return std::string("false");
            }
        }
        return std::string("true");
    }
}

int max_varname_lenght(IdentList* idents, int varNo) {
    int max = 0, varlen;
    for(size_t i = 0; i < varNo; ++i) {
        varlen = strlen(symbolTable.lookupSymbol(idents->get(i)));
        if(varlen > max) {
            max = varlen;
        }
    }
    return max;
}

void ProjectionAutomaton::_DumpExampleCore(std::ostream& out, ExampleType e, InterpretationType& interpretations) {
    Term* example = (e == ExampleType::SATISFYING ? this->_satExample : this->_unsatExample);

    // Print the bounded part
    size_t varNo = this->projectedVars->size();
    std::string* examples = new std::string[varNo];
    this->projectedVars->sort();
    int max_len = max_varname_lenght(this->projectedVars, varNo);

    std::vector<Term_ptr> processed;
    while(example != nullptr && example->link->succ != nullptr && example != example->link->succ) {
    //                                                           ^--- not sure this is right
        if(std::find_if(processed.begin(), processed.end(), [&example](Term_ptr i) { return example == i; }) != processed.end())
            break;
        processed.push_back(example);
        for(size_t i = 0; i < varNo; ++i) {
            examples[i] += example->link->symbol->GetSymbolAt(varMap[this->projectedVars->get(i)]);
        }
        example = example->link->succ;
    }

    for(size_t i = 0; i < varNo; ++i) {
        auto varname = (symbolTable.lookupSymbol(this->projectedVars->get(i)));
        out << varname << std::string(max_len - strlen(varname) + 1, ' ') << ": " << examples[i] << "\n";
    }
    out << "\n";
    for(size_t i = 0; i < varNo; ++i) {
        std::string interpretation = (symbolTable.lookupSymbol(this->projectedVars->get(i)));
        interpretation += " = " + interpretModel(examples[i], symbolTable.lookupType(this->projectedVars->get(i)));
        interpretations.push_back(interpretation);
        out << interpretation << "\n";
    }

    delete[] examples;
}

void BinaryOpAutomaton::DumpAutomaton() {
    const char* product_colour = ProductTypeToColour(static_cast<int>(this->_productType));
    const char* product_symbol = ProductTypeToAutomatonSymbol(this->_productType);
    #if (DEBUG_AUTOMATA_ADDRESSES == true)
        std::cout << "[" << this << "]";
    #endif
    if(this->_isRestriction) {
        std::cout << "\033[1;35m[\033[0m";
    }
    std::cout << "\033[" << product_colour << "(\033[0m";
    this->_lhs_aut.aut->DumpAutomaton();
    std::cout << "\033[" << product_colour << " " << product_symbol << "\u00B2 \033[0m";
    if(this->_rhs_aut.aut != nullptr) {
        this->_rhs_aut.aut->DumpAutomaton();
    } else {
        std::cout << "??";
    }
    std::cout << "\033[" << product_colour << ")\033[0m";
    if(this->_isRestriction) {
        std::cout << "\033[1;35m]\033[0m";
    }
}

void TernaryOpAutomaton::DumpAutomaton() {
    const char* product_colour = ProductTypeToColour(static_cast<int>(this->_productType));
    const char* product_symbol = ProductTypeToAutomatonSymbol(this->_productType);
#   if (DEBUG_AUTOMATA_ADDRESSES == true)
    std::cout << "[" << this << "]";
#   endif
    if(this->_isRestriction) {
        std::cout << "\033[1;35m[\033[0m";
    }
    std::cout << "\033[" << product_colour << "(\033[0m";
    this->_lhs_aut.aut->DumpAutomaton();
    std::cout << "\033[" << product_colour << " " << product_symbol << "\u00B3 \033[0m";
    if(this->_mhs_aut.aut != nullptr) {
        this->_mhs_aut.aut->DumpAutomaton();
    } else {
        std::cout << "??";
    }
    std::cout << "\033[" << product_colour << " " << product_symbol << "\u00B3 \033[0m";
    if(this->_rhs_aut.aut != nullptr) {
        this->_rhs_aut.aut->DumpAutomaton();
    } else {
        std::cout << "??";
    }
    std::cout << "\033[" << product_colour << ")\033[0m";
    if(this->_isRestriction) {
        std::cout << "\033[1;35m]\033[0m";
    }
}

void NaryOpAutomaton::DumpAutomaton() {
    const char* product_colour = ProductTypeToColour(static_cast<int>(this->_productType));
    const char* product_symbol = ProductTypeToAutomatonSymbol(this->_productType);
#   if (DEBUG_AUTOMATA_ADDRESSES == true)
    std::cout << "[" << this << "]";
#   endif
    if(this->_isRestriction) {
        std::cout << "\033[1;35m[\033[0m";
    }
    std::cout << "\033[" << product_colour << "(\033[0m";
    for (int i = 0; i < this->_arity; ++i) {
        if(i != 0) {
            std::cout << "\033[" << product_colour << " " << product_symbol << "\u207F \033[0m";
        }
        if(this->_auts[i].aut != nullptr) {
            this->_auts[i].aut->DumpAutomaton();
        } else {
            std::cout << "??";
        }
    }

    std::cout << "\033[" << product_colour << ")\033[0m";
    if(this->_isRestriction) {
        std::cout << "\033[1;35m]\033[0m";
    }
}

void ComplementAutomaton::DumpAutomaton() {
    #if (DEBUG_AUTOMATA_ADDRESSES == true)
        std::cout << "[" << this << "]";
    #endif
    if(this->_isRestriction) {
        std::cout << "\033[1;35m[\033[0m";
    }
    std::cout << "\033[1;31m\u2201(\033[0m";
    this->_aut.aut->DumpAutomaton();
    std::cout << "\033[1;31m)\033[0m";
    if(this->_isRestriction) {
        std::cout << "\033[1;35m]\033[0m";
    }
}

void ProjectionAutomaton::DumpAutomaton() {
    #if (DEBUG_AUTOMATA_ADDRESSES == true)
        std::cout << "[" << this << "]";
    #endif
    if(this->_isRestriction) {
        std::cout << "\033[1;35m[\033[0m";
    }
    std::cout << "\033[1;34m";
    std::cout << "\u2203";
    for(auto it = this->projectedVars->begin(); it != this->projectedVars->end(); ++it) {
        std::cout << (*it) << ":" << symbolTable.lookupSymbol(*it);
        if((it + 1) != this->projectedVars->end()) {
            std::cout << ", ";
        }
    }
    std::cout << (*this->_guide);
    std::cout << "(\033[0m";
    this->_aut.aut->DumpAutomaton();
    std::cout << "\033[1;34m)\033[0m";
    if(this->_isRestriction) {
        std::cout << "\033[1;35m]\033[0m";
    }
}

void GenericBaseAutomaton::DumpAutomaton() {
    #if (DEBUG_AUTOMATA_ADDRESSES == true)
        std::cout << "[" << this << "]";
    #endif
    if(this->_isRestriction) {
        std::cout << "\033[1;35m[\033[0m";
    }
    std::cout << "Automaton";
    _form->dump();
    #if (DEBUG_BASE_AUTOMATA == true || DEBUG_RESTRICTION_AUTOMATA == true)
    if(this->_isRestriction || DEBUG_BASE_AUTOMATA)
        this->BaseAutDump();
    #endif
    if(this->_isRestriction) {
        std::cout << "\033[1;35m]\033[0m";
    }
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

void print_gaston_optimization_to_dot(std::ofstream& os, std::string opt, bool ison) {
    os << "\t\t\t<tr>";
    os << "<td>" << opt << "</td>";
    os << "<td" << (ison ? ">ON" : " bgcolor=\"grey\"><font color=\"white\">OFF</font>") << "</td>";
    os << "</tr>\n";
}

void SymbolicAutomaton::GastonInfoToDot(std::ofstream& os) {
    os << "\tsubgraph clusterGastonInfo {\n";
    os << "\t\tlabel=\"Gaston Parameters\\n[" << inputFileName << "]\";\n";
    os << "\t\trankdir=\"LR\";\n";
    os << "\t\tranksep=0.1;\n";

    // Output info about optimizations
    os << "\t\topts [shape=plaintext label=<<table border=\"0\" cellborder=\"1\" cellspacing=\"1\" cellpadding=\"5\">\n";
    print_gaston_optimization_to_dot(os, "DAG", OPT_USE_DAG);
    print_gaston_optimization_to_dot(os, "AntiPrenexing", OPT_ANTIPRENEXING);
    print_gaston_optimization_to_dot(os, "SubformulaeConversion", OPT_CREATE_QF_AUTOMATON);
    print_gaston_optimization_to_dot(os, "TernaryProducts", OPT_USE_TERNARY_AUTOMATA);
    print_gaston_optimization_to_dot(os, "NaryProducts", OPT_USE_NARY_AUTOMATA);
    print_gaston_optimization_to_dot(os, "Continuations", OPT_EARLY_EVALUATION);
    os << "\t\t</table>>];\n";

    // Output info about filter phases
    os << "\t\tfilters [shape=plaintext label=<<table border=\"0\" cellborder=\"1\" cellspacing=\"1\" cellpadding=\"5\">\n";
#   define PRINT_FILTER_INFO(filter) \
        os << "\t\t\t<tr>"; \
        os << "<td>" << #filter << "</td>"; \
        os << "</tr>\n";
    FILTER_LIST(PRINT_FILTER_INFO)
    if(OPT_SHUFFLE_FORMULA) {
        PRINT_FILTER_INFO(ShuffleVisitor)
    }
#   undef PRINT_FILTER_INFO
    os << "\t\t</table>>];\n";

    os << "\t}\n";
}

void SymbolicAutomaton::AutomatonToDot(std::string filename, SymbolicAutomaton *aut, bool inComplement) {
    std::ofstream os;
    os.open(filename);
    // TODO: Add exception handling
    os << "strict graph aut {\n";
    aut->DumpToDot(os, inComplement);
    SymbolicAutomaton::GastonInfoToDot(os);
    os << "}\n";
    os.close();
}

void SymbolicAutomaton::DumpProductHeader(std::ofstream & os, bool inComplement, ProductType productType) {
    os << "\t" << (uintptr_t) &*this << "[label=\"";
    os << this->_factory.ToSimpleStats() << "\\n";
    os << this->_form->fixpoints_from_root << ", ";
    os << this->_form->fixpoint_number << ": ";
    os << "\u03B5 " << (inComplement ? "\u2209 " : "\u2208 ");
    os << ProductTypeToAutomatonSymbol(productType);
    os << "\\n(" << this->_trueCounter << "\u22A8, " << this->_falseCounter << "\u22AD)\"";
#   if (PRINT_DOT_BLACK_AND_WHITE == false)
    if(this->_trueCounter == 0 && this->_falseCounter != 0) {
        os << ",style=filled, fillcolor=red";
    }
#   endif
    os << "];\n";
}

void BinaryOpAutomaton::DumpToDot(std::ofstream & os, bool inComplement) {
    this->DumpProductHeader(os, inComplement, this->_productType);
    os << "\t" << (uintptr_t) &*this << " -- " << (uintptr_t) (this->_lhs_aut.aut) << " [label=\"lhs\"];\n";
    os << "\t" << (uintptr_t) &*this << " -- " << (uintptr_t) (this->_rhs_aut.aut) << " [label=\"rhs\"];\n";
    this->_lhs_aut.aut->DumpToDot(os, inComplement);
    if(this->_rhs_aut.aut != nullptr) {
        this->_rhs_aut.aut->DumpToDot(os, inComplement);
    }
}

void TernaryOpAutomaton::DumpToDot(std::ofstream &os, bool inComplement) {
    this->DumpProductHeader(os, inComplement, this->_productType);
    os << "\t" << (uintptr_t) &*this << " -- " << (uintptr_t) (this->_lhs_aut.aut) << " [label=\"lhs\"];\n";
    os << "\t" << (uintptr_t) &*this << " -- " << (uintptr_t) (this->_mhs_aut.aut) << " [label=\"mhs\"];\n";
    os << "\t" << (uintptr_t) &*this << " -- " << (uintptr_t) (this->_rhs_aut.aut) << " [label=\"rhs\"];\n";
    this->_lhs_aut.aut->DumpToDot(os, inComplement);
    if(this->_mhs_aut.aut != nullptr) {
        this->_mhs_aut.aut->DumpToDot(os, inComplement);
    }
    if(this->_rhs_aut.aut != nullptr) {
        this->_rhs_aut.aut->DumpToDot(os, inComplement);
    }
}

void NaryOpAutomaton::DumpToDot(std::ofstream &os, bool inComplement) {
    this->DumpProductHeader(os, inComplement, this->_productType);
    for (int i = 0; i < this->_arity; ++i) {
        os << "\t" << (uintptr_t) &*this << " -- " << (uintptr_t) (this->_auts[i].aut) << " [label=\"" << (i) << "hs\"];\n";
    }
    for (int i = 0; i < this->_arity; ++i) {
        if(this->_auts[i].aut != nullptr) {
            this->_auts[i].aut->DumpToDot(os, inComplement);
        }
    }
}

void ComplementAutomaton::DumpToDot(std::ofstream & os, bool inComplement) {
    os << "\t" << (uintptr_t) &*this << "[label=\"";
    os << this->_factory.ToSimpleStats() << "\\n";
    os << this->_form->fixpoint_number << ": ";
    os << "\u03B5 " << (inComplement ? "\u2209 " : "\u2208 ");
    os << "\u00AC\\n(" << this->_trueCounter << "\u22A8, " << this->_falseCounter << "\u22AD)\"";
#   if (PRINT_DOT_BLACK_AND_WHITE == false)
    if(this->_trueCounter == 0 && this->_falseCounter != 0) {
        os << ",style=filled, fillcolor=red";
    }
#   endif
    os << "];\n";
    os << "\t" << (uintptr_t) &*this << " -- " << (uintptr_t) (this->_aut.aut) << ";\n";
    this->_aut.aut->DumpToDot(os, !inComplement);
}

void ProjectionAutomaton::DumpToDot(std::ofstream & os, bool inComplement) {
    os << "\t" << (uintptr_t) &*this << "[label=\"";
    os << this->_factory.ToSimpleStats() << "\\n";
    os << this->_form->fixpoint_number << ", " << this->stats.max_symbol_path_len << ": ";
    os << "\u03B5 " << (inComplement ? "\u2209 " : "\u2208 ");
    os << "\u2203";
    for(auto id = this->projectedVars->begin(); id != this->projectedVars->end(); ++id) {
        os << symbolTable.lookupSymbol(*id) << ",";
    }
    os << "\\n(" << this->_trueCounter << "\u22A8, " << this->_falseCounter << "\u22AD)\"";
#   if (PRINT_DOT_BLACK_AND_WHITE == false)
    if(this->_trueCounter == 0 && this->_falseCounter != 0) {
        os << ",style=filled, fillcolor=red";
    }
#   endif
    os << "];\n";
    os << "\t" << (uintptr_t) &*this << " -- " << (uintptr_t) (this->_aut.aut) << ";\n";
    this->_aut.aut->DumpToDot(os, inComplement);
}

std::string utf8_substr(std::string originalString, int maxLength)
{
    std::string resultString = originalString;

    if(maxLength == -1) {
        return originalString;
    }

    int len = 0;
    int byteCount = 0;

    const char* aStr = originalString.c_str();

    while(*aStr)
    {
        if( (*aStr & 0xc0) != 0x80 )
            len += 1;

        if(len>maxLength)
        {
            resultString = resultString.substr(0, byteCount);
            resultString += "...";
            break;
        }
        byteCount++;
        aStr++;
    }

    return resultString;
}

void BaseAutomaton::DumpToDot(std::ofstream & os, bool inComplement) {
    os << "\t" << (uintptr_t) &*this << "[label=\"";
    os << this->_factory.ToSimpleStats() << "\\n";
    os << this->_form->fixpoint_number << ": ";
    os << "\u03B5 " << (inComplement ? "\u2209 " : "\u2208 ") << utf8_substr(this->_form->ToString(), PRINT_DOT_LIMIT);
    os << "\\n(" << this->_trueCounter << "\u22A8, " << this->_falseCounter << "\u22AD)\"";
#   if (PRINT_DOT_BLACK_AND_WHITE == false)
    if(this->_trueCounter == 0 && this->_falseCounter != 0) {
        os << ",style=filled, fillcolor=red";
    }
#   endif
    if(this->_isRestriction) {
        os << ", shape=rect";
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
    this->_autWrapper.DumpDFA(std::cout);
    std::cout << "[----------------------->]\n";
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

void print_stat(std::string statName, double stat, std::string unit) {
    std::cout << "  \u2218 " << statName << " -> " << std::fixed << std::setprecision(2) << stat << unit << "\n";
}

void BinaryOpAutomaton::DumpComputationStats() {
    if(this->marked) {
        return;
    }
    this->marked = true;
#   if (PRINT_STATS_PRODUCT == true)
    this->_form->dump();
    std::cout << "\n";
#   if (MEASURE_SUBAUTOMATA_TIMING == true)
    std::cout << "  \u2218 Workload time: "; this->timer.PrintElapsed();
#   endif
    print_stat("Refs", this->_refs);
    std::cout << "  \u2218 Cache stats -> ";
#       if (MEASURE_CACHE_HITS == true)
        this->_resCache.dumpStats();
#       endif
#       if (DEBUG_WORKSHOPS == true)
        this->_factory.Dump();
#       endif
#       if (DEBUG_SYMBOL_CREATION == true)
        this->symbolFactory.Dump();
#       endif
        print_stat("True Hits", this->_trueCounter);
        print_stat("False Hits", this->_falseCounter);
        print_stat("Continuation Generation", this->_contCreationCounter);
        print_stat("Continuation Evaluation", this->_contUnfoldingCounter);
    std::cout << "\n";
#   endif
    this->_lhs_aut.aut->DumpComputationStats();
    if(this->_rhs_aut.aut != nullptr) {
        this->_rhs_aut.aut->DumpComputationStats();
    }
}

void TernaryOpAutomaton::DumpComputationStats() {
    if(this->marked) {
        return;
    }
    this->marked = true;
#   if (PRINT_STATS_TERNARY_PRODUCT == true)
    this->_form->dump();
    std::cout << "\n";
#   if (MEASURE_SUBAUTOMATA_TIMING == true)
    std::cout << "  \u2218 Workload time: "; this->timer.PrintElapsed();
#   endif
    print_stat("Refs", this->_refs);
    std::cout << "  \u2218 Cache stats -> ";
#       if (MEASURE_CACHE_HITS == true)
    this->_resCache.dumpStats();
#       endif
#       if (DEBUG_WORKSHOPS == true)
    this->_factory.Dump();
#       endif
#       if (DEBUG_SYMBOL_CREATION == true)
    this->symbolFactory.Dump();
#       endif
    print_stat("True Hits", this->_trueCounter);
    print_stat("False Hits", this->_falseCounter);
    print_stat("Continuation Generation", this->_contCreationCounter);
    print_stat("Continuation Evaluation", this->_contUnfoldingCounter);
    std::cout << "\n";
#   endif
    this->_lhs_aut.aut->DumpComputationStats();
    if(this->_mhs_aut.aut != nullptr) {
        this->_mhs_aut.aut->DumpComputationStats();
    }
    if(this->_rhs_aut.aut != nullptr) {
        this->_rhs_aut.aut->DumpComputationStats();
    }
}

void NaryOpAutomaton::DumpComputationStats() {
    if(this->marked) {
        return;
    }
    this->marked = true;
#   if (PRINT_STATS_NARY == true)
    this->_form->dump();
    std::cout << "\n";
#   if (MEASURE_SUBAUTOMATA_TIMING == true)
    std::cout << "  \u2218 Workload time: "; this->timer.PrintElapsed();
#   endif
    print_stat("Refs", this->_refs);
    std::cout << "  \u2218 Cache stats -> ";
#       if (MEASURE_CACHE_HITS == true)
    this->_resCache.dumpStats();
#       endif
#       if (DEBUG_WORKSHOPS == true)
    this->_factory.Dump();
#       endif
#       if (DEBUG_SYMBOL_CREATION == true)
    this->symbolFactory.Dump();
#       endif
    print_stat("True Hits", this->_trueCounter);
    print_stat("False Hits", this->_falseCounter);
    print_stat("Continuation Generation", this->_contCreationCounter);
    print_stat("Continuation Evaluation", this->_contUnfoldingCounter);
    std::cout << "\n";
#   endif
    for (int i = 0; i < this->_arity; ++i) {
        if(this->_auts[i].aut != nullptr) {
            this->_auts[i].aut->DumpComputationStats();
        }
    }
}

void ProjectionAutomaton::DumpComputationStats() {
    if(this->marked) {
        return;
    }
    this->marked = true;
#   if (PRINT_STATS_PROJECTION == true)
#       if (PRINT_STATS_QF_PROJECTION == true)
        if (this->_form->fixpoint_number == 1){
#       endif
                this->_form->dump();
                std::cout << "\n";
#               if (MEASURE_SUBAUTOMATA_TIMING == true)
                std::cout << "  \u2218 Workload time: "; this->timer.PrintElapsed();
#               endif
                print_stat("Refs", this->_refs);
                std::cout << "  \u2218 Cache stats -> ";
#           if (MEASURE_CACHE_HITS == true)
                this->_resCache.dumpStats();
#           endif
#           if (DEBUG_WORKSHOPS)
                this->_factory.Dump();
#           endif
#           if (DEBUG_SYMBOL_CREATION == true)
                this->symbolFactory.Dump();
#           endif
#           if (MEASURE_PROJECTION == true)
                print_stat("Fixpoint Nexts", this->fixpointNext);
                print_stat("Fixpoint Results", this->fixpointRes);
                print_stat("FixpointPre Nexts", this->fixpointPreNext);
                print_stat("FixpointPre Results", this->fixpointPreRes);
#           endif
                print_stat("True Hits", this->_trueCounter);
                print_stat("False Hits", this->_falseCounter);
                print_stat("Continuation Evaluation", this->_contUnfoldingCounter);
                std::cout << "\n";
#       if (PRINT_STATS_QF_PROJECTION == true)
        }
#       endif
#   endif
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
#   if (MEASURE_SUBAUTOMATA_TIMING == true)
    std::cout << "  \u2218 Workload time: "; this->timer.PrintElapsed();
#   endif
    print_stat("Refs", this->_refs);
    std::cout << "  \u2218 Cache stats -> ";
#       if (MEASURE_CACHE_HITS == true)
        this->_resCache.dumpStats();
#       endif
#       if (DEBUG_WORKSHOPS)
        this->_factory.Dump();
#       endif
#       if (DEBUG_SYMBOL_CREATION == true)
        this->symbolFactory.Dump();
#       endif
        print_stat("True Hits", this->_trueCounter);
        print_stat("False Hits", this->_falseCounter);
        print_stat("Continuation Evaluation", this->_contUnfoldingCounter);
    std::cout << "\n";
#   endif
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
#   if (MEASURE_SUBAUTOMATA_TIMING == true)
    std::cout << "  \u2218 Workload time: "; this->timer.PrintElapsed();
#   endif
    print_stat("Refs", this->_refs);
    std::cout << "  \u2218 Cache stats -> ";
#       if (MEASURE_CACHE_HITS == true)
        this->_resCache.dumpStats();
#       endif
#       if (DEBUG_WORKSHOPS)
        this->_factory.Dump();
#       endif
#       if (DEBUG_SYMBOL_CREATION == true)
        this->symbolFactory.Dump();
#       endif
        print_stat("True Hits", this->_trueCounter);
        print_stat("False Hits", this->_falseCounter);
        print_stat("Continuation Evaluation", this->_contUnfoldingCounter);
    std::cout << "\n";
#   endif
}

void SymbolicAutomaton::DumpAutomatonMetrics() {
    this->FillStats();

    std::cout << "\u2218 Overall SymbolicAutomaton Metrics:\n";
    print_stat("Vars", varMap.TrackLength());
    print_stat("Nodes", this->stats.nodes);
    print_stat("Real nodes", this->stats.real_nodes);
    print_stat("DAG gain", ((double)this->stats.nodes / this->stats.real_nodes));
    print_stat("Fixpoint Computations", this->stats.fixpoint_computations);
    print_stat("Maximal Fixpoint Nesting", this->stats.max_fixpoint_nesting);
    print_stat("Maximal Fixpoint Width", this->stats.max_fixpoint_width);
    print_stat("Average Fixpoint Width", this->stats.avg_fixpoint_width);
    print_stat("Automaton Height", this->stats.height);
    print_stat("Maximal References", this->stats.max_refs);
#   if (MEASURE_AUTOMATA_CYCLES == true)
    print_stat("Cycles on all final states", (MonaWrapper<size_t>::noAllFinalStatesHasCycles / (double) MonaWrapper<size_t>::_wrapperCount)*100, "%");
    print_stat("Cycles on some final states", (MonaWrapper<size_t>::noSomeFinalStatesHasCycles / (double) MonaWrapper<size_t>::_wrapperCount)*100, "%");
#   endif
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
    this->stats.max_fixpoint_width = std::max(this->projectedVars->size(), this->_aut.aut->stats.max_fixpoint_width);
    this->stats.avg_fixpoint_width = (this->projectedVars->size() + this->_aut.aut->stats.avg_fixpoint_width) / 2.0;
}

void BinaryOpAutomaton::FillStats() {
    bool count_left = !this->_lhs_aut.remap;
    volatile bool count_right = !this->_rhs_aut.remap && this->_rhs_aut.aut != nullptr;

    if(count_left) {
        this->_lhs_aut.aut->FillStats();
    }
    if(count_right) {
        this->_rhs_aut.aut->FillStats();
    }

    this->stats.fixpoint_computations = (count_left ? this->_lhs_aut.aut->stats.fixpoint_computations : 0) + (count_right ? this->_rhs_aut.aut->stats.fixpoint_computations : 0);
    this->stats.height = std::max(this->_lhs_aut.aut->stats.height, (this->_rhs_aut.aut != nullptr ? this->_rhs_aut.aut->stats.height : 0)) + 1;
    this->stats.nodes = (count_left ? this->_lhs_aut.aut->stats.nodes : 0) + (count_right ? this->_rhs_aut.aut->stats.nodes : 0) + 1;
    this->stats.real_nodes = 1 + this->_lhs_aut.aut->stats.real_nodes + (this->_rhs_aut.aut != nullptr ? this->_rhs_aut.aut->stats.real_nodes : 0);
    this->stats.max_refs = std::max({this->_refs, this->_lhs_aut.aut->stats.max_refs, (this->_rhs_aut.aut != nullptr ? this->_rhs_aut.aut->stats.max_refs : 0)});
    this->stats.max_fixpoint_nesting = std::max(this->_lhs_aut.aut->stats.max_fixpoint_nesting,
        (this->_rhs_aut.aut != nullptr ? this->_rhs_aut.aut->stats.max_fixpoint_nesting : 0));
    this->stats.max_fixpoint_width =  std::max(this->_lhs_aut.aut->stats.max_fixpoint_width,
        (this->_rhs_aut.aut != nullptr ? this->_rhs_aut.aut->stats.max_fixpoint_width : this->_lhs_aut.aut->stats.max_fixpoint_width));
    this->stats.avg_fixpoint_width = (this->_lhs_aut.aut->stats.avg_fixpoint_width +
        (this->_rhs_aut.aut != nullptr ? this->_rhs_aut.aut->stats.avg_fixpoint_width : this->_lhs_aut.aut->stats.avg_fixpoint_width)) / 2.0;

}

void TernaryOpAutomaton::FillStats() {
    bool count_left = !this->_lhs_aut.remap;
    volatile bool count_middle = !this->_mhs_aut.remap && this->_mhs_aut.aut != nullptr;
    volatile bool count_right = !this->_rhs_aut.remap && this->_rhs_aut.aut != nullptr;

    if(count_left) this->_lhs_aut.aut->FillStats();
    if(count_middle) this->_mhs_aut.aut->FillStats();
    if(count_right) this->_rhs_aut.aut->FillStats();

    this->stats.fixpoint_computations = (count_left ? this->_lhs_aut.aut->stats.fixpoint_computations : 0) +
            (count_middle ? this->_mhs_aut.aut->stats.fixpoint_computations : 0) +
            (count_right ? this->_rhs_aut.aut->stats.fixpoint_computations : 0);
    this->stats.height = 1 +std::max({this->_lhs_aut.aut->stats.height,
            (this->_mhs_aut.aut != nullptr ? this->_mhs_aut.aut->stats.height : 0),
            (this->_rhs_aut.aut != nullptr ? this->_rhs_aut.aut->stats.height : 0)});
    this->stats.nodes = 1 + (count_left ? this->_lhs_aut.aut->stats.nodes : 0) +
            (count_middle ? this->_mhs_aut.aut->stats.nodes : 0) + (count_right ? this->_rhs_aut.aut->stats.nodes : 0);
    this->stats.real_nodes = 1 + this->_lhs_aut.aut->stats.real_nodes +
            (this->_mhs_aut.aut != nullptr ? this->_mhs_aut.aut->stats.real_nodes : 0) +
            (this->_rhs_aut.aut != nullptr ? this->_rhs_aut.aut->stats.real_nodes : 0);
    this->stats.max_refs = std::max({this->_refs, this->_lhs_aut.aut->stats.max_refs,
            (this->_mhs_aut.aut != nullptr ? this->_mhs_aut.aut->stats.max_refs : 0),
            (this->_rhs_aut.aut != nullptr ? this->_rhs_aut.aut->stats.max_refs : 0)});
    this->stats.max_fixpoint_nesting = std::max({this->_lhs_aut.aut->stats.max_fixpoint_nesting,
                                     (this->_mhs_aut.aut != nullptr ? this->_mhs_aut.aut->stats.max_fixpoint_nesting : 0),
                                     (this->_rhs_aut.aut != nullptr ? this->_rhs_aut.aut->stats.max_fixpoint_nesting : 0)});
    this->stats.max_fixpoint_width = std::max({this->_lhs_aut.aut->stats.max_fixpoint_width,
                                                 (this->_mhs_aut.aut != nullptr ? this->_mhs_aut.aut->stats.max_fixpoint_width: 0),
                                                 (this->_rhs_aut.aut != nullptr ? this->_rhs_aut.aut->stats.max_fixpoint_width : 0)});
    this->stats.max_fixpoint_width = (this->_lhs_aut.aut->stats.max_fixpoint_width +
                                              (this->_mhs_aut.aut != nullptr ? this->_mhs_aut.aut->stats.max_fixpoint_width: 0) +
                                              (this->_rhs_aut.aut != nullptr ? this->_rhs_aut.aut->stats.max_fixpoint_width : 0)) / 2.0;
}

void NaryOpAutomaton::FillStats() {
    volatile bool count;
    for (int i = 0; i < this->_arity; ++i) {
        count = !this->_auts[i].remap && this->_auts[i].aut != nullptr;
        if(count) this->_auts[i].aut->FillStats();
    }

    this->stats.fixpoint_computations = 1;
    this->stats.height = 1;
    this->stats.nodes = 1;
    this->stats.real_nodes = 1;
    this->stats.max_refs = this->_refs;
    this->stats.max_fixpoint_nesting = 0;

    for (int j = 0; j < this->_arity; ++j) {
        count = !this->_auts[j].remap && this->_auts[j].aut != nullptr;
        if(count) {
            this->stats.fixpoint_computations += this->_auts[j].aut->stats.fixpoint_computations;
            this->stats.nodes += this->_auts[j].aut->stats.nodes;
        }
        if(this->_auts[j].aut != nullptr) {
            this->stats.real_nodes += this->_auts[j].aut->stats.real_nodes;
            if(this->stats.height < this->_auts[j].aut->stats.height) this->stats.height = this->_auts[j].aut->stats.height;
            if(this->stats.max_refs < this->_auts[j].aut->stats.height) this->stats.max_refs = this->_auts[j].aut->stats.max_refs;
            if(this->stats.max_fixpoint_nesting < this->_auts[j].aut->stats.max_fixpoint_nesting) this->stats.max_fixpoint_nesting = this->_auts[j].aut->stats.max_fixpoint_nesting;
            if(this->stats.max_fixpoint_width < this->_auts[j].aut->stats.max_fixpoint_width) this->stats.max_fixpoint_width = this->_auts[j].aut->stats.max_fixpoint_width;
            this->stats.avg_fixpoint_width = (this->stats.avg_fixpoint_width + this->_auts[j].aut->stats.avg_fixpoint_width) / 2.0;
        }
    }
}

void BaseAutomaton::FillStats() {
    this->stats.fixpoint_computations = 0;
    this->stats.height = 1;
    this->stats.nodes = 1;
    this->stats.real_nodes = 1;
    this->stats.max_refs = this->_refs;
    this->stats.max_fixpoint_nesting = 0;
    this->stats.max_fixpoint_width = 0;
    this->stats.avg_fixpoint_width = 0.0;
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
    this->stats.max_fixpoint_width = this->_aut.aut->stats.max_fixpoint_width;
    this->stats.avg_fixpoint_width = this->_aut.aut->stats.avg_fixpoint_width;
}

bool BinaryOpAutomaton::WasLastExampleValid() {
    if(this->_isRestriction && this->_lastResult == false) {
        return false;
    } else {
        return this->_lhs_aut.aut->WasLastExampleValid() && (this->_rhs_aut.aut != nullptr && this->_rhs_aut.aut->WasLastExampleValid());
    }
}

bool TernaryOpAutomaton::WasLastExampleValid() {
    if(this->_isRestriction && this->_lastResult == false) {
        return false;
    } else {
        return this->_lhs_aut.aut->WasLastExampleValid() && (this->_mhs_aut.aut != nullptr && this->_mhs_aut.aut->WasLastExampleValid())
                && (this->_rhs_aut.aut != nullptr && this->_rhs_aut.aut->WasLastExampleValid());
    }
}

bool NaryOpAutomaton::WasLastExampleValid() {
    if(this->_isRestriction && this->_lastResult == false) {
        return false;
    } else {
        for (int i = 0; i < this->_arity; ++i) {
            if(this->_auts[i].aut != nullptr && !this->_auts[i].aut->WasLastExampleValid()) {
                return false;
            }
        }
        return true;
    }
}

bool BaseAutomaton::WasLastExampleValid() {
    if(this->_isRestriction && this->_lastResult == false) {
        return false;
    } else {
        return true;
    }
}

bool ComplementAutomaton::WasLastExampleValid() {
    if(this->_isRestriction && this->_lastResult == false) {
        return false;
    } else {
        return this->_aut.aut->WasLastExampleValid();
    }
}

bool ProjectionAutomaton::WasLastExampleValid() {
    return true;
}

std::pair<SymLink*, Term_ptr> BinaryOpAutomaton::LazyInit(Term_ptr term) {
    if (this->_rhs_aut.aut == nullptr) {
        ASTForm_ff *form = static_cast<ASTForm_ff *>(this->_form);
        this->_rhs_aut.aut = form->f2->toSymbolicAutomaton(form->f2->under_complement);
        this->_rhs_aut.InitializeSymLink(form->f2);
        this->_rhs_aut.aut->IncReferences();
    }
    if(term == nullptr) {
        return std::make_pair(&this->_rhs_aut, this->_rhs_aut.aut->GetFinalStates());
    } else {
        return std::make_pair(&this->_rhs_aut, term);
    }
}