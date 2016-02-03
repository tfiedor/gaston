//
// Created by Raph on 02/02/2016.
//

#ifndef WSKS_CHECKER_H_H
#define WSKS_CHECKER_H_H

#include "../../Frontend/ast.h"

#endif //WSKS_CHECKER_H_H


/*******************************
 * DEFINITION OF FILTER PHASES *
 *******************************/

#if (ANTIPRENEXING_DISTRIBUTIVE == true)
#define FILTER_LIST(code) \
	code(ZeroOrderRemover)				\
	code(SyntaxRestricter)				\
	code(BinaryReorderer)				\
	code(DistributiveAntiPrenexer)		\
	code(SecondOrderRestricter)			/* Restrict the formula to second order*/\
	code(QuantificationMerger)			/* Merge some quantifications */
#elif (ANTIPRENEXING_FULL == true)
#define FILTER_LIST(code) \
	code(ZeroOrderRemover)				/* Transform zero-order variables to second-order interpretation */ \
	code(SyntaxRestricter)				/* Restrict unsupported formula constructs to supported subset*/ \
	code(BinaryReorderer)				/* Reorder the formula for better antiprenexing */ \
	code(FullAntiPrenexer)				/* Push quantifiers as deep as possible */ \
	code(BooleanUnfolder)				/* Simplify formula through various boolean laws*/ \
	code(UniversalQuantifierRemover)	/* Remove universal quantifier from formula*/ \
	code(NegationUnfolder)				/* Push negations deeply*/ \
	code(QuantificationMerger)			/* Merge some quantifications */ \
	code(SecondOrderRestricter)			/* Restrict the formula to second order*/\
	code(QuantificationMerger)			/* Merge some quantifications */
#else
#define FILTER_LIST(code) \
	code(ZeroOrderRemover)				/* Transform zero-order variables to second-order interpretation */ \
	code(SyntaxRestricter)				/* Restrict unsupported formula constructs to supported subset*/ \
	code(BooleanUnfolder)				/* Simplify formula through various boolean laws*/ \
	code(UniversalQuantifierRemover)	/* Remove universal quantifier from formula*/ \
	code(NegationUnfolder)				/* Push negations deeply*/ \
	code(SecondOrderRestricter)			/* Restrict the formula to second order*/\
	code(QuantificationMerger)			/* Merge some quantifications */
#endif

class Checker {
public:
    // <<< PUBLIC CONSTRUCTORS AND DESTRUCTORS >>>
    Checker() {}
    virtual ~Checker();

    // <<< PUBLIC API >>>
    void LoadFormulaFromFile();
    void CloseUngroundFormula();
    void PreprocessFormula();
    void CreateAutomataSizeEstimations();
    virtual void ConstructAutomaton() = 0;
    virtual void Decide() = 0;
protected:
    // <<< PRIVATE MEMBERS >>>
    MonaAST* _monaAST;
};