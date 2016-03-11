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
    assert(child != nullptr);

    return (child->kind == aEx0)  | (child->kind == aEx1)  | (child->kind == aEx2) |
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
ASTForm* switchNodeWithQuantifier(ASTForm_q* quantifier, ASTForm* & current, ASTForm_ff* & parent) {
    ASTForm* formula;
    formula = quantifier->f;
    quantifier->f = parent;
    if(current != nullptr) {
        static_cast<ASTForm_q*>(current)->f = quantifier;
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

    ASTForm* root, *current;
    root = nullptr;
    current = nullptr;

    do {
        leftHasQuantifier = hasQuantifier(form->f1);
        rightHasQuantifier = hasQuantifier(form->f2);

        if(leftHasQuantifier) {
            form->f1 = switchNodeWithQuantifier(static_cast<ASTForm_q*>(form->f1), current, form);
            root = (root == nullptr) ? current : root;
            continue;
        }

        if(rightHasQuantifier) {
            form->f2 = switchNodeWithQuantifier(static_cast<ASTForm_q*>(form->f2), current, form);
            root = (root == nullptr) ? current : root;
        }
    } while (leftHasQuantifier | rightHasQuantifier);

    return (root == nullptr) ? form : root;
}

/**
 * Performs the transformation of the node to the negated version. This funcion is
 * templated and types are restricted to ASTForm_q, class representing quantifiers,
 * moreover this function is restricted to zero order, that has one less parameter.
 *
 * @class FromQuantifier    type of Quantifier class we are negating
 * @class ToQuantifier      type of oposite quantifier
 *
 * @param[in] from      node we are negating
 * @param[in] visitor   visitor for further transformation
 */
template <class FromQuantifier, class ToQuantifier>
ToQuantifier* createNegatedQuantifier(ASTForm* from, TransformerVisitor &visitor) {
    static_assert(std::is_base_of<ASTForm_q, FromQuantifier>::value, "FromQuantifier is not derived from ASTForm_q");
    static_assert(std::is_base_of<ASTForm_q, ToQuantifier>::value, "ToQuantifier is not derived from ASTForm_q");

    // ex not phi = not all phi
    // all not phi = ex not phi
    FromQuantifier* q = static_cast<FromQuantifier*>(from);
    ASTForm_Not* not_q = new ASTForm_Not(q->f, q->pos);
    return new ToQuantifier(q->ul, q->vl, static_cast<ASTForm*>(not_q->accept(visitor)), q->pos);
};

/**
 * Performs the transformation of the node to the negated version. This funcion is
 * templated and types are restricted to ASTForm_q, class representing quantifiers.
 *
 * @class FromQuantifier    type of Quantifier class we are negating
 * @class ToQuantifier      type of oposite quantifier
 *
 * @param[in] from      node we are negating
 * @param[in] visitor   visitor for further transformation
 */
template <class FromQuantifier, class ToQuantifier>
ToQuantifier* createZeroOrderNegatedQuantifier(ASTForm* from, TransformerVisitor &visitor) {
    static_assert(std::is_base_of<ASTForm_vf, FromQuantifier>::value, "FromQuantifier is not derived from ASTForm_q");
    static_assert(std::is_base_of<ASTForm_vf, ToQuantifier>::value, "ToQuantifier is not derived from ASTForm_q");
    assert(from != nullptr);

    // ex not phi = not all phi
    // all not phi = ex not phi
    FromQuantifier* q = static_cast<FromQuantifier*>(from);
    ASTForm_Not* not_q = new ASTForm_Not(q->f, q->pos);
    return new ToQuantifier(q->vl, static_cast<ASTForm*>(not_q->accept(visitor)), q->pos);
};

/**
 * Moves quantifier through the negation, so exists is transformed to forall
 * and vice versa.
 *
 * @param node: input AST node representing negation formula
 * @return: formula with negated quantifier
 */
ASTForm* negateQuantifier(ASTForm_Not* node, TransformerVisitor &visitor) {
    assert(node != nullptr);
    assert(node->f != nullptr);

    ASTForm* q = node->f;
    switch(q->kind) {
        case aEx0:
            return createZeroOrderNegatedQuantifier<ASTForm_Ex0, ASTForm_All0>(q, visitor);
        case aEx1:
            return createNegatedQuantifier<ASTForm_Ex1, ASTForm_All1>(q, visitor);
        case aEx2:
            return createNegatedQuantifier<ASTForm_Ex2, ASTForm_All2>(q, visitor);
        case aAll0:
            return createZeroOrderNegatedQuantifier<ASTForm_All0, ASTForm_All0>(q, visitor);
        case aAll1:
            return createNegatedQuantifier<ASTForm_All1, ASTForm_Ex1>(q, visitor);
        case aAll2:
            return createNegatedQuantifier<ASTForm_All2, ASTForm_Ex2>(q, visitor);
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
    assert(form != nullptr);

    return negateQuantifier(form, *this);
}