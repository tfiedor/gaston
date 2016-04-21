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
#ifndef WSKS_ANTIPRENEXER_H
#define WSKS_ANTIPRENEXER_H

#include "../../Frontend/ast.h"
#include "../../Frontend/ast_visitor.h"

class AntiPrenexer : public TransformerVisitor {
public:
    AntiPrenexer(Traverse td) : TransformerVisitor(td) {}

    // Works in preorder
    virtual AST* visit(ASTForm_Ex0* form);
    virtual AST* visit(ASTForm_Ex1* form);
    virtual AST* visit(ASTForm_Ex2* form);
    virtual AST* visit(ASTForm_All0* form);
    virtual AST* visit(ASTForm_All1* form);
    virtual AST* visit(ASTForm_All2* form);
};

/**
 * Full Anti-Prenex tries to push quantifications as deep as possible
 *
 * *-------------------------------------------------------------------------*
 * | Ex X . f1          ->    f1                 -- if X \notin freeVars(f1) |
 * | Ex X . f1 /\ f2    ->    (Ex X. f1) /\ f2   -- if X \notin freeVars(f2) |
 * | Ex X . f1 /\ f2    ->    f1 /\ (Ex X. f2)   -- if X \notin freeVars(f1) |
 * | Ex X . f1 \/ f2    ->    (Ex X. f1) \/ (Ex X. f2)                       |
 * *-------------------------------------------------------------------------*
 * |All X . f1          ->    f1                 -- if X \notin freeVars(f1) |
 * |All X . f1 \/ f2    ->    (All X. f1) \/ f2  -- if X \notin freeVars(f2) |
 * |All X . f1 \/ f2    ->    f1 \/ (All X. f2)  -- if X \notin freeVars(f1) |
 * |All X . f1 /\ f2    ->    (All X. f1) /\ (All X. f2)\                    |
 * *-------------------------------------------------------------------------*
 */
class FullAntiPrenexer : public AntiPrenexer {
public:
    FullAntiPrenexer() : AntiPrenexer(Traverse::CustomOrder) {}

    template<class QuantifierClass, class BinopClass>
    ASTForm* distributiveRule(QuantifierClass *qForm, bool onlyByOne = false);
    template<class QuantifierClass, class BinopClass>
    ASTForm* nonDistributiveRule(QuantifierClass *qForm, bool onlyByOne = false);
    template<class ExistClass>
    ASTForm* existentialAntiPrenex(ASTForm* form, bool onlyByOne = false);
    template<class ForallClass>
    ASTForm* universalAntiPrenex(ASTForm* form, bool onlyByOne = false);
    template<class OuterQuantifier, class InnerQantifier>
    ASTForm* _pushUniversalByOne(OuterQuantifier* form, bool byOne = false);
    template<class OuterQuantifier, class InnerQuantifier>
    ASTForm* _pushExistentialByOne(OuterQuantifier* form, bool byOne = false);

    // Works in preorder
    virtual AST* visit(ASTForm_Ex1* form);
    virtual AST* visit(ASTForm_Ex2* form);
    virtual AST* visit(ASTForm_All1* form);
    virtual AST* visit(ASTForm_All2* form);

    virtual AST* visit(ASTForm_And* form);
    virtual AST* visit(ASTForm_Or* form);
    virtual AST* visit(ASTForm_Not* form);
};

/**
 * Full Anti-Prenexer that tries to unfold some and/or sequences in order
 * to maximize the anti-prenexing
 *
 * *----------------------------------------------------------------------*
 * | Ex X. f1 /\ (f2 \/ f3)     -> Ex X. (f1 /\ f2) \/ (f2 /\ f3)         |
 * |                            -> (Ex X. f1 /\ f2) \/  (Ex X. f2 /\ f3)  |
 * *----------------------------------------------------------------------*
 * | All X. f1 \/ (f2 /\ f3)    -> All X. (f1 \/ f2) /\ (f2 \/ f3)        |
 * |                            -> (All X. f1 \/ f2) /\ (All X. f2 \/ f3) |
 * *----------------------------------------------------------------------*
 */
class DistributiveAntiPrenexer : public FullAntiPrenexer {
public:
    DistributiveAntiPrenexer() {};

    ASTForm* findDisjunctiveDistributivePoint(ASTForm *form);
    ASTForm* findConjunctiveDistributivePoint(ASTForm *form);

    template <class ExistClass>
    ASTForm* existentialDistributiveAntiPrenex(ASTForm *form);
    template <class QuantifierClass>
    ASTForm* distributeDisjunction(QuantifierClass *form);
    template <class ForallClass>
    ASTForm* universalDistributiveAntiPrenex(ASTForm *form);
    template <class QuantifierClass>
    ASTForm* distributeConjunction(QuantifierClass *form);

    // Works in preorder
    virtual AST* visit(ASTForm_Ex1* form);
    virtual AST* visit(ASTForm_Ex2* form);
    virtual AST* visit(ASTForm_All1* form);
    virtual AST* visit(ASTForm_All2* form);
};

#endif //WSKS_ANTIPRENEXER_H
