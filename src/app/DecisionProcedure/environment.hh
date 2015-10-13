/*****************************************************************************
 *  dWiNA - Deciding WSkS using non-deterministic automata
 *
 *  Copyright (c) 2014  Tomas Fiedor <xfiedo01@stud.fit.vutbr.cz>
 *
 *  Description:
 *    Some common things
 *
 *****************************************************************************/

#ifndef __DWINA_ENV__H__
#define __DWINA_ENV__H__

#include <exception>
#include <iostream>

class NotImplementedException : public std::exception {
	public:
		virtual const char* what() const throw () {
			return "Functionality not implemented yet";
		}
};

enum Decision {SATISFIABLE, UNSATISFIABLE, VALID, INVALID};

/**
 * Configuration macros
 */
#define CONSTRUCT_ALWAYS_DTA false

/**
 * Enabling debugging
 */
#define USE_PRUNED_UNION_FUNCTOR false

#define DEBUG_FORMULA_PREFIX false
#define DEBUG_VALIDITY_TEST false
#define DEBUG_GROUDNESS false
#define DEBUG_FINAL_STATES false
#define DEBUG_PRUNING_OF_FINAL_STATES false
#define DEBUG_VARIABLE_SETS false
#define DEBUG_BDDS true

#define DEBUG_BASE_AUTOMATA false
#define DEBUG_FIXPOINT true
#define DEBUG_INITIAL_APPROX true
#define DEBUG_INTERSECT_NON_EMPTY false

/**
 * Enabling the optimizations
 */
#define PRUNE_BY_RELATION false		// [TODO] What's the difference with BY_SUBSUMPTION?
#define PRUNE_BY_SUBSUMPTION false
#define SMART_FLATTEN true
#define USE_STATECACHE true
#define USE_BDDCACHE false 			// BDD Cache is temporary disable due to the memory leaks
#define SMART_BINARY true

/*********************************************
 * NOVEL OPTIMIZATIONS IN SYMBOLIC COMPUTING *
 *********************************************/

#define OPT_DRAW_NEGATION_IN_BASE true
#define OPT_CREATE_QF_AUTOMATON false
#define OPT_REDUCE_AUTOMATA true
#endif
