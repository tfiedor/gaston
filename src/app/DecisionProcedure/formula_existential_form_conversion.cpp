/*
 * MONA
 * Copyright (C) 1997-2013 Aarhus University.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program; if not, write to the  Free Software
 * Foundation, Inc., 51 Franklin Street, Suite 500, Boston, MA 02110-1335,
 * USA.
 */

#include "../Frontend/ast.h"
#include "../Frontend/symboltable.h"

#include <cstring>

using std::cout;

extern SymbolTable symbolTable;


/**
 * Set of functions used for conversion of input formula to the restricted
 * syntax as defined in diploma thesis. Restricted syntax takes only few
 * types of atomic formulae (thanks to flattening) and only uses logical
 * connectives &, |, ~ and Exists.
 */


/**
 * Transformation of formula to the restricted syntax, according to the
 * rules for transformation to restricted syntax.
 *
 * Rule: A -> B ~= ~ A | B
 *
 * @return: AST Formula in restricted syntax
 */
ASTForm* ASTForm_Impl::toRestrictedSyntax() {
   f1 = f1->toRestrictedSyntax();
   f2 = f2->toRestrictedSyntax();

   ASTForm* not_f1 = new ASTForm_Not(f1, pos);
   return (ASTForm*) new ASTForm_Or(not_f1, f2, pos);
}

/**
 * Transformation of formula to the restricted syntax, according to the
 * rules for transformation to restricted syntax.
 * TODO: Cloning does somehow segfault, should repair this
 *
 * Rule: A <-> B = (~A | B) & (A | ~B)
 *
 * @return: AST Formula in restricted syntax
 */
ASTForm* ASTForm_Biimpl::toRestrictedSyntax() {
   f1 = f1->toRestrictedSyntax();
   f2 = f2->toRestrictedSyntax();
   
   ASTForm* not_f1 = new ASTForm_Not(f1, pos);
   ASTForm* not_f2 = new ASTForm_Not(f2, pos);
   ASTForm* ff1 = f1->clone();
   ASTForm* ff2 = f2->clone();
   ASTForm* impl1 = new ASTForm_Or(not_f1, ff2, pos);
   ASTForm* impl2 = new ASTForm_Or(ff1, not_f2, pos);

   return (ASTForm*) new ASTForm_And(impl1, impl2, pos);
}

/**
 * Transformation of formula to the restricted syntax, according to the
 * rules for transformation to restricted syntax.
 *
 * @return: AST Formula in restricted syntax
 */
ASTForm* ASTForm_IdLeft::toRestrictedSyntax() {
   f1 = f1->toRestrictedSyntax();
   f2 = f2->toRestrictedSyntax();
   return this;
}

/**
 * Transformation of formula to the restricted syntax, according to the
 * rules for transformation to restricted syntax. Or is left as it is,
 * as it is one of the valid logical connectives.
 *
 * @return: AST Formula in restricted syntax
 */
ASTForm* ASTForm_Or::toRestrictedSyntax() {
   f1 = f1->toRestrictedSyntax();
   f2 = f2->toRestrictedSyntax();
   return this;
}

/**
 * Transformation of formula to the restricted syntax, according to the
 * rules for transformation to restricted syntax. And is also used in
 * restricted syntax, as with this, we can get negation straight to the
 * atoms and thus not needing complementation of automaton.
 *
 * @return: AST Formula in restricted syntax
 */
ASTForm* ASTForm_And::toRestrictedSyntax() {
   f1 = f1->toRestrictedSyntax();
   f2 = f2->toRestrictedSyntax();
   return this;
}

/**
 * Transformation of formula to the restricted syntax, according to the
 * rules for transformation to restricted syntax.
 *
 * @return: AST Formula in restricted syntax
 */
ASTForm* ASTForm_Ex0::toRestrictedSyntax() {
   f = f->toRestrictedSyntax();
   return this;
}

/**
 * Transformation of formula to the restricted syntax, according to the
 * rules for transformation to restricted syntax.
 * TODO: This shouldn't be needed, as first order should be flattened
 *
 * @return: AST Formula in restricted syntax
 */
ASTForm* ASTForm_Ex1::toRestrictedSyntax() {
   f = f->toRestrictedSyntax();
   return this;
}

/**
 * Transformation of formula to the restricted syntax, according to the
 * rules for transformation to restricted syntax.
 *
 * @return: AST Formula in restricted syntax
 */
ASTForm* ASTForm_Ex2::toRestrictedSyntax() {
   f = f->toRestrictedSyntax();
   return this;
}

/**
 * Transformation of formula to the restricted syntax, according to the
 * rules for transformation to restricted syntax.
 *
 * @return: AST Formula in restricted syntax
 */
ASTForm* ASTForm_All0::toRestrictedSyntax() {
   f = f->toRestrictedSyntax();
   return this;
}

/**
 * Transformation of formula to the restricted syntax, according to the
 * rules for transformation to restricted syntax.
 *
 * @return: AST Formula in restricted syntax
 */
ASTForm* ASTForm_All1::toRestrictedSyntax() {
   f = f->toRestrictedSyntax();
   return this;
}

/**
 * Transformation of formula to the restricted syntax, according to the
 * rules for transformation to restricted syntax.
 *
 * @return: AST Formula in restricted syntax
 */
ASTForm* ASTForm_All2::toRestrictedSyntax() {
   f = f->toRestrictedSyntax();
   return this;
}

/**
 * Transformation of formula to the restricted syntax, according to the
 * rules for transformation to restricted syntax.
 *
 * @return: AST Formula in restricted syntax
 */
ASTForm* ASTForm_Not::toRestrictedSyntax() {
   f = f->toRestrictedSyntax();
   return this;
}

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
            return new ASTForm_All0(((ASTForm_Ex0*)q)->vl, formula, ((ASTForm_Ex0*)q)->pos);
            break;
        case aEx1:
            formula = new ASTForm_Not(((ASTForm_Ex1*)q)->f, node->pos);
            return new ASTForm_All1(((ASTForm_Ex1*)q)->ul, ((ASTForm_Ex1*)q)->vl, formula, ((ASTForm_Ex1*)q)->pos);
            break;
        case aEx2:
            formula = new ASTForm_Not(((ASTForm_Ex2*)q)->f, node->pos);
            return new ASTForm_All2(((ASTForm_Ex2*)q)->ul, ((ASTForm_Ex2*)q)->vl, formula, ((ASTForm_Ex2*)q)->pos);
            break;
        case aAll0:
            formula = new ASTForm_Not(((ASTForm_All0*)q)->f, node->pos);
            return new ASTForm_Ex0(((ASTForm_All0*)q)->vl, formula, ((ASTForm_All0*)q)->pos);
            break;
        case aAll1:
            formula = new ASTForm_Not(((ASTForm_All1*)q)->f, node->pos);
            return new ASTForm_Ex1(((ASTForm_All1*)q)->ul, ((ASTForm_All1*)q)->vl, formula, ((ASTForm_All1*)q)->pos);
            break;
        case aAll2:
            formula = new ASTForm_Not(((ASTForm_All2*)q)->f, node->pos);
            return new ASTForm_Ex2(((ASTForm_All2*)q)->ul, ((ASTForm_All2*)q)->vl, formula, ((ASTForm_All2*)q)->pos);
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
 * Transformation to Existentional form that uses only existential quantifier
 * and not universal
 */

/**
 * Transforms universal quantifier to the existential quantifier according to
 * the rule:
 * forall fi = not exists not fi
 *
 * @return: formula in existentional form
 */
ASTForm* ASTForm_All0::removeUniversalQuantifier() {
    f = f->removeUniversalQuantifier();
    ASTForm *negFi, *exNegFi, *formula;
    negFi = new ASTForm_Not(f, pos);
    exNegFi = new ASTForm_Ex0(vl, negFi, pos);
    formula = new ASTForm_Not(exNegFi, pos);
    return formula;
}

/**
 * Transforms universal quantifier to the existential quantifier according to
 * the rule:
 * forall fi = not exists not fi
 *
 * @return: formula in existentional form
 */
ASTForm* ASTForm_All1::removeUniversalQuantifier() {
    f = f->removeUniversalQuantifier();
    ASTForm *negFi, *exNegFi, *formula;
    negFi = new ASTForm_Not(f, pos);
    exNegFi = new ASTForm_Ex1(ul, vl, negFi, pos);
    formula = new ASTForm_Not(exNegFi, pos);
    return formula;
}

/**
 * Transforms universal quantifier to the existential quantifier according to
 * the rule:
 * forall fi = not exists not fi
 *
 * @return: formula in existentional form
 */
ASTForm* ASTForm_All2::removeUniversalQuantifier() {
    f = f->removeUniversalQuantifier();
    ASTForm *negFi, *exNegFi, *formula;
    negFi = new ASTForm_Not(f, pos);
    exNegFi = new ASTForm_Ex2(ul, vl, negFi, pos);
    formula = new ASTForm_Not(exNegFi, pos);
    return formula;
}

/**
 * Generic transformation for wide range of formulae
 *
 * @return: formula in existentional form
 */
ASTForm* ASTForm_Not::removeUniversalQuantifier() {
    f = f->removeUniversalQuantifier();
    return this;
}

/**
 * Generic transformation for wide range of formulae
 *
 * @return: formula in existentional form
 */
ASTForm* ASTForm_ff::removeUniversalQuantifier() {
    f1 = f1->removeUniversalQuantifier();
    f2 = f2->removeUniversalQuantifier();
    return this;
}

/**
 * Generic transformation for wide range of formulae
 *
 * @return: formula in existentional form
 */
ASTForm* ASTForm_vf::removeUniversalQuantifier() {
    f = f->removeUniversalQuantifier();
    return this;
}

/**
 * Generic transformation for wide range of formulae
 *
 * @return: formula in existentional form
 */
ASTForm* ASTForm_uvf::removeUniversalQuantifier() {
    f = f->removeUniversalQuantifier();
    return this;
}

/**
 * Unfolds negations to the atomic formulae and shift them to the atoms
 *
 * @return: Formula with negations in atomic formulae
 */
ASTForm* ASTForm_Not::unfoldNegations() {
    f = f->unfoldNegations();
    switch(f->kind) {
    //  not not A = A
        case aNot:
	    return ((ASTForm_Not*)f)->f;
    //  not (A or B) = not A and not B
        case aOr:
            ASTForm_Or *child1;
            ASTForm *l1, *r1;
            child1 = (ASTForm_Or*) f;
            l1 = (ASTForm*) new ASTForm_Not(child1->f1, pos);
            r1 = (ASTForm*) new ASTForm_Not(child1->f2, pos);
            return new ASTForm_And(l1->unfoldNegations(), r1->unfoldNegations(), pos);
    //  not (A and B) = not A or not B
        case aAnd:
            ASTForm_And *child2;
            ASTForm *l2, *r2;
            child2 = (ASTForm_And*) f;
            l2 = (ASTForm*) new ASTForm_Not(child2->f1, pos);
            r2 = (ASTForm*) new ASTForm_Not(child2->f2, pos);
            return new ASTForm_Or(l2->unfoldNegations(), r2->unfoldNegations(), pos);
	default:
	    return this;
    }
}

/**
 * Generic function for wide range of formulas
 *
 * @return: formula with unfolded negations
 */
ASTForm* ASTForm_ff::unfoldNegations() {
    f1 = f1->unfoldNegations();
    f2 = f2->unfoldNegations();
    return this;
}

/**
 * Generic function for wide range of formulas
 *
 * @return: formula with unfolded negations
 */
ASTForm* ASTForm_vf::unfoldNegations() {
    f = f->unfoldNegations();
    return this;
}

/**
 * Generic function for wide range of formulas
 *
 * @return: formula with unfolded negations
 */
ASTForm* ASTForm_uvf::unfoldNegations() {
    f = f->unfoldNegations();
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
   temp = this->toRestrictedSyntax();
   temp = temp->toPrenexNormalForm();
   temp = temp->removeUniversalQuantifier();
   return temp->unfoldNegations();
}

