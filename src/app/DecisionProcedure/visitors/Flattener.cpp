/*****************************************************************************
 *  dWiNA - Deciding WSkS using non-deterministic automata
 *
 *  Copyright (c) 2015  Tomas Fiedor <ifiedortom1@fit.vutbr.cz>
 *
 *  Description:
 *    Visitor for flattening of the formula
 *
 *****************************************************************************/
#include "Flattener.h"

extern SymbolTable symbolTable;
extern PredicateLib predicateLib;
extern IdentList inFirstOrder;

/**
 * Generates fresh first-order variable that can be used for quantification
 *
 * @return: fresh first-order variable
 */
ASTTerm1_Var1* Flattener::generateFreshFirstOrder() {
    unsigned int z;
    z = symbolTable.insertFresh(Varname1);
    return new ASTTerm1_Var1(z, Pos());
}

/**
 * Generates fresh second-order variable that can be used for quantification
 *
 * @return: fresh second-order variable
 */
ASTTerm2_Var2* Flattener::generateFreshSecondOrder() {
    unsigned int Z;
    Z = symbolTable.insertFresh(Varname2);
    return new ASTTerm2_Var2(Z, Pos());
}

/**
 * Flattens formula by unfolding the n to the base case 1
 *
 * @param[in] term      traversed Plus term
 */
AST* Flattener::visit(ASTTerm1_Plus* term) {
    assert(term != nullptr);
    assert(term->n >= 1); // ASSERTION = y = x + i; i in N

    int n = term->n;

    ASTTerm1 *prev = static_cast<ASTTerm1*>((term->t)->accept(*this));
    while(n != 1) {
        prev = new ASTTerm1_Plus(prev, 1, term->pos);
        --n;
    }

    // Return this, cannot unfold anymore
    return new ASTTerm1_Plus(prev, 1, term->pos);
}

/**
 * Flattens formula to second-order variables and restricted syntax so it uses
 * only certain atomic formulae
 *
 *  xi = yi -> ex z: z = xi & z = yj
 *  s = ti  -> ex z: z = t & zi = s
 *  y = xi  -> Xy = Xx i
 *  x = e   -> Xx = e
 *  x = y   -> Xx = Xy
 *
 * @param[in] form:     traversed Equal1 node
 */
AST* Flattener::visit(ASTForm_Equal1* form) {
    // TODO: This is still huge pile of shit that needs serious refactoring
    assert(form != nullptr);

    // No need to flatten
    if(form->t1->kind == aVar1 && form->t2->kind == aVar1) {
        return form;
    // switch formula to format x = ...
    } else if(form->t2->kind == aVar1) {
        ASTTerm1* temp;
        temp = form->t2;
        form->t2 = form->t1;
        form->t1 = temp;
    }

    assert(form->t1->kind == aPlus1 || form->t1->kind == aVar1);

    ASTTerm2_Var2* y;
    ASTTerm2_Var2* x;
    ASTForm_Equal1* leftEqual;
    ASTForm_Equal1* rightEqual;
    ASTForm_And* conjuction;

    if (form->t1->kind == aVar1 && form->t2->kind == aPlus1) {
        ASTTerm1_Plus* temp = static_cast<ASTTerm1_Plus*>(form->t2->accept(*this));
        // y = xi -> Xy = Xx i
        assert(temp->n == 1 && "Expected Plus(Var, 1)"); // ASSERTION = {y = x + 1}
        if (temp->t->kind == aVar1) {
            y = new ASTTerm2_Var2(static_cast<ASTTerm1_Var1*>(form->t1)->getVar(), form->pos);
            x = new ASTTerm2_Var2(static_cast<ASTTerm1_Var1*>(temp->t)->getVar(), form->pos);
            ASTTerm2_Plus* plus = new ASTTerm2_Plus(x, temp->n, form->pos);
            return new ASTForm_Equal2(y, plus, form->pos);
            // y = ti -> ex z: z = t & y = zi
        } else {
            unsigned int z = symbolTable.insertFresh(Varname1);
            ASTTerm1_Var1* zVar = new ASTTerm1_Var1(z, form->pos);
            leftEqual = new ASTForm_Equal1(zVar, temp->t, form->pos);
            rightEqual = new ASTForm_Equal1(form->t1, new ASTTerm1_Plus(zVar, temp->n, form->pos), form->pos);
            conjuction = new ASTForm_And(leftEqual, rightEqual, form->pos);
            return (new ASTForm_Ex1(0, new IdentList(z), conjuction, form->pos))->accept(*this);
        }
        // xi = yi -> ex z: z = xi & z = yj
    } else if (form->t1->kind == aPlus1 && form->t2->kind == aPlus1) {
        unsigned int z = symbolTable.insertFresh(Varname1);
        ASTTerm1_Var1* zVar = new ASTTerm1_Var1(z, form->pos);
        leftEqual = new ASTForm_Equal1(zVar, form->t1, form->pos);
        rightEqual = new ASTForm_Equal1(zVar, form->t2, form->pos);
        conjuction = new ASTForm_And(leftEqual, rightEqual, form->pos);
        return (new ASTForm_Ex1(0, new IdentList(z), conjuction, form->pos))->accept(*this);
        // x = e ???
    } else if(form->t1->kind == aVar1 && form->t2->kind == aInt) {
#if (SMART_FLATTEN == true)
		// smart flattening will deal with form during construction of automaton
		return form;
#else
        // other ints are specially handled
        unsigned int val = static_cast<ASTTerm1_Int*>(form->t2)->value();
        if(val == 0) {
            x = new ASTTerm2_Var2(((ASTTerm1_Var1*)form->t1)->getVar(), form->pos);
            ASTList* set = new ASTList();
            set->push_back(static_cast<AST*>(new ASTTerm1_Int(0, form->pos)));
            ASTTerm2_Set* e = new ASTTerm2_Set(set, form->pos);
            return new ASTForm_Equal2(x, e, form->pos);
        } else {
            ASTTerm1_Var1* z = Flattener::generateFreshFirstOrder();
            ASTForm_Equal1* zLess = new ASTForm_Equal1(z, new ASTTerm1_Int(val-1, form->pos), form->pos);
            ASTForm_Equal1* plus = new ASTForm_Equal1(form->t1, new ASTTerm1_Plus(z, 1, form->pos), form->pos);
            ASTForm_And* conjuction = new ASTForm_And(zLess->accept(*this), plus->accept(*this), form->pos);
            return new ASTForm_Ex2(0, new IdentList(z->getVar()), conjuction, form->pos);
        }
#endif
    // x = y - C => x + C = y
    // TODO: form is probably not valid
    } else if(form->t1->kind == aVar1 && form->t2->kind == aMinus1) {
        ASTTerm1_tn* ft2 = static_cast<ASTTerm1_tn*>(form->t2);
        ASTTerm1_Plus* subPlus = new ASTTerm1_Plus(form->t1, ft2->n, form->pos);
        return new ASTForm_Equal1(ft2->t, subPlus, form->pos);
    // Else throw exception
    } else {
        std::cerr << "[!] Unsupported 'Equal1' operation\n";
        throw NotImplementedException();
    }
    return form;
}

/**
 * Flattens the X = Y to simplify the formulae
 *
 * @param[in] form      traversed Equal2 node
 */
AST* Flattener::visit(ASTForm_Equal2* form) {
    assert(form != nullptr);
    assert(form->T1 != nullptr);
    assert(form->T2 != nullptr);

    // TODO: Flattening of +X
    if(form->T2->kind == aSet) {
        ASTList* vars = static_cast<ASTTerm2_Set*>(form->T2)->elements;
        ASTForm* formula = new ASTForm_True(form->pos);
        for(auto var = vars->begin(); var != vars->end(); ++var) {
            ASTForm_In* newIn = new ASTForm_In(static_cast<ASTTerm1*>(*var), form->T1, form->pos);
            formula = new ASTForm_And(formula, newIn, form->pos);
        }
        return formula;
        // Switch the plus
    } else if(form->T1->kind == aPlus2 && form->T2->kind != aPlus2) {
        return new ASTForm_Equal2(form->T2, form->T1, form->pos);
    } else if(form->T1->kind == aPlus2 && form->T2->kind == aPlus2) {
        ASTForm_And* conj;
        ASTForm_Equal2* eqXZ;
        ASTForm_Equal2* eqZY;
        ASTTerm2_Var2* Z = Flattener::generateFreshSecondOrder();

        eqXZ = new ASTForm_Equal2(Z, form->T1, form->pos);
        eqZY = new ASTForm_Equal2(Z, form->T2, form->pos);
        conj = new ASTForm_And(eqXZ, eqZY, form->pos);

        return new ASTForm_Ex2(0, new IdentList(Z->getVar()), conj, form->pos);
    } else {
        return form;
    }
}

/**
 * Flattens the x != y to simplify the formulae
 *
 * @param[in] form      traversed NotEqual1 node
 */
AST* Flattener::visit(ASTForm_NotEqual1* form) {
    // TODO: still needs heavy refactoring
    assert(form != nullptr);
    assert(form->t1 != nullptr);
    assert(form->t2 != nullptr);

    ASTTerm2_Var2* y;
    ASTTerm2_Var2* x;
    ASTForm_NotEqual1* leftEqual;
    ASTForm_NotEqual1* rightEqual;
    ASTForm_And* conjuction;
    // substitute the plus on left side
    if(form->t1->kind == aPlus1) {
        ASTTerm1_Var1* z = Flattener::generateFreshFirstOrder();
        ASTForm_Equal1* zEq;
        ASTForm_NotEqual1 *zNeq;
        ASTForm_And* conj;

        zEq = new ASTForm_Equal1(z, form->t1, form->pos);
        zNeq = new ASTForm_NotEqual1(z, form->t2, form->pos);

        conj = new ASTForm_And(zEq, zNeq, form->pos);

        return (new ASTForm_Ex1(0, new IdentList(z->getVar()), conj, form->pos))->accept(*this);
    } else if(form->t2->kind == aPlus1) {
        ASTTerm1_Var1* z = Flattener::generateFreshFirstOrder();
        ASTForm_Equal1* zEq;
        ASTForm_NotEqual1 *zNeq;
        ASTForm_And* conj;

        zEq = new ASTForm_Equal1(z, form->t2, form->pos);
        zNeq = new ASTForm_NotEqual1(z, form->t1, form->pos);

        conj = new ASTForm_And(zEq, zNeq, form->pos);

        return (new ASTForm_Ex1(0, new IdentList(z->getVar()), conj, form->pos))->accept(*this);
    } else {
        ASTForm_Equal1* eq = new ASTForm_Equal1(form->t1, form->t2, form->pos);
        return new ASTForm_Not(static_cast<ASTForm*>(eq->accept(*this)), form->pos);
    }

    return form;
}

/**
 * Flattens the X != Y to simplify the formulae
 *
 * @param[in] form      traversed NotEqual2 node
 */
AST* Flattener::visit(ASTForm_NotEqual2* form) {
    assert(form != nullptr);

    // TODO: this is wrong i guess
    ASTForm_Equal2* eq = new ASTForm_Equal2(form->T1, form->T2, form->pos);
    return new ASTForm_Not(static_cast<ASTForm*>(eq->accept(*this)), form->pos);
}

/**
 * Substitutes xi < y or y x < yi to new fresh variable with equality so
 * final formula is ex z: z = yi & x < z
 *
 * @param leftTerm: left side of Less (x)
 * @param rightTerm: right side of Less (y)
 * @param substituteLeft: true if left side of expression should be substituted or right
 * @return flattened and freshened formula
 */

ASTForm* substituteFreshLess(ASTTerm1* leftTerm, ASTTerm1* rightTerm, bool substituteLeft, TransformerVisitor& visitor) {
    ASTTerm1_Var1* z;
    ASTForm_Ex1* exists;
    ASTForm_And* conjuction;
    ASTForm* left;
    ASTForm* right;

    z = Flattener::generateFreshFirstOrder();
    if(substituteLeft) {
        left = new ASTForm_Equal1(z, leftTerm,Pos());
        right = new ASTForm_Less(z, rightTerm, Pos());
    } else {
        left = new ASTForm_Equal1(z, rightTerm,Pos());
        right = new ASTForm_Less(leftTerm, z, Pos());
    }
    conjuction = new ASTForm_And(left, right, Pos());

    ASTForm_Ex1* newFormula = new ASTForm_Ex1(0, new IdentList(z->getVar()), conjuction, Pos());
    return static_cast<ASTForm*>(newFormula->accept(visitor));
}

/**
 * Flattens the x < y to simplify the formula
 *
 * @param[in] form:     traversed Less node
 */
AST* Flattener::visit(ASTForm_Less* form) {
    assert(form != nullptr);
    assert(form->t1 != nullptr);
    assert(form->t2 != nullptr);

#if (SMART_FLATTEN == true)
    if (form->t1->kind != aVar1) {
        return substituteFreshLess(form->t1, form->t2, true, *this);
    } else if (form->t2->kind != aVar1) {
        return substituteFreshLess(form->t1, form->t2, false, *this);
    } else {
        return form;
    }
#else
	ASTForm_NotEqual1* leftSide = new ASTForm_NotEqual1(form->t1, form->t2, form->pos);
	ASTForm_LessEq* rightSide = new ASTForm_LessEq(form->t1, form->t2, form->pos);
	ASTForm_And* conjuction = new ASTForm_And(leftSide, rightSide, form->pos);
	return conjuction->accept(*this);
#endif
}

/**
 * Substitutes xi <= y or y x <= yi to new fresh variable with equality so
 * final formula is ex z: z = yi & x <= z
 *
 * @param leftTerm: left side of LessEq (x)
 * @param rightTerm: right side of LessEq (y)
 * @param substituteLeft: true if left side of expression should be substituted or right
 * @return flattened and freshened formula
 */

ASTForm* substituteFreshLessEq(ASTTerm1* leftTerm, ASTTerm1* rightTerm, bool substituteLeft, TransformerVisitor &visitor) {
    ASTTerm1_Var1* z;
    ASTForm_Ex1* exists;
    ASTForm_And* conjuction;
    ASTForm* left;
    ASTForm* right;

    z = Flattener::generateFreshFirstOrder();
    if(substituteLeft) {
        left = new ASTForm_Equal1(z, leftTerm, leftTerm->pos);
        right = new ASTForm_LessEq(z, rightTerm, rightTerm->pos);
    } else {
        left = new ASTForm_Equal1(z, rightTerm, rightTerm->pos);
        right = new ASTForm_LessEq(leftTerm, z, leftTerm->pos);
    }
    conjuction = new ASTForm_And(left, right, leftTerm->pos);

    ASTForm_Ex1* newFormula = new ASTForm_Ex1(0, new IdentList(z->getVar()), conjuction, leftTerm->pos);
    return static_cast<ASTForm*>(newFormula->accept(visitor));
}

/**
 * Flattens the x <= y to simplify the formula
 *
 * @param[in] form:    traversed LessEq node
 */
AST* Flattener::visit(ASTForm_LessEq* form) {
    assert(form != nullptr);
    assert(form->t1 != nullptr);
    assert(form->t2 != nullptr);

    if (form->t1->kind != aVar1) {
        return substituteFreshLessEq(form->t1, form->t2, true, *this);
    } else if (form->t2->kind != aVar1) {
        return substituteFreshLessEq(form->t1, form->t2, false, *this);
    } else {
#if (SMART_FLATTEN == true)
        return form;
#else
		ASTTerm2_Var2* X = Flattener::generateFreshSecondOrder();
		ASTTerm1_Var1* z = Flattener::generateFreshFirstOrder();

		// construction of innerDisjunction (forall Z: z1 in X | z2 in X)
		ASTForm_All1* innerDisjunction;
		if (options.mode != TREE) {
			ASTTerm1_Plus* z1 = new ASTTerm1_Plus(z, 1, form->pos);
			ASTForm_In* z1InX = new ASTForm_In(z1, X, form->pos);
			innerDisjunction = new ASTForm_All1(0, new IdentList(z->getVar()),z1InX , form->pos);
		// TODO: WS2S
		} else {

		}

		// Construction of innerImplication innerDisjuction => z in X
		ASTForm_Impl* innerImplication;
		ASTForm_In* zInX = new ASTForm_In(z, X, form->pos);
		innerImplication = new ASTForm_Impl(innerDisjunction, zInX, form->pos);

		// Construction of innerConjuction (y in X) & innerImplication
		ASTForm_And* innerConjuction;
		ASTForm_In* yInX = new ASTForm_In(form->t2, X, form->pos);
		innerConjuction = new ASTForm_And(yInX, innerImplication, form->pos);

		// Construction outer Implication (innerConjuction) => x in X
		ASTForm_Impl* outerImplication;
		ASTForm_In* xInX = new ASTForm_In(form->t1, X, form->pos);
		outerImplication = new ASTForm_Impl(innerConjuction, xInX, form->pos);

		ASTForm_All2* newFormula = new ASTForm_All2(0, new IdentList(X->getVar()), outerImplication->accept(*this) , form->pos);
		return newFormula->accept(*this);
#endif
    }
    return form;
}

/**
 * Substitutes xi in X to new fresh variable with in predicate so
 * final formula is ex z: z = yi & z in X
 *
 * @param leftTerm: left side of In (yi)
 * @param rightTerm: right side of In (X)
 * @return flattened and freshened formula
 */

ASTForm* substituteFreshIn(ASTTerm1* leftTerm, ASTTerm2* rightTerm, TransformerVisitor &visitor) {
    ASTTerm1_Var1* z;
    ASTForm_Ex1* exists;
    ASTForm_And* conjuction;
    ASTForm* left;
    ASTForm* right;

    z = Flattener::generateFreshFirstOrder();
    left = new ASTForm_Equal1(z, leftTerm,Pos());
    right = new ASTForm_In(z, rightTerm, Pos());
    conjuction = new ASTForm_And(left, right, Pos());

    return new ASTForm_Ex2(0, new IdentList(z->getVar()), static_cast<ASTForm*>(conjuction->accept(visitor)), Pos());
}

/**
 * Flattens the x in X to simplify the formula
 *
 * @param[in] form:    traversed In node
 */
AST* Flattener::visit(ASTForm_In* form) {
    // x in X + i => ex2 Z: x in Z & Z = X + i
    if(form->T2->kind != aVar2) {
        ASTForm_Equal2* zSub;
        ASTForm_In* inSub;
        ASTForm_And* conj;
        ASTTerm2_Var2* Z;

        Z = Flattener::generateFreshSecondOrder();
        zSub = new ASTForm_Equal2(Z, form->T2, form->pos);
        inSub = new ASTForm_In(form->t1, Z, form->pos);
        conj = new ASTForm_And(static_cast<ASTForm*>(inSub->accept(*this)), static_cast<ASTForm*>(zSub->accept(*this)), form->pos);
        return new ASTForm_Ex2(0, new IdentList(Z->getVar()), conj, form->pos);
    } else if (form->t1->kind != aVar1) {
#if (SMART_FLATTEN == true)
        if(form->t1->kind == aInt) {
            return form;
        } else {
            return substituteFreshIn(form->t1, form->T2, *this);
        }
#else
		return substituteFreshIn(form->t1, form->T2, *this);
#endif
    } else {
#if (SMART_FLATTEN == true)
        return form;
#else
		ASTTerm2_Var2 *secondOrderX = new ASTTerm2_Var2(((ASTTerm1_Var1*)form->t1)->n, Pos());
		ASTForm_Sub* subX = new ASTForm_Sub(secondOrderX, form->T2, Pos());
		ASTForm_FirstOrder *singleton = new ASTForm_FirstOrder(new ASTTerm1_Var1(((ASTTerm1_Var1*)form->t1)->n, Pos()), Pos());
		inFirstOrder.insert(((ASTTerm1_Var1*)form->t1)->n);
		return new ASTForm_And(singleton, subX, Pos());
#endif
    }
    return form;
}

/**
 * Flattens the x notin X to simplify the formula
 *
 * @param[in] form:     traversed Notin node
 */
AST* Flattener::visit(ASTForm_Notin* form) {
    // TODO: Consider having this as automat alone
    assert(form != nullptr);
    assert(form->t1 != nullptr);
    assert(form->T2 != nullptr);

    if (form->t1->kind != aVar1) {
        ASTForm_Equal1* zSub;
        ASTForm_Notin* inSub;
        ASTForm_And* conj;
        ASTTerm1_Var1* z;

        z = Flattener::generateFreshFirstOrder();
        zSub = new ASTForm_Equal1(z, form->t1, form->pos);
        inSub = new ASTForm_Notin(z, form->T2, form->pos);
        conj = new ASTForm_And(inSub, zSub, form->pos);
        return (new ASTForm_Ex1(0, new IdentList(z->getVar()), conj, form->pos))->accept(*this);
    } else if(form->T2->kind != aVar2) {
        ASTForm_Equal2* zSub;
        ASTForm_Notin* inSub;
        ASTForm_And* conj;
        ASTTerm2_Var2* Z;

        Z = Flattener::generateFreshSecondOrder();
        zSub = new ASTForm_Equal2(Z, form->T2, form->pos);
        inSub = new ASTForm_Notin(form->t1, Z, form->pos);
        conj = new ASTForm_And(static_cast<ASTForm*>(inSub->accept(*this)), static_cast<ASTForm*>(zSub->accept(*this)), form->pos);
        return new ASTForm_Ex2(0, new IdentList(Z->getVar()), conj, form->pos);
    } else {
        ASTForm_In *newIn = new ASTForm_In(form->t1, form->T2, form->pos);
        return new ASTForm_Not(static_cast<ASTForm*>(newIn->accept(*this)), form->pos);
    }
}

/**
 * Flattens the X sub Y  to simplify the formula
 *
 * @param[in] form:     traversed Sub node
 */
AST* Flattener::visit(ASTForm_Sub* form) {
    assert(form != nullptr);
    assert(form->T1 != nullptr);
    assert(form->T2 != nullptr);

    // TODO: still needs more refactoring
    if(form->T1->kind == aPlus2) {
        ASTForm_And* conj;
        ASTForm_Equal2* subEq;
        ASTForm_Sub* newSub;
        ASTTerm2_Var2* Z = Flattener::generateFreshSecondOrder();

        subEq = new ASTForm_Equal2(Z, form->T1, form->pos);
        newSub = new ASTForm_Sub(Z, form->T2, form->pos);
        conj = new ASTForm_And(static_cast<ASTForm*>(subEq->accept(*this)), static_cast<ASTForm*>(newSub->accept(*this)), form->pos);

        return new ASTForm_Ex2(0, new IdentList(Z->getVar()), conj, form->pos);
    } else if(form->T2->kind == aPlus2) {
        ASTForm_And* conj;
        ASTForm_Equal2* subEq;
        ASTForm_Sub* newSub;
        ASTTerm2_Var2* Z = Flattener::generateFreshSecondOrder();

        subEq = new ASTForm_Equal2(Z, form->T2, form->pos);
        newSub = new ASTForm_Sub(form->T1, Z, form->pos);
        conj = new ASTForm_And(static_cast<ASTForm*>(subEq->accept(*this)), static_cast<ASTForm*>(newSub->accept(*this)), form->pos);

        return new ASTForm_Ex2(0, new IdentList(Z->getVar()), conj, form->pos);
    } else {
        return form;
    }
}

/**
 * Flattens the Ex1 phi to simplify the formula
 *
 * @param[in] form:     traversed Ex1 node
 */
AST* Flattener::visit(ASTForm_Ex1* form) {
    assert(form != nullptr);
    assert(form->f != nullptr);

    Ident* it = form->vl->begin();
    ASTForm_And* conjuction;
    ASTForm* restrictedFormula = new ASTForm_FirstOrder(new ASTTerm1_Var1(*it, form->pos), form->pos);
    inFirstOrder.insert(*it);
    ++it;
    while(it != form->vl->end()) {
        ASTForm_FirstOrder *singleton = new ASTForm_FirstOrder(new ASTTerm1_Var1(*it, form->pos), form->pos);
        inFirstOrder.insert(*it);
        restrictedFormula = new ASTForm_And(restrictedFormula, singleton, form->pos);
        ++it;
    }
    restrictedFormula = new ASTForm_And(restrictedFormula, form->f, form->pos);

    return new ASTForm_Ex2(form->ul, form->vl, restrictedFormula, form->pos);
}

/**
 * Flattens the All1 phi to simplify the formula
 *
 * @param[in] form:     traversed All1 node
 */
AST* Flattener::visit(ASTForm_All1* form) {
    assert(form != nullptr);
    assert(form->f != nullptr);

    Ident* it = form->vl->begin();
    ASTForm_Impl* implication;
    ASTForm* restrictedFormula = new ASTForm_FirstOrder(new ASTTerm1_Var1(*it, form->pos), form->pos);
    inFirstOrder.insert(*it);
    ++it;

    while(it != form->vl->end()) {
        ASTForm_FirstOrder *singleton = new ASTForm_FirstOrder(new ASTTerm1_Var1(*it, form->pos), form->pos);
        inFirstOrder.insert(*it);
        restrictedFormula = new ASTForm_Impl(restrictedFormula, singleton, form->pos);
        ++it;
    }
    restrictedFormula = new ASTForm_Impl(restrictedFormula, form->f, form->pos);

    return new ASTForm_All2(form->ul, form->vl, restrictedFormula, form->pos);
}

/**
 * Unfolds the called macro by substituting its formal parameters with real
 * parameters
 *
 * @param called: called macro
 * @param realParams: real parameters
 * @return: unfolded formula
 */
ASTForm* unfoldFormula(PredLibEntry* called, ASTList* realParams) {
    IdentList* formalParams = called->formals;

    ASTForm* clonnedFormula = (called->ast)->clone();
    ASTForm* unfoldedFormula = clonnedFormula->unfoldMacro(formalParams, realParams);

    Flattener f_visitor;
    return static_cast<ASTForm*>(unfoldedFormula->accept(f_visitor));
}

/**
 * Flattens the Call function to simplify the formula
 *
 * @param[in] form:     traversed Call node
 */
AST* Flattener::visit(ASTForm_Call* form) {
    int calledNumber = form->n;

    PredLibEntry* called = predicateLib.lookup(calledNumber);

    /* So far, we will treat predicate and macro the same */
    if (!called->isMacro) {
        return unfoldFormula(called, form->args);
    } else {
        return unfoldFormula(called, form->args);
    }
}
