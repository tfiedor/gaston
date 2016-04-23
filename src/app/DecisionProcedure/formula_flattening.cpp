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
#include "visitors/restricters/Flattener.h"
#include "visitors/restricters/PredicateUnfolder.h"
#include "environment.hh"
#include <vector>

using std::cout;

extern SymbolTable symbolTable;
extern Options options;
extern PredicateLib predicateLib;

void update_restriction(unsigned int oldVar, unsigned int newVar, IdentList* ffParams, ASTList* rrParams) {
	ASTForm *restriction = symbolTable.lookupRestriction(oldVar);
	if (restriction != nullptr) {
		restriction = restriction->clone()->unfoldMacro(ffParams, rrParams);
		symbolTable.updateRestriction(newVar, restriction);
	}
}

template<class Var, Var* (*Generator)(void)>
size_t generate_variable(Ident* iter, IdentList* ffParams, ASTList* rrParams) {
	Var* newVar = Generator();
	ffParams->push_back(*iter);
	rrParams->push_back(newVar);

	update_restriction(*iter, newVar->n, ffParams, rrParams);
	return newVar->n;
}
/**
 * Unfolds formal parameters to real parameters in called function
 *
 * @param fParams: list of formal parameters
 * @param rParams: list of real parameters
 * @return: unfolded macro
 */
ASTForm* unfoldCall(ASTForm* form, IdentList* fParams, ASTList* rParams) {
	assert(form->kind == aCall);
	ASTForm_Call* callForm = static_cast<ASTForm_Call*>(form);

	PredLibEntry* called = predicateLib.lookup(callForm->n);
	ASTList* realParams = static_cast<ASTList*>(callForm->args->copy());

	for(AST** ast = realParams->begin(); ast != realParams->end(); ++ast) {
		(*ast) = (*ast)->unfoldMacro(fParams, rParams);
	}

	ASTForm* clonnedFormula = (called->ast)->clone();
	ASTForm* unfoldedFormula = _unfoldCore(clonnedFormula, called->formals, realParams);
	// Fixme: this is segfaulting something i guess? 
	//delete realParams;
	callForm->detach();
	//delete callForm;

	return unfoldedFormula;
}

ASTForm* _unfoldCore(ASTForm* form, IdentList* fParams, ASTList* rParams) {
	if(form->kind == aCall) {
		return unfoldCall(form, fParams, rParams);
	} else {
		return form->unfoldMacro(fParams, rParams);
	}
}

template<class TermClass, ASTKind varKind>
TermClass* unfoldOrderTerm(TermClass* term, IdentList* fParams, ASTList* rParams) {
	if(term->kind == varKind) {
		TermClass* temp = term->unfoldMacro(fParams, rParams);
		if(temp != term) {
			delete term;
		}
		return temp;
	} else {
		return term->unfoldMacro(fParams, rParams);
	}
}

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

ASTForm_Var0* generateFreshZeroOrder() {
	unsigned int z;
	z = symbolTable.insertFresh(Varname0);

	return new ASTForm_Var0(z, Pos());
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
			if (symbolTable.lookupType(*it) == Varname1) {
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
	f = _unfoldCore(f, fParams, rParams);
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
	f1 = _unfoldCore(f1, fParams, rParams);
	f2 = _unfoldCore(f2, fParams, rParams);
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
		assert(iter != nullptr);
		if(this->kind == aAll1 || this->kind == aEx1) {
			*iter = generate_variable<ASTTerm1_Var1, generateFreshFirstOrder>(iter, ffParams, rrParams);
		} else {
			assert(this->kind != aAll0 && this->kind != aEx0);
			*iter = generate_variable<ASTTerm2_Var2, generateFreshSecondOrder>(iter, ffParams, rrParams);
		}
	}

	f = _unfoldCore(f, ffParams, rrParams);

	// TODO: Refactor this maybe
	for(Ident* iter = this->vl->begin(); iter != this->vl->end(); ++iter) {
		AST* temp = rrParams->pop_back();
		delete temp;
	}
	while(!rrParams->empty())
		rrParams->pop_back();
	delete rrParams;
	delete ffParams;
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
		if (this->kind == aAll0 || this->kind == aEx0) {
			*iter = generate_variable<ASTForm_Var0, generateFreshZeroOrder>(iter, ffParams, rrParams);
		} else if(this->kind == aAll1 || this->kind == aEx1) {
			*iter = generate_variable<ASTTerm1_Var1, generateFreshFirstOrder>(iter, ffParams, rrParams);
		} else {
			assert(this->kind == aAll2 || this->kind == aEx2);
			*iter = generate_variable<ASTTerm2_Var2, generateFreshSecondOrder>(iter, ffParams, rrParams);
		}
	}

	f = _unfoldCore(f, ffParams, rrParams);
	for(Ident* iter = this->vl->begin(); iter != this->vl->end(); ++iter) {
		AST* temp = rrParams->pop_back();
		delete temp;
	}
	while(!rrParams->empty())
		rrParams->pop_back();
	delete rrParams;
	delete ffParams;

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
	t1 = unfoldOrderTerm<ASTTerm1, aVar1>(t1, fParams, rParams);
	T2 = unfoldOrderTerm<ASTTerm2, aVar2>(T2, fParams, rParams);
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
	t1 = unfoldOrderTerm<ASTTerm1, aVar1>(t1, fParams, rParams);
	t2 = unfoldOrderTerm<ASTTerm1, aVar1>(t2, fParams, rParams);
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
	T1 = unfoldOrderTerm<ASTTerm2, aVar2>(T1, fParams, rParams);
	T2 = unfoldOrderTerm<ASTTerm2, aVar2>(T2, fParams, rParams);
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
	T1 = unfoldOrderTerm<ASTTerm2, aVar2>(T1, fParams, rParams);
	T2 = unfoldOrderTerm<ASTTerm2, aVar2>(T2, fParams, rParams);
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
	T = unfoldOrderTerm<ASTTerm2, aVar2>(T, fParams, rParams);
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
	T = unfoldOrderTerm<ASTTerm2, aVar2>(T, fParams, rParams);
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
	t = unfoldOrderTerm<ASTTerm1, aVar1>(t, fParams, rParams);
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
	f = _unfoldCore(f, fParams, rParams);
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
		return reinterpret_cast<ASTTerm1*>(rParams->get(index))->clone();
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
		return reinterpret_cast<ASTTerm2*>(rParams->get(index))->clone();
	} else {
		return (ASTTerm2*) this;
	}
}

ASTForm* ASTForm_Var0::unfoldMacro(IdentList* fParams, ASTList* rParams) {
	int index = fParams->index(this->n);

	if(index != -1) {
		return reinterpret_cast<ASTForm*>(rParams->get(index))->clone();
	} else {
		return reinterpret_cast<ASTForm*>(this);
	}
}

ASTForm* ASTForm_Call::unfoldMacro(IdentList *, ASTList *) {
	assert(false);
	return nullptr;
}