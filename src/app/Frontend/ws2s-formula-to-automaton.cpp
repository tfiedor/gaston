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
 * Constructs automaton for formula True
 * @return: Automaton corresponding to the formula True
 */
Automaton* ASTForm_True::toBinaryAutomaton() {
    cout << "True -> automaton\n";
	return 0;
}

/**
 * Constructs automaton for formula False
 * @return: Automaton corresponding to the formula False
 */
Automaton* ASTForm_False::toBinaryAutomaton() {
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
Automaton* ASTForm_Not::toBinaryAutomaton() {
	// Inner formula is first conversed to binary automaton
	Automaton* autF = this->f->toBinaryAutomaton();
	cout << "Not -> automaton\n";
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
Automaton* ASTForm_And::toBinaryAutomaton() {
	// Inner formulas are first conversed to binary automatons
	cout << "And -> automaton\n";
	Automaton *autF1 = this->f1->toBinaryAutomaton();
	Automaton *autF2 = this->f2->toBinaryAutomaton();
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
Automaton* ASTForm_Or::toBinaryAutomaton() {
	// Inner formulas are first conversed to binary automatons
	cout << "Or -> automaton\n";
	Automaton *autF1 = this->f1->toBinaryAutomaton();
	Automaton *autF2 = this->f2->toBinaryAutomaton();
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
Automaton* ASTForm_Equal2::toBinaryAutomaton() {
	cout << "Eq2 -> automaton\n";
	return 0;
}

/**
 * Constructs automaton for atomic formula T1 ~= T2, first constructs automaton
 * T1 = T2 and then flip the states
 *
 * @return Automaton corresponding to the formula T1 ~= T2
 */
Automaton* ASTForm_NotEqual2::toBinaryAutomaton() {
	cout << "Neq2 -> automaton\n";
	return 0;
}


/**
 * Constructs automaton for atomic formula T1 sub T2
 *
 * @return Automaton corresponding to the formula T1 sub T2
 */
Automaton* ASTForm_Sub::toBinaryAutomaton() {
	cout << "Sub -> automaton\n";
	return 0;
}


/**
 * Constructs automaton for formula denoting, that set is a singleton
 *
 * @return Automaton corresponding to the formula Singleton(X)
 */
Automaton* ASTForm_FirstOrder::toBinaryAutomaton() {
	cout << "Sing -> automaton\n";
	return 0;
}
