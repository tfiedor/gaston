/*****************************************************************************
 *  dWiNA - Deciding WSkS using non-deterministic automata
 *
 *  Copyright (c) 2014  Tomas Fiedor <xfiedo01@stud.fit.vutbr.cz>
 *
 *  Description:
 *    Flattening of formula
 *
 *****************************************************************************/

#include "../Frontend/ast.h"
#include "../Frontend/symboltable.h"
#include "../Frontend/env.h"
#include "../Frontend/predlib.h"
#include "environment.hh"
#include <cstring>
#include <vector>
#include "decision_procedures.hh"

using std::cout;

extern SymbolTable symbolTable;
extern Options options;
extern PredicateLib predicateLib;
extern IdentList inFirstOrder;

/**
 * Generates fresh first-order variable that can be used for quantification
 *
 * @return: fresh first-order variable
 */
ASTTerm1_Var1* generateFreshFirstOrder() {
	unsigned int z;
	z = symbolTable.insertFresh(Varname1);
	return new ASTTerm1_Var1(z, Pos());
}

/**
 * Generates fresh second-order variable that can be used for quantification
 *
 * @return: fresh second-order variable
 */
ASTTerm2_Var2* generateFreshSecondOrder() {
	unsigned int Z;
	Z = symbolTable.insertFresh(Varname2);
	return new ASTTerm2_Var2(Z, Pos());
}


/**
 * Conversion of formula to Second Order, that means all the formulas are
 * flattened according to certain rules and all first-order variables are
 * converted to second order, so for each Singleton predicate is added
 *
 * @return: Flattened formula in second order
 */
ASTForm* ASTForm::toSecondOrder() {
	return this->flatten();
}

/**
 * Adds constraints for first-order variables to be singletons, i.e. contain
 * only one element
 *
 * @return: restricted formula
 */
ASTForm* ASTForm::restrictFormula() {
	ASTForm* restrictedFormula = this;

	// For all used first-order variables FirstOrder(x) is appended to formulae
	IdentList free, bound;
	ASTForm_FirstOrder* singleton;
	this->freeVars(&free, &bound);
	IdentList *allVars = ident_union(&free, &bound);
	if (allVars != 0) {
		Ident* it = allVars->begin();
		while(it != allVars->end()) {
			// only variables that are not already singletoned are appended to formula
			if (symbolTable.lookupType(*it) == Varname1 && !inFirstOrder.exists(*it)) {
				singleton = new ASTForm_FirstOrder(new ASTTerm1_Var1((*it), Pos()), Pos());
				restrictedFormula = new ASTForm_And(singleton, restrictedFormula, Pos());
			}
			++it;
		}
	}
	return restrictedFormula;
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
 * @return: flattened formula
 */
ASTForm* ASTForm_Equal1::flatten() {
	ASTTerm2_Var2* y;
	ASTTerm2_Var2* x;
	ASTForm_Equal1* leftEqual;
	ASTForm_Equal1* rightEqual;
	ASTForm_And* conjuction;
	// y = x -> Xx = Xy
	if(this->t1->kind == aVar1 && this->t2->kind == aVar1) {
		y = new ASTTerm2_Var2(((ASTTerm1_Var1*)this->t1)->getVar(), Pos());
		x = new ASTTerm2_Var2(((ASTTerm1_Var1*)this->t2)->getVar(), Pos());
		return new ASTForm_Equal2(y, x, Pos());
	// TODO: Not tested yet
	} else if (this->t1->kind == aPlus1 && this->t2->kind == aVar1) {
		ASTTerm1* temp;
		temp = this->t2;
		this->t2 = this->t1;
		this->t1 = temp;
		return this->flatten();
	} else if (this->t1->kind == aVar1 && this->t2->kind == aPlus1) {
		ASTTerm1_Plus* temp = (ASTTerm1_Plus*) this->t2;
		// y = xi -> Xy = Xx i
		if (temp->t->kind == aVar1) {
			y = new ASTTerm2_Var2(((ASTTerm1_Var1*)this->t1)->getVar(), Pos());
			x = new ASTTerm2_Var2(((ASTTerm1_Var1*)temp->t)->getVar(), Pos());
			ASTTerm2_Plus* plus = new ASTTerm2_Plus(x, temp->n, Pos());
			return new ASTForm_Equal2(y, plus, Pos());
		// y = ti -> ex z: z = t & y = zi
		} else {
			unsigned int z = symbolTable.insertFresh(Varname1);
			ASTTerm1_Var1* zVar = new ASTTerm1_Var1(z, Pos());
			leftEqual = new ASTForm_Equal1(zVar, temp->t, Pos());
			rightEqual = new ASTForm_Equal1(this->t1, new ASTTerm1_Plus(zVar, temp->n, Pos()), Pos());
			conjuction = new ASTForm_And(leftEqual, rightEqual, Pos());
			return (new ASTForm_Ex1(0, new IdentList(z), conjuction, Pos()))->flatten();
		}
	// xi = yi -> ex z: z = xi & z = yj
	} else if (this->t1->kind == aPlus1 && this->t2->kind == aPlus1) {
		unsigned int z = symbolTable.insertFresh(Varname1);
		ASTTerm1_Var1* zVar = new ASTTerm1_Var1(z, Pos());
		leftEqual = new ASTForm_Equal1(zVar, this->t1, Pos());
		rightEqual = new ASTForm_Equal1(zVar, this->t2, Pos());
		conjuction = new ASTForm_And(leftEqual, rightEqual, Pos());
		return (new ASTForm_Ex2(0, new IdentList(z), conjuction, Pos()))->flatten();
	// x = e ???
	} else if(this->t1->kind == aVar1 && this->t2->kind == aInt) {
#ifdef SMART_FLATTEN
		// smart flattening will deal with this during construction of automaton
#else
		// other ints are specially handled
		unsigned int val = ((ASTTerm1_Int*) this->t2)->value();
		if(val == 0) {
			x = new ASTTerm2_Var2(((ASTTerm1_Var1*)this->t1)->getVar(), Pos());
			ASTList* set = new ASTList();
			set->push_back((AST*) new ASTTerm1_Int(0, Pos()));
			ASTTerm2_Set* e = new ASTTerm2_Set(set, Pos());
			return new ASTForm_Equal2(x, e, Pos());
		} else {
			ASTTerm1_Var1* z = generateFreshFirstOrder();
			ASTForm_Equal1* zLess = new ASTForm_Equal1(z, new ASTTerm1_Int(val-1, Pos()), Pos());
			ASTForm_Equal1* plus = new ASTForm_Equal1(this->t1, new ASTTerm1_Plus(z, 1, Pos()), Pos());
			ASTForm_And* conjuction = new ASTForm_And(zLess->flatten(), plus->flatten(), Pos());
			return new ASTForm_Ex2(0, new IdentList(z->getVar()), conjuction, Pos());
		}
#endif
	// TODO: not fully tested
	} else if(this->t1->kind == aInt && this->t2->kind == aVar1) {
		ASTTerm1* temp;
		temp = this->t2;
		this->t2 = this->t1;
		this->t1 = temp;
		return this->flatten();
	} else {
		std::cerr << "Other forms of Equal1 not Implemented yet!\n";
		std::cerr << "Dumping formula: ";
		this->dump();
		std::cerr << "\n";
	}
	return this;
}

/**
 * Flattens formula to second-order variables and restricted syntax so it uses
 * only certain atomic formulae
 *
 * x ~= y  -> not x = y
 *
 * @return: flattened formula
 */
ASTForm* ASTForm_NotEqual1::flatten() {
	ASTForm_Equal1* eq = new ASTForm_Equal1(this->t1, this->t2, Pos());
	return new ASTForm_Not(eq->flatten(), Pos());
}

/**
 * Flattens formula so it only uses certain atomic formulae
 *
 * X ~= Y -> not X = Y
 *
 * @return: flattened formula
 */
ASTForm* ASTForm_NotEqual2::flatten() {
	ASTForm_Equal2* eq = new ASTForm_Equal2(this->T1, this->T2, Pos());
	return new ASTForm_Not(eq->flatten(), Pos());
}

/*
 * X = {1, 2, 3...} -> 1 \in X & ....
 */
ASTForm* ASTForm_Equal2::flatten() {
	if(this->T2->kind == aSet) {
		ASTList* vars = ((ASTTerm2_Set*)this->T2)->elements;
		ASTForm* formula = new ASTForm_True(Pos());
		for(auto var = vars->begin(); var != vars->end(); ++var) {
			ASTForm_In* newIn = new ASTForm_In((ASTTerm1*)*var, this->T1, Pos());
			formula = new ASTForm_And(formula, newIn, Pos());
		}
		return formula;
	} else {
		return this;
	}
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

ASTForm* substituteFreshLess(ASTTerm1* leftTerm, ASTTerm1* rightTerm, bool substituteLeft) {
	ASTTerm1_Var1* z;
	ASTForm_Ex1* exists;
	ASTForm_And* conjuction;
	ASTForm* left;
	ASTForm* right;

	z = generateFreshFirstOrder();
	if(substituteLeft) {
		left = new ASTForm_Equal1(z, leftTerm,Pos());
		right = new ASTForm_Less(z, rightTerm, Pos());
	} else {
		left = new ASTForm_Equal1(z, rightTerm,Pos());
		right = new ASTForm_Less(leftTerm, z, Pos());
	}
	conjuction = new ASTForm_And(left, right, Pos());

	ASTForm_Ex1* newFormula = new ASTForm_Ex1(0, new IdentList(z->getVar()), conjuction, Pos());
	return newFormula->flatten();
}

/**
 * Flattens formula to second-order variables and restricted syntax so it uses
 * only certain atomic formulae
 *
 * x < y  -> x ~= y & x <= y
 *
 * @return: flattened formula
 */
ASTForm* ASTForm_Less::flatten() {
#ifdef SMART_FLATTEN
	if (this->t1->kind != aVar1) {
		return substituteFreshLess(this->t1, this->t2, true);
	} else if (this->t2->kind != aVar1) {
		return substituteFreshLess(this->t1, this->t2, false);
	} else {
		return this;
	}
#else
	ASTForm_NotEqual1* leftSide = new ASTForm_NotEqual1(this->t1, this->t2, Pos());
	ASTForm_LessEq* rightSide = new ASTForm_LessEq(this->t1, this->t2, Pos());
	ASTForm_And* conjuction = new ASTForm_And(leftSide, rightSide, Pos());
	return conjuction->flatten();
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

ASTForm* substituteFreshLessEq(ASTTerm1* leftTerm, ASTTerm1* rightTerm, bool substituteLeft) {
	ASTTerm1_Var1* z;
	ASTForm_Ex1* exists;
	ASTForm_And* conjuction;
	ASTForm* left;
	ASTForm* right;

	z = generateFreshFirstOrder();
	if(substituteLeft) {
		left = new ASTForm_Equal1(z, leftTerm,Pos());
		right = new ASTForm_LessEq(z, rightTerm, Pos());
	} else {
		left = new ASTForm_Equal1(z, rightTerm,Pos());
		right = new ASTForm_LessEq(leftTerm, z, Pos());
	}
	conjuction = new ASTForm_And(left, right, Pos());

	ASTForm_Ex1* newFormula = new ASTForm_Ex1(0, new IdentList(z->getVar()), conjuction, Pos());
	return newFormula->flatten();
}

/**
 * Flattens formula to second-order variables and restricted syntax so it uses
 * only certain atomic formulae. Is variable according to the ws1s and ws2s
 *
 * x <= y  -> forall X: (y in X & (forall Z: z1 in X | z2 in X) => z in X) => x in X
 *
 * @return: flattened formula
 */
ASTForm* ASTForm_LessEq::flatten() {
	if (this->t1->kind != aVar1) {
		return substituteFreshLessEq(this->t1, this->t2, true);
	} else if (this->t2->kind != aVar1) {
		return substituteFreshLessEq(this->t1, this->t2, false);
	} else {
#ifdef SMART_FLATTEN
		return this;
#else
		ASTTerm2_Var2* X = generateFreshSecondOrder();
		ASTTerm1_Var1* z = generateFreshFirstOrder();

		// construction of innerDisjunction (forall Z: z1 in X | z2 in X)
		ASTForm_All1* innerDisjunction;
		if (options.mode != TREE) {
			ASTTerm1_Plus* z1 = new ASTTerm1_Plus(z, 1, Pos());
			ASTForm_In* z1InX = new ASTForm_In(z1, X, Pos());
			innerDisjunction = new ASTForm_All1(0, new IdentList(z->getVar()),z1InX , Pos());
		// TODO: WS2S
		} else {

		}

		// Construction of innerImplication innerDisjuction => z in X
		ASTForm_Impl* innerImplication;
		ASTForm_In* zInX = new ASTForm_In(z, X, Pos());
		innerImplication = new ASTForm_Impl(innerDisjunction, zInX, Pos());

		// Construction of innerConjuction (y in X) & innerImplication
		ASTForm_And* innerConjuction;
		ASTForm_In* yInX = new ASTForm_In(this->t2, X, Pos());
		innerConjuction = new ASTForm_And(yInX, innerImplication, Pos());

		// Construction outer Implication (innerConjuction) => x in X
		ASTForm_Impl* outerImplication;
		ASTForm_In* xInX = new ASTForm_In(this->t1, X, Pos());
		outerImplication = new ASTForm_Impl(innerConjuction, xInX, Pos());

		ASTForm_All2* newFormula = new ASTForm_All2(0, new IdentList(X->getVar()), outerImplication->flatten() , Pos());
		return newFormula->flatten();
#endif
	}
	return this;
}

/**
 * Substitutes xi in X to new fresh variable with in predicate so
 * final formula is ex z: z = yi & z in X
 *
 * @param leftTerm: left side of In (yi)
 * @param rightTerm: right side of In (X)
 * @return flattened and freshened formula
 */

ASTForm* substituteFreshIn(ASTTerm1* leftTerm, ASTTerm2* rightTerm) {
	ASTTerm1_Var1* z;
	ASTForm_Ex1* exists;
	ASTForm_And* conjuction;
	ASTForm* left;
	ASTForm* right;

	z = generateFreshFirstOrder();
	left = new ASTForm_Equal1(z, leftTerm,Pos());
	right = new ASTForm_In(z, rightTerm, Pos());
	conjuction = new ASTForm_And(left, right, Pos());

	return new ASTForm_Ex2(0, new IdentList(z->getVar()), conjuction->flatten(), Pos());
}

/**
 * Flattens formula to second-order variables and restricted sytnax.
 *
 * t in X -> Xy subseteq X
 *
 * @return: flattened formula
 */
ASTForm* ASTForm_In::flatten() {
	if (this->t1->kind != aVar1) {
#ifdef SMART_FLATTEN
		if(this->t1->kind == aInt) {
			return this;
		} else {
			return substituteFreshIn(this->t1, this->T2);
		}
#else
		return substituteFreshIn(this->t1, this->T2);
#endif
	} else {
#ifdef SMART_FLATTEN
		//inFirstOrder.insert(((ASTTerm1_Var1*)this->t1)->n);
		return this;
#else
		ASTTerm2_Var2 *secondOrderX = new ASTTerm2_Var2(((ASTTerm1_Var1*)this->t1)->n, Pos());
		ASTForm_Sub* subX = new ASTForm_Sub(secondOrderX, this->T2, Pos());
		ASTForm_FirstOrder *singleton = new ASTForm_FirstOrder(new ASTTerm1_Var1(((ASTTerm1_Var1*)this->t1)->n, Pos()), Pos());
		inFirstOrder.insert(((ASTTerm1_Var1*)this->t1)->n);
		return new ASTForm_And(singleton, subX, Pos());
#endif
	}
	return this;
}

/**
 * Flattens formula to second-order variables and restricted sytnax.
 *
 * t notin X -> not Xy subseteq X
 *
 * @return: flattened formula
 */
ASTForm* ASTForm_Notin::flatten() {
	ASTForm_In *newIn = new ASTForm_In(this->t1, this->T2, Pos());
	return new ASTForm_Not(newIn->flatten(), Pos());
}

/**
 * Flattens formula to second-order
 *
 * @return: flattened formula
 */
ASTForm* ASTForm_Ex1::flatten() {
	this->f = this->f->flatten();

	Ident* it = this->vl->begin();
	ASTForm_And* conjuction;
	ASTForm* restrictedFormula = new ASTForm_FirstOrder(new ASTTerm1_Var1(*it, Pos()), Pos());
	inFirstOrder.insert(*it);
	++it;
	while(it != this->vl->end()) {
		ASTForm_FirstOrder *singleton = new ASTForm_FirstOrder(new ASTTerm1_Var1(*it, Pos()), Pos());
		inFirstOrder.insert(*it);
		restrictedFormula = new ASTForm_And(restrictedFormula, singleton, Pos());
		++it;
	}
	restrictedFormula = new ASTForm_And(restrictedFormula, this->f, Pos());

	return new ASTForm_Ex2(this->ul, this->vl, restrictedFormula, this->pos);
}

/**
 * Flattens formula to second-order
 *
 * @return: flattened formula
 */
ASTForm* ASTForm_All1::flatten() {
	this->f = this->f->flatten();

	Ident* it = this->vl->begin();
	ASTForm_Impl* implication;
	ASTForm* restrictedFormula = new ASTForm_FirstOrder(new ASTTerm1_Var1(*it, Pos()), Pos());
	inFirstOrder.insert(*it);
	++it;

	while(it != this->vl->end()) {
		ASTForm_FirstOrder *singleton = new ASTForm_FirstOrder(new ASTTerm1_Var1(*it, Pos()), Pos());
		inFirstOrder.insert(*it);
		restrictedFormula = new ASTForm_Impl(restrictedFormula, singleton, Pos());
		++it;
	}
	restrictedFormula = new ASTForm_Impl(restrictedFormula, this->f, Pos());

	return new ASTForm_All2(this->ul, this->vl, restrictedFormula, this->pos);
}

/**
 * Transformation of Not
 *
 * @return: flattened formula
 */
ASTForm* ASTForm_Not::flatten() {
	f = f->flatten();
	return this;
}

/**
 * Generic transformation for wide range of formulae
 *
 * @return: flattened formula
 */
ASTForm* ASTForm_f::flatten() {
	f = f->flatten();
	return this;
}

/**
 * Generic transformation for wide range of formulae
 *
 * @return: flattened formula
 */
ASTForm* ASTForm_ff::flatten() {
    f1 = f1->flatten();
    f2 = f2->flatten();
    return this;
}

/**
 * Generic transformation for wide range of formulae
 *
 * @return: flattened formula
 */
ASTForm* ASTForm_vf::flatten() {
    f = f->flatten();
    return this;
}

/**
 * Generic transformation for wide range of formulae
 *
 * @return: flattened formula
 */
ASTForm* ASTForm_uvf::flatten() {
    f = f->flatten();
    return this;
}

/**
 * Flatten the conjunction to optimize AND(True, phi), that is appearing
 * due to the parsing approach of the MONA
 *
 * @return: flattened formula
 */
ASTForm* ASTForm_And::flatten() {
	f1 = f1->flatten();
	f2 = f2->flatten();

	if(f1->kind == aTrue) {
		return f2;
	} else if (f2->kind == aTrue) {
		return f1;
	} else {
		return this;
	}
}

/**
 * Unfolds formal parameters to real parameters
 *
 * @param fParams: list of formal parameters
 * @param rParams: list of real parameters
 * @return: unfolded macro
 */
ASTForm* ASTForm_f::unfoldMacro(IdentList* fParams, ASTList* rParams) {
	f = f->unfoldMacro(fParams, rParams);
	return this;
}

/**
 * Unfolds formal parameters to real parameters
 *
 * @param fParams: list of formal parameters
 * @param rParams: list of real parameters
 * @return: unfolded macro
 */
ASTForm* ASTForm_ff::unfoldMacro(IdentList* fParams, ASTList* rParams) {
	f1 = f1->unfoldMacro(fParams, rParams);
	f2 = f2->unfoldMacro(fParams, rParams);
	return this;
}

/**
 * Unfolds formal parameters to real parameters
 *
 * @param fParams: list of formal parameters
 * @param rParams: list of real parameters
 * @return: unfolded macro
 */
ASTForm* ASTForm_uvf::unfoldMacro(IdentList* fParams, ASTList* rParams) {
	IdentList *ffParams = fParams->copy();
	ASTList *rrParams = (ASTList*) rParams->copy();

	for(Ident* iter = this->vl->begin(); iter != this->vl->end(); ++iter) {
		if(this->kind == aAll1 | this->kind == aEx1) {
			ASTTerm1_Var1* newVar = generateFreshFirstOrder();
			ffParams->push_back(*iter);
			rrParams->push_back(newVar);
			*iter = newVar->n;
		} else {
			ASTTerm2_Var2* newVar = generateFreshSecondOrder();
			ffParams->push_back(*iter);
			rrParams->push_back(newVar);
			*iter = newVar->n;
		}
	}

	f = f->unfoldMacro(ffParams, rrParams);
	return this;
}

/**
 * Unfolds formal parameters to real parameters
 *
 * @param fParams: list of formal parameters
 * @param rParams: list of real parameters
 * @return: unfolded macro
 */
ASTForm* ASTForm_vf::unfoldMacro(IdentList* fParams, ASTList* rParams) {
	IdentList *ffParams = fParams->copy();
	ASTList *rrParams = (ASTList*) rParams->copy();

	for(Ident* iter = this->vl->begin(); iter != this->vl->end(); ++iter) {
		if(this->kind == aAll1 | this->kind == aEx1) {
			ASTTerm1_Var1* newVar = generateFreshFirstOrder();
			ffParams->push_back(*iter);
			rrParams->push_back(newVar);
			*iter = newVar->n;
		} else {
			ASTTerm2_Var2* newVar = generateFreshSecondOrder();
			ffParams->push_back(*iter);
			rrParams->push_back(newVar);
			*iter = newVar->n;
		}
	}

	f = f->unfoldMacro(ffParams, rrParams);

	return this;
}

/**
 * Unfolds formal parameters to real parameters
 *
 * @param fParams: list of formal parameters
 * @param rParams: list of real parameters
 * @return: unfolded macro
 */
ASTForm* ASTForm_tT::unfoldMacro(IdentList* fParams, ASTList* rParams) {
	t1 = t1->unfoldMacro(fParams, rParams);
	T2 = T2->unfoldMacro(fParams, rParams);
	return this;
}

/**
 * Unfolds formal parameters to real parameters
 *
 * @param fParams: list of formal parameters
 * @param rParams: list of real parameters
 * @return: unfolded macro
 */
ASTForm* ASTForm_tt::unfoldMacro(IdentList* fParams, ASTList* rParams) {
	t1 = t1->unfoldMacro(fParams, rParams);
	t2 = t2->unfoldMacro(fParams, rParams);
	return this;
}

/**
 * Unfolds formal parameters to real parameters
 *
 * @param fParams: list of formal parameters
 * @param rParams: list of real parameters
 * @return: unfolded macro
 */
ASTForm* ASTForm_TT::unfoldMacro(IdentList *fParams, ASTList* rParams) {
	T1 = T1->unfoldMacro(fParams, rParams);
	T2 = T2->unfoldMacro(fParams, rParams);
	return this;
}

/**
 * Unfolds formal parameters to real parameters
 *
 * @param fParams: list of formal parameters
 * @param rParams: list of real parameters
 * @return: unfolded macro
 */
ASTTerm2* ASTTerm2_TT::unfoldMacro(IdentList* fParams, ASTList* rParams) {
	T1 = T1->unfoldMacro(fParams, rParams);
	T2 = T2->unfoldMacro(fParams, rParams);
	return this;
}

/**
 * Unfolds formal parameters to real parameters
 *
 * @param fParams: list of formal parameters
 * @param rParams: list of real parameters
 * @return: unfolded macro
 */
ASTTerm2* ASTTerm2_Tn::unfoldMacro(IdentList *fParams, ASTList* rParams) {
	T = T->unfoldMacro(fParams, rParams);
	return this;
}

/**
 * Unfolds formal parameters to real parameters
 *
 * @param fParams: list of formal parameters
 * @param rParams: list of real parameters
 * @return: unfolded macro
 */
ASTForm* ASTForm_T::unfoldMacro(IdentList *fParams, ASTList* rParams) {
	T = T->unfoldMacro(fParams, rParams);
	return this;
}

/**
 * Unfolds formal parameters to real parameters
 *
 * @param fParams: list of formal parameters
 * @param rParams: list of real parameters
 * @return: unfolded macro
 */
ASTTerm1* ASTTerm1_tn::unfoldMacro(IdentList *fParams, ASTList* rParams) {
	t = t->unfoldMacro(fParams, rParams);
	return this;
}

/**
 * Unfolds formal parameters to real parameters
 *
 * @param fParams: list of formal parameters
 * @param rParams: list of real parameters
 * @return: unfolded macro
 */
ASTForm* ASTForm_Not::unfoldMacro(IdentList* fParams, ASTList* rParams) {
	f = f->unfoldMacro(fParams, rParams);
	return this;
}

/**
 * Unfolds formal parameters to real parameters
 *
 * @param fParams: list of formal parameters
 * @param rParams: list of real parameters
 * @return: unfolded macro
 */
ASTTerm1* ASTTerm1_Var1::unfoldMacro(IdentList* fParams, ASTList* rParams) {
	int index = fParams->index(this->n);

	if (index != -1) {
		return (ASTTerm1*) rParams->get(index);
	} else {
		return (ASTTerm1*) this;
	}
}

/**
 * Unfolds formal parameters to real parameters
 *
 * @param fParams: list of formal parameters
 * @param rParams: list of real parameters
 * @return: unfolded macro
 */
ASTTerm2* ASTTerm2_Var2::unfoldMacro(IdentList* fParams, ASTList* rParams) {
	int index = fParams->index(this->n);

	if (index != -1) {
		return (ASTTerm2*) rParams->get(index);
	} else {
		return (ASTTerm2*) this;
	}
}

/**
 * Unfolds formal parameters to real parameters
 *
 * @param fParams: list of formal parameters
 * @param rParams: list of real parameters
 * @return: unfolded macro
 */
ASTForm* ASTForm_Var0::unfoldMacro(IdentList* fParams, ASTList* rParams) {
	int index = fParams->index(this->n);
	if (index != -1) {
		ASTForm* formula = ((ASTForm*) rParams->get(index));
		return formula->clone();
	} else {
		return (ASTForm*) this;
	}
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

	return unfoldedFormula->flatten();
}

/**
 * Unfolds formal parameters to real parameters in called function
 *
 * @param fParams: list of formal parameters
 * @param rParams: list of real parameters
 * @return: unfolded macro
 */
ASTForm* ASTForm_Call::unfoldMacro(IdentList* fParams, ASTList* rParams) {
	PredLibEntry* called = predicateLib.lookup(this->n);
	ASTList* realParams = (ASTList*) this->args->copy();

	for(AST** ast = realParams->begin(); ast != realParams->end(); ++ast) {
		(*ast) = (*ast)->unfoldMacro(fParams, rParams);
	}

	ASTForm* clonnedFormula = (called->ast)->clone();
	ASTForm* unfoldedFormula = clonnedFormula->unfoldMacro(called->formals, realParams);

	return unfoldedFormula;
}

/**
 * Unfolds the macro according to the real arguments
 *
 * @return: flattened formula
 */
ASTForm* ASTForm_Call::flatten() {
	int calledNumber = this->n;

	PredLibEntry* called = predicateLib.lookup(calledNumber);

	/* So far, we will treat predicate and macro the same */
	if (!called->isMacro) {
		return unfoldFormula(called, this->args);
	} else {
		return unfoldFormula(called, this->args);
	}
}
