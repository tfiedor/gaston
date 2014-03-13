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
 * Constructs automaton for formula True
 *
 * @param[out] trueAutomaton: created automaton
 * @param doComplement: whether automaton should be complemented
 */
void ASTForm_True::toBinaryAutomaton(Automaton &trueAutomaton, bool doComplement) {
    cout << "True -> automaton\n";
}

/**
 * Constructs automaton for formula False
 *
 * @param[out] falseAutomaton: created automaton
 * @param doComplement: whether automaton should be complemented
 */
void ASTForm_False::toBinaryAutomaton(Automaton &falseAutomaton, bool doComplement) {
	cout << "False -> automaton\n";
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
void ASTForm_Not::toBinaryAutomaton(Automaton &notAutomaton, bool doComplement) {
	// Inner formula is first conversed to binary automaton
	this->f->toBinaryAutomaton(notAutomaton, true);
	cout << "Not -> automaton\n";
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
void ASTForm_And::toBinaryAutomaton(Automaton &andAutomaton, bool doComplement) {
	// Inner formulas are first conversed to binary automatons
	cout << "And -> automaton\n";
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
void ASTForm_Or::toBinaryAutomaton(Automaton &orAutomaton, bool doComplement) {
	// Inner formulas are first conversed to binary automatons
	cout << "Or -> automaton\n";
}

/**
 * Constructs automaton for atomic formula T1 = T2, according to its structure:
 *  1) T1 = T2
 *  2) X = Y1
 *  3) X = e
 *
 *  Constructs template automaton, that does acceptance of the formula
 *
 * @param[out] eqAutomaton: created automaton
 * @param doComplement: whether automaton should be complemented
 */
void ASTForm_Equal2::toBinaryAutomaton(Automaton &eqAutomaton, bool doComplement) {
	cout << "Eq2 -> automaton\n";
}

/**
 * Constructs automaton for atomic formula T1 ~= T2, first constructs automaton
 * T1 = T2 and then flip the states
 *
 * @param[out] neqAutomaton: created automaton
 * @param doComplement: whether automaton should be complemented
 */
void ASTForm_NotEqual2::toBinaryAutomaton(Automaton &neqAutomaton, bool doComplement) {
	cout << "Neq2 -> automaton\n";
}


/**
 * Constructs automaton for atomic formula T1 sub T2
 *
 * @param[out] subAutomaton: created automaton
 * @param doComplement: whether automaton should be complemented
 */
void ASTForm_Sub::toBinaryAutomaton(Automaton &subAutomaton, bool doComplement) {
	cout << "Sub -> automaton\n";
}


/**
 * Constructs automaton for formula denoting, that set is a singleton
 *
 * @param[out] singAutomaton: created automaton
 * @param doComplement: whether automaton should be complemented
 */
void ASTForm_FirstOrder::toBinaryAutomaton(Automaton &singAutomaton, bool doComplement) {
	cout << "Sing -> automaton\n";
}
