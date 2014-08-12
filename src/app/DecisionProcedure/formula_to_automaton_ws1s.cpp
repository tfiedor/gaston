/*****************************************************************************
 *  dWiNA - Deciding WSkS using non-deterministic automata
 *
 *  Copyright (c) 2014  Tomas Fiedor <xfiedo01@stud.fit.vutbr.cz>
 *
 *  Description:
 *    Conversion of formula to automaton
 *
 *****************************************************************************/

#include "../Frontend/ast.h"
#include "../Frontend/symboltable.h"

#include <cstring>
#include <list>
#include <algorithm>

// VATA headers
#include <vata/bdd_bu_tree_aut.hh>
#include <vata/parsing/timbuk_parser.hh>
#include <vata/serialization/timbuk_serializer.hh>
#include <vata/util/binary_relation.hh>

#include "automata.hh"

using std::cout;

extern SymbolTable symbolTable;

using Automaton = VATA::BDDBottomUpTreeAut;

/**
 * Constructs automaton for unary automaton True
 *
 * @param[out] trueAutomaton: created automaton
 * @param doComplement: whether automaton should be complemented
 */
void ASTForm_True::toUnaryAutomaton(Automaton &trueAutomaton, bool doComplement) {
    // (X^k) -> q0
	addUniversalTransition(trueAutomaton, Automaton::StateTuple(), 0);

    // q0 -(X^k)-> q0
	addUniversalTransition(trueAutomaton, Automaton::StateTuple({0}), 0);

    // Final state q0
	setFinalState(trueAutomaton, doComplement, 0);
}

/**
 * Constructs automaton for unary automaton False
 *
 * @param[out] falseAutomaton: created automaton
 * @param doComplement: whether automaton should be complemented
 */
void ASTForm_False::toUnaryAutomaton(Automaton &falseAutomaton, bool doComplement) {
	// (X^k) -> q0
	addUniversalTransition(falseAutomaton, Automaton::StateTuple(), 0);

	// q0 -(X^k)-> q0
	addUniversalTransition(falseAutomaton, Automaton::StateTuple({0}), 0);

	// no final states
	setNonFinalState(falseAutomaton, doComplement, 0);
}

/**
 * Constructs automaton for formula not phi
 *
 * Does complementation of states. Since negations should be thanks to
 * formula being conversed to exPNF form, so determinization does not need
 * to take place
 *
 * @param[out] notAutomaton: created automaton
 * @param doComplement: whether automaton should be complemented
 */
void ASTForm_Not::toUnaryAutomaton(Automaton &notAutomaton, bool doComplement) {
	// Inner formula is first conversed to unary automaton
	this->f->toUnaryAutomaton(notAutomaton, true);
}

/**
 * Constructs automaton for formula phi and psi
 *
 * First converts formulae phi and psi to automatons and then does a automata
 * product to compute the final automaton.
 *
 * @param[out] andAutomaton: created automaton
 * @param doComplement: whether automaton should be complemented
 */
void ASTForm_And::toUnaryAutomaton(Automaton &andAutomaton, bool doComplement) {
	// Inner formulas are first conversed to unary automatons
	Automaton left, right;
	this->f1->toUnaryAutomaton(left, doComplement);
	this->f2->toUnaryAutomaton(right, doComplement);

	andAutomaton = Automaton::Intersection(left, right);
}

/**
 * Constructs automaton for formula phi or psi
 *
 * First converts formulae phi or psi to automatons and then does a automata
 * union to compute the final automaton.
 *
 * @param[out] orAutomaton: created automaton
 * @param doComplement: whether automaton should be complemented
 */
void ASTForm_Or::toUnaryAutomaton(Automaton &orAutomaton, bool doComplement) {
	// Inner formulas are first conversed to unary automatons
	Automaton left, right;
	this->f1->toUnaryAutomaton(left, doComplement);
	this->f2->toUnaryAutomaton(right, doComplement);

	orAutomaton = Automaton::Union(left, right);
}

/**
 * Constructs automaton for atomic formula T1 = T2, according to its structure:
 *  1) X = Y1
 *  2) T1 = T2
 *  3) X = e
 *
 *  Constructs template automaton, that does acceptance of the formula
 *
 *  @return Automaton corresponding to the formula phi or psi
 */
void ASTForm_Equal2::toUnaryAutomaton(Automaton &aut, bool doComplement) {
	// 1) X = Y1
	if(this->T1->kind == aVar2 && this->T2->kind == aPlus2) {
		ASTTerm2_Var2* YTerm = (ASTTerm2_Var2*) this->T1;
		unsigned int Y = YTerm->n;
		ASTTerm2_Var2* XTerm = (ASTTerm2_Var2*) ((ASTTerm2_Plus*) this->T2)->T;
		unsigned int X = XTerm->n;
		// (xxxx) -> q0
		addUniversalTransition(aut, Automaton::StateTuple(), 0);

		// q0 -(x00x)-> q0
		addTransition(aut, Automaton::StateTuple({0}), X, Y, (char *) "00", 0);

		// q0 -(x10x)-> q1
		addTransition(aut, Automaton::StateTuple({0}), X, Y, (char *) "10", 1);

		// q0 -(x01x)-> q3
		addTransition(aut, Automaton::StateTuple({0}), X, Y, (char *) "01", 3);

		// q0 -(x11x)-> q3
		addTransition(aut, Automaton::StateTuple({0}), X, Y, (char *) "11", 3);

		// q1 -(x01x)-> q2
		addTransition(aut, Automaton::StateTuple({1}), X, Y, (char *) "01", 2);

		// q1 -(x00x)-> q3
		addTransition(aut, Automaton::StateTuple({1}), X, Y, (char *) "00", 3);

		// q1 -(x10x)-> q3
		addTransition(aut, Automaton::StateTuple({1}), X, Y, (char *) "10", 3);

		// q1 -(x11x)-> q3
		addTransition(aut, Automaton::StateTuple({1}), X, Y, (char *) "11", 3);

		// q2 -(x00x)-> q2
		addTransition(aut, Automaton::StateTuple({2}), X, Y, (char *) "00", 2);

		// q2 -(x01x)-> q3
		addTransition(aut, Automaton::StateTuple({2}), X, Y, (char *) "01", 3);

		// q2 -(x10x)-> q3
		addTransition(aut, Automaton::StateTuple({2}), X, Y, (char *) "10", 3);

		// q2 -(x11x)-> q3
		addTransition(aut, Automaton::StateTuple({2}), X, Y, (char *) "11", 3);

		// set q2 final
		setNonFinalState(aut, doComplement, 0);
		setNonFinalState(aut, doComplement, 1);
		setFinalState(aut, doComplement, 2);
		setNonFinalState(aut, doComplement, 3);

	// 2) T1 = T2
	} else if (this->T1->kind == aVar2 && this->T2->kind == aVar2) {
		ASTTerm2_Var2* T1Term = (ASTTerm2_Var2*) this->T1;
		unsigned int T1 = T1Term->n;
		ASTTerm2_Var2* T2Term = (ASTTerm2_Var2*) this->T2;
		unsigned int T2 = T2Term->n;

		// (xxxx) -> q0
		addUniversalTransition(aut, Automaton::StateTuple(), 0);

		// q0 -(x00x)-> q0
		addTransition(aut, Automaton::StateTuple({0}), T1, T2, (char *) "00", 0);

		// q0 -(x11x)-> q0
		addTransition(aut, Automaton::StateTuple({0}), T1, T2, (char *) "11", 0);

		// q0 -(x01x)-> q1
		addTransition(aut, Automaton::StateTuple({0}), T1, T2, (char *) "01", 1);

		// q0 -(x10x)-> q1
		addTransition(aut, Automaton::StateTuple({0}), T1, T2, (char *) "10", 1);

		// q1 -(xxxx)-> q1
		addUniversalTransition(aut, Automaton::StateTuple({1}), 1);

		// set q0 final
		setFinalState(aut, doComplement, 0);
		setNonFinalState(aut, doComplement, 1);
	// X = {...}
	} else if(this->T2->kind == aSet) {
		ASTTerm2_Var2 *T1Term = (ASTTerm2_Var2*) this->T1;
		unsigned int X = T1Term->n;
		ASTList* vars = ((ASTTerm2_Set*)this->T2)->elements;
		std::list<unsigned int> l;
		for (auto var = vars->begin(); var != vars->end(); ++var) {
			l.push_back((unsigned int)((ASTTerm1_Int*)*var)->value());
		}
		l.sort([](unsigned int a, unsigned int b) {return a < b;});

		unsigned int sink = 0;
		unsigned int current = 1;

		// (xxx)-> q1
		addUniversalTransition(aut, Automaton::StateTuple({}), current);

		while(!l.empty()) {
			unsigned int front = l.front();
			l.pop_front();
			while(current != (front+1)) {
				addTransition(aut, Automaton::StateTuple({current}), X, '0', current+1);
				addTransition(aut, Automaton::StateTuple({current}), X, '1', sink);
				setNonFinalState(aut, doComplement, current++);
			}
			addTransition(aut, Automaton::StateTuple({current}), X, '1', current+1);
			addTransition(aut, Automaton::StateTuple({current}), X, '0', sink);
			setNonFinalState(aut, doComplement, current++);
		}

		setFinalState(aut, doComplement, current);
		addTransition(aut, Automaton::StateTuple({current}), X, '0', current);
		addTransition(aut, Automaton::StateTuple({current}), X, '1', sink);

	// 3) X = e
	// this should be X = {0} hopefully
	} else {
		ASTTerm2_Var2 *XTerm = (ASTTerm2_Var2*) this->T1;
		unsigned int X = XTerm->n;

		// (xxx)-> q0
		addUniversalTransition(aut, Automaton::StateTuple({}), 0);

		// q0 -(x1x)-> q1
		addTransition(aut, Automaton::StateTuple({0}), X, '1', 1);

		// q0 -(x0x)-> q2
		addTransition(aut, Automaton::StateTuple({0}), X, '0', 2);

		// q1 -(xxx)-> q2
		addUniversalTransition(aut, Automaton::StateTuple({1}), 2);

		// q2 -(xxx)-> q2
		addUniversalTransition(aut, Automaton::StateTuple({2}), 2);

		// set q0 final
		setNonFinalState(aut, doComplement, 0);
		setFinalState(aut, doComplement, 1);
		setNonFinalState(aut, doComplement, 2);
	}
}

/**
 * Constructs automaton for atomic formula T1 sub T2
 *
 * @return Automaton corresponding to the formula T1 sub T2
 */
void ASTForm_Sub::toUnaryAutomaton(Automaton &aut, bool doComplement) {
	ASTTerm2_Var2* T1Var = (ASTTerm2_Var2*) this->T1;
	unsigned int T1 = (unsigned int) T1Var->n;

	ASTTerm2_Var2* T2Var = (ASTTerm2_Var2*) this->T2;
	unsigned int T2 = (unsigned int) T2Var->n;
	//  -(xxxx)-> q0
	addUniversalTransition(aut, Automaton::StateTuple({}), 0);

	// q0 -(x00x)-> q0
	addTransition(aut, Automaton::StateTuple({0}), T1, T2, (char *) "00", 0);

	// q0 -(x11x)-> q0
	addTransition(aut, Automaton::StateTuple({0}), T1, T2, (char *) "11", 0);

	// q0 -(x01x)-> q0
	addTransition(aut, Automaton::StateTuple({0}), T1, T2, (char *) "01", 0);

	// q0 -(x10x)-> q1 = sink
	addTransition(aut, Automaton::StateTuple({0}), T1, T2, (char *) "10", 1);

	// q1 -(xxxx)-> q1
	addUniversalTransition(aut, Automaton::StateTuple({1}), 1);

	// final state q0
	setFinalState(aut, doComplement, 0);
	// nonfinal state q1
	setNonFinalState(aut, doComplement, 1);
}

/**
 * Constructs automaton for formula denoting, that set is a singleton
 *
 * @return Automaton corresponding to the formula Singleton(X)
 */
void ASTForm_FirstOrder::toUnaryAutomaton(Automaton &aut, bool doComplement) {
	ASTTerm2_Var2 *XVar = (ASTTerm2_Var2*) this->t;
	unsigned int X = (unsigned int) XVar->n;

	// -(xxx)-> q0
	addUniversalTransition(aut, Automaton::StateTuple({}), 0);

	// q0 -(x0x)-> q0
	addTransition(aut, Automaton::StateTuple({0}), X, '0', 0);

	// q0 -(x1x)-> q1
	addTransition(aut, Automaton::StateTuple({0}), X, '1', 1);

	// q1 -(x0x)-> q1
	addTransition(aut, Automaton::StateTuple({1}), X, '0', 1);

	// q1 -(x1x)-> q2
	addTransition(aut, Automaton::StateTuple({1}), X, '1', 2);

	// q2 -(xxx)-> q2
	addUniversalTransition(aut, Automaton::StateTuple({2}), 2);

	// final state q1
	setNonFinalState(aut, doComplement, 0);
	setFinalState(aut, doComplement, 1);
	setNonFinalState(aut, doComplement, 2);
}

/**
 * Constructs automaton for formula denoting that x is less or equal than yX
 *
 * @return Automaton corresponding to the formula x <= y
 */
void ASTForm_LessEq::toUnaryAutomaton(Automaton &aut, bool doComplement) {
	ASTTerm1_Var1 *xVar = (ASTTerm1_Var1*) this->t1;
	unsigned int x = (unsigned int) xVar->n;
	ASTTerm1_Var1 *yVar = (ASTTerm1_Var1*) this->t2;
	unsigned int y = (unsigned int) yVar->n;

	// -(xxx)-> q0
	addUniversalTransition(aut, Automaton::StateTuple({}), 0);

	// q0 -(x00x)-> q0
	addTransition(aut, Automaton::StateTuple({0}), x, y, (char *) "00", 0);

	// q0 -(x01x)-> q3
	addTransition(aut, Automaton::StateTuple({0}), x, y, (char *) "01", 3);

	// q0 -(x10x)-> q2
	addTransition(aut, Automaton::StateTuple({0}), x, y, (char *) "10", 2);

	// q0 -(x11x)-> q3
	addTransition(aut, Automaton::StateTuple({0}), x, y, (char *) "11", 3);

	// q1 -(x00x)-> q1
	addTransition(aut, Automaton::StateTuple({1}), x, y, (char *) "00", 1);

	// q1 -(x01x)-> q3
	addTransition(aut, Automaton::StateTuple({1}), x, y, (char *) "01", 3);

	// q1 -(x01x)-> q3
	addTransition(aut, Automaton::StateTuple({1}), x, y, (char *) "10", 3);

	// q1 -(x01x)-> q3
	addTransition(aut, Automaton::StateTuple({1}), x, y, (char *) "11", 3);

	// q2 -(x00x)-> q2
	addTransition(aut, Automaton::StateTuple({0}), x, y, (char *) "00", 2);

	// q2 -(x01x)-> q1
	addTransition(aut, Automaton::StateTuple({2}), x, y, (char *) "01", 1);

	// q2 -(x10x)-> q3
	addTransition(aut, Automaton::StateTuple({2}), x, y, (char *) "10", 3);

	// q2 -(x11x)-> q3
	addTransition(aut, Automaton::StateTuple({2}), x, y, (char *) "11", 3);

	// q3 -(xxx)-> q3
	addUniversalTransition(aut, Automaton::StateTuple({3}), 3);

	setNonFinalState(aut, doComplement, 0);
	setFinalState(aut, doComplement, 1);
	setNonFinalState(aut, doComplement, 2);
	setNonFinalState(aut, doComplement, 3);
}

/**
 * Constructs automaton for formula denoting that x is in X
 *
 * @return Automaton corresponding to the formula x in X
 */
void ASTForm_In::toUnaryAutomaton(Automaton &aut, bool doComplement) {
	// int in X
	if(this->t1->kind == aInt) {
		ASTTerm2_Var2 *XVar = (ASTTerm2_Var2*) this->T2;
		unsigned int X = (unsigned int) XVar->n;
		ASTTerm1_Int *iVar = (ASTTerm1_Int*) this->t1;
		unsigned int i = (unsigned int) iVar->n;

		// -(xxx)-> q0
		addUniversalTransition(aut, Automaton::StateTuple({}), 0);
		setNonFinalState(aut, doComplement, 0);

		// j -(xxx)-> j+1
		for(unsigned int j = 0; j != i;) {
			addUniversalTransition(aut, Automaton::StateTuple({j}), j+1);
			setNonFinalState(aut, doComplement, j++);
		}

		//i -(x0x)-> i+2
		addTransition(aut, Automaton::StateTuple({i}), X, '0', i+2);
		setNonFinalState(aut, doComplement, i+2);

		//i -(x1x)-> i+1
		addTransition(aut, Automaton::StateTuple({i}), X, '1', i+1);
		setFinalState(aut, doComplement, i+1);

		//i+1 -(xxx)-> i+1
		addUniversalTransition(aut, Automaton::StateTuple({i+1}), i+1);

		//i+2 -(xxx)-> i+2
		addUniversalTransition(aut, Automaton::StateTuple({i+2}), i+2);

	// x in X
	} else {
		ASTTerm2_Var2 *XVar = (ASTTerm2_Var2*) this->T2;
		unsigned int X = (unsigned int) XVar->n;
		ASTTerm1_Var1 *xVar = (ASTTerm1_Var1*) this->t1;
		unsigned int x = (unsigned int) xVar->n;

		// -(xxx)-> q0
		addUniversalTransition(aut, Automaton::StateTuple({}), 0);

		// q0 -(x00x)-> q0
		addTransition(aut, Automaton::StateTuple({0}), x, X, (char *) "00", 0);

		// q0 -(x01x)-> q0
		addTransition(aut, Automaton::StateTuple({0}), x, X, (char *) "01", 0);

		// q0 -(x11x)-> q1
		addTransition(aut, Automaton::StateTuple({0}), x, X, (char *) "11", 1);

		// q0 -(x10x)-> q2
		addTransition(aut, Automaton::StateTuple({0}), x, X, (char *) "10", 2);

		// q1 -(x00x)-> q1
		addTransition(aut, Automaton::StateTuple({1}), x, X, (char *) "00", 1);

		// q1 -(x01x)-> q1
		addTransition(aut, Automaton::StateTuple({1}), x, X, (char *) "01", 1);

		// q1 -(x10x)-> q2
		addTransition(aut, Automaton::StateTuple({1}), x, X, (char *) "10", 2);

		// q1 -(x11x)-> q2
		addTransition(aut, Automaton::StateTuple({1}), x, X, (char *) "11", 2);

		// q2 -(xXXx)-> q2
		addUniversalTransition(aut, Automaton::StateTuple({2}), 2);

		setNonFinalState(aut, doComplement, 0);
		setFinalState(aut, doComplement, 1);
		setNonFinalState(aut, doComplement, 2);
	}
}
