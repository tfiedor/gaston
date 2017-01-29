/*****************************************************************************
 *  dWiNA - Deciding WSkS using non-deterministic automata
 *
 *  Copyright (c) 2015  Tomas Fiedor <ifiedortom1@fit.vutbr.cz>
 *
 *  Description:
 *    Visitor for doing the anti-prenexing. This means instead of pushing
 *    the quantifiers higher to root, we push them deeper towards leaves.
 *    We do this if we see that some variable is not bound in the formula.
 *    Thus we can push the quantifier to the lhs or rhs.
 *
 *****************************************************************************/

#include "AntiPrenexer.h"
#include "BooleanUnfolder.h"
#include "../restricters/NegationUnfolder.h"
#include "../restricters/UniversalQuantifierRemover.h"
#include "../decorators/OccuringVariableDecorator.h"
#include "../../../Frontend/ast.h"
#include "../../environment.hh"

/**
 * @param[in] form:     traversed Ex0 node
 */
AST* AntiPrenexer::visit(ASTForm_Ex0* form) {
    assert(false && "Called base AntiPrenexer method!");
    return form;
}

/**
 * @param[in] form:     traversed Ex1 node
 */
AST* AntiPrenexer::visit(ASTForm_Ex1* form) {
    assert(false && "Called base AntiPrenexer method!");
    return form;
}

/**
 * @param[in] form:     traversed Ex2 node
 */
AST* AntiPrenexer::visit(ASTForm_Ex2* form) {
    assert(false && "Called base AntiPrenexer method!");
    return form;
}

/**
 * @param[in] form:     traversed All0 node
 */
AST* AntiPrenexer::visit(ASTForm_All0* form) {
    assert(false && "Called base AntiPrenexer method!");
    return form;
}

/**
 * @param[in] form:     traversed All1 node
 */
AST* AntiPrenexer::visit(ASTForm_All1* form) {
    assert(false && "Called base AntiPrenexer method!");
    return form;
}

/**
 * @param[in] form:     traversed All2 node
 */
AST* AntiPrenexer::visit(ASTForm_All2* form) {
    assert(false && "Called base AntiPrenexer method!");
    return form;
}

/**********************
 * FULL ANTI-PRENEXER *
 *********************/
/*-------------------------------------------------------------------------*
 | Ex X . f1          ->    f1                 -- if X \notin freeVars(f1) |
 |      ^-- TODO: Move this to different prenexer                          |
 | Ex X . f1 /\ f2    ->    (Ex X. f1) /\ f2   -- if X \notin freeVars(f2) |
 | Ex X . f1 /\ f2    ->    f1 /\ (Ex X. f2)   -- if X \notin freeVars(f1) |
 | Ex X . f1 \/ f2    ->    (Ex X. f1) \/ (Ex X. f2)                       |
 *-------------------------------------------------------------------------*
 |All X . f1          ->    f1                 -- if X \notin freeVars(f1) |
 |      ^-- TODO: Move this to different prenexer                          |
 |All X . f1 \/ f2    ->    (All X. f1) \/ f2  -- if X \notin freeVars(f2) |
 |All X . f1 \/ f2    ->    f1 \/ (All X. f2)  -- if X \notin freeVars(f1) |
 |All X . f1 /\ f2    ->    (All X. f1) /\ (All X. f2)\                    |
 *-------------------------------------------------------------------------*/

template<class QuantifierClass, class BinopClass>
ASTForm* FullAntiPrenexer::distributiveRule(QuantifierClass *qForm, bool onlyByOne) {
    static_assert(std::is_base_of<ASTForm_q, QuantifierClass>::value, "QuantifierClass is not derived from 'ASTForm_q' class");
    static_assert(std::is_base_of<ASTForm_ff, BinopClass>::value, "BinopClass is not derived from 'ASTForm_ff' class");
    // Ex . f1 op f2 -> (Ex X. f1) op (Ex X. f2)

    BinopClass *binopForm = static_cast<BinopClass*>(qForm->f);

    IdentList *bound = qForm->vl;
    IdentList left, right, middle;
    assert(binopForm->f1->allVars != nullptr);
    assert(binopForm->f2->allVars != nullptr);

    for (auto var = bound->begin(); var != bound->end(); ++var) {
        bool varInLeft = binopForm->f1->allVars->exists(*var);
        bool varInRight = binopForm->f2->allVars->exists(*var);

        if(varInLeft) {
            left.push_back(*var);
        };

        if(varInRight) {
            right.push_back(*var);
        };
    }

    if(!left.empty()) {
        binopForm->f1 = new QuantifierClass(nullptr, new IdentList(left), binopForm->f1, binopForm->f1->pos);
        binopForm->f1->allVars = copy(static_cast<ASTForm_q*>(binopForm->f1)->f->allVars);
        if(!onlyByOne) {
            binopForm->f1 = static_cast<ASTForm *>(binopForm->f1->accept(*this));
        }
    }

    if(!right.empty()) {
        binopForm->f2 = new QuantifierClass(nullptr, new IdentList(right), binopForm->f2, binopForm->f2->pos);
        binopForm->f2->allVars = copy(static_cast<ASTForm_q*>(binopForm->f2)->f->allVars);
        if(!onlyByOne) {
            binopForm->f2 = static_cast<ASTForm *>(binopForm->f2->accept(*this));
        }
    }

    // Cleanup
    qForm->f = nullptr;
    delete qForm;

    return binopForm;
}

template<class QuantifierClass, class BinopClass>
ASTForm* FullAntiPrenexer::nonDistributiveRule(QuantifierClass *qForm, bool onlyByOne) {
    static_assert(std::is_base_of<ASTForm_q, QuantifierClass>::value, "QuantifierClass is not derived from 'ASTForm_q' class");
    static_assert(std::is_base_of<ASTForm_ff, BinopClass>::value, "BinopClass is not derived from 'ASTForm_ff' class");

    // Ex . f1 op f2 -> (Ex X. f1) op f2
    // Ex . f2 op f2 -> f1 op (Ex X. f2)
    BinopClass *binopForm = static_cast<BinopClass*>(qForm->f);

    IdentList *bound = qForm->vl;
    IdentList left, right, middle;
    assert(binopForm->f1->allVars != nullptr);
    assert(binopForm->f2->allVars != nullptr);

    for (auto var = bound->begin(); var != bound->end(); ++var) {
        bool varInLeft = binopForm->f1->allVars->exists(*var);
        bool varInRight = binopForm->f2->allVars->exists(*var);

        // Ex var. f1 op f2     | var in f1 && var in f2
        if (varInLeft && varInRight) {
            middle.push_back(*var);
        // (Ex var. f1) op f2   | var notin f2
        } else if(varInLeft) {
            left.push_back(*var);
        // f1 op (Ex var. f2)   | var notin f1
        } else if(varInRight) {
            right.push_back(*var);
        } // f1 op f2           | var notin f1 && var notin f2
    }

    if(left.empty() && right.empty()) {
        return qForm;
    }

    if(!left.empty()) {
        binopForm->f1 = new QuantifierClass(nullptr, new IdentList(left), binopForm->f1, binopForm->f1->pos);
        binopForm->f1->allVars = copy(static_cast<ASTForm_q*>(binopForm->f1)->f->allVars);
        if(!onlyByOne) {
            binopForm->f1 = static_cast<ASTForm *>(binopForm->f1->accept(*this));
        }
    }

    if(!right.empty()) {
        binopForm->f2 = new QuantifierClass(nullptr, new IdentList(right), binopForm->f2, binopForm->f2->pos);
        binopForm->f2->allVars = copy(static_cast<ASTForm_q*>(binopForm->f2)->f->allVars);
        if(!onlyByOne) {
            binopForm->f2 = static_cast<ASTForm *>(binopForm->f2->accept(*this));
        }
    }

    if(!middle.empty()) {
        qForm->vl->reset();
        qForm->vl->insert(&middle);
        return qForm;
    } else {
        qForm->f = nullptr;
        delete qForm;

        return binopForm;
    }
}

template<class OuterQuantifier, class InnerQuantifier>
ASTForm* FullAntiPrenexer::_pushExistentialByOne(OuterQuantifier *form, bool byOne) {
    ASTForm* f = existentialAntiPrenex<InnerQuantifier>(form->f, true);
    assert(f != nullptr);

    if(f == form->f) {
        form->f = static_cast<ASTForm*>(form->f->accept(*this));
        assert(form->f != nullptr);
        return form;
    } else {
        form->f = f;
        return universalAntiPrenex<OuterQuantifier>(form, byOne);
    }
}

template<class OuterQuantifier, class InnerQuantifier>
ASTForm* FullAntiPrenexer::_pushUniversalByOne(OuterQuantifier *form, bool byOne) {
    ASTForm* f = universalAntiPrenex<InnerQuantifier>(form->f, true);
    assert(f != nullptr);

    if(f == form->f) {
        form->f = static_cast<ASTForm*>(form->f->accept(*this));
        assert(form->f != nullptr);
        return form;
    } else {
        form->f = f;
        return existentialAntiPrenex<OuterQuantifier>(form, byOne);
    }
}

template<class OuterQuantifier, class InnerQuantifier>
ASTForm* FullAntiPrenexer::_pushCompatibleExistentialByOne(OuterQuantifier *form, bool byOne) {
    ASTForm* f = existentialAntiPrenex<InnerQuantifier>(form->f, true);
    assert(f != nullptr);

    if(f == form->f) {
        form->f = static_cast<ASTForm*>(form->f->accept(*this));
        assert(form->f != nullptr);
        return form;
    } else {
        form->f = f;
        return existentialAntiPrenex<OuterQuantifier>(form, byOne);
    }
}

template<class OuterQuantifier, class InnerQuantifier>
ASTForm* FullAntiPrenexer::_pushCompatibleUniversalByOne(OuterQuantifier *form, bool byOne) {
    ASTForm* f = universalAntiPrenex<InnerQuantifier>(form->f, true);
    assert(f != nullptr);

    if(f == form->f) {
        form->f = static_cast<ASTForm*>(form->f->accept(*this));
        assert(form->f != nullptr);
        return form;
    } else {
        form->f = f;
        return universalAntiPrenex<OuterQuantifier>(form, byOne);
    }
}

template<class OuterQuantifier>
ASTForm* FullAntiPrenexer::_pushNegationByOne(OuterQuantifier* form, bool byOne) {
    ASTForm_Not* not_form = static_cast<ASTForm_Not*>(form->f);
    ASTForm* result, *left, *right;
    ASTForm_And* andForm;
    ASTForm_Or* orForm;
    ASTForm_ff* child_form;

    switch(not_form->f->kind) {
        case aAnd:
            andForm = static_cast<ASTForm_And*>(not_form->f);
            left = new ASTForm_Not(andForm->f1, Pos());
            left->allVars = copy(andForm->f1->allVars);
            right = new ASTForm_Not(andForm->f2, Pos());
            right->allVars = copy(andForm->f2->allVars);
            result = new ASTForm_Or(left, right, Pos());
            result->allVars = copy(andForm->allVars);
            andForm->detach();
            delete andForm;
            break;
        case aOr:
            orForm = static_cast<ASTForm_Or*>(not_form->f);
            left = new ASTForm_Not(orForm->f1, Pos());
            left->allVars = copy(orForm->f1->allVars);
            right = new ASTForm_Not(orForm->f2, Pos());
            right->allVars = copy(orForm->f2->allVars);
            result = new ASTForm_And(left, right, Pos());
            result->allVars = copy(orForm->allVars);
            orForm->detach();
            delete orForm;
            break;
        case aImpl:
            // not (A => B) = A and not B
            child_form = static_cast<ASTForm_ff*>(not_form->f);
            left = child_form->f1;
            right = new ASTForm_Not(child_form->f2, Pos());
            right->allVars = copy(child_form->f2->allVars);
            result = new ASTForm_And(left, right, Pos());
            result->allVars = copy(child_form->allVars);
            child_form->detach();
            delete child_form;
            break;
        case aBiimpl:
            // not (A <=> B) = (not A) <=> B
            child_form = static_cast<ASTForm_ff*>(not_form->f);
            right = new ASTForm_Not(child_form->f2, Pos());
            right->allVars = copy(child_form->f2->allVars);
            child_form->f2 = right;
            result = child_form;
            break;
        case aNot:
            result = static_cast<ASTForm_Not*>(not_form->f)->f;
            not_form->f->detach();
            delete not_form;
            break;
        case aAll1:
        case aAll2:
        case aEx1:
        case aEx2:
        default:
            result = form->f;
    }

    if(result == form->f) {
        form->f = static_cast<ASTForm*>(form->f->accept(*this));
        return form;
    } else {
        form->f = result;
        if(form->kind == aAll1 || form->kind == aAll2) {
            return universalAntiPrenex<OuterQuantifier>(form, byOne);
        } else {
            assert(form->kind == aEx1 || form->kind == aEx2);
            return existentialAntiPrenex<OuterQuantifier>(form, byOne);
        }
    }
}

/*-------------------------------------------------------------------------*
 | Ex X . f1          ->    f1                 -- if X \notin freeVars(f1) |
 | Ex X . f1 /\ f2    ->    (Ex X. f1) /\ f2   -- if X \notin freeVars(f2) |
 | Ex X . f1 /\ f2    ->    f1 /\ (Ex X. f2)   -- if X \notin freeVars(f1) |
 | Ex X . f1 \/ f2    ->    (Ex X. f1) \/ (Ex X. f2)                       |
 *-------------------------------------------------------------------------*/
template<class ExistClass>
ASTForm* FullAntiPrenexer::existentialAntiPrenex(ASTForm *form, bool onlyByOne) {
    static_assert(std::is_base_of<ASTForm_q, ExistClass>::value, "ExistClass is not derived from 'ASTForm_q' class");

    ExistClass* exForm = static_cast<ExistClass*>(form);
    switch(exForm->f->kind) {
        case aOr:
            // Process Or Rule
            return distributiveRule<ExistClass, ASTForm_Or>(exForm, false);
        case aAnd:
            // Process And Rule
            return nonDistributiveRule<ExistClass, ASTForm_And>(exForm, false);
        case aImpl:
            assert(false && "Implication is unsupported in Anti-Prenexing");
        case aBiimpl:
#           if (ALT_NO_BI_PRENEXING == true)
            return exForm;
#           else
            return nonDistributiveRule<ExistClass, ASTForm_Biimpl>(exForm, false);
#           endif
        case aNot:
            return this->_pushNegationByOne<ExistClass>(exForm, onlyByOne);
        case aEx1:
            return this->_pushCompatibleExistentialByOne<ExistClass, ASTForm_Ex1>(exForm, onlyByOne);
        case aEx2:
            return this->_pushCompatibleExistentialByOne<ExistClass, ASTForm_Ex2>(exForm, onlyByOne);
        case aAll1:
            return this->_pushUniversalByOne<ExistClass, ASTForm_All1>(exForm, onlyByOne);
        case aAll2:
            return this->_pushUniversalByOne<ExistClass, ASTForm_All2>(exForm, onlyByOne);
        default:
            return exForm;
    }
}

AST* FullAntiPrenexer::visit(ASTForm_Ex0 *form) {
    return existentialAntiPrenex<ASTForm_Ex0>(form, false);
}

AST* FullAntiPrenexer::visit(ASTForm_Ex1 *form) {
    return existentialAntiPrenex<ASTForm_Ex1>(form, false);
}

AST* FullAntiPrenexer::visit(ASTForm_Ex2 *form) {
    return existentialAntiPrenex<ASTForm_Ex2>(form, false);
}

/*-------------------------------------------------------------------------*
 |All X . f1          ->    f1                 -- if X \notin freeVars(f1) |
 |All X . f1 \/ f2    ->    (All X. f1) \/ f2  -- if X \notin freeVars(f2) |
 |All X . f1 \/ f2    ->    f1 \/ (All X. f2)  -- if X \notin freeVars(f1) |
 |All X . f1 /\ f2    ->    (All X. f1) /\ (All X. f2)\                    |
 *-------------------------------------------------------------------------*/
template<class ForallClass>
ASTForm* FullAntiPrenexer::universalAntiPrenex(ASTForm *form, bool onlyByOne) {
    static_assert(std::is_base_of<ASTForm_q, ForallClass>::value, "ForallClass is not derived from 'ASTForm_q' class");

    ForallClass* allForm = static_cast<ForallClass*>(form);
    switch(allForm->f->kind) {
        case aOr:
            // Process Or Rule
            return nonDistributiveRule<ForallClass, ASTForm_Or>(allForm, false);
        case aAnd:
            // Process And Rule
            return distributiveRule<ForallClass, ASTForm_And>(allForm, false);
        case aImpl:
            assert(false && "Implication is unsupported in Anti-Prenexing");
        case aBiimpl:
#           if (ALT_NO_BI_PRENEXING == true)
            return allForm;
#           else
            return nonDistributiveRule<ForallClass, ASTForm_Biimpl>(allForm, false);
#           endif
        case aNot:
           return this->_pushNegationByOne<ForallClass>(allForm, onlyByOne);
        case aAll1:
            return this->_pushCompatibleUniversalByOne<ForallClass, ASTForm_All1>(allForm, onlyByOne);
        case aAll2:
            return this->_pushCompatibleUniversalByOne<ForallClass, ASTForm_All2>(allForm, onlyByOne);
        case aEx1:
            return this->_pushExistentialByOne<ForallClass, ASTForm_Ex1>(allForm, onlyByOne);
        case aEx2:
            return this->_pushExistentialByOne<ForallClass, ASTForm_Ex2>(allForm, onlyByOne);
        default:
            return allForm;
    }
}

AST* FullAntiPrenexer::visit(ASTForm_All0 *form) {
    return universalAntiPrenex<ASTForm_All0>(form, false);
}

AST* FullAntiPrenexer::visit(ASTForm_All1 *form) {
    return universalAntiPrenex<ASTForm_All1>(form, false);
}

AST* FullAntiPrenexer::visit(ASTForm_All2 *form) {
    return universalAntiPrenex<ASTForm_All2>(form, false);
}

AST* FullAntiPrenexer::visit(ASTForm_And *form) {
    form->f1 = static_cast<ASTForm*>(form->f1->accept(*this));
    assert(form->f1 != nullptr);
    form->f2 = static_cast<ASTForm*>(form->f2->accept(*this));
    assert(form->f2 != nullptr);
    return form;
}

AST* FullAntiPrenexer::visit(ASTForm_Or *form) {
    form->f1 = static_cast<ASTForm*>(form->f1->accept(*this));
    assert(form->f1 != nullptr);
    form->f2 = static_cast<ASTForm*>(form->f2->accept(*this));
    assert(form->f2 != nullptr);
    return form;
}

AST* FullAntiPrenexer::visit(ASTForm_Not *form) {
    // Fixme: Probably everything could be pushed now
    form->f = static_cast<ASTForm*>(form->f->accept(*this));
    return form;
}

/*********************************
 **  DISTRIBUTIVE ANTI PRENEXER **
 **    (currently obsolete)     **
 *********************************/


/*----------------------------------------------------------------------*
 | Ex X. f1 /\ (f2 \/ f3)     -> Ex X. (f1 /\ f2) \/ (f2 /\ f3)         |
 |                            -> (Ex X. f1 /\ f2) \/  (Ex X. f2 /\ f3)  |
 *----------------------------------------------------------------------*/
ASTForm* DistributiveAntiPrenexer::findConjunctiveDistributivePoint(ASTForm *form) {
    // First look to the left
    if(form->kind == aAnd) {
        ASTForm_And *andForm = reinterpret_cast<ASTForm_And *>(form);
        if(andForm->f1->kind == aOr) {
            return form;
        } else if(andForm->f1->kind == aAnd) {
            return this->findConjunctiveDistributivePoint(andForm->f1);
        } else if(andForm->f2->kind == aOr) {
            return form;
        } else if(andForm->f2->kind == aAnd) {
            return this->findConjunctiveDistributivePoint(andForm->f2);
        } else {
            return nullptr;
        }
    } else {
        return nullptr;
    }
}

template <class QuantifierClass>
ASTForm* DistributiveAntiPrenexer::distributeDisjunction(QuantifierClass *form) {
    // First try to find the conjunctive distributive point---i.e. the point
    // where Conjunction is followed by disjunction and thus can be transformed
    // by distributive law
    ASTForm* distPoint = this->findConjunctiveDistributivePoint(form->f);
    if(distPoint == nullptr) {
        // Conjunctive Distributive point was not found and so it should not exists,
        // we can end
        return form;
    } else {
        // Do some things
        ASTForm_And* andForm = reinterpret_cast<ASTForm_And*>(distPoint);
        ASTForm_Or* orForm = nullptr;
        // Switch the OR node to true (neutral to conjunction)
        // f1 /\ (f2 \/ f3) -> f1 /\ true
        if(andForm->f1->kind == aOr) {
            orForm = reinterpret_cast<ASTForm_Or*>(andForm->f1);
            andForm->f1 = new ASTForm_True(Pos());
        } else {
            assert(andForm->f2->kind == aOr);
            orForm = reinterpret_cast<ASTForm_Or*>(andForm->f2);
            andForm->f2 = new ASTForm_True(Pos());
        }
        assert(orForm != nullptr);

        ASTForm* f1 = orForm->f1;
        ASTForm* f2 = orForm->f2;

        ASTForm_And* leftConjunction = new ASTForm_And(f1, form->f, Pos());
        ASTForm_And* rightConjunction = new ASTForm_And(f2, form->f, Pos());
        ASTForm_Or* newRoot = new ASTForm_Or(leftConjunction, rightConjunction, Pos());
        form->f = newRoot;
        // Unfold the True/False
        BooleanUnfolder bu_visitor;
        form = reinterpret_cast<QuantifierClass*>(form->accept(bu_visitor));
    }

    // Restart the computation
    return existentialDistributiveAntiPrenex<QuantifierClass>(form);
}

template <class ExistClass>
ASTForm* DistributiveAntiPrenexer::existentialDistributiveAntiPrenex(ASTForm *form) {
    static_assert(std::is_base_of<ASTForm_q, ExistClass>::value, "ExistClass is not derived from 'ASTForm_q' class");

    // First call the anti-prenexing rule to push the quantifier as deep as possible
    //ASTForm* antiPrenexedForm = existentialAntiPrenex<ExistClass>(form);

    // Expand Universal Quantifier
    UniversalQuantifierRemover universalUnfolding;
    ASTForm* antiPrenexedForm = reinterpret_cast<ASTForm*>(form->accept(universalUnfolding));

    // Push negations down as well
    NegationUnfolder negationUnfolding;
    antiPrenexedForm = reinterpret_cast<ASTForm*>(antiPrenexedForm->accept(negationUnfolding));

    if(antiPrenexedForm->kind == aEx1 || antiPrenexedForm->kind == aEx2) {
        ExistClass* exForm = reinterpret_cast<ExistClass*>(antiPrenexedForm);
        switch(exForm->f->kind) {
            case aOr:
                return distributiveRule<ExistClass, ASTForm_Or>(exForm);
            case aAnd:
                return distributeDisjunction<ExistClass>(exForm);
            default:
                return antiPrenexedForm;
        }
    } else {
        // We are done
        return antiPrenexedForm;
    }
}

AST* DistributiveAntiPrenexer::visit(ASTForm_Ex1 *form) {
    return existentialDistributiveAntiPrenex<ASTForm_Ex1>(form);
}

AST* DistributiveAntiPrenexer::visit(ASTForm_Ex2 *form) {
    return existentialDistributiveAntiPrenex<ASTForm_Ex2>(form);
}

/*----------------------------------------------------------------------*
 | All X. f1 \/ (f2 /\ f3)    -> All X. (f1 \/ f2) /\ (f2 \/ f3)        |
 |                            -> (All X. f1 \/ f2) /\ (All X. f2 \/ f3) |
 *----------------------------------------------------------------------*/
ASTForm* DistributiveAntiPrenexer::findDisjunctiveDistributivePoint(ASTForm *form) {
    if(form->kind == aOr) {
        ASTForm_Or* orForm = reinterpret_cast<ASTForm_Or*>(form);
        if(orForm->f1->kind == aAnd) {
            return form;
        } else if(orForm->f1->kind == aOr) {
            return this->findDisjunctiveDistributivePoint(orForm->f1);
        } else if(orForm->f2->kind == aAnd) {
            return form;
        } else if(orForm->f2->kind == aOr) {
            return this->findDisjunctiveDistributivePoint(orForm->f2);
        } else {
            return nullptr;
        }
    } else {
        return nullptr;
    }
}

template <class QuantifierClass>
ASTForm* DistributiveAntiPrenexer::distributeConjunction(QuantifierClass *form) {
    // First try to find the disjunctive distributive point---i.e. the point
    // where Disjunction is followed by conjunction and thus can be distributed
    // by distributive law
    ASTForm* distPoint = this->findDisjunctiveDistributivePoint(form->f);
    if(distPoint == nullptr) {
        // Disjunctive Distributive point was not found and so it should not exists,
        // we can end
        // Expand Universal Quantifier
        UniversalQuantifierRemover universalUnfolding;
        ASTForm* form2 = reinterpret_cast<ASTForm*>(form->accept(universalUnfolding));
        return reinterpret_cast<ASTForm*>(form2->accept(*this));
    } else {
        // Do some things
        ASTForm_Or* orForm = reinterpret_cast<ASTForm_Or*>(distPoint);
        ASTForm_And* andForm = nullptr;
        // Switch the OR node to true (neutral to conjunction)
        // f1 \/ (f2 /\ f3) -> f1 \/ false
        if(orForm->f1->kind == aAnd) {
            andForm = reinterpret_cast<ASTForm_And*>(orForm->f1);
            orForm->f1 = new ASTForm_False(Pos());
        } else {
            assert(orForm->f2->kind == aAnd);
            andForm = reinterpret_cast<ASTForm_And*>(orForm->f2);
            orForm->f2 = new ASTForm_False(Pos());
        }
        assert(andForm != nullptr);

        ASTForm* f1 = andForm->f1;
        ASTForm* f2 = andForm->f2;

        ASTForm_Or* leftConjunction = new ASTForm_Or(f1, form->f, Pos());
        ASTForm_Or* rightConjunction = new ASTForm_Or(f2, form->f, Pos());
        ASTForm_And* newRoot = new ASTForm_And(leftConjunction, rightConjunction, Pos());
        form->f = newRoot;

        // Unfold the True/False
        BooleanUnfolder bu_visitor;
        form = reinterpret_cast<QuantifierClass*>(form->accept(bu_visitor));
    }

    return universalDistributiveAntiPrenex<QuantifierClass>(form);
}

template <class ForallClass>
ASTForm* DistributiveAntiPrenexer::universalDistributiveAntiPrenex(ASTForm *form) {
    static_assert(std::is_base_of<ASTForm_q, ForallClass>::value, "ForallClass is not derived from 'ASTForm_q' class");

    // First call the anti-prenexing rule to push the quantifier as deep as possible
    //ASTForm* antiPrenexedForm = universalAntiPrenex<ForallClass>(form);

    // Push negations down as well
    NegationUnfolder negationUnfolding;
    ASTForm* antiPrenexedForm = reinterpret_cast<ASTForm*>(form->accept(negationUnfolding));

    if(antiPrenexedForm->kind == aAll1 || antiPrenexedForm->kind == aAll2) {
        // Not everything was pushed, we can try to call the distribution
        ForallClass* allForm = reinterpret_cast<ForallClass*>(antiPrenexedForm);
        switch(allForm->f->kind) {
            case aOr:
                return distributeConjunction<ForallClass>(allForm);
            case aAnd:
                return distributiveRule<ForallClass, ASTForm_And>(allForm);
            default:
                UniversalQuantifierRemover universalRemover;
                antiPrenexedForm = reinterpret_cast<ASTForm*>(antiPrenexedForm->accept(universalRemover));
                return reinterpret_cast<ASTForm*>(antiPrenexedForm->accept(negationUnfolding));
        }
    } else {
        // We are done
        return antiPrenexedForm;
    }
}

AST* DistributiveAntiPrenexer::visit(ASTForm_All1 *form) {
    return universalDistributiveAntiPrenex<ASTForm_All1>(form);
}

AST* DistributiveAntiPrenexer::visit(ASTForm_All2 *form) {
    return universalDistributiveAntiPrenex<ASTForm_All2>(form);
}

