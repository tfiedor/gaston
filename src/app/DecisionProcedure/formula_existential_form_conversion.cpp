/*****************************************************************************
 *  dWiNA - Deciding WSkS using non-deterministic automata
 *
 *  Copyright (c) 2014  Tomas Fiedor <xfiedo01@stud.fit.vutbr.cz>
 *
 *  Description:
 *    Conversion of formula to exPNF
 *
 *****************************************************************************/

#include "../Frontend/ast.h"
#include "../Frontend/symboltable.h"
#include "visitors/SyntaxRestricter.h"
#include "visitors/NegationUnfolder.h"
#include "visitors/UniversalQuantifierRemover.h"

#include <cstring>

using std::cout;

extern SymbolTable symbolTable;

/* Transformations to Prenex Normal Form */
//  1) childs are transformed to prenex normal form 
//  2) While, there exists a child node with prenex 

/**
 * Transformation of formula to Prenex Normal Form.
 *
 * Formula is in Prenex Normal Form, if every quantifiers is at the leftmost
 * point of the formula, i.e. it can be broken to prefix (chain of quantifiers)
 * and matrix (quantifier free formula).
 *
 * Several transformations take place
 *  - Childs are transformed into prenex normal form and then according
 *    to the rules of the logic, quantifiers from childs are moved up at the
 *    formula
 */

/**
 * Tests whether formula, child, has any quantifier
 *
 * @param child: child of the formula
 * @return: true if childs has quantifier, i.e. forall or exists.
 */
bool hasQuantifier(ASTForm* child) {
   return (child->kind == aEx0) | (child->kind == aEx1) | (child->kind == aEx2) |
          (child->kind == aAll0) | (child->kind == aAll1) | (child->kind == aAll2);  
}

/**
 * Moves quantifier up, i.e. quantifier is moved from being child to
 * being a parent of its parent.
 *
 *	@param quantifier: node with quantifier
 *	@param current: current node, that is processed
 *	@param parent: parent, that is switched with quantifier
 *	@return: formula with switched quantifier
 */
ASTForm* switchNodeWithQuantifier(ASTForm_vf* quantifier, ASTForm* & current, ASTForm* parent) {
    ASTForm* formula;
    formula = quantifier->f;
    quantifier->f = parent;
    if(current != 0) {
      ((ASTForm_q*)current)->f = quantifier;
    }
    current = quantifier;
    return formula;
}

/**
 * Moves quantifier up, i.e. quantifier is moved from being child to
 * being a parent of its parent.
 *
 *	@param quantifier: node with quantifier
 *	@param current: current node, that is processed
 *	@param parent: parent, that is switched with quantifier
 *	@return: formula with switched quantifier
 */
ASTForm *switchNodeWithQuantifier(ASTForm_uvf* quantifier, ASTForm* & current, ASTForm* parent) {
    ASTForm* formula;
    formula = quantifier->f;
    quantifier->f = parent;
    if(current != 0) {
      ((ASTForm_q*)current)->f = quantifier;
    }
    current = quantifier;
    return formula;
}

/**
 * Expects, that given formula is not biimplication or implication! Changes
 * the formula in form of "phi op quant psi" to "quant phi op2 psi". Its
 * children are first changed to PNF form and then they are processed by
 * moving the quantifiers to the left, i.e. up its parents
 *
 * @return: formula in PNF
 */
ASTForm* ASTForm_ff::toPrenexNormalForm() {
   bool leftHasQuantifier, rightHasQuantifier;
   f1 = f1->toPrenexNormalForm();
   f2 = f2->toPrenexNormalForm();
   
   ASTForm* root, *current;
   root = 0;
   current = 0;   

   do {
       leftHasQuantifier = hasQuantifier(f1);
       rightHasQuantifier = hasQuantifier(f2);

       if(leftHasQuantifier) {
           f1 = (f1->kind == aEx0 | f1->kind == aAll0) ?
              switchNodeWithQuantifier((ASTForm_vf*)f1, current, this) :
              switchNodeWithQuantifier((ASTForm_uvf*)f1, current, this);
           root = (root == 0) ? current : root;
           continue;
       }

       if(rightHasQuantifier) {
           f2 = (f2->kind == aEx0 | f2->kind == aAll0) ?
              switchNodeWithQuantifier((ASTForm_vf*)f2, current, this) :
              switchNodeWithQuantifier((ASTForm_uvf*)f2, current, this);
           root = (root == 0) ? current : root;
       }
   } while (leftHasQuantifier | rightHasQuantifier); 
   return (root == 0) ? this : root;
}

/**
 * Moves quantifier through the negation, so exists is transformed to forall
 * and vice versa.
 *
 * @param node: input AST node representing negation formula
 * @return: formula with negated quantifier
 */
ASTForm* negateQuantifier(ASTForm_Not* node) {
    ASTForm* formula, *q;
    q = node->f;
    switch(q->kind) {
        case aEx0:
            formula = new ASTForm_Not(((ASTForm_Ex0*)q)->f, node->pos);
            return new ASTForm_All0(((ASTForm_Ex0*)q)->vl, formula->toPrenexNormalForm(), ((ASTForm_Ex0*)q)->pos);
            break;
        case aEx1:
            formula = new ASTForm_Not(((ASTForm_Ex1*)q)->f, node->pos);
            return new ASTForm_All1(((ASTForm_Ex1*)q)->ul, ((ASTForm_Ex1*)q)->vl, formula->toPrenexNormalForm(), ((ASTForm_Ex1*)q)->pos);
            break;
        case aEx2:
            formula = new ASTForm_Not(((ASTForm_Ex2*)q)->f, node->pos);
            return new ASTForm_All2(((ASTForm_Ex2*)q)->ul, ((ASTForm_Ex2*)q)->vl, formula->toPrenexNormalForm(), ((ASTForm_Ex2*)q)->pos);
            break;
        case aAll0:
            formula = new ASTForm_Not(((ASTForm_All0*)q)->f, node->pos);
            return new ASTForm_Ex0(((ASTForm_All0*)q)->vl, formula->toPrenexNormalForm(), ((ASTForm_All0*)q)->pos);
            break;
        case aAll1:
            formula = new ASTForm_Not(((ASTForm_All1*)q)->f, node->pos);
            return new ASTForm_Ex1(((ASTForm_All1*)q)->ul, ((ASTForm_All1*)q)->vl, formula->toPrenexNormalForm(), ((ASTForm_All1*)q)->pos);
            break;
        case aAll2:
            formula = new ASTForm_Not(((ASTForm_All2*)q)->f, node->pos);
            return new ASTForm_Ex2(((ASTForm_All2*)q)->ul, ((ASTForm_All2*)q)->vl, formula->toPrenexNormalForm(), ((ASTForm_All2*)q)->pos);
            break;
        default:
            return node;
    }
}

/**
 * Quantifier is moved through negation according to the rules:
 * not(ex fi) = all(not fi)
 * not(all fi) = ex(not fi)
 *
 * @return: formula in PNF
 */
ASTForm* ASTForm_Not::toPrenexNormalForm() {
    f = f->toPrenexNormalForm();
    ASTForm* temp;
    return negateQuantifier(this);
}

/**
 * Generic transform of wide range of formulae to PNF
 *
 * @return: formula in PNF
 */
ASTForm* ASTForm_vf::toPrenexNormalForm() {
    f = f->toPrenexNormalForm();
    return this;
}

/**
 * Generic transform of wide range of formulae to PNF
 *
 * @return: formula in PNF
 */
ASTForm* ASTForm_uvf::toPrenexNormalForm() {
    f = f->toPrenexNormalForm();
    return this;
}

/**
 * Given AST tree for formula phi, it is first transformed to restricted syntax
 * then to Prenex Normal Form, moving all quantifiers to the leftmost of
 * formula. All universal quantifiers are transformed to existentional and
 * negation is shifted to atoms.
 *
 * @return: AST representation of formula in Existentional Normal Form
 */
ASTForm* ASTForm::toExistentionalPNF() {
    ASTForm* temp;

    SyntaxRestricter sr_visitor;
    temp = static_cast<ASTForm*>(this->accept(sr_visitor));

    temp = temp->toPrenexNormalForm();

    UniversalQuantifierRemover uqr_visitor;
    temp = static_cast<ASTForm*>(temp->accept(uqr_visitor));

    NegationUnfolder nu_visitor;
    return static_cast<ASTForm*>(temp->accept(nu_visitor));
}