//
// Created by Raph on 02/02/2016.
//

#ifndef WSKS_CHECKER_H_H
#define WSKS_CHECKER_H_H

#include "../../Frontend/ast.h"
#include "../../Frontend/timer.h"
#include "../environment.hh"

using TimerType = Timer;

/*******************************
 * DEFINITION OF FILTER PHASES *
 *******************************/

#if (MONA_FAIR_MODE == true)
#define FILTER_LIST(code) \
	code(ZeroOrderRemover)				/* Transform zero-order variables to second-order interpretation */ \
	code(SyntaxRestricter)				/* Restrict unsupported formula constructs to supported subset*/ \
	code(BooleanUnfolder)				/* Simplify formula through various boolean laws*/ \
	code(UniversalQuantifierRemover)	/* Remove universal quantifier from formula*/ 
#elif (ANTIPRENEXING_DISTRIBUTIVE == true)
#define FILTER_LIST(code) \
	code(ZeroOrderRemover)				\
	code(SyntaxRestricter)				\
	code(BinaryReorderer)				\
	code(DistributiveAntiPrenexer)
#elif (ANTIPRENEXING_FULL == true)
#define FILTER_LIST(code) \
	code(ZeroOrderRemover)				/* Transform zero-order variables to second-order interpretation */ \
	code(SyntaxRestricter)				/* Restrict unsupported formula constructs to supported subset*/ \
	code(BinaryReorderer)				/* Reorder the formula for better antiprenexing */ \
	code(FullAntiPrenexer)				/* Push quantifiers as deep as possible */ \
	code(BooleanUnfolder)				/* Simplify formula through various boolean laws*/ \
	code(UniversalQuantifierRemover)	/* Remove universal quantifier from formula*/ \
	code(NegationUnfolder)				/* Push negations deeply*/
#else
#define FILTER_LIST(code) \
	code(ZeroOrderRemover)				/* Transform zero-order variables to second-order interpretation */ \
	code(SyntaxRestricter)				/* Restrict unsupported formula constructs to supported subset*/ \
	code(BooleanUnfolder)				/* Simplify formula through various boolean laws*/ \
	code(UniversalQuantifierRemover)	/* Remove universal quantifier from formula*/ \
	/*code(NegationUnfolder)				/* Push negations deeply*/\
	/*code(BaseAutomataMerger)			/* Merge some of the base automata*/
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
	virtual bool Run() = 0;
protected:
    // <<< PRIVATE MEMBERS >>>
    MonaAST* _monaAST;
	bool _printProgress;
	bool _isGround;

	// <<< PRIVATE METHODS >>>
	void _startTimer(Timer& t);
	void _stopTimer(Timer& t, char* s);
	template<class ZeroOrderQuantifier, class FirstOrderQuantifier, class SecondOrderQuantifier>
	ASTForm* _ClosePrefix(IdentList* freeVars, ASTForm* formula);
};
#endif //WSKS_CHECKER_H_H