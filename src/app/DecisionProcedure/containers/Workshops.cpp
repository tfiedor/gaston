#include "Workshops.h"
#include "Term.h"
#include "VarToTrackMap.hh"
#include <stdint.h>
#include <algorithm>    // std::copy
#include <iterator>

extern VarToTrackMap varMap;

namespace Workshops {
    NEVER_INLINE TermWorkshop::TermWorkshop(SymbolicAutomaton* aut) :
            _bCache(nullptr), _ubCache(nullptr), _pCache(nullptr), _tpCache(nullptr), _npCache(nullptr), _lCache(nullptr),
            _fpCache(nullptr), _fppCache(nullptr), _contCache(nullptr),_compCache(nullptr), _aut(aut) { }

    template<class A, class B, class C, class D, void (*E)(const A&), void (*F)(B&)>
    BinaryCache<A, B, C, D, E, F>* TermWorkshop::_cleanCache(BinaryCache<A, B, C, D, E, F>* cache, bool noMemberDelete) {
        if(cache != nullptr) {
            if(!noMemberDelete) {
                for (auto it = cache->begin(); it != cache->end(); ++it) {
                    delete it->second;
                }
            }
            delete cache;
        }

        return nullptr;
    }

    NEVER_INLINE TermWorkshop::~TermWorkshop() {
        this->_bCache = TermWorkshop::_cleanCache(this->_bCache, OPT_USE_BOOST_POOL_FOR_ALLOC);
        this->_ubCache = TermWorkshop::_cleanCache(this->_ubCache, true);
        this->_fpCache = TermWorkshop::_cleanCache(this->_fpCache, OPT_USE_BOOST_POOL_FOR_ALLOC);
        this->_fppCache = TermWorkshop::_cleanCache(this->_fppCache, OPT_USE_BOOST_POOL_FOR_ALLOC);
        this->_pCache = TermWorkshop::_cleanCache(this->_pCache, OPT_USE_BOOST_POOL_FOR_ALLOC);
        this->_tpCache = TermWorkshop::_cleanCache(this->_tpCache, OPT_USE_BOOST_POOL_FOR_ALLOC);
        this->_npCache = TermWorkshop::_cleanCache(this->_npCache, OPT_USE_BOOST_POOL_FOR_ALLOC);
        this->_lCache = TermWorkshop::_cleanCache(this->_lCache);
        this->_contCache = TermWorkshop::_cleanCache(this->_contCache);
        this->_compCache = TermWorkshop::_cleanCache(this->_compCache, true);

        if(TermWorkshop::_empty != nullptr) {
            delete TermWorkshop::_empty;
            TermWorkshop::_empty = nullptr;
        }
        if(TermWorkshop::_emptyComplement != nullptr) {
            delete TermWorkshop::_emptyComplement;
            TermWorkshop::_emptyComplement = nullptr;
        }
    }

    // ComputationKey    = std::pair<FixpointType*, WorklistType*>;
    struct ComputationCompare : public std::binary_function<ComputationKey, ComputationKey, bool>
    {
        /**
         * @param lhs: left operand
         * @param rhs: right operand
         * @return true if lhs = rhs
         */
        bool operator()(ComputationKey const& lhs, ComputationKey const& rhs) const {
#           if (OPT_UNIQUE_FIXPOINTS_BY_SUB == true)
            bool result = lhs->IsSubsumed(rhs, true) == E_TRUE;
#           else
            bool result = *lhs == *rhs;
#           endif
            return result;
        }
    };

    struct ComputationHash {
        size_t operator()(ComputationKey const& set) const {
            size_t seed = boost::hash_value(set->stateSpaceApprox);
#           if (OPT_UNIQUE_FIXPOINTS_BY_SUB == false)
            boost::hash_combine(seed, boost::hash_value(set->MeasureStateSpace()));
#           endif
            return seed;
        }
    };

    void TermWorkshop::InitializeWorkshop() {
        switch(this->_aut->type) {
            case AutType::BASE:
                this->_bCache = new BaseCache();
                this->_ubCache = new ProductCache();
                break;
            case AutType::BINARY:
            case AutType::INTERSECTION:
            case AutType::UNION:
            case AutType::IMPLICATION:
            case AutType::BIIMPLICATION:
                this->_pCache = new ProductCache();
#               if (OPT_EARLY_EVALUATION == true && MONA_FAIR_MODE == false)
                this->_contCache = new FixpointCache();
#               endif
                break;
            case AutType::TERNARY:
            case AutType::TERNARY_INTERSECTION:
            case AutType::TERNARY_UNION:
            case AutType::TERNARY_IMPLICATION:
            case AutType::TERNARY_BIIMPLICATION:
                this->_tpCache = new TernaryCache();
                break;
            case AutType::NARY:
            case AutType::NARY_INTERSECTION:
            case AutType::NARY_UNION:
            case AutType::NARY_IMPLICATION:
            case AutType::NARY_BIIMPLICATION:
                this->_npCache = new NaryCache();
                break;
            case AutType::COMPLEMENT:
                break;
            case AutType::PROJECTION:
                this->_lCache = new ListCache();
                this->_fpCache = new ListCache();
                this->_fppCache = new FixpointCache();
                this->_compCache = new ComputationCache();
                break;
            default:
                assert(false && "Missing implementation for this type");
                break;
        }
    }

    TermEmpty* TermWorkshop::_empty = nullptr;
    TermEmpty* TermWorkshop::_emptyComplement = nullptr;
    unsigned long TermWorkshop::monaAutomataStates = 0;
#   if (OPT_USE_BOOST_POOL_FOR_ALLOC == true)
    boost::object_pool<TermFixpoint> TermWorkshop::_fixpointPool;
    boost::object_pool<TermProduct> TermWorkshop::_productPool;
    boost::object_pool<TermTernaryProduct> TermWorkshop::_ternaryProductPool;
    boost::object_pool<TermNaryProduct> TermWorkshop::_naryProductPool;
    boost::object_pool<TermBaseSet> TermWorkshop::_basePool;
#   endif

    TermEmpty* TermWorkshop::CreateEmpty() {
        if(TermWorkshop::_empty == nullptr) {
            TermWorkshop::_empty = new TermEmpty(nullptr);
        }
        assert(TermWorkshop::_empty != nullptr);
        return TermWorkshop::_empty;
    }

    TermEmpty* TermWorkshop::CreateComplementedEmpty() {
        if(TermWorkshop::_emptyComplement == nullptr) {
            TermWorkshop::_emptyComplement = new TermEmpty(nullptr, true);
        }
        assert(TermWorkshop::_emptyComplement != nullptr);
        return TermWorkshop::_emptyComplement;
    }

    /**
     * Checks if there is already created TermBaseSet in cache, in case there is not
     * it creates the new object and populates the cache.
     *
     * @param[in] states:       parameters used to create TermBaseSet
     * @param[in] offset:       offset for bitmask
     * @param[in] stateno:      state number for bitmask
     * @return:                 unique pointer for TermBaseSet
     */
    Term* TermWorkshop::CreateBaseSet(VATA::Util::OrdVector<size_t> && states) {
#       if (OPT_GENERATE_UNIQUE_TERMS == true && UNIQUE_BASE == true)
            assert(this->_bCache != nullptr);

            if(states.size() == 0) {
                return this->CreateEmpty();
            }

            Term* termPtr = nullptr;
            if(!this->_bCache->retrieveFromCache(states, termPtr)) {
#               if (DEBUG_WORKSHOPS == true && DEBUG_TERM_CREATION == true)
                std::cout << "[*] Creating BaseSet: ";
#               endif
                // The object was not created yet, so we create it and store it in cache
#               if (OPT_USE_BOOST_POOL_FOR_ALLOC == true)
                termPtr = TermWorkshop::_basePool.construct(this->_aut, states);
#               else
                termPtr = new TermBaseSet(this->_aut, std::move(states));
#               endif
                this->_bCache->StoreIn(states, termPtr);
            }
            assert(termPtr != nullptr);
            return termPtr;
#       else
            return new TermBaseSet(states;
#       endif
    }

    Term* TermWorkshop::CreateUnionBaseSet(const Term_ptr& lhs, const Term_ptr& rhs) {
        if(lhs == nullptr) {
            return rhs;
        } else if(lhs == rhs) {
            return lhs;
        } else if(lhs->type == TermType::EMPTY) {
            return rhs;
        } else if(rhs->type == TermType::EMPTY) {
            return lhs;
        } else {
            Term_ptr result;
            auto key = std::make_pair(lhs, rhs);
            if(!this->_ubCache->retrieveFromCache(key, result)) {
                TermBaseSet* llhs = static_cast<TermBaseSet*>(lhs);
                result = this->CreateBaseSet(llhs->states.Union(static_cast<TermBaseSet*>(rhs)->states));
                this->_ubCache->StoreIn(key, result);
            }
            return result;
        }
    }

    Term* TermWorkshop::CreateTernaryProduct(const Term_ptr& lhs, const Term_ptr& mhs, const Term_ptr& rhs, ProductType type) {
#       if (OPT_GENERATE_UNIQUE_TERMS == true && UNIQUE_PRODUCTS == true)
        assert(this->_tpCache != nullptr);

        Term* termPtr = nullptr;
        auto ternaryKey = std::make_tuple(lhs, mhs, rhs);
        if(!this->_tpCache->retrieveFromCache(ternaryKey, termPtr)) {
#           if (OPT_USE_BOOST_POOL_FOR_ALLOC == true)
            termPtr = TermWorkshop::_ternaryProductPool.construct(this->_aut, std::make_tuple(lhs, mhs, rhs), type);
#           else
            termPtr = new TermTernaryProduct(this->_aut, lhs, mhs, rhs, type);
#           endif
            this->_tpCache->StoreIn(ternaryKey, termPtr);
        }
        assert(termPtr != nullptr);
        return termPtr;
#       else
        return new TermTernaryProduct(lhs, mhs, rhs, type);
#       endif
    }

    Term* TermWorkshop::CreateNaryProduct(Term_ptr const& base, Symbol* symbol, size_t arity, ProductType pt) {
#       if (OPT_USE_BOOST_POOL_FOR_ALLOC == true)
        return TermWorkshop::_naryProductPool.construct(this->_aut, std::make_pair(base, symbol), std::make_pair(pt, arity));
#       else
        return new TermNaryProduct(this->_aut, base, symbol, pt, arity);
#       endif
    }

    Term* TermWorkshop::CreateNaryProduct(Term_ptr* const& terms, size_t arity, ProductType pt) {
#       if (OPT_GENERATE_UNIQUE_TERMS == true && UNIQUE_PRODUCTS)
        assert(this->_npCache != nullptr);

        Term* termPtr = nullptr;
        auto naryKey = std::make_pair(terms, arity);
        if(!this->_npCache->retrieveFromCache(naryKey, termPtr)) {
#           if (OPT_USE_BOOST_POOL_FOR_ALLOC == true)
            termPtr = TermWorkshop::_naryProductPool.construct(this->_aut, terms, std::make_pair(pt, arity));
#           else
            termPtr = new TermNaryProduct(this->_aut, terms, pt, arity);
#           endif
            this->_npCache->StoreIn(naryKey, termPtr);
        }
        assert(termPtr != nullptr);
        return termPtr;
#       else
        return new TermNaryProduct(this->_aut, terms, pt, arity);
#       endif
    }

    Term* TermWorkshop::CreateUniqueNaryProduct(Term_ptr* const& terms, size_t arity, ProductType pt) {
        return nullptr;
    }

    Term* TermWorkshop::CreateBaseNaryProduct(SymLink* symlinks, size_t arity, StatesSetType st, ProductType pt) {
#       if (OPT_USE_BOOST_POOL_FOR_ALLOC == true)
        return TermWorkshop::_naryProductPool.construct(this->_aut, symlinks, std::make_tuple(st, pt, arity));
#       else
        return new TermNaryProduct(this->_aut, symlinks, st, pt, arity);
#       endif
    }

    /**
     * Checks if there is already created TermProduct in cache, in case there is not
     * it creates the new object and populates the cache
     *
     * @param[in] lptr:     left term of product
     * @param[in] rptr:     right term of product
     * @param[in] type:     type of the product
     */
    TermProduct* TermWorkshop::CreateProduct(Term_ptr const& lptr, Term_ptr const&rptr, ProductType type) {
        // TODO: Can there be sets that have same lhs rhs, but different product type??
        // TODO: I don't think so actually, because this is on the node, so it cannot generate different things
        // TODO: And same thing goes for complements
#       if (OPT_GENERATE_UNIQUE_TERMS == true && UNIQUE_PRODUCTS == true)
        assert(this->_pCache != nullptr);

        Term* termPtr = nullptr;
        auto productKey = std::make_pair(lptr, rptr);
        if(!this->_pCache->retrieveFromCache(productKey, termPtr)) {
#           if (DEBUG_WORKSHOPS == true && DEBUG_TERM_CREATION == true)
            std::cout << "[*] Creating Product: ";
            std::cout << "from ["<< lptr << "] + [" << rptr << "] to ";
#           endif
            // The object was not created yet, so we create it and store it in cache
#           if (OPT_USE_BOOST_POOL_FOR_ALLOC == true)
            termPtr = TermWorkshop::_productPool.construct(this->_aut, std::make_pair(lptr, rptr), type);
#           else
            termPtr = new TermProduct(this->_aut, lptr, rptr, type);
#           endif
            this->_pCache->StoreIn(productKey, termPtr);
        }
        assert(termPtr != nullptr);
        return reinterpret_cast<TermProduct*>(termPtr);
#       else
        return new TermProduct(this->_aut, lptr, rptr, type);
#       endif
    }

    /**
     * Checks if there is already created list with one symbol, in case there is
     * not it creates the new object and populates the cache
     *
     * @param[in] startTerm:        first term in list
     * @param[in] inComplement:     whether the list is complemented
     * @return: unique pointer
     */
    Term* TermWorkshop::CreateList(Term_ptr const& startTerm, bool inComplement) {
        if(startTerm->type == TermType::EMPTY) {
            return startTerm;
        }
        #if (OPT_GENERATE_UNIQUE_TERMS == true && UNIQUE_LISTS == true)
            assert(this->_lCache != nullptr);

            Term* termPtr = nullptr;
            auto productKey = startTerm;
            if(!this->_lCache->retrieveFromCache(productKey, termPtr)) {
                #if (DEBUG_WORKSHOPS == true && DEBUG_TERM_CREATION == true)
                std::cout << "[*] Creating List: ";
                std::cout << "from ["<< startTerm << "] to ";
                #endif
                termPtr = new TermList(this->_aut, startTerm, inComplement);
                this->_lCache->StoreIn(productKey, termPtr);
            }
            assert(termPtr != nullptr);
            return reinterpret_cast<TermList*>(termPtr);
        #else
            return new TermList(this->_aut, startTerm, inComplement);
        #endif
    }

    TermFixpoint* TermWorkshop::CreateFixpoint(Term_ptr const& source, Symbol* symbol, bool inCompl, bool initValue, WorklistSearchType search) {
        #if (OPT_GENERATE_UNIQUE_TERMS == true && UNIQUE_FIXPOINTS == true)
            assert(this->_fpCache != nullptr);

            Term* termPtr = nullptr;
            auto fixpointKey = source;
            if(!this->_fpCache->retrieveFromCache(fixpointKey, termPtr)) {
                #if (DEBUG_WORKSHOPS == true && DEBUG_TERM_CREATION == true)
                std::cout << "[*] Creating Fixpoint: ";
                std::cout << "from [" << source << "] to ";
                #endif
#               if (OPT_USE_BOOST_POOL_FOR_ALLOC == true)
                termPtr = TermWorkshop::_fixpointPool.construct(this->_aut, std::make_tuple(source, symbol, inCompl, initValue, search));
#               else
                termPtr = new TermFixpoint(this->_aut, source, symbol, inCompl, initValue, search);
#               endif
                this->_fpCache->StoreIn(fixpointKey, termPtr);
            }
            assert(termPtr != nullptr);
            return reinterpret_cast<TermFixpoint*>(termPtr);
        #else
            return new TermFixpoint(this->_aut, source, symbol, inCompl, initValue);
        #endif
    };

    TermFixpoint* TermWorkshop::CreateFixpointPre(Term_ptr const& source, Symbol* symbol, bool inCompl) {
        #if (OPT_GENERATE_UNIQUE_TERMS == true && UNIQUE_FIXPOINTS == true)
            // Project the symbol as key
            assert(this->_fppCache != nullptr);
            assert(source != nullptr);
            Term_ptr unique_source = source;
            if(source->type == TermType::FIXPOINT) {
                TermFixpoint* fp = static_cast<TermFixpoint*>(source);
                unique_source = this->GetUniqueFixpoint(fp);
            }

            Term* termPtr = nullptr;
            Symbol* symbolKey = symbol;

            auto fixpointKey = std::make_pair(unique_source, symbol);
            if(!this->_fppCache->retrieveFromCache(fixpointKey, termPtr)) {
                #if (DEBUG_WORKSHOPS == true && DEBUG_TERM_CREATION == true)
                std::cout << "[*] Creating FixpointPre: ";
                std::cout << "from [" << source << "] to ";
                #endif
#               if (OPT_USE_BOOST_POOL_FOR_ALLOC == true)
                termPtr = TermWorkshop::_fixpointPool.construct(this->_aut, std::make_pair(unique_source, symbolKey), inCompl);
#               else
                termPtr = new TermFixpoint(this->_aut, source, symbolKey, inCompl);
#               endif
                this->_fppCache->StoreIn(fixpointKey, termPtr);
            }
            assert(termPtr != nullptr);
            return reinterpret_cast<TermFixpoint*>(termPtr);
        #else
            return new TermFixpoint(this->_aut, source, symbol, inCompl);
        #endif
    }

    /**
     * @brief Clones the @p fixpoint up to the level given by params
     *
     * Takes the fixpoint @p fixpoint, and clones it to brand new fixpoint, that has the
     * same source, same symbol parts, fixpoint members and worklist items up to the @p params.level.
     *
     * @param[in]  fixpoint  fixpoint we are cloning
     * @param[in]  params  parameters we are currently subtracting
     * @return  cloned fixpoint
     */
    TermFixpoint* TermWorkshop::CreateClonedFixpoint(TermFixpoint* const& fixpoint, IntersectNonEmptyParams& params) {
        assert(fixpoint->type == TermType::FIXPOINT);

        // DEBUG std::cout << "Cloning: ";
        // DEBUG fixpoint->dump();
        // DEBUG std::cout << "\n";
        // DEBUG std::cout << "With symbol parts: {";
        for (auto &symPart : fixpoint->_symbolParts) {
            // DEBUG std::cout << symPart << ", ";
        }
        // DEBUG std::cout << "}\n";

        TermFixpoint* clonedFixpoint = TermWorkshop::_fixpointPool.construct(this->_aut,
            std::make_pair(fixpoint->_sourceTerm, fixpoint->_sourceSymbol), GET_NON_MEMBERSHIP_TESTING(fixpoint));

        // Clone only those worklisted items that have level greater than @p params.variableLevel
        for(WorklistItem& item : fixpoint->_worklist) {
            if(item.level > params.variableLevel) {
                clonedFixpoint->_worklist.emplace_back(item.term, item.symbol, item.level, item.value);
            }
        }

        // Clone only the part of the symbol parts
        int copy_number = varMap.TrackLength() - 1 - params.variableLevel;
        // DEBUG std::cout << "Copying: " << copy_number << "\n";
        // DEBUG std::cout << "Before: " << clonedFixpoint->_symbolParts.size() << "\n";
        std::copy_n(fixpoint->_symbolParts.begin(), copy_number, std::back_inserter(clonedFixpoint->_symbolParts));
        // DEBUG std::cout << "After : " << clonedFixpoint->_symbolParts.size() << "\n";

        // Only terms from variable level greater than the @p params.variableLevel are included
        for(FixpointMember& member : fixpoint->_fixpoint) {
            if(!member.isValid || member.term == nullptr || member.level <= params.variableLevel) {
                continue;
            }

            // Add to fixpoint
            clonedFixpoint->_fixpoint.emplace_back(member.term, member.isValid, member.level);
        }

        clonedFixpoint->_bValue = fixpoint->_bValue;

        // DEBUG std::cout << "Cloned: "; clonedFixpoint->dump(); std::cout << "\n";
        // DEBUG std::cout << "With symbol parts: {";
        for(auto &symPart : clonedFixpoint->_symbolParts) {
            // DEBUG std::cout << symPart << ", ";
        }
        // DEBUG std::cout << "}\n";
        return clonedFixpoint;
    }

    /**
     * Tries to look into the cache, if there is already created fixpoint,
     * with the same parameters. Otherwise it stores it in the cache
     *
     * @param[in] fixpoint:         list of terms (fixpoint representation)
     * @param[in] worklist:         list of pairs (term, symbol)
     * @return:                     pointer to unique fixpoint
     */
    TermFixpoint* TermWorkshop::GetUniqueFixpoint(TermFixpoint* &fixpoint) {
        assert(this->_compCache != nullptr);
        Term* termPtr = nullptr;

        //std::cout << "Getting unique fixpoint for: "; fixpoint->dump(); std::cout << "\n";

        if(!fixpoint->TestAndSetUpdate()) {
            //std::cout << "Fixpoint was not updated\n";
            return fixpoint;
        }

        auto compKey = fixpoint;
        if(!this->_compCache->retrieveFromCache(compKey, termPtr)) {
            termPtr = fixpoint;
            this->_compCache->StoreIn(compKey, termPtr);
        }
        assert(termPtr != nullptr);
        //std::cout << "Returning: "; termPtr->dump(); std::cout << "\n";
        return reinterpret_cast<TermFixpoint*>(termPtr);
    }

    Term* TermWorkshop::CreateContinuation(SymLink* aut, SymbolicAutomaton* init, Term* const& term, Symbol* symbol, bool underComplement, bool createLazy) {
        #if (OPT_GENERATE_UNIQUE_TERMS == true && UNIQUE_CONTINUATIONS == true)
            assert(this->_contCache != nullptr);

            Term* termPtr = nullptr;
            auto contKey = std::make_pair(term, symbol);
            if(!this->_contCache->retrieveFromCache(contKey, termPtr)) {
                #if (DEBUG_WORKSHOPS == true && DEBUG_TERM_CREATION == true)
                std::cout << "[*] Creating Continuation: ";
                std::cout << "from [" << term << "] + " << *symbol << " to ";
                #endif
                termPtr = new TermContinuation(this->_aut, aut, init, term, symbol, underComplement, createLazy);
                this->_contCache->StoreIn(contKey, termPtr);
            }
            assert(termPtr != nullptr);
            assert(termPtr->type == TermType::CONTINUATION);
            Term* unfoldedPtr = static_cast<TermContinuation*>(termPtr)->GetUnfoldedTerm();
            return (unfoldedPtr == nullptr ? termPtr : unfoldedPtr);
        #else
            return new TermContinuation(this->_aut, aut, term, symbol, underComplement);
        #endif
    }

    /***
     * Dump stats for Workshop. Dumps the cache if it is created.
     */
    void TermWorkshop::Dump() {
        if(this->_bCache != nullptr) {
            std::cout << "  \u2218 BaseCache stats -> ";
            this->_bCache->dumpStats();
        }
        if(this->_pCache != nullptr) {
            std::cout << "  \u2218 ProductCache stats -> ";
            this->_pCache->dumpStats();
        }
        if(this->_tpCache != nullptr) {
            std::cout << "  \u2218 TernaryProductCache stats -> ";
            this->_tpCache->dumpStats();
        }
        if(this->_npCache != nullptr) {
            std::cout << "  \u2218 NaryProductcache stats -> ";
            this->_npCache->dumpStats();
        }
        if(this->_contCache != nullptr) {
            std::cout << "  \u2218 ContinuationCache stats -> ";
            this->_contCache->dumpStats();
        }
        if(this->_lCache != nullptr) {
            std::cout << "  \u2218 ListCache stats -> ";
            this->_lCache->dumpStats();
        }
        if(this->_fpCache != nullptr) {
            std::cout << "  \u2218 FixpointCache stats -> ";
            this->_fpCache->dumpStats();
        }
        if(this->_fppCache != nullptr) {
            std::cout << "  \u2218 FixpointCachePre stats -> ";
            this->_fppCache->dumpStats();
        }
        if(this->_compCache != nullptr) {
            std::cout << "  \u2218 UniqueFixpointCache stats -> ";
            this->_compCache->dumpStats();
        }
    }

    std::string TermWorkshop::ToSimpleStats() {
        std::string out("");
        out += (this->_bCache != nullptr ? std::to_string(this->_bCache->GetSize()) + "b, " : "");
        out += (this->_compCache != nullptr ? std::to_string(this->_compCache->GetSize()) + "uf, " : "");
        out += (this->_tpCache != nullptr ? std::to_string(this->_tpCache->GetSize()) + "tp, " : "");
        out += (this->_npCache != nullptr ? std::to_string(this->_npCache->GetSize()) + "np, " : "");
        out += (this->_pCache != nullptr ? std::to_string(this->_pCache->GetSize()) + "p, " : "");
        out += (this->_contCache != nullptr ? std::to_string(this->_contCache->GetSize()) + "c, " : "");
        out += (this->_fpCache != nullptr ? std::to_string(this->_fpCache->GetSize()) + "fp, " : "");
        out += (this->_fppCache != nullptr ? std::to_string(this->_fppCache->GetSize()) + "fpp, " : "");
        return out;
    }

    NEVER_INLINE SymbolWorkshop::SymbolWorkshop() {
        this->_symbolCache = new SymbolCache();
        this->_trimmedSymbolCache = new SimpleSymbolCache();
        this->_remappedSymbolCache = new RemapCache();
    }

    NEVER_INLINE SymbolWorkshop::~SymbolWorkshop() {
#       if (OPT_USE_BOOST_POOL_FOR_ALLOC == false)
        // Only nonpool implementation needs to take care of deallocation
        if(_zeroSymbol != nullptr) {
            delete SymbolWorkshop::_zeroSymbol;
            SymbolWorkshop::_zeroSymbol = nullptr;
        }
        for(auto it = this->_symbolCache->begin(); it != this->_symbolCache->end(); ++it) {
            // Delete the symbol, unless it is trimmed -> it is in different structure
            if(std::get<2>(it->first) != 'T')
                delete it->second;
        }

        for(auto symb : this->_trimmedSymbols) {
            delete symb;
        }
#       endif
        delete this->_trimmedSymbolCache;
        delete this->_symbolCache;
    }

    Symbol* SymbolWorkshop::_zeroSymbol = nullptr;
#   if (OPT_USE_BOOST_POOL_FOR_ALLOC == true)
    boost::object_pool<Symbol> SymbolWorkshop::_pool;
#   endif

    Symbol* SymbolWorkshop::CreateZeroSymbol() {
        if(_zeroSymbol == nullptr) {
#           if (OPT_USE_BOOST_POOL_FOR_ALLOC == true)
            SymbolWorkshop::_zeroSymbol = SymbolWorkshop::_pool.construct();
#           else
            SymbolWorkshop::_zeroSymbol = new Symbol();
#           endif
        }
        return SymbolWorkshop::_zeroSymbol;
    }

    Symbol* SymbolWorkshop::CreateTrimmedSymbol(Symbol* src, Gaston::VarList* varList) {
        // Fixme: Refactor
        size_t varNum = varMap.TrackLength();
        // There are no symbols to trim, so we avoid the useless copy
        if(varList->size() == 0) {
            return src;
        // Check if maybe everything was trimmed?
        } else {
            bool allTrimmed = true;
            auto it = varList->begin();
            auto end = varList->end();
            // For each var, if it is not present in the free vars, we project it away
            for(; it != end;) {
                if(!src->IsDontCareAt(*(it++))) {
                    allTrimmed = false;
                    break;
                }
            }

            if(allTrimmed) {
                return src;
            }
        }

        auto symbolKey = src;
        Symbol* sPtr;
        if(!this->_trimmedSymbolCache->retrieveFromCache(symbolKey, sPtr)) {
#           if (OPT_USE_BOOST_POOL_FOR_ALLOC == true)
            sPtr = SymbolWorkshop::_pool.construct(src->GetTrackMask());
#           else
            sPtr = new Symbol(src->GetTrackMask());
#           endif
            auto it = varList->begin();
            auto end = varList->end();
            // For each var, if it is not present in the free vars, we project it away
            for(; it != end;) {
                sPtr->ProjectVar(*(it++));
            }
#           if (OPT_UNIQUE_TRIMMED_SYMBOLS == true)
            for(auto item = this->_trimmedSymbols.begin(); item != this->_trimmedSymbols.end(); ++item) {
                if(*(*item) == *sPtr) {
                    Symbol* uniqPtr = (*item);
                    this->_trimmedSymbolCache->StoreIn(symbolKey, uniqPtr);
#                   if (OPT_USE_BOOST_POOL_FOR_ALLOC == false)
                    delete sPtr;
#                   endif
                    return uniqPtr;
                }
            }
#           endif
            this->_trimmedSymbols.insert(this->_trimmedSymbols.begin(), sPtr);
            this->_trimmedSymbolCache->StoreIn(symbolKey, sPtr);
        }
        return sPtr;
    }

    Symbol* SymbolWorkshop::_CreateProjectedSymbol(Symbol* src, VarType var, ValType val) {
        assert(val != '0');
        auto symbolKey = std::make_tuple(src, var, val);
        Symbol* symPtr;
        if(!this->_symbolCache->retrieveFromCache(symbolKey, symPtr)) {
#           if (OPT_USE_BOOST_POOL_FOR_ALLOC == true)
            symPtr = SymbolWorkshop::_pool.construct(src->GetTrackMask(), var, val);
#           else
            symPtr  = new Symbol(src->GetTrackMask(), var, val);
#           endif
            this->_symbolCache->StoreIn(symbolKey, symPtr);
        }
        return symPtr;
    }

    Symbol* SymbolWorkshop::CreateSymbol(Symbol* src, VarType var, ValType val) {
        // Note that we assume some things about the symbols
        switch(val) {
            case 'X':
                if(src->IsDontCareAt(var)) {
                    return src;
                } else {
                    return this->_CreateProjectedSymbol(src, var, val);
                }
            case '0':
                // We know that there cannot be X
                return src;
            case '1':
                return this->_CreateProjectedSymbol(src, var, val);
            default:
                assert(false && "Projecting unknown value");
                break;
        }
    }

    Symbol* SymbolWorkshop::CreateRemappedSymbol(Symbol* str, std::map<unsigned int, unsigned int>*& map, size_t tag) {
        // There should be Map of Ptr -> Ptr
        // Fixme: this could be optimized to something more sufficient, like removing the 'R' part?
        auto symbolKey = std::make_pair(str, tag);
        Symbol* symPtr;
        if(!this->_remappedSymbolCache->retrieveFromCache(symbolKey, symPtr)) {
#           if (OPT_USE_BOOST_POOL_FOR_ALLOC == true)
            symPtr = SymbolWorkshop::_pool.construct(str, map);
#           else
            symPtr = new Symbol(str, map);
#           endif

#           if (OPT_UNIQUE_REMAPPED_SYMBOLS == true)
            for(auto item = this->_remappedSymbols.begin(); item != this->_remappedSymbols.end(); ++item) {
                if(*(*item) == *symPtr) {
                    Symbol* uniqPtr = (*item);
                    this->_remappedSymbolCache->StoreIn(symbolKey, uniqPtr);
#                   if (OPT_USE_BOOST_POOL_FOR_ALLOC == false)
                    delete symPtr;
#                   endif
                    return uniqPtr;
                }
            }
            this->_remappedSymbols.insert(this->_remappedSymbols.begin(), symPtr);
#           endif
            this->_remappedSymbolCache->StoreIn(symbolKey, symPtr);
        }
        return symPtr;
    }

    void SymbolWorkshop::Dump() {
        std::cout << "  \u2218 SymbolWorkshop stats -> ";
        this->_symbolCache->dumpStats();
    }

    template<class KeyType>
    void dumpDefaultKey(KeyType const& key) {
        std::cout << (key);
    }

    template<class DataType>
    void dumpDefaultData(DataType& data) {
        std::cout << (data);
    }

    void dumpRemapKey(RemapKey const& s) {
        std::cout << "(" << (*s.first) << ", " << s.second << ")";
    }

    void dumpSymbolKey(SymbolKey const&s) {
        std::cout << "<[" <<  (std::get<0>(s)) << "]" << (*std::get<0>(s)) << ", " << std::get<1>(s) << ", " << std::get<2>(s) << ">";
    }

    void dumpTernaryKey(TernaryKey const&s) {
        std::cout << "<" << (*std::get<0>(s)) << ", " << (*std::get<1>(s)) << ", " << (*std::get<2>(s)) << ">";
    }

    void dumpNaryKey(NaryKey const& s) {
        std::cout << "<";
        for(auto i = 0; i < s.second; ++i) {
            if(i)
                std::cout << ", ";
            std::cout << (*s.first[i]);
        }
        std::cout << ">";
    }

    void dumpSymbolData(Symbol*&s) {
        std::cout  << "<[" <<  (s) << "]" << (*s);
    }

    void dumpBaseKey(BaseKey const&s) {
        std::cout << s;
    }

    void dumpProductKey(ProductKey const&s) {
        std::cout << "<" << (*s.first) << ", " << (*s.second) << ">";
    }

    void dumpListKey(ListKey const&s) {
        std::cout << s;
    }

    void dumpFixpointKey(FixpointKey const&s) {
        if(s.second != nullptr) {
            std::cout << "<[" << (s.first) << "]" << (*s.first) << ", " << (*s.second) << ">";
        } else {
            std::cout << "<[" << (s.first) << "]" << (*s.first) << ", \u0437>";
        }
    }

    void dumpComputationKey(ComputationKey const&s) {
        std::cout << "<" << (s) << ">";
    }

    void dumpCacheData(Term *&s) {
        std::cout << "[" << s << "]" << (*s);
    }
}