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

#include "../Frontend/ast.h"
#include "../Frontend/ast_visitor.h"

class AntiPrenexer : public TransformerVisitor {
public:
    AntiPrenexer() : TransformerVisitor(Traverse::PreOrder) {}

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
    FullAntiPrenexer() {}

    // Works in preorder
    virtual AST* visit(ASTForm_Ex1* form);
    virtual AST* visit(ASTForm_Ex2* form);
    virtual AST* visit(ASTForm_All1* form);
    virtual AST* visit(ASTForm_All2* form);
};

#endif //WSKS_ANTIPRENEXER_H
