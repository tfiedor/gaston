/*****************************************************************************
 *  gaston - We pay homage to Gaston, an Africa-born brown fur seal who
 *    escaped the Prague Zoo during the floods in 2002 and made a heroic
 *    journey for freedom of over 300km all the way to Dresden. There he
 *    was caught and subsequently died due to exhaustion and infection.
 *    Rest In Piece, brave soldier.
 *
 *  Copyright (c) 2015  Tomas Fiedor <ifiedortom@fit.vutbr.cz>
 *      Notable mentions: Ondrej Lengal <ondra.lengal@gmail.com>
 *
 *  Description:
 *		Global header file for Gaston tool, containing options for
 *		enabling/disabling debug messages, optimizations, measuring
 *		and some globally used enumerations and using directives.
 *****************************************************************************/

#ifndef __DWINA_ENV__H__
#define __DWINA_ENV__H__

#include <boost/dynamic_bitset.hpp>
#include <exception>
#include <iostream>
#include <memory>
#include <list>
#include <vata/bdd_bu_tree_aut.hh>
#include <vata/parsing/timbuk_parser.hh>
#include <vata/serialization/timbuk_serializer.hh>
#include "utils/cached_binary_op.hh"
#include "mtbdd/ondriks_mtbdd.hh"
#include "containers/SymbolicCache.hh"

/*****************************
 * FORWARD CLASS DECLARATION *
 *****************************/
class SymbolicAutomaton;
class ZeroSymbol;
class Term;
class ASTForm;

struct ResultHashType {
	/**
      * @param set: set we are computing hash of
      * @return hash of @p set
      */
	int operator()(std::pair<Term*, std::shared_ptr<ZeroSymbol>> set) const {
		// TODO: OPTIMIZE THIS
		return 1;
	}
};

/***************************
 * GLOBAL USING DIRECTIVES *
 ***************************/
namespace Gaston {
	using Automaton 			 = VATA::BDDBottomUpTreeAut;
	using Formula_ptr            = ASTForm*;
	using Term_ptr				 = std::shared_ptr<Term>;
	using Term_weak				 = std::weak_ptr<Term>;
	using Term_raw 				 = Term*;
	using ResultType			 = std::pair<Term_ptr, bool>;

	using StateType				 = size_t;
	using StateTuple 			 = std::vector<StateType>;
	using StateToStateTranslator = VATA::AutBase::StateToStateTranslWeak;
	using StateToStateMap        = std::unordered_map<StateType, StateType>;

	using SymbolicAutomaton_ptr	 = std::shared_ptr<SymbolicAutomaton>;
	using SymbolicAutomaton_raw	 = SymbolicAutomaton*;

	using Symbol				 = ZeroSymbol;
	using Symbol_ptr			 = Symbol*;
	using Symbol_shared			 = std::shared_ptr<Symbol>;
	using SymbolList			 = std::list<Symbol>;

	using BitMask				 = boost::dynamic_bitset<>;
	using VarType				 = size_t;
	using VarList                = VATA::Util::OrdVector<StateType>;
	using VarValue			     = char;
	using TrackType				 = Automaton::SymbolType;

	using ResultCache            = BinaryCache<Term_raw, Symbol_shared, ResultType, ResultHashType>;
	using SubsumptionCache       = VATA::Util::CachedBinaryOp<Term_ptr, Term_ptr, bool>;

	using WorkListTerm           = Term;
	using WorkListTerm_raw       = Term*;
	using WorkListTerm_ptr       = Term_ptr;
	using WorkListSet            = std::vector<std::shared_ptr<WorkListTerm>>;

	using BaseAutomatonType      = VATA::BDDBottomUpTreeAut;
	using BaseAutomatonStateSet  = VATA::Util::OrdVector<StateType>;
	using BaseAutomatonMTBDD	 = VATA::MTBDDPkg::OndriksMTBDD<BaseAutomatonStateSet>;
}

/*************************
 * ADDITIONAL STRUCTURES *
 *************************/

class NotImplementedException : public std::exception {
public:
	virtual const char* what() const throw () {
		return "Functionality not implemented yet";
	}
};

/***********************
 * GLOBAL ENUMERATIONS *
 ***********************/
enum Decision {SATISFIABLE, UNSATISFIABLE, VALID, INVALID};
enum AutType {SYMBOLIC_BASE, BINARY, INTERSECTION, UNION, PROJECTION, BASE, COMPLEMENT};
enum TermType {TERM, TERM_EMPTY, TERM_FIXPOINT, TERM_PRODUCT, TERM_BASE, TERM_LIST, TERM_CONTINUATION};
enum ProductType {E_INTERSECTION, E_UNION};
enum FixpointTermSem {E_FIXTERM_FIXPOINT, E_FIXTERM_PRE};

/*********************************
 * OTHER METHODS RELATED DEFINES *
 *********************************/

/* >>> Debugging Options <<< *
 *****************************/
#define DEBUG_FORMULA_PREFIX 			false
#define DEBUG_VALIDITY_TEST 			false
#define DEBUG_GROUDNESS 				false
#define DEBUG_FINAL_STATES 				false
#define DEBUG_PRUNING_OF_FINAL_STATES 	false
#define DEBUG_VARIABLE_SETS 			false
#define DEBUG_BDDS 						true

/* >>> Optimizations <<< *
 *************************/
#define USE_PRUNED_UNION_FUNCTOR 		false
#define PRUNE_BY_RELATION 				false		// [TODO] What's the difference with BY_SUBSUMPTION?
#define PRUNE_BY_SUBSUMPTION 			false
#define USE_STATECACHE 					true
#define USE_BDDCACHE 					false 		// BDD Cache is temporary disable due to the memory leaks
#define SMART_BINARY 					true
#define SMART_FLATTEN 					true
#define CONSTRUCT_ALWAYS_DTA 			true

/***********************************
 * SYMBOLIC METHOD RELATED DEFINES *
 ***********************************/

/* >>> Printing Options <<< *
 ****************************/
#define PRINT_PRETTY					true

/* >>> Other Options <<< *
 *************************/
#define AUT_ALWAYS_DETERMINISTIC		false
#define AUT_ALWAYS_CONSTRAINT_FO		false

/* >>> Debugging Options <<< *
 *****************************/
#define DEBUG_BASE_AUTOMATA 			false
#define DEBUG_FIXPOINT 					true
#define DEBUG_INITIAL_APPROX 			true
#define DEBUG_INTERSECT_NON_EMPTY 		false
#define DEBUG_TERM_SUBSUMPTION 			false
#define DEBUG_CONTINUATIONS 			false
#define DEBUG_COMPUTE_FULL_FIXPOINT 	false

/* >>> Measuring Options <<< *
 *****************************/
#define MEASURE_STATE_SPACE 			true
#define MEASURE_CACHE_HITS 				true
#define MEASURE_CONTINUATION_CREATION	true
#define MEASURE_CONTINUATION_EVALUATION	true
#define MEASURE_RESULT_HITS				true
#define MEASURE_PROJECTION				true
#define MEASURE_ALL						true

/* >>> Optimizations <<< *
 *************************/
#define OPT_DRAW_NEGATION_IN_BASE 		true
#define OPT_CREATE_QF_AUTOMATON 		false
#define OPT_REDUCE_AUTOMATA 			false
#define OPT_EARLY_EVALUATION 			false
#define OPT_CACHE_RESULTS 				true
#endif
