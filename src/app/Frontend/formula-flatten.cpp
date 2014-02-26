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
