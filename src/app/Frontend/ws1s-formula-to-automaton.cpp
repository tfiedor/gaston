#include "ast.h"
#include "symboltable.h"

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

inline void setState(Automaton &automaton, bool setFinal, unsigned int state) {
	if(setFinal) {
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
    // _ -> q0
	setState(trueAutomaton, !doComplement, 0);

	trueAutomaton.AddTransition(
    		Automaton::StateTuple(),
    		Automaton::SymbolType(),
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
	setState(falseAutomaton, doComplement, 0);

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
 *  * TODO: do Complement
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
 *  1) T1 = T2
 *  2) X = Y1
 *  3) X = e
 *
 *  Constructs template automaton, that does acceptance of the formula
 *
 *  @return Automaton corresponding to the formula phi or psi
 */
void ASTForm_Equal2::toUnaryAutomaton(Automaton &aut, bool doComplement) {
	cout << "Eq2 -> automaton\n";
}

/**
 * Constructs automaton for atomic formula T1 ~= T2, first constructs automaton
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
	cout << "Sub -> automaton\n";
}

/**
 * Constructs automaton for formula denoting, that set is a singleton
 *
 * @return Automaton corresponding to the formula Singleton(X)
 */
void ASTForm_FirstOrder::toUnaryAutomaton(Automaton &aut, bool doComplement) {
	cout << "Sing -> automaton\n";
}
