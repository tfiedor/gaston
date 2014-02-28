#include "ast.h"
#include "symboltable.h"
#include <cstring>

using std::cout;

extern SymbolTable symbolTable;

/**
 * Flattens formula to second-order variables and restricted syntax so it uses
 * only certain atomic formulae
 *
 * xi = yi -> ex z: z = xi & z = yj
 * s = ti  -> ex z: z = t & zi = s
 * y = xi  -> Xy = Xx i
 * x = e   -> Xx = e
 * x = y   -> Xx = Xy
 *
 * @return: flattened formula
 */
ASTForm* ASTForm_Equal1::flatten() {
	cout << "Flattening formula Equal1\n";
	return this;
}

/**
 * Flattens formula to second-order variables and restricted syntax so it uses
 * only certain atomic formulae
 *
 * x ~= y  -> not x = y
 *
 * @return: flattened formula
 */
ASTForm* ASTForm_NotEqual1::flatten() {
	cout << "Flatting formula NotEqual1\n";
	return this;
}

/**
 * Flattens formula to second-order variables and restricted syntax so it uses
 * only certain atomic formulae
 *
 * x < y  -> x ~= y & x <= y
 *
 * @return: flattened formula
 */
ASTForm* ASTForm_Less::flatten() {
	cout << "Flattening formula Less\n";
	return this;
}

/**
 * Flattens formula to second-order variables and restricted syntax so it uses
 * only certain atomic formulae. Is variable according to the ws1s and ws2s
 *
 * x <= y  -> forall X: (y in X & (forall Z: z1 in X | z2 in X) => z in X) => x in X
 *
 * @return: flattened formula
 */
ASTForm* ASTForm_LessEq::flatten() {
	cout << "Flattening formula LessEq\n";
	return this;
}

/**
 * Flattens formula to second-order variables and restricted sytnax.
 *
 * t in X -> Xy subseteq X
 *
 * @return: flattened formula
 */
ASTForm* ASTForm_In::flatten() {
	cout << "Flattening formula In\n";
	return this;
}

/**
 * Flattens formula to second-order variables and restricted sytnax.
 *
 * t notin X -> not Xy subseteq X
 *
 * @return: flattened formula
 */
ASTForm* ASTForm_Notin::flatten() {
	cout << "Flattening formula Notin\n";
	return this;
}

/**
 * Generic transformation for wide range of formulae
 *
 * @return: flattened formula
 */
ASTForm* ASTForm_f::flatten() {
	cout << "Flattening generic formula f\n";
	f = f->flatten();
	return this;
}

/**
 * Generic transformation for wide range of formulae
 *
 * @return: flattened formula
 */
ASTForm* ASTForm_ff::flatten() {
	cout << "Flattening generic formula ff\n";
    f1 = f1->flatten();
    f2 = f2->flatten();
    return this;
}

/**
 * Generic transformation for wide range of formulae
 *
 * @return: flattened formula
 */
ASTForm* ASTForm_vf::flatten() {
	cout << "Flattening generic formula vf\n";
    f = f->flatten();
    return this;
}

/**
 * Generic transformation for wide range of formulae
 *
 * @return: flattened formula
 */
ASTForm* ASTForm_uvf::flatten() {
	cout << "Flattening generic formula uvf\n";
    f = f->flatten();
    return this;
}
