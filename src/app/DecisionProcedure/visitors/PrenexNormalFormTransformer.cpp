/*****************************************************************************
 *  dWiNA - Deciding WSkS using non-deterministic automata
 *
 *  Copyright (c) 2015  Tomas Fiedor <ifiedortom1@fit.vutbr.cz>
 *
 *  Description:
 *    Visitor for transforming the formula to Prenex Normal Form, i.e. in
 *    the form of:
 *      ex Xn: ... not ex X1: phi
 *
 *****************************************************************************/

#include "PrenexNormalFormTransformer.h"


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
ASTForm* switchNodeWithQuantifier(ASTForm_vf* quantifier, ASTForm* & current, ASTForm_ff* & parent) {
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
ASTForm *switchNodeWithQuantifier(ASTForm_uvf* quantifier, ASTForm* & current, ASTForm_ff* & parent) {
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
 * Pushes quantifiers to the root of the formulae, creating a prefix
 *
 * @param[in] form:     traversed binary connective node
 */
AST* PrenexNormalFormTransformer::visit(ASTForm_ff* form) {
    bool leftHasQuantifier, rightHasQuantifier;

    std::cout << "Visiting ASTForm_ff:";
    form->dump();
    std::cout << "\n";

    ASTForm* root, *current;
    root = 0;
    current = 0;

    do {
        leftHasQuantifier = hasQuantifier(form->f1);
        rightHasQuantifier = hasQuantifier(form->f2);

        if(leftHasQuantifier) {
            form->f1 = (form->f1->kind == aEx0 | form->f1->kind == aAll0) ?
                 switchNodeWithQuantifier((ASTForm_vf*)form->f1, current, form) :
                 switchNodeWithQuantifier((ASTForm_uvf*)form->f1, current, form);
            root = (root == 0) ? current : root;
            continue;
        }

        if(rightHasQuantifier) {
            form->f2 = (form->f2->kind == aEx0 | form->f2->kind == aAll0) ?
                 switchNodeWithQuantifier((ASTForm_vf*)form->f2, current, form) :
                 switchNodeWithQuantifier((ASTForm_uvf*)form->f2, current, form);
            root = (root == 0) ? current : root;
        }
    } while (leftHasQuantifier | rightHasQuantifier);
    std::cout << "Done visiting: ";
    if (root != 0) {
        std::cout << "Dumping root:";
        root->dump();
        std::cout << "\n";
    }
    std::cout << "Dumping form:";
    form->dump();
    std::cout << "\n";

    return (root == 0) ? form : root;
}

/**
 * Moves quantifier through the negation, so exists is transformed to forall
 * and vice versa.
 *
 * @param node: input AST node representing negation formula
 * @return: formula with negated quantifier
 */
ASTForm* negateQuantifier(ASTForm_Not* node, ASTTransformer &visitor) {
    ASTForm* formula, *q;
    q = node->f;
    switch(q->kind) {
        case aEx0:
            formula = new ASTForm_Not(((ASTForm_Ex0*)q)->f, node->pos);
            return new ASTForm_All0(((ASTForm_Ex0*)q)->vl, static_cast<ASTForm*>(formula->accept(visitor)), ((ASTForm_Ex0*)q)->pos);
            break;
        case aEx1:
            formula = new ASTForm_Not(((ASTForm_Ex1*)q)->f, node->pos);
            return new ASTForm_All1(((ASTForm_Ex1*)q)->ul, ((ASTForm_Ex1*)q)->vl, static_cast<ASTForm*>(formula->accept(visitor)), ((ASTForm_Ex1*)q)->pos);
            break;
        case aEx2:
            formula = new ASTForm_Not(((ASTForm_Ex2*)q)->f, node->pos);
            return new ASTForm_All2(((ASTForm_Ex2*)q)->ul, ((ASTForm_Ex2*)q)->vl, static_cast<ASTForm*>(formula->accept(visitor)), ((ASTForm_Ex2*)q)->pos);
            break;
        case aAll0:
            formula = new ASTForm_Not(((ASTForm_All0*)q)->f, node->pos);
            return new ASTForm_Ex0(((ASTForm_All0*)q)->vl, static_cast<ASTForm*>(formula->accept(visitor)), ((ASTForm_All0*)q)->pos);
            break;
        case aAll1:
            formula = new ASTForm_Not(((ASTForm_All1*)q)->f, node->pos);
            return new ASTForm_Ex1(((ASTForm_All1*)q)->ul, ((ASTForm_All1*)q)->vl,static_cast<ASTForm*>(formula->accept(visitor)), ((ASTForm_All1*)q)->pos);
            break;
        case aAll2:
            formula = new ASTForm_Not(((ASTForm_All2*)q)->f, node->pos);
            return new ASTForm_Ex2(((ASTForm_All2*)q)->ul, ((ASTForm_All2*)q)->vl, static_cast<ASTForm*>(formula->accept(visitor)), ((ASTForm_All2*)q)->pos);
            break;
        default:
            return node;
    }
}

/**
 * Negates the quantifier during the pushing
 *
 * @param[in] form:     traversed Not node
 */
AST* PrenexNormalFormTransformer::visit(ASTForm_Not* form) {
    return negateQuantifier(form, *this);
}