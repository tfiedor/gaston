/*****************************************************************************
 *  dWiNA - Deciding WSkS using non-deterministic automata
 *
 *  Copyright (c) 2014  Tomas Fiedor <xfiedo01@stud.fit.vutbr.cz>
 *
 *  Description:
 *    Implementation of generic cache
 *
 *****************************************************************************/

#ifndef __SYM_CACHE__H__
#define __SYM_CACHE__H__

#include <iomanip>
#include <map>
#include <vector>
#include <unordered_map>
#include "StateSet.hh"
#include "../environment.hh"

#include <boost/functional/hash.hpp>

template<class Key>
struct PairCompare : public std::binary_function<Key, Key, bool>
{
	/**
     * @param lhs: left operand
     * @param rhs: right operand
     * @return true if lhs = rhs
     */
	bool operator()(Key const& lhs, Key const& rhs) const {
#if (DEBUG_TERM_CACHE_COMPARISON == true)
		auto keyFirst = lhs.first;
		auto keySecond = lhs.second;
		if(keySecond == nullptr) {
			std::cout << "(" << (*keyFirst) << ", \u03B5) vs";
		} else {
			std::cout << "(" << (*keyFirst) << ", " << (*keySecond) << ") vs";
		}
		auto dkeyFirst = rhs.first;
		auto dkeySecond = rhs.second;
		if(dkeySecond == nullptr) {
			std::cout << "(" << (*dkeyFirst) << ", \u03B5)";
		} else {
			std::cout << "(" << (*dkeyFirst) << ", " << (*dkeySecond) << ")";
		}
		bool lhsresult = (*lhs.first == *rhs.first);
		bool rhsresult = (lhs.second == rhs.second);
		bool result = lhsresult && rhsresult;
			std::cout << " = (" << lhsresult << " + " << rhsresult << ") =  " << result << "\n";
		return  result;
#else
		return (*lhs.second == *rhs.second) && (*lhs.first == *rhs.first);
#endif
	}
};

/**
 * Class representing cache for storing @p CacheData according to the
 * @p CacheKey
 *
 * CacheKey represents the key for lookup of CacheData
 * CacheData represents pure data that are stored inside cache
 */
template<class Key, class CacheData, class KeyHash, class KeyCompare, void (*KeyDump)(Key const&), void (*DataDump)(CacheData&)>
class BinaryCache {
//	                                     this could be done better ---^----------------------------^
private:
	// < Typedefs >
	typedef std::unordered_map<Key, CacheData, KeyHash, KeyCompare> KeyToValueMap;

	// < Private Members >
	KeyToValueMap _cache;
	unsigned int cacheHits = 0;
	unsigned int cacheMisses = 0;

public:
	// < Public Methods >
	/**
	 * @param key: key of the looked up macro state
	 * @return: data corresponding to the @p key
	 */
	CacheData LookUp(Key& key){
		auto search = this->_cache.find(key);
		if (search != this->_cache.end()) {
			return search->second;
		}
	}

	/**
	 * @param key: key we are storing to
	 * @param data: data we are storing
	 */
	void StoreIn(Key & key, const CacheData & data){
		auto itBoolPair = _cache.insert(std::make_pair(key, data));
	}

	/**
	 * @param key: key we are looking for
	 * @param data: reference to the data
	 * @return true if found;
	 */
	bool retrieveFromCache(Key& key, CacheData & data) {
		auto search = this->_cache.find(key);
		if (search == this->_cache.end()) {
			++cacheMisses;
			return false;
		} else {
			data = search->second;
			++cacheHits;
			return true;
		}
	}

	/**
	 * Clears the cache
	 */
	void clear() {
		for(auto itPair = this->_cache.begin(); itPair != this->_cache.end(); ++itPair) {
			delete itPair->first;
			itPair = this->_cache.erase(itPair);
		}
	}

	/**
	 * @param level: level of cache
	 * @return number of dumped keys
	 */
	unsigned int dumpStats() {
		unsigned int size = this->_cache.size();

		std::cout << "Size: " << size;
		if(this->cacheHits+this->cacheMisses != 0)
			std::cout << ", Hit:Miss (" << this->cacheHits << ":" << this->cacheMisses << ")	->	"<< std::fixed << std::setprecision(2) << (this->cacheHits/(double)(this->cacheHits+this->cacheMisses))*100 <<"%\n";
		else
			std::cout << "\n";

		#if (DEBUG_CACHE_MEMBERS == true)
        if(size) {
			std::cout << "{\n";
			for (auto it = this->_cache.begin(); it != this->_cache.end(); ++it) {
				std::cout << "\t";
				KeyDump(it->first);
				std::cout << " : ";
				DataDump(it->second);
				std::cout << "\n";
			}
			std::cout << "}\n";
		}
		#endif

		return size;
	}
};

#endif