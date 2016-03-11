#include "Workshops.h"
#include "Term.h"
#include "VarToTrackMap.hh"

extern VarToTrackMap varMap;

namespace Workshops {
    TermWorkshop::TermWorkshop(SymbolicAutomaton* aut) :
            _bCache(nullptr), _pCache(nullptr), _lCache(nullptr), _fpCache(nullptr), _fppCache(nullptr),
            _contCache(nullptr),
            _compCache(nullptr), _aut(aut) { }

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

    TermWorkshop::~TermWorkshop() {
        this->_bCache = TermWorkshop::_cleanCache(this->_bCache);
        this->_fpCache = TermWorkshop::_cleanCache(this->_fpCache);
        this->_fppCache = TermWorkshop::_cleanCache(this->_fppCache);
        this->_pCache = TermWorkshop::_cleanCache(this->_pCache);
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
            bool result;

            if(lhs.first == nullptr || rhs.first == nullptr) {
                return lhs.first == rhs.first;
            }

            // Compare fixpoints
            // Fixme: The fuck is this, this is not correct...
            for(auto& item : (*lhs.first)) {
                if(!item.second) {
                    continue;
                }

                result = false;
                for(auto& titem : (*rhs.first)) {
                    if(!titem.second) {
                        continue;
                    }
                    if(item.first == titem.first) {
                        result = true;
                        break;
                    }
                }
                if(!result) {
                    return false;
                }
            }

            for(auto& item : (*rhs.first)) {
                if(!item.second) {
                    continue;
                }

                result = false;
                for(auto& titem : (*lhs.first)) {
                    if(!titem.second) {
                        continue;
                    }
                    if(item.first == titem.first) {
                        result = true;
                        break;
                    }
                }
                if(!result) {
                    return false;
                }
            }

            // Compare worklists
            for(auto item : (*lhs.second)) {
                result = false;
                for(auto titem : (*rhs.second)) {
                    if(item == titem) {
                        result = true;
                        break;
                    }
                }
                if(!result) {
                    return false;
                }
            }

            return true;
        }
    };

    struct ComputationHash {
        size_t operator()(ComputationKey const& set) const {
            size_t seed = 0;
            if(set.first) {
                for (auto& item : (*set.first)) {
                    if(!item.second) {
                        continue;
                    }
                    boost::hash_combine(seed, boost::hash_value(item.first));
                }
            }
            return seed;
        }
    };

    void TermWorkshop::InitializeWorkshop() {
        switch(this->_aut->type) {
            case AutType::BASE:
                this->_bCache = new BaseCache();
                break;
            case AutType::BINARY:
            case AutType::INTERSECTION:
            case AutType::UNION:
                this->_pCache = new ProductCache();
                this->_contCache = new FixpointCache();
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

    TermEmpty* TermWorkshop::CreateEmpty() {
        if(TermWorkshop::_empty == nullptr) {
            TermWorkshop::_empty = new TermEmpty();
        }
        assert(TermWorkshop::_empty != nullptr);
        return TermWorkshop::_empty;
    }

    TermEmpty* TermWorkshop::CreateComplementedEmpty() {
        if(TermWorkshop::_emptyComplement == nullptr) {
            TermWorkshop::_emptyComplement = new TermEmpty(true);
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
    Term* TermWorkshop::CreateBaseSet(VATA::Util::OrdVector<unsigned int>& states, unsigned int offset, unsigned int stateno) {
        #if (OPT_GENERATE_UNIQUE_TERMS == true && UNIQUE_BASE == true)
            assert(this->_bCache != nullptr);

            if(states.size() == 0) {
                return this->CreateEmpty();
            }

            Term* termPtr = nullptr;
            if(!this->_bCache->retrieveFromCache(states, termPtr)) {
                #if (DEBUG_WORKSHOPS == true && DEBUG_TERM_CREATION == true)
                std::cout << "[*] Creating BaseSet: ";
                #endif
                // The object was not created yet, so we create it and store it in cache
                termPtr = new TermBaseSet(states, offset, stateno);
                this->_bCache->StoreIn(states, termPtr);
            }
            assert(termPtr != nullptr);
            return reinterpret_cast<TermBaseSet*>(termPtr);
        #else
            return new TermBaseSet(states, offset, stateno);
        #endif
    }

    /**
     * Checks if there is already created TermProduct in cache, in case there is not
     * it creates the new object and populates the cache
     *
     * @param[in] lptr:     left term of product
     * @param[in] rptr:     right term of product
     * @param[in] type:     type of the product
     */
    TermProduct* TermWorkshop::CreateProduct(Term_ptr const&lptr, Term_ptr const&rptr, ProductType type) {
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
            termPtr = new TermProduct(lptr, rptr, type);
            this->_pCache->StoreIn(productKey, termPtr);
        }
        assert(termPtr != nullptr);
        return reinterpret_cast<TermProduct*>(termPtr);
#       else
        return new TermProduct(lptr, rptr, type);
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
    TermList* TermWorkshop::CreateList(Term_ptr const& startTerm, bool inComplement) {
        #if (OPT_GENERATE_UNIQUE_TERMS == true && UNIQUE_LISTS == true)
            assert(this->_lCache != nullptr);

            Term* termPtr = nullptr;
            auto productKey = startTerm;
            if(!this->_lCache->retrieveFromCache(productKey, termPtr)) {
                #if (DEBUG_WORKSHOPS == true && DEBUG_TERM_CREATION == true)
                std::cout << "[*] Creating List: ";
                std::cout << "from ["<< startTerm << "] to ";
                #endif
                termPtr = new TermList(startTerm, inComplement);
                this->_lCache->StoreIn(productKey, termPtr);
            }
            assert(termPtr != nullptr);
            return reinterpret_cast<TermList*>(termPtr);
        #else
            return new TermList(startTerm, inComplement);
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
                termPtr = new TermFixpoint(this->_aut, source, symbol, inCompl, initValue, search);
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

            Term* termPtr = nullptr;
            auto fixpointKey = std::make_pair(source, symbol);
            if(!this->_fppCache->retrieveFromCache(fixpointKey, termPtr)) {
                #if (DEBUG_WORKSHOPS == true && DEBUG_TERM_CREATION == true)
                std::cout << "[*] Creating FixpointPre: ";
                std::cout << "from [" << source << "] to ";
                #endif
                termPtr = new TermFixpoint(this->_aut, source, symbol, inCompl);
                this->_fppCache->StoreIn(fixpointKey, termPtr);
            }
            assert(termPtr != nullptr);
            return reinterpret_cast<TermFixpoint*>(termPtr);
        #else
            return new TermFixpoint(this->_aut, source, symbol, inCompl);
        #endif
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

        if(!fixpoint->TestAndSetUpdate()) {
            return fixpoint;
        }

        auto compKey = std::make_pair(&fixpoint->_fixpoint, &fixpoint->_worklist);
        if(!this->_compCache->retrieveFromCache(compKey, termPtr)) {
            termPtr = fixpoint;
            this->_compCache->StoreIn(compKey, termPtr);
        }
        assert(termPtr != nullptr);
        return reinterpret_cast<TermFixpoint*>(termPtr);
    }

    Term* TermWorkshop::CreateContinuation(SymbolicAutomaton* aut, Term* const& term, Symbol* symbol, bool underComplement) {
        #if (OPT_GENERATE_UNIQUE_TERMS == true && UNIQUE_CONTINUATIONS == true)
            assert(this->_contCache != nullptr);

            Term* termPtr = nullptr;
            auto contKey = std::make_pair(term, symbol);
            if(!this->_contCache->retrieveFromCache(contKey, termPtr)) {
                #if (DEBUG_WORKSHOPS == true && DEBUG_TERM_CREATION == true)
                std::cout << "[*] Creating Continuation: ";
                std::cout << "from [" << term << "] + " << *symbol << " to ";
                #endif
                termPtr = new TermContinuation(aut, term, symbol, underComplement);
                this->_contCache->StoreIn(contKey, termPtr);
            }
            assert(termPtr != nullptr);
            return termPtr;
        #else
            return new TermContinuation(aut, term, symbol, underComplement);
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
    }

    SymbolWorkshop::SymbolWorkshop() {
        this->_symbolCache = new SymbolCache();
    }

    SymbolWorkshop::~SymbolWorkshop() {
        if(_zeroSymbol != nullptr) {
            delete SymbolWorkshop::_zeroSymbol;
            SymbolWorkshop::_zeroSymbol = nullptr;
        }
        for(auto it = this->_symbolCache->begin(); it != this->_symbolCache->end(); ++it) {
            // Delete the symbol
            delete it->second;
        }
        delete this->_symbolCache;
    }

    Symbol* SymbolWorkshop::_zeroSymbol = nullptr;

    Symbol* SymbolWorkshop::CreateZeroSymbol() {
        if(_zeroSymbol == nullptr) {
            SymbolWorkshop::_zeroSymbol = new ZeroSymbol();
        }
        return SymbolWorkshop::_zeroSymbol;
    }

    Symbol* SymbolWorkshop::CreateTrimmedSymbol(Symbol* src, Gaston::VarList* varList) {
        // Fixme: Refactor
        // There are no symbols to trim, so we avoid the useless copy
        if(varList->size() == varMap.TrackLength()) {
            return src;
        // Check if maybe everything was trimmed?
        } else {
            bool allTrimmed = true;
            auto it = varList->begin();
            auto end = varList->end();
            auto varNum = varMap.TrackLength();
            // For each var, if it is not present in the free vars, we project it away
            for (size_t var = 0; var < varNum; ++var) {
                if (it != end && var == *it) {
                    ++it;
                } else {
                    if(!src->IsDontCareAt(var)) {
                        allTrimmed = false;
                        break;
                    }
                }
            }
            if(allTrimmed) {
                return src;
            }
        }

        auto symbolKey = std::make_tuple(src, 0u, 'T');
        Symbol* sPtr;
        if(!this->_symbolCache->retrieveFromCache(symbolKey, sPtr)) {
            sPtr = new ZeroSymbol(src->GetTrackMask());
            auto it = varList->begin();
            auto end = varList->end();
            auto varNum = varMap.TrackLength();
            // For each var, if it is not present in the free vars, we project it away
            for (size_t var = 0; var < varNum; ++var) {
                if (it != end && var == *it) {
                    ++it;
                } else {
                    sPtr->ProjectVar(var);
                }
            }
#           if (OPT_UNIQUE_TRIMMED_SYMBOLS == true)
            for(auto item = this->_symbolCache->begin(); item != this->_symbolCache->end(); ++item) {
                if(std::get<2>((*item).first) == 'T') {
                    if(*(*item).second == *sPtr) {
                        Symbol* uniqPtr = (*item).second;
                        this->_symbolCache->StoreIn(symbolKey, uniqPtr);
                        delete sPtr;
                        return uniqPtr;
                    }
                }
            }
#           endif
            this->_symbolCache->StoreIn(symbolKey, sPtr);
        }
        return sPtr;
    }

    Symbol* SymbolWorkshop::_CreateProjectedSymbol(Symbol* src, VarType var, ValType val) {
        assert(val != '0');
        auto symbolKey = std::make_tuple(src, var, val);
        Symbol* symPtr;
        if(!this->_symbolCache->retrieveFromCache(symbolKey, symPtr)) {
            symPtr  = new ZeroSymbol(src->GetTrackMask(), var, val);
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

    void SymbolWorkshop::Dump() {
        this->_symbolCache->dumpStats();
    }

    void dumpSymbolKey(SymbolKey const&s) {
        std::cout << "<[" <<  (std::get<0>(s)) << "]" << (*std::get<0>(s)) << ", " << std::get<1>(s) << ", " << std::get<2>(s) << ">";
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
            std::cout << "<" << (*s.first) << ", " << (*s.second) << ">";
        } else {
            std::cout << "<" << (*s.first) << ", \u0437>";
        }
    }

    void dumpComputationKey(ComputationKey const&s) {
        std::cout << "<" << (s.first) << ", " << (s.second) << ">";
    }

    void dumpCacheData(Term *&s) {
        std::cout << s;
    }
}