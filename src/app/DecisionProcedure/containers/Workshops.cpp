#include "Workshops.h"
#include "Term.h"

namespace Workshops {
    TermWorkshop::TermWorkshop() {
        this->_bCache = new BaseCache();
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
    TermBaseSet* TermWorkshop::CreateBaseSet(VATA::Util::OrdVector<unsigned int>& states, unsigned int offset, unsigned int stateno) {
#if (OPT_GENERATE_UNIQUE_TERMS == true)
        assert(this->_bCache != nullptr);

        Term* termPtr = nullptr;
        if(!this->_bCache->retrieveFromCache(states, termPtr)) {
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
    TermProduct* TermWorkshop::CreateProduct(Term *&lptr, Term *&rptr, ProductType type) {
        // TODO: Can there be sets that have same lhs rhs, but different product type??
        // TODO: I don't think so actually, because this is on the node, so it cannot generate different things
        // TODO: And same thing goes for complements
#if (OPT_GENERATE_UNIQUE_TERMS == true)
        assert(this->_pCache != nullptr);

        Term* termPtr = nullptr;
        auto productKey = std::make_pair(lptr, rptr);
        if(!this->_pCache->retrieveFromCache(productKey, termPtr)) {
            // The object was not created yet, so we create it and store it in cache
            termPtr = new TermProduct(std::shared_ptr<Term>(lptr), std::shared_ptr<Term>(rptr), type);
            this->_pCache->StoreIn(productKey, termPtr);
        }
        assert(termPtr != nullptr);
        return reinterpret_cast<TermProduct*>(termPtr);
#else
        return new TermProduct(std::shared_ptr<Term>(lptr), std::shared_ptr<Term>(rptr), type);
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
    }

    void dumpBaseKey(BaseKey const&s) {
        std::cout << s;
    }

    void dumpProductKey(ProductKey const&s) {
        std::cout << "<" << (*s.first) << ", " << (*s.second) << ">";
    }

    void dumpCacheData(Term *&s) {
        std::cout << s;
    }
}