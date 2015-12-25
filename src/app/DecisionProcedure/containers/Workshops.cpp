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
        assert(this->_bCache != nullptr);

        Term* termPtr = nullptr;
        if(!this->_bCache->retrieveFromCache(states, termPtr)) {
            // The object was not created yet, se we create it and store it in cache
            termPtr = new TermBaseSet(states, offset, stateno);
            this->_bCache->StoreIn(states, termPtr);
        }
        assert(termPtr != nullptr);
        return reinterpret_cast<TermBaseSet*>(termPtr);
    }

    /***
     * Dump stats for Workshop. Dumps the cache if it is created.
     */
    void TermWorkshop::Dump() {
        if(this->_bCache != nullptr) {
            std::cout << "  \u2218 BaseCache stats -> ";
            this->_bCache->dumpStats();
        }
    }

    void dumpBaseKey(VATA::Util::OrdVector<unsigned int> const&s) {
        std::cout << s;
    }

    void dumpCacheData(Term *&s) {
        std::cout << s;
    }
}