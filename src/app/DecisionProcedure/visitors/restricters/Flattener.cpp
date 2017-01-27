/*****************************************************************************
 *  dWiNA - Deciding WSkS using non-deterministic automata
 *
 *  Copyright (c) 2015  Tomas Fiedor <ifiedortom1@fit.vutbr.cz>
 *
 *  Description:
 *    Visitor for flattening of the formula
 *
 *****************************************************************************/
#include "Flattener.h"

extern SymbolTable symbolTable;
extern PredicateLib predicateLib;
extern Options options;

/**
 * Generates fresh first-order variable that can be used for quantification
 *
 * @return: fresh first-order variable
 */
ASTTerm1_Var1* Flattener::generateFreshFirstOrder() {
    return new ASTTerm1_Var1(symbolTable.insertFresh(Varname1), Pos());
}

/**
 * Generates fresh second-order variable that can be used for quantification
 *
 * @return: fresh second-order variable
 */
ASTTerm2_Var2* Flattener::generateFreshSecondOrder() {
    return new ASTTerm2_Var2(symbolTable.insertFresh(Varname2), Pos());
}

/**
 * Flattens formula by unfolding the n to the base case 1
 *
 * @param[in] term      traversed Plus term
 */
AST* Flattener::visit(ASTTerm1_Plus* term) {
    assert(term != nullptr);
    assert(term->n >= 1);

    int i = term->n;

    ASTTerm1 *prev = static_cast<ASTTerm1*>((term->t)->accept(*this));
    while(i != 1) {
        prev = new ASTTerm1_Plus(prev, 1, term->pos);
        --i;
    }

    // Return this, cannot unfold anymore
    return new ASTTerm1_Plus(prev, 1, term->pos);
}

/**
 * Flattens formula to second-order variables and restricted syntax so it uses
 * only certain atomic formulae
 *
 *  xi = yi -> ex z: z = xi & z = yj
 *  s = ti  -> ex z: z = t & zi = s
 *  y = xi  -> Xy = Xx i
 *  x = e   -> Xx = e
 *  x = y   -> Xx = Xy
 *
 * @param[in] form:     traversed Equal1 node
 */
AST* Flattener::visit(ASTForm_Equal1* form) {
    // TODO: This is still huge pile of shit that needs serious refactoring
    assert(form != nullptr);
    return form;

}

/**
 * Flattens the X = Y to simplify the formulae
 *
 * @param[in] form      traversed Equal2 node
 */
AST* Flattener::visit(ASTForm_Equal2* form) {
    assert(form != nullptr);
    assert(form->T1 != nullptr);
    assert(form->T2 != nullptr);
    return form;
}

/**
 * Flattens the x != y to simplify the formulae
 *
 * @param[in] form      traversed NotEqual1 node
 */
AST* Flattener::visit(ASTForm_NotEqual1* form) {
    // TODO: still needs heavy refactoring
    assert(form != nullptr);
    assert(form->t1 != nullptr);
    assert(form->t2 != nullptr);
    return form;

}

/**
 * Flattens the X != Y to simplify the formulae
 *
 * @param[in] form      traversed NotEqual2 node
 */
AST* Flattener::visit(ASTForm_NotEqual2* form) {
    assert(form != nullptr);

    // TODO: this is wrong i guess
    ASTForm_Equal2* eq = new ASTForm_Equal2(form->T1, form->T2, form->pos);
    return new ASTForm_Not(static_cast<ASTForm*>(eq->accept(*this)), form->pos);
}

/**
 * Substitutes xi < y or y x < yi to new fresh variable with equality so
 * final formula is ex z: z = yi & x < z
 *
 * @param leftTerm: left side of Less (x)
 * @param rightTerm: right side of Less (y)
 * @param substituteLeft: true if left side of expression should be substituted or right
 * @return flattened and freshened formula
 */

ASTForm* substituteFreshLess(ASTTerm1* leftTerm, ASTTerm1* rightTerm, bool substituteLeft, TransformerVisitor& visitor) {
    ASTTerm1_Var1* z;
    ASTForm_And* conjuction;
    ASTForm* left;
    ASTForm* right;

    z = Flattener::generateFreshFirstOrder();
    if(substituteLeft) {
        left = new ASTForm_Equal1(z, leftTerm,Pos());
        right = new ASTForm_Less(z, rightTerm, Pos());
    } else {
        left = new ASTForm_Equal1(z, rightTerm,Pos());
        right = new ASTForm_Less(leftTerm, z, Pos());
    }
    conjuction = new ASTForm_And(left, right, Pos());

    ASTForm_Ex1* newFormula = new ASTForm_Ex1(0, new IdentList(z->getVar()), conjuction, Pos());
    return static_cast<ASTForm*>(newFormula->accept(visitor));
}

/**
 * Flattens the x < y to simplify the formula
 *
 * @param[in] form:     traversed Less node
 */
AST* Flattener::visit(ASTForm_Less* form) {
    assert(form != nullptr);
    assert(form->t1 != nullptr);
    assert(form->t2 != nullptr);
    return form;

#if (OPT_SMARTER_FLATTENING == true)
    if (form->t1->kind != aVar1) {
        return substituteFreshLess(form->t1, form->t2, true, *this);
    } else if (form->t2->kind != aVar1) {
        return substituteFreshLess(form->t1, form->t2, false, *this);
    } else {
        return form;
    }
#else
	ASTForm_NotEqual1* leftSide = new ASTForm_NotEqual1(form->t1, form->t2, form->pos);
	ASTForm_LessEq* rightSide = new ASTForm_LessEq(form->t1, form->t2, form->pos);
	ASTForm_And* conjuction = new ASTForm_And(leftSide, rightSide, form->pos);
	return conjuction->accept(*this);
#endif
}

/**
 * Substitutes xi <= y or y x <= yi to new fresh variable with equality so
 * final formula is ex z: z = yi & x <= z
 *
 * @param leftTerm: left side of LessEq (x)
 * @param rightTerm: right side of LessEq (y)
 * @param substituteLeft: true if left side of expression should be substituted or right
 * @return flattened and freshened formula
 */

ASTForm* substituteFreshLessEq(ASTTerm1* leftTerm, ASTTerm1* rightTerm, bool substituteLeft, TransformerVisitor &visitor) {
    ASTTerm1_Var1* z;
    ASTForm_And* conjuction;
    ASTForm* left;
    ASTForm* right;

    z = Flattener::generateFreshFirstOrder();
    if(substituteLeft) {
        left = new ASTForm_Equal1(z, leftTerm, leftTerm->pos);
        right = new ASTForm_LessEq(z, rightTerm, rightTerm->pos);
    } else {
        left = new ASTForm_Equal1(z, rightTerm, rightTerm->pos);
        right = new ASTForm_LessEq(leftTerm, z, leftTerm->pos);
    }
    conjuction = new ASTForm_And(left, right, leftTerm->pos);

    ASTForm_Ex1* newFormula = new ASTForm_Ex1(0, new IdentList(z->getVar()), conjuction, leftTerm->pos);
    return static_cast<ASTForm*>(newFormula->accept(visitor));
}

/**
 * Flattens the x <= y to simplify the formula
 *
 * @param[in] form:    traversed LessEq node
 */
 AST* Flattener::visit(ASTForm_LessEq* form) {
    assert(form != nullptr);
    assert(form->t1 != nullptr);
    assert(form->t2 != nullptr);
    return form;

    if (form->t1->kind != aVar1) {
        return substituteFreshLessEq(form->t1, form->t2, true, *this);
    } else if (form->t2->kind != aVar1) {
        return substituteFreshLessEq(form->t1, form->t2, false, *this);
    } else {
#if (OPT_SMARTER_FLATTENING == true)
        return form;
#else
		ASTTerm2_Var2* X = Flattener::generateFreshSecondOrder();
		ASTTerm1_Var1* z = Flattener::generateFreshFirstOrder();

		// construction of innerDisjunction (forall Z: z1 in X | z2 in X)
		ASTForm_All1* innerDisjunction;
		if (options.mode != TREE) {
			ASTTerm1_Plus* z1 = new ASTTerm1_Plus(z, 1, form->pos);
			ASTForm_In* z1InX = new ASTForm_In(z1, X, form->pos);
			innerDisjunction = new ASTForm_All1(0, new IdentList(z->getVar()),z1InX , form->pos);
		// TODO: WS2S
		} else {

		}

		// Construction of innerImplication innerDisjuction => z in X
		ASTForm_Impl* innerImplication;
		ASTForm_In* zInX = new ASTForm_In(z, X, form->pos);
		innerImplication = new ASTForm_Impl(innerDisjunction, zInX, form->pos);

		// Construction of innerConjuction (y in X) & innerImplication
		ASTForm_And* innerConjuction;
		ASTForm_In* yInX = new ASTForm_In(form->t2, X, form->pos);
		innerConjuction = new ASTForm_And(yInX, innerImplication, form->pos);

		// Construction outer Implication (innerConjuction) => x in X
		ASTForm_Impl* outerImplication;
		ASTForm_In* xInX = new ASTForm_In(form->t1, X, form->pos);
		outerImplication = new ASTForm_Impl(innerConjuction, xInX, form->pos);

		ASTForm_All2* newFormula = new ASTForm_All2(0, new IdentList(X->getVar()), reinterpret_cast<ASTForm*>(outerImplication->accept(*this)), form->pos);
		return newFormula->accept(*this);
#endif
    }
    return form;
}

/**
 * Substitutes xi in X to new fresh variable with in predicate so
 * final formula is ex z: z = yi & z in X
 *
 * @param leftTerm: left side of In (yi)
 * @param rightTerm: right side of In (X)
 * @return flattened and freshened formula
 */

ASTForm* substituteFreshIn(ASTTerm1* leftTerm, ASTTerm2* rightTerm, TransformerVisitor &visitor) {
    ASTTerm1_Var1* z;
    ASTForm_And* conjuction;
    ASTForm* left;
    ASTForm* right;

    z = Flattener::generateFreshFirstOrder();
    left = new ASTForm_Equal1(z, leftTerm,Pos());
    right = new ASTForm_In(z, rightTerm, Pos());
    conjuction = new ASTForm_And(left, right, Pos());

    return new ASTForm_Ex2(0, new IdentList(z->getVar()), static_cast<ASTForm*>(conjuction->accept(visitor)), Pos());
}

/**
 * Flattens the x in X to simplify the formula
 *
 * @param[in] form:    traversed In node
 */
AST* Flattener::visit(ASTForm_In* form) {
    // x in X + i => ex2 Z: x in Z & Z = X + i
    return form;
}

/**
 * Flattens the x notin X to simplify the formula
 *
 * @param[in] form:     traversed Notin node
 */
AST* Flattener::visit(ASTForm_Notin* form) {
    // TODO: Consider having this as automat alone
    assert(form != nullptr);
    assert(form->t1 != nullptr);
    assert(form->T2 != nullptr);
    return form;

}

/**
 * Flattens the X sub Y  to simplify the formula
 *
 * @param[in] form:     traversed Sub node
 */
AST* Flattener::visit(ASTForm_Sub* form) {
    assert(form != nullptr);
    assert(form->T1 != nullptr);
    assert(form->T2 != nullptr);
    return form;

}

/**
 * Unfolds the called macro by substituting its formal parameters with real
 * parameters
 *
 * @param called: called macro
 * @param realParams: real parameters
 * @return: unfolded formula
 */
ASTForm* unfoldFormula(PredLibEntry* called, ASTList* realParams) {
    IdentList* formalParams = called->formals;

    ASTForm* clonnedFormula = (called->ast)->clone();
    ASTForm* unfoldedFormula = clonnedFormula->unfoldMacro(formalParams, realParams);

    Flattener f_visitor;
    return static_cast<ASTForm*>(unfoldedFormula->accept(f_visitor));
}

/**
 * Flattens the Call function to simplify the formula
 *
 * @param[in] form:     traversed Call node
 */
AST* Flattener::visit(ASTForm_Call* form) {
    int calledNumber = form->n;

    PredLibEntry* called = predicateLib.lookup(calledNumber);

    /* So far, we will treat predicate and macro the same */
    return unfoldFormula(called, form->args);
}
