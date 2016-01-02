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
#include <boost/functional/hash.hpp>
#include <exception>
#include <iostream>
#include <memory>
#include <list>
#include <typeinfo>
#include <vata/bdd_bu_tree_aut.hh>
#include <vata/parsing/timbuk_parser.hh>
#include <vata/serialization/timbuk_serializer.hh>
#include "utils/cached_binary_op.hh"
#include "mtbdd/ondriks_mtbdd.hh"

/*****************************
 * FORWARD CLASS DECLARATION *
 *****************************/
class SymbolicAutomaton;
class ZeroSymbol;
class Term;
class ASTForm;
template<class A, class B, class C, class D, void (*E)(A const&),void (*F)(B&)>
class BinaryCache;
template<class A>
class PairCompare;

namespace Gaston {
	extern size_t hash_value(Term*);
	extern size_t hash_value(std::shared_ptr<ZeroSymbol>&);
	extern size_t hash_value(const ZeroSymbol&);
	extern size_t hash_value(ZeroSymbol*);

	struct ResultHashType {
		size_t operator()(std::pair<Term*, ZeroSymbol*> set) const {
			size_t seed = 0;
			boost::hash_combine(seed, hash_value(set.first));
			boost::hash_combine(seed, hash_value(set.second));
			return seed;
		}
	};

	struct SubsumptionHashType {
		size_t operator()(std::pair<Term*, Term*> set) const {
			size_t seed = 0;
			boost::hash_combine(seed, hash_value(set.first));
			boost::hash_combine(seed, hash_value(set.second));
			return seed;
		}
	};

	void dumpResultKey(std::pair<Term*, ZeroSymbol*> const& s);
	void dumpResultData(std::pair<std::shared_ptr<Term>, bool>& s);
	void dumpTermKey(Term* const& s);
	void dumpSubsumptionKey(std::pair<Term*, Term*> const& s);
	void dumpSubsumptionData(bool& s);

/***************************
 * GLOBAL USING DIRECTIVES *
 ***************************/
	using Automaton 			 = VATA::BDDBottomUpTreeAut;
	using Formula_ptr            = ASTForm*;
	using Term_ptr				 = std::shared_ptr<Term>;
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

	using TermHash 				 = boost::hash<Term_raw>;
	using TermCompare			 = std::equal_to<Term_raw>;
	using TermCache				 = BinaryCache<Term_raw, bool, TermHash, TermCompare, dumpTermKey, dumpSubsumptionData>;
	using ResultKey				 = std::pair<Term_raw, Symbol_ptr>;
	using ResultCache            = BinaryCache<ResultKey, ResultType, ResultHashType, PairCompare<ResultKey>, dumpResultKey, dumpResultData>;
	using SubsumptionKey		 = std::pair<Term_raw, Term_raw>;
	using SubsumptionCache       = BinaryCache<SubsumptionKey, bool, SubsumptionHashType, PairCompare<SubsumptionKey>, dumpSubsumptionKey, dumpSubsumptionData>;

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
enum ComparisonType {E_BY_SAME_PTR, E_BY_DIFFERENT_TYPE, E_BY_STRUCTURE};

/****************
 * DEBUG MACROS *
 ****************/
#define G_DEBUG_FORMULA_AFTER_PHASE(str) cout << "\n\n[*] Formula after '" << str << "' phase:\n"
#define G_NOT_IMPLEMENTED_YET(str) assert(false && "TODO: '" str "' Not Supported yet")

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
#define AUT_CONSTRUCT_BY_MONA			true

/* >>> Debugging Options <<< *
 *****************************/
#define DEBUG_BASE_AUTOMATA 			false
#define DEBUG_FIXPOINT 					false
#define DEBUG_INITIAL_APPROX 			false
#define DEBUG_INTERSECT_NON_EMPTY 		false
#define DEBUG_TERM_UNIQUENESS			false
#define DEBUG_TERM_CREATION				false
#define DEBUG_CACHE_MEMBERS				false
#define DEBUG_WORKSHOPS					false
#define DEBUG_TERM_SUBSUMPTION 			false
#define DEBUG_TERM_CACHE_COMPARISON		false
#define DEBUG_CONTINUATIONS 			false
#define DEBUG_COMPUTE_FULL_FIXPOINT 	false
#define DEBUG_NO_WORKSHOPS				false

/* >>> Measuring Options <<< *
 *****************************/
#define MEASURE_STATE_SPACE 			true
#define MEASURE_CACHE_HITS 				true
#define MEASURE_CONTINUATION_CREATION	true
#define MEASURE_CONTINUATION_EVALUATION	true
#define MEASURE_RESULT_HITS				true
#define MEASURE_PROJECTION				true
#define MEASURE_ALL						true
#define MEASURE_COMPARISONS				false
#define MEASURE_SUBSUMEDBY_HITS			true

/* >>> Anti-Prenexing Options <<< *
 **********************************/
#define ANTIPRENEXING_FULL				true

/* >>> Optimizations <<< *
 *************************/
#define OPT_EQ_THROUGH_POINTERS			true	// < Tests equality through pointers not by structure
#define OPT_GENERATE_UNIQUE_TERMS		true	// < Uses Workshop to generate unique pointers
#define OPT_TERM_HASH_BY_APPROX			true	// < Includes stateSpaceApprox into hash
#define OPT_ANTIPRENEXING				true	// < Transforms formula to anti-prenex form (i.e. all of the quantifiers are deepest on leaves)
#define OPT_DRAW_NEGATION_IN_BASE 		false	// < Negation is handled on formula level and not on computation level
#define OPT_CREATE_QF_AUTOMATON 		true	// < Transforms Quantifier-free automaton to formula
#define OPT_REDUCE_AUT_EVERYTIME		false	// < Calls reduce everytime VATA automaton is created
#define OPT_REDUCE_AUT_LAST				true	// < Calls reduce after the final automaton is created
#define OPT_EARLY_EVALUATION 			false	// < Evaluates early interesection of products
#define OPT_PRUNE_EMPTY					true	// < Prunes empty sets
#define OPT_PRUNE_FIXPOINT				true	// < Prunes fixpoint during IsSubsumedBy TODO: For BaseSet only for now
#define OPT_CACHE_RESULTS 				true	// < Cache results
#define OPT_CACHE_SUBSUMES				true	// < Caches the results of IsSubsumed function on terms
#define OPT_CACHE_SUBSUMED_BY			true	// < Caches the results of IsSubsumedBy function in fixpoints

/*******************************
 * DEFINITION OF FILTER PHASES *
 *******************************/

#define FILTER_LIST(code) \
	code(SyntaxRestricter)				\
	code(BinaryReorderer)				\
	code(FullAntiPrenexer)				\
	code(UniversalQuantifierRemover)	\
	code(NegationUnfolder)				\
	code(SecondOrderRestricter)

#endif
