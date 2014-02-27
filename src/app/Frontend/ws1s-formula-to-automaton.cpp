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
 * Constructs automaton for unary automaton True
 * @return: Automaton corresponding to the formula True
 */
Automaton* ASTForm_True::toUnaryAutomaton() {
    cout << "True -> automaton\n";
	return 0;
}

/**
 * Constructs automaton for unary automaton False
 * @return: Automaton corresponding to the formula False
 */
Automaton* ASTForm_False::toUnaryAutomaton() {
	cout << "False -> automaton\n";
	return 0;
}

/**
 * Constructs automaton for formula not phi
 *
 * Does complementation of states. Since negations should be thanks to
 * formula being conversed to exPNF form, so determinization does not need
 * to take place
 *
 * @return: Automaton corresponding to the formula not phi
 */
Automaton* ASTForm_Not::toUnaryAutomaton() {
	// Inner formula is first conversed to unary automaton
	cout << "Not -> automaton\n";
	Automaton* autF = this->f->toUnaryAutomaton();
	return 0;
}

/**
 * Constructs automaton for formula phi and psi
 *
 * First converts formulae phi and psi to automatons and then does a automata
 * product to compute the final automaton.
 *
 * @return: Automaton corresponding to the formula phi and psi
 */
Automaton* ASTForm_And::toUnaryAutomaton() {
	// Inner formulas are first conversed to unary automatons
	cout << "And -> automaton\n";
	Automaton *autF1 = this->f1->toUnaryAutomaton();
	Automaton *autF2 = this->f2->toUnaryAutomaton();
	return 0;
}

/**
 * Constructs automaton for formula phi or psi
 *
 * First converts formulae phi or psi to automatons and then does a automata
 * union to compute the final automaton.
 *
 * @return: Automaton corresponding to the formula phi or psi
 */
Automaton* ASTForm_Or::toUnaryAutomaton() {
	// Inner formulas are first conversed to unary automatons
	cout << "Or -> automaton\n";
	Automaton *autF1 = this->f1->toUnaryAutomaton();
	Automaton *autF2 = this->f2->toUnaryAutomaton();
	return 0;
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
Automaton* ASTForm_Equal2::toUnaryAutomaton() {
	cout << "Eq2 -> automaton\n";
	return 0;
}

/**
 * Constructs automaton for atomic formula T1 ~= T2, first constructs automaton
 * T1 = T2 and then flip the states
 *
 * @return Automaton corresponding to the formula T1 ~= T2
 */
Automaton* ASTForm_NotEqual2::toUnaryAutomaton() {
	cout << "Neq2 -> automaton\n";
	return 0;
}
