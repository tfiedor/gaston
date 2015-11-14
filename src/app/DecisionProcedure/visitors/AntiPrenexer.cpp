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

/**
 * @param[in] form:     traversed Ex0 node
 */
AST* AntiPrenexer::visit(ASTForm_Ex0* form) {
    // TODO: implement
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
 | Ex X . f1 /\ f2    ->    (Ex X. f1) /\ f2   -- if X \notin freeVars(f2) |
 | Ex X . f1 /\ f2    ->    f1 /\ (Ex X. f2)   -- if X \notin freeVars(f1) |
 | Ex X . f1 \/ f2    ->    (Ex X. f1) \/ (Ex X. f2)                       |
 *-------------------------------------------------------------------------*
 |All X . f1          ->    f1                 -- if X \notin freeVars(f1) |
 |All X . f1 \/ f2    ->    (All X. f1) \/ f2  -- if X \notin freeVars(f2) |
 |All X . f1 \/ f2    ->    f1 \/ (All X. f2)  -- if X \notin freeVars(f1) |
 |All X . f1 /\ f2    ->    (All X. f1) /\ (All X. f2)\                    |
 *-------------------------------------------------------------------------*/

/*-------------------------------------------------------------------------*
 | Ex X . f1          ->    f1                 -- if X \notin freeVars(f1) |
 | Ex X . f1 /\ f2    ->    (Ex X. f1) /\ f2   -- if X \notin freeVars(f2) |
 | Ex X . f1 /\ f2    ->    f1 /\ (Ex X. f2)   -- if X \notin freeVars(f1) |
 | Ex X . f1 \/ f2    ->    (Ex X. f1) \/ (Ex X. f2)                       |
 *-------------------------------------------------------------------------*/
template<class ExistClass>
ASTForm* existentialAntiPrenex(ASTForm *form) {
    static_assert(std::is_base_of<ASTForm_q, ExistClass>::value, "ExistClass is not derived from 'ASTForm_q' class");

    ExistClass* exForm = static_cast<ExistClass*>(form);
    std::cout << "Exists with [";
    IdentList *bound = exForm->vl;
    for(auto it = bound->begin(); it != bound->end(); ++it) {
        std::cout << (*it) << ",";
    }
    std::cout << "]\n";

    return form;
}

AST* FullAntiPrenexer::visit(ASTForm_Ex1 *form) {
    return existentialAntiPrenex<ASTForm_Ex1>(form);
}

AST* FullAntiPrenexer::visit(ASTForm_Ex2 *form) {
    return existentialAntiPrenex<ASTForm_Ex1>(form);
}

/*-------------------------------------------------------------------------*
 |All X . f1          ->    f1                 -- if X \notin freeVars(f1) |
 |All X . f1 \/ f2    ->    (All X. f1) \/ f2  -- if X \notin freeVars(f2) |
 |All X . f1 \/ f2    ->    f1 \/ (All X. f2)  -- if X \notin freeVars(f1) |
 |All X . f1 /\ f2    ->    (All X. f1) /\ (All X. f2)\                    |
 *-------------------------------------------------------------------------*/
template<class ForallClass>
ASTForm* universalAntiPrenex(ASTForm *form) {
    static_assert(std::is_base_of<ASTForm_q, ForallClass>::value, "ForallClass is not derived from 'ASTForm_q' class");

    return form;
}

AST* FullAntiPrenexer::visit(ASTForm_All1 *form) {
    return universalAntiPrenex<ASTForm_All1>(form);
}

AST* FullAntiPrenexer::visit(ASTForm_All2 *form) {
    return universalAntiPrenex<ASTForm_All2>(form);
}