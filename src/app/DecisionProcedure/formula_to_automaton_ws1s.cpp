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
#include "../Frontend/env.h"
#include "../Frontend/offsets.h"

#include <cstring>
#include <list>
#include <algorithm>

// VATA headers
#include <vata/bdd_bu_tree_aut.hh>
#include <vata/parsing/timbuk_parser.hh>
#include <vata/serialization/timbuk_serializer.hh>
#include <vata/util/binary_relation.hh>

#include "automata.hh"
#include "decision_procedures.hh"
#include "environment.hh"

using std::cout;

extern SymbolTable symbolTable;
extern Options options;
extern Offsets offsets;
extern VarToTrackMap varMap;
extern CodeTable *codeTable;

using Automaton = VATA::BDDBottomUpTreeAut;

void reduce(Automaton &aut) {
	auto aut1 = aut.RemoveUnreachableStates();
	aut = aut1.RemoveUselessStates();
}

/**
 * Constructs automaton for unary automaton True
 *
 * @param[out] trueAutomaton: created automaton
 * @param doComplement: whether automaton should be complemented
 */
void ASTForm_True::toUnaryAutomaton(Automaton &trueAutomaton, bool doComplement) {
	// (X^k) -> q0
	setInitialState(trueAutomaton, 0);

    // q0 -(X^k)-> q0
	addUniversalTransition(trueAutomaton, 0, 0);

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
	setInitialState(falseAutomaton, 0);

	// q0 -(X^k)-> q0
	addUniversalTransition(falseAutomaton, 0, 0);

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
	assert(this->f->kind != aOr && this->f->kind != aAnd && this->f->kind != aEx2 && "Negation should be only on leaves!");
	assert(this->f->kind != aTrue && this->f->kind != aFalse && "True/False should be optimized away by BooleanUnfolder!");

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
	assert(this->f1->kind != aTrue && this->f1->kind != aFalse && "True/False should be optimized away by BooleanUnfolder!");
	assert(this->f2->kind != aTrue && this->f2->kind != aFalse && "True/False should be optimized away by BooleanUnfolder!");

	// Inner formulas are first conversed to unary automatons
	Automaton left, right;
	this->f1->toUnaryAutomaton(left, doComplement);
	this->f2->toUnaryAutomaton(right, doComplement);

	#if (OPT_REDUCE_AUT_EVERYTIME == true)
	left = left.RemoveUnreachableStates();
	right = right.RemoveUnreachableStates();
	/*auto l1 = left.RemoveUnreachableStates();
	left = l1.RemoveUselessStates();
	auto r1 = left.RemoveUnreachableStates();
	right = r1.RemoveUselessStates();*/
	#endif

	// TODO: Dangerous, not sure if this is valid!!!
	// if CheckInclusion(left, right) returns 1, that means that left is
	// smaller so for intersection of automata this means we only have to
	// use the smaller one
	// ????
	// Profit!
	if(options.optimize > 1) {
		if (Automaton::CheckInclusion(left, right) == 1) {
			andAutomaton = left;
		} else if(Automaton::CheckInclusion(right, left) == 1) {\
			andAutomaton = right;
		} else {
			andAutomaton = Automaton::Intersection(left, right);
		}
	} else {
		andAutomaton = Automaton::Intersection(left, right);
	}

	#if (OPT_REDUCE_AUT_EVERYTIME == true)
	andAutomaton = andAutomaton.RemoveUnreachableStates();
	//auto a1 = andAutomaton.RemoveUnreachableStates();
	//andAutomaton = a1.RemoveUselessStates();
	#endif

	/*StateHT reachable;
	andAutomaton.RemoveUnreachableStates(&reachable);*/
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
	assert(this->f1->kind != aTrue && this->f1->kind != aFalse && "True/False should be optimized away by BooleanUnfolder!");
	assert(this->f2->kind != aTrue && this->f2->kind != aFalse && "True/False should be optimized away by BooleanUnfolder!");

	// Inner formulas are first conversed to unary automatons
	Automaton left, right;
	this->f1->toUnaryAutomaton(left, doComplement);
	this->f2->toUnaryAutomaton(right, doComplement);

	#if (OPT_REDUCE_AUT_EVERYTIME == true)
	left = left.RemoveUnreachableStates();
	right = right.RemoveUnreachableStates();
	/*auto l1 = left.RemoveUnreachableStates();
	left = l1.RemoveUselessStates();
	auto r1 = left.RemoveUnreachableStates();
	right = r1.RemoveUselessStates();*/
	#endif

	// TODO: This may be dangerous as well
	// if CheckInclusion(left,right) returns 1, that means that right
	// is bigger and so for union of automata we only have to use the
	// bigger one
	if(options.optimize > 1) {
		if (Automaton::CheckInclusion(left, right) == 1) {
			orAutomaton = right;
		} else if(Automaton::CheckInclusion(right, left) == 1) {
			orAutomaton = left;
		} else {
			orAutomaton = Automaton::Union(left, right);
		}
	} else {
		orAutomaton = Automaton::Union(left, right);
	}

	#if (OPT_REDUCE_AUT_EVERYTIME == true)
	orAutomaton = orAutomaton.RemoveUnreachableStates();
	//auto a1 = orAutomaton.RemoveUnreachableStates();
	//orAutomaton = a1.RemoveUselessStates();
	#endif
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
	// TODO: This is not right
	if(this->T1->kind == aVar2 && this->T2->kind == aPlus2) {
		ASTTerm2_Var2* XTerm = (ASTTerm2_Var2*) this->T1;
		unsigned int X = XTerm->n;
		ASTTerm2_Var2* YTerm = (ASTTerm2_Var2*) ((ASTTerm2_Plus*) this->T2)->T;
		unsigned int Y = YTerm->n;
		// (xxxx) -> q0
		setInitialState(aut, 0);

		// q0 -(x00x)-> q0
		addTransition(aut, 0, X, Y, (char *) "00", 0);

		// q0 -(x01x)-> q1
		addTransition(aut, 0, X, Y, (char *) "01", 1);

		// q1 -(x11x)-> q1
		addTransition(aut, 1, X, Y, (char *) "11", 1);

		// q1 -(x10x)-> q0
		addTransition(aut, 1, X, Y, (char *) "10", 0);

		setFinalState(aut, doComplement, 0);
		setNonFinalState(aut, doComplement, 1);


#if CONSTRUCT_ALWAYS_DTA
		if(doComplement) {
#endif
			// q0 -(x1Xx)-> q2
			addTransition(aut, 0, X, Y, (char *) "1X", 2);

			// q1 -(x0Xx)-> q2
			addTransition(aut, 1, X, Y, (char *) "0X", 2);

			// q2 -(xXXx)-> q2
			addUniversalTransition(aut, 2, 2);

			setNonFinalState(aut, doComplement, 2);
#if CONSTRUCT_ALWAYS_DTA
		}
#endif

	// 2) T1 = T2
	} else if (this->T1->kind == aVar2 && this->T2->kind == aVar2) {
		ASTTerm2_Var2* T1Term = (ASTTerm2_Var2*) this->T1;
		unsigned int T1 = T1Term->n;
		ASTTerm2_Var2* T2Term = (ASTTerm2_Var2*) this->T2;
		unsigned int T2 = T2Term->n;

		// (xxxx) -> q0
		setInitialState(aut, 0);

		// q0 -(x00x)-> q0
		addTransition(aut, 0, T1, T2, (char *) "00", 0);

		// q0 -(x11x)-> q0
		addTransition(aut, 0, T1, T2, (char *) "11", 0);

		// set q0 final
		setFinalState(aut, doComplement, 0);

#if CONSTRUCT_ALWAYS_DTA
		if(doComplement) {
#endif
			// q0 -(x01x)-> q1
			addTransition(aut, 0, T1, T2, (char *) "01", 1);

			// q0 -(x10x)-> q1
			addTransition(aut, 0, T1, T2, (char *) "10", 1);

			// q1 -(xxxx)-> q1
			addUniversalTransition(aut, 1, 1);

			setNonFinalState(aut, doComplement, 1);
#if CONSTRUCT_ALWAYS_DTA
		}
#endif
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
		setInitialState(aut, current);

		while(!l.empty()) {
			unsigned int front = l.front();
			l.pop_front();
			while(current != (front+1)) {
				addTransition(aut, current, X, '0', current+1);
#if CONSTRUCT_ALWAYS_DTA
				if(doComplement) {
#endif
					addTransition(aut, current, X, '1', sink);
#if CONSTRUCT_ALWAYS_DTA
				}
#endif
				setNonFinalState(aut, doComplement, current++);
			}
			addTransition(aut, current, X, '1', current+1);
#if CONSTRUCT_ALWAYS_DTA
			if(doComplement) {
#endif
				addTransition(aut, current, X, '0', sink);
#if CONSTRUCT_ALWAYS_DTA
			}
#endif
			setNonFinalState(aut, doComplement, current++);
		}

		setFinalState(aut, doComplement, current);
		addTransition(aut, current, X, '0', current);
#if CONSTRUCT_ALWAYS_DTA
		if(doComplement) {
#endif
			addTransition(aut, current, X, '1', sink);
			setNonFinalState(aut, doComplement, sink);
#if CONSTRUCT_ALWAYS_DTA
		}
#endif

	// 3) X = e
	// this should be X = {0} hopefully
	} else {
		ASTTerm2_Var2 *XTerm = (ASTTerm2_Var2*) this->T1;
		unsigned int X = XTerm->n;

		// (xxx)-> q0
		setInitialState(aut, 0);

		// q0 -(x1x)-> q1
		addTransition(aut, 0, X, '1', 1);

		// set q0 final
		setNonFinalState(aut, doComplement, 0);
		setFinalState(aut, doComplement, 1);

#if CONSTRUCT_ALWAYS_DTA
		if(doComplement) {
#endif
			// q0 -(x0x)-> q2
			addTransition(aut, 0, X, '0', 2);

			// q1 -(xxx)-> q2
			addUniversalTransition(aut, 1, 2);

			// q2 -(xxx)-> q2
			addUniversalTransition(aut, 2, 2);

			setNonFinalState(aut, doComplement, 2);
#if CONSTRUCT_ALWAYS_DTA
		}
#endif
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
	setInitialState(aut, 0);

	// q0 -(x00x)-> q0
	addTransition(aut, 0, T1, T2, (char *) "00", 0);

	// q0 -(x11x)-> q0
	addTransition(aut, 0, T1, T2, (char *) "11", 0);

	// q0 -(x01x)-> q0
	addTransition(aut, 0, T1, T2, (char *) "01", 0);

	// final state q0
	setFinalState(aut, doComplement, 0);

	// q0 -(x10x)-> q1 = sink
	addTransition(aut, 0, T1, T2, (char *) "10", 1);

	// q1 -(xxxx)-> q1
	addUniversalTransition(aut, 1, 1);

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
	setInitialState(aut, 0);

	// q0 -(x0x)-> q0
	addTransition(aut, 0, X, '0', 0);

	// q0 -(x1x)-> q1
	addTransition(aut, 0, X, '1', 1);

	// q1 -(x0x)-> q1
	addTransition(aut, 1, X, '0', 1);

	// final state q1
	setNonFinalState(aut, doComplement, 0);
	setFinalState(aut, doComplement, 1);

#if CONSTRUCT_ALWAYS_DTA
	if(doComplement) {
#endif
		// q1 -(x1x)-> q2
		addTransition(aut, 1, X, '1', 2);

		// q2 -(xxx)-> q2
		addUniversalTransition(aut, 2, 2);

		setNonFinalState(aut, doComplement, 2);
#if CONSTRUCT_ALWAYS_DTA
	}
#endif
}

/**
 * Constructs automaton for formula denoting that x is less or equal than yX
 *
 * @return Automaton corresponding to the formula x <= y
 */
void ASTForm_LessEq::toUnaryAutomaton(Automaton &aut, bool doComplement) {
	assert(this->t1->kind == aVar1 && this->t2->kind == aVar1);

	ASTTerm1_Var1 *xVar = (ASTTerm1_Var1*) this->t1;
	unsigned int x = (unsigned int) xVar->n;
	ASTTerm1_Var1 *yVar = (ASTTerm1_Var1*) this->t2;
	unsigned int y = (unsigned int) yVar->n;

	// -(xxx)-> q0
	setInitialState(aut, 0);

	// q0 -(x00x)-> q0
	addTransition(aut, 0, x, y, (char *) "00", 0);

	// q0 -(x11x)-> q2
	addTransition(aut, 0, x, y, (char *) "11", 2);

	// q0 -(x10x)-> q1
	addTransition(aut, 0, x, y, (char *) "10", 1);

	// q1 -(xX0x)-> q1
	addTransition(aut, 1, x, y, (char *) "X0", 1);

	// q1 -(xX1x)-> q2
	addTransition(aut, 1, x, y, (char *) "X1", 2);

	// q2 -(xXXx)-> q2
	addUniversalTransition(aut, 2, 2);

	setNonFinalState(aut, doComplement, 0);
	setNonFinalState(aut, doComplement, 1);
	setFinalState(aut, doComplement, 2);

#if CONSTRUCT_ALWAYS_DTA
	if(doComplement){
#endif
		// q0 -(xX1x)-> q3
		addTransition(aut, 0, x, y, (char *) "01", 3);

		// q3 -(xxx)-> q3
		addUniversalTransition(aut, 3, 3);
		setNonFinalState(aut, doComplement, 3);
#if CONSTRUCT_ALWAYS_DTA
	}
#endif

}

/**
 * Constructs automaton for formula denoting that x is less or equal than yX
 *
 * @return Automaton corresponding to the formula x <= y
 */
void ASTForm_Less::toUnaryAutomaton(Automaton &aut, bool doComplement) {
	assert(this->t1->kind == aVar1 && this->t2->kind == aVar1);

	ASTTerm1_Var1 *xVar = (ASTTerm1_Var1*) this->t1;
	unsigned int x = (unsigned int) xVar->n;
	ASTTerm1_Var1 *yVar = (ASTTerm1_Var1*) this->t2;
	unsigned int y = (unsigned int) yVar->n;

	// -(xxx)-> q0
	setInitialState(aut, 0);

	// q0 -(x00x)-> q0
	addTransition(aut, 0, x, y, (char *) "00", 0);

	// q0 -(x10x)-> q1
	addTransition(aut, 0, x, y, (char *) "10", 1);

	// q1 -(xX0x)-> q1
	addTransition(aut, 1, x, y, (char *) "X0", 1);

	// q1 -(xX1x)-> q2
	addTransition(aut, 1, x, y, (char *) "X1", 2);

	// q2 -(xXXx)-> q2
	addUniversalTransition(aut, 2, 2);

	setNonFinalState(aut, doComplement, 0);
	setNonFinalState(aut, doComplement, 1);
	setFinalState(aut, doComplement, 2);

#if CONSTRUCT_ALWAYS_DTA
	if(doComplement){
#endif
		// q0 -(xX1x)-> q3
		addTransition(aut, 0, x, y, (char *) "X1", 3);

		// q3 -(xxx)-> q3
		addUniversalTransition(aut, 3, 3);
		setNonFinalState(aut, doComplement, 3);
#if CONSTRUCT_ALWAYS_DTA
	}
#endif
}

/**
 * Constructs automaton for formula denoting that x = Int
 *
 * @param aut: created automaton
 * @param doComplement: whether automaton should be complemented or not
 * @return Automaton corresponding to the formula x = Int
 */
void ASTForm_Equal1::toUnaryAutomaton(Automaton &aut, bool doComplement) {
	if(this->t1->kind == aVar1 && this->t2->kind == aPlus1) {
		ASTTerm1_Var1 *xVar = (ASTTerm1_Var1 *) this->t1;
		unsigned int x = (unsigned int) xVar->n;
		ASTTerm1_Var1 *yTerm = (ASTTerm1_Var1 *) ((ASTTerm1_Plus *) this->t2)->t;
		unsigned int y = yTerm->n;

		// -(xxx)-> q0
		setInitialState(aut, 0);

		// q0 -(x00x)-> q0
		addTransition(aut, 0, x, y, (char *) "00", 0);

		// q0 -(x01x)-> q1
		addTransition(aut, 0, x, y, (char *) "01", 1);

		// q0 -(x1Xx)-> q3
		addTransition(aut, 0, x, y, (char *) "1X", 3);

		// q1 -(x1Xx)-> q2
		addTransition(aut, 1, x, y, (char *) "1X", 2);

		// q1 -(x0Xx)-> q3
		addTransition(aut, 1, x, y, (char *) "0X", 3);

		// q2 -(xXXx)-> q2
		addUniversalTransition(aut, 2, 2);

		// q3 -(xXXx)-> q3
		addUniversalTransition(aut, 3, 3);

		setNonFinalState(aut, doComplement, 0);
		setNonFinalState(aut, doComplement, 1);
		setFinalState(aut, doComplement, 2);
		setNonFinalState(aut, doComplement, 3); // < SINK >
	} else if(this->t1->kind == aVar1 && this->t2->kind == aVar1) {
		ASTTerm1_Var1 *xVar = (ASTTerm1_Var1 *) this->t1;
		unsigned int x = (unsigned int) xVar->n;
		ASTTerm1_Var1 *yVar = (ASTTerm1_Var1 *) this->t2;
		unsigned int y = (unsigned int) yVar->n;

		setInitialState(aut, 0);

		addTransition(aut, 0, x, y, (char *) "00", 0);    // q0 -(x00x)-> q0
		addTransition(aut, 0, x, y, (char *) "01", 2);    // q0 -(x01x)-> q2
		addTransition(aut, 0, x, y, (char *) "10", 2);    // q0 -(x10x)-> q2
		addTransition(aut, 0, x, y, (char *) "11", 1);    // q0 -(x11x)-> q1

		addUniversalTransition(aut, 1, 1);                // q1 -(xXXx)-> q1

		addUniversalTransition(aut, 2, 2);                // q2 -(xXXx)-> q2

		setNonFinalState(aut, doComplement, 0);
		setFinalState(aut, doComplement, 1);
		setNonFinalState(aut, doComplement, 2);	// < SINK >
	} else if(this->t1->kind == aVar1 && this->t2->kind == aInt) {
		ASTTerm1_Var1 *xVar = reinterpret_cast<ASTTerm1_Var1*>(this->t1);
		unsigned int x = (unsigned int) xVar->n;
		ASTTerm1_Int *cVar = reinterpret_cast<ASTTerm1_Int*>(this->t2);
		unsigned int c = cVar->n;

		assert(c == 0 && "Constants > 0 are not supported");

		setInitialState(aut, 0);

		addTransition(aut, 0, x, '1', 1);				// q0 -(x1x)-> q1
		addTransition(aut, 0, x, '0', 2);				// q0 -(x0x)-> q2

		addUniversalTransition(aut, 1, 1);				// q1 -(xXx)-> q1

		addUniversalTransition(aut, 2, 2);				// q2 -(xXx)-> q2

		setNonFinalState(aut, doComplement, 0);
		setFinalState(aut, doComplement, 1);
		setNonFinalState(aut, doComplement, 2);	// < SINK >
	} else {
		assert(false && "Unsupported Equal1 operation");
	}
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
		setInitialState(aut, 0);
		setNonFinalState(aut, doComplement, 0);

		// j -(xxx)-> j+1
		for(unsigned int j = 0; j != i;) {
			addUniversalTransition(aut, j, j+1);
			setNonFinalState(aut, doComplement, j++);
		}

#if CONSTRUCT_ALWAYS_DTA
		//i -(x0x)-> i+2
		if(doComplement) {
			addTransition(aut, i, X, '0', i+2);
			setNonFinalState(aut, doComplement, i+2);
		}
#endif

		//i -(x1x)-> i+1
		addTransition(aut, i, X, '1', i+1);
		setFinalState(aut, doComplement, i+1);

		//i+1 -(xxx)-> i+1
		addUniversalTransition(aut, i+1, i+1);

#if CONSTRUCT_ALWAYS_DTA
		//i+2 -(xxx)-> i+2
		if(doComplement) {
			addUniversalTransition(aut, i+2, i+2);
		}
#endif

	// x in X
	} else {
		ASTTerm2_Var2 *XVar = (ASTTerm2_Var2*) this->T2;
		unsigned int X = (unsigned int) XVar->n;
		ASTTerm1_Var1 *xVar = (ASTTerm1_Var1*) this->t1;
		unsigned int x = (unsigned int) xVar->n;

		// -(xxx)-> q0
		setInitialState(aut, 0);

		addTransition(aut, 0, x, X, (char *) "0X", 0);	// q0 -(x0Xx)-> q0
		addTransition(aut, 0, x, X, (char *) "11", 1);	// q0 -(x11x)-> q1
		addTransition(aut, 0, x, X, (char *) "10", 2);	// q0 -(x10x)-> q2
		addUniversalTransition(aut, 1, 1);				// q1 -(xXXx)-> q1
		addUniversalTransition(aut, 2, 2);				// q2 -(xXXx)-> q2

		setNonFinalState(aut, doComplement, 0);
		setFinalState(aut, doComplement, 1);
		setNonFinalState(aut, doComplement, 2); 		// < SINK >
	}
}

void constructAutomatonByMona(ASTForm *form, Automaton& v_aut) {
	assert(form != nullptr);

	int numVars = varMap.TrackLength();
	unsigned *offs = new unsigned[numVars];
	IdentList *vars = nullptr;
	DFA *dfa = nullptr;

	vars = initializeVars(form);
	initializeOffsets(offs, vars);
	toMonaAutomaton(form, dfa);
	convertMonaToVataAutomaton(v_aut, dfa, vars, numVars, offs);
}

void toMonaAutomaton(ASTForm* form, DFA*& dfa) {
	assert(form != nullptr);

	// Conversion of formula representation from AST to DAG
	codeTable = new CodeTable;
	VarCode formulaCode = form->makeCode();

	dfa = nullptr;

	// Initialization of BDD
	bdd_init();
	codeTable->init_print_progress();

	// Translation to DFA
	dfa = formulaCode.DFATranslate();
	formulaCode.remove();

	// Unrestriction of MONA automaton
	// Note: This is optimization of MONA
	DFA *temp = dfaCopy(dfa);
	dfaUnrestrict(temp);
	dfa = dfaMinimize(temp);
	dfaFree(temp);
}

IdentList* initializeVars(ASTForm *form) {
	assert(form != nullptr);

	IdentList free, bounded;
	form->freeVars(&free, &bounded);
	return ident_union(&free, &bounded);
}

void initializeOffsets(unsigned *offs, IdentList *vars) {
	assert(offs != nullptr);
	assert(vars != nullptr);

	// some freaking crappy initializations
	IdentList::iterator id;
	int ix = 0;

	// iterate through all variables
	for (id = vars->begin(); id != vars->end(); id++, ix++) {
		offs[ix] = offsets.off(*id);
	}
}

/**
 * Alternative way of constructing automaton. We use deterministic automata
 * and construct the automaton by MONA, use the minimization and then convert
 * the automaton from MONA representation to VATA representation by switching
 * the post transitions to pre transitions
 *
 * @param[out] v_aut: 		output automaton in VATA representation
 * @param[in] m_aut: 		input automaton in MONA representation
 * @param[in] varNum: 		numberof variables in automaton
 * @param[in] offsets: 		offsets of concrete variables
 */
void convertMonaToVataAutomaton(Automaton& v_aut, DFA* m_aut, IdentList* vars, int varNum, unsigned* offsets) {
	assert(m_aut != nullptr);
	assert(offsets != nullptr);
	assert(vars != nullptr);

	char* transition = new char[varNum];
	int usedVarNum = vars->size();

	paths state_paths, pp;
	trace_descr tp;

	// add initial transition
	setInitialState(v_aut, 1);

	#if (DUMP_INTERMEDIATE_AUTOMATA == true)
		if(options.dump) {
			std::cout << "[*] Converting MONA deterministic automaton with: " << m_aut->ns << " states\n";
		}
	#endif

	for (unsigned int i = 1; i < m_aut->ns; ++i) {
	//                ^--- my assumption why this is correct:
	// MONA has one additional transition 0 -(X*)-> 1 for zeroth order variables
	// since we are not using this, we can skip it
	// set final states
		if(m_aut->f[i] == 1) {
			setFinalState(v_aut, false, i);
		}

		state_paths = pp = make_paths(m_aut->bddm, m_aut->q[i]);

		while(pp) {
			// construct the transition
			// TODO: this may need few augmentation, because mona probably
			// has different order of variables
			int j;
			// Initialize transition
			for(j = 0; j < varNum; ++j) {
				transition[j] = 'X';
			}
			transition[j] = '\0';

			for (j = 0; j < usedVarNum; ++j) {
				for (tp = pp->trace; tp && (tp->index != offsets[j]); tp = tp->next);
				if(tp) {
					if (tp->value) {
						transition[varMap[vars->get(j)]] = '1';
					} else {
						transition[varMap[vars->get(j)]] = '0';
					}
				} else {
					transition[varMap[vars->get(j)]] = 'X';
				}
			}

			//std::cout << i << " -(" << transition << ")-> " << pp->to << "\n";
			addTransition(v_aut, i, transition, pp->to);

			pp = pp->next;
		}

		kill_paths(state_paths);
	}

	#if (DUMP_INTERMEDIATE_AUTOMATA == true)
		if(options.dump) {
			std::cout << "\033[1;32m[DONE]\033[0m\n";
		}
	#endif
}
