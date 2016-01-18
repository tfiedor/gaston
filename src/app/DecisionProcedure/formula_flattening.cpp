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
#include "visitors/Flattener.h"
#include "environment.hh"
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
	Flattener f_visitor;
	return static_cast<ASTForm*>(this->accept(f_visitor));
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
 * Unfolds formal parameters to real parameters in called function
 *
 * @param fParams: list of formal parameters
 * @param rParams: list of real parameters
 * @return: unfolded macro
 */
ASTForm* ASTForm_Call::unfoldMacro(IdentList* fParams, ASTList* rParams) {
	PredLibEntry* called = predicateLib.lookup(this->n);
	ASTList* realParams = static_cast<ASTList*>(this->args->copy());

	for(AST** ast = realParams->begin(); ast != realParams->end(); ++ast) {
		(*ast) = (*ast)->unfoldMacro(fParams, rParams);
	}

	ASTForm* clonnedFormula = (called->ast)->clone();
	ASTForm* unfoldedFormula = clonnedFormula->unfoldMacro(called->formals, realParams);

	return unfoldedFormula;
}