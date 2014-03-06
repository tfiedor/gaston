#include "../Frontend/ast.h"
#include "../Frontend/symboltable.h"

#include <cstring>

// VATA headers
#include <vata/bdd_bu_tree_aut.hh>
#include <vata/parsing/timbuk_parser.hh>
#include <vata/serialization/timbuk_serializer.hh>
#include <vata/util/binary_relation.hh>

using std::cout;

extern SymbolTable symbolTable;

using Automaton = VATA::BDDBottomUpTreeAut;

/**
 * Constructs universal track X^k according to the number of variables used
 * in formula, i.e. in symbol table
 *
 * @return: universal track for transition
 */
Automaton::SymbolType constructUniversalTrack() {
	unsigned int trackLen = symbolTable.noIdents;
	Automaton::SymbolType transitionTrack;
	transitionTrack.AddVariablesUpTo(trackLen);
	return transitionTrack;
}

/**
 * Sets state as final in automaton, according to the whether we are
 * complementing the automaton or not
 *
 * @param[in] automaton: automaton, where we are setting states
 * @param[in] complement: whether we are constructing complement automaton
 * @param[in] state: which state we are setting as final
 */
inline void setFinalState(Automaton &automaton, bool complement, unsigned int state) {
	if(!complement) {
		automaton.SetStateFinal(state);
	}
}

/**
 * Sets state as non-final in automaton, according to the whether we are
 * complementing the automaton or not
 *
 * @param[in] automaton: automaton, where we are setting states
 * @param[in] complement: whether we are constructing complement automaton
 * @param[in] state: which state we are setting as non final
 */
inline void setNonFinalState(Automaton &automaton, bool complement, unsigned int state) {
	if(complement) {
		automaton.SetStateFinal(state);
	}
}

/**
 * Constructs automaton for unary automaton True
 *
 * @param[out] trueAutomaton: created automaton
 * @param doComplement: whether automaton should be complemented
 * TODO: do Complement
 */
void ASTForm_True::toUnaryAutomaton(Automaton &trueAutomaton, bool doComplement) {
    // (X^k) -> q0
	setFinalState(trueAutomaton, doComplement, 0);

	trueAutomaton.AddTransition(
    		Automaton::StateTuple(),
    		constructUniversalTrack(),
    		0);

    // q0 -(X^k)-> q0
    trueAutomaton.AddTransition(
    		Automaton::StateTuple({0}),
    		constructUniversalTrack(),
    		0);
}

/**
 * Constructs automaton for unary automaton False
 *
 * @param[out] falseAutomaton: created automaton
 * @param doComplement: whether automaton should be complemented
 * TODO: do Complement
 */
void ASTForm_False::toUnaryAutomaton(Automaton &falseAutomaton, bool doComplement) {
	// _ -> q0
	setNonFinalState(falseAutomaton, doComplement, 0);

	falseAutomaton.AddTransition(
			Automaton::StateTuple(),
			Automaton::SymbolType(),
			0);

	// q0 -(X^k)-> q0
	falseAutomaton.AddTransition(
			Automaton::StateTuple({0}),
			constructUniversalTrack(),
			0);
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
 * TODO: Y1 = X? Switch?
 *  Constructs template automaton, that does acceptance of the formula
 *
 *  @return Automaton corresponding to the formula phi or psi
 */
void ASTForm_Equal2::toUnaryAutomaton(Automaton &aut, bool doComplement) {
	// 1) X = Y1
	if(this->T1->kind == aVar2 && this->T2->kind == aPlus2) {
		ASTTerm2_Var2* XTerm = (ASTTerm2_Var2*) this->T1;
		unsigned int X = XTerm->n;
		ASTTerm2_Var2* YTerm = (ASTTerm2_Var2*) ((ASTTerm2_Plus*) this->T2)->T;
		unsigned int Y = YTerm->n;
		// (x00x) -> q0
		Automaton::SymbolType q0 = constructUniversalTrack();
		q0.SetIthVariableValue(X, '0');
		q0.SetIthVariableValue(Y, '0');
		aut.AddTransition(
				Automaton::StateTuple(),
				q0,
				0);

		// (x10x) -> q0
		Automaton::SymbolType q1 = constructUniversalTrack();
		q1.SetIthVariableValue(X, '1');
		q1.SetIthVariableValue(Y, '0');
		aut.AddTransition(
				Automaton::StateTuple(),
				q1,
				0);

		// q0 -(x00x)-> q0
		Automaton::SymbolType q0q0 = constructUniversalTrack();
		q0q0.SetIthVariableValue(X, '0');
		q0q0.SetIthVariableValue(Y, '0');
		aut.AddTransition(
				Automaton::StateTuple({0}),
				q0q0,
				0);

		// q0 -(x10x)-> q1
		Automaton::SymbolType q0q1 = constructUniversalTrack();
		q0q1.SetIthVariableValue(X, '1');
		q0q1.SetIthVariableValue(X, '0');
		aut.AddTransition(
				Automaton::StateTuple({0}),
				q0q1,
				1);

		// q1 -(x01x)-> q2
		Automaton::SymbolType q1q2 = constructUniversalTrack();
		q1q2.SetIthVariableValue(X, '0');
		q1q2.SetIthVariableValue(Y, '1');
		aut.AddTransition(
				Automaton::StateTuple({1}),
				q1q2,
				2);

		// q2 -(x00x)-> q2
		Automaton::SymbolType q2q2 = constructUniversalTrack();
		q2q2.SetIthVariableValue(X, '0');
		q2q2.SetIthVariableValue(Y, '0');
		aut.AddTransition(
				Automaton::StateTuple({2}),
				q2q2,
				2);
		// set q2 final
		setNonFinalState(aut, doComplement, 0);
		setNonFinalState(aut, doComplement, 1);
		setFinalState(aut, doComplement, 2);

	// 2) T1 = T2
	} else if (this->T1->kind == aVar2 && this->T2->kind == aVar2) {

	// 3) X = e
	}
}

/**
 * Constructs automaton for atomic formula T1 ~= T2, first constructs automaton
 * TODO: This probably should be flattened
 * T1 = T2 and then flip the states
 *
 * @return Automaton corresponding to the formula T1 ~= T2
 */
void ASTForm_NotEqual2::toUnaryAutomaton(Automaton &aut, bool doComplement) {
	cout << "Neq2 -> automaton\n";
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
	//  -(x00x)-> q0
	Automaton::SymbolType q0 = constructUniversalTrack();
	q0.SetIthVariableValue(T1, '0');
	q0.SetIthVariableValue(T2, '0');
	aut.AddTransition(
			Automaton::StateTuple(),
			q0,
			0);

	//  -(x11x)-> q0
	Automaton::SymbolType q0b = constructUniversalTrack();
	q0b.SetIthVariableValue(T1, '1');
	q0b.SetIthVariableValue(T2, '1');
	aut.AddTransition(
			Automaton::StateTuple(),
			q0b,
			0);

	// -(x01x)-> q0
	Automaton::SymbolType q0c = constructUniversalTrack();
	q0c.SetIthVariableValue(T1, '0');
	q0c.SetIthVariableValue(T2, '1');
	aut.AddTransition(
			Automaton::StateTuple(),
			q0c,
			0);

	// q0 -(x00x)-> q0
	Automaton::SymbolType q0q0 = constructUniversalTrack();
	q0q0.SetIthVariableValue(T1, '0');
	q0q0.SetIthVariableValue(T2, '0');
	aut.AddTransition(
			Automaton::StateTuple({0}),
			q0q0,
			0);

	// q0 -(x11x)-> q0
	Automaton::SymbolType q0q0b = constructUniversalTrack();
	q0q0b.SetIthVariableValue(T1, '1');
	q0q0b.SetIthVariableValue(T2, '1');
	aut.AddTransition(
			Automaton::StateTuple({0}),
			q0q0b,
			0);

	// q0 -(x01x)-> q0
	Automaton::SymbolType q0q0c = constructUniversalTrack();
	q0q0c.SetIthVariableValue(T1, '0');
	q0q0c.SetIthVariableValue(T2, '1');
	aut.AddTransition(
			Automaton::StateTuple({0}),
			q0q0c,
			0);

	// final state q0
	setFinalState(aut, doComplement, 0);
}

/**
 * Constructs automaton for formula denoting, that set is a singleton
 *
 * @return Automaton corresponding to the formula Singleton(X)
 */
void ASTForm_FirstOrder::toUnaryAutomaton(Automaton &aut, bool doComplement) {
	ASTTerm2_Var2 *XVar = (ASTTerm2_Var2*) this->t;
	unsigned int X = (unsigned int) XVar->n;

	// -(x0x)-> q0
	Automaton::SymbolType q0 = constructUniversalTrack();
	q0.SetIthVariableValue(X, '0');
	aut.AddTransition(
			Automaton::StateTuple(),
			q0,
			0);

	// -(x1x)-> q1
	Automaton::SymbolType q1 = constructUniversalTrack();
	q1.SetIthVariableValue(X, '1');
	aut.AddTransition(
			Automaton::StateTuple(),
			q1,
			1);

	// q0 -(x0x)-> q0
	Automaton::SymbolType q0q0 = constructUniversalTrack();
	q0q0.SetIthVariableValue(X, '0');
	aut.AddTransition(
			Automaton::StateTuple({0}),
			q0q0,
			0);

	// q0 -(x1x)-> q1
	Automaton::SymbolType q0q1 = constructUniversalTrack();
	q0q1.SetIthVariableValue(X, '1');
	aut.AddTransition(
			Automaton::StateTuple({0}),
			q0q1,
			1);

	// q1 -(x0x)-> q1
	Automaton::SymbolType q1q1 = constructUniversalTrack();
	q1q1.SetIthVariableValue(X, '0');
	aut.AddTransition(
			Automaton::StateTuple({1}),
			q1q1,
			1);

	// final state q1
	setNonFinalState(aut, doComplement, 0);
	setFinalState(aut, doComplement, 1);
}
