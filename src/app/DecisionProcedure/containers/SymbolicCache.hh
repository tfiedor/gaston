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

#include <map>
#include <vector>
#include <unordered_map>
#include "StateSet.hh"

#include <boost/functional/hash.hpp>


/**
 * Class representing cache for storing @p CacheData according to the
 * @p CacheKey
 *
 * CacheKey represents the key for lookup of CacheData
 * CacheData represents pure data that are stored inside cache
 */
template<class KeyFirst, class KeySecond, class CacheData, class Binary_Hash>
class BinaryCache {
private:
	typedef std::pair<KeyFirst, KeySecond> Key;

	struct BinaryCompare : public std::binary_function<Key, Key, bool>
	{
		/**
         * @param lhs: left operand
         * @param rhs: right operand
         * @return true if lhs = rhs
         */
		bool operator()(Key lhs, Key rhs) const
		{
			return (lhs.first == rhs.first) && (lhs.second == rhs.second);
		}
	};

	// < Typedefs >
	typedef std::unordered_map<Key, CacheData, Binary_Hash, BinaryCompare> KeyToValueMap;

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
	CacheData LookUp(Key key){
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
		if (!itBoolPair.second)
		{
			(itBoolPair.first)->second = data;
		}
	}

	void StoreIn(KeyFirst k1, KeySecond k2, const CacheData &data) {
		auto itBoolPair = _cache.insert(std::make_pair(std::make_pair(k1, k2), data));
		if(!itBoolPair.second) {
			(itBoolPair.first)->second = data;
		}
	}

	bool retrieveFromCache(KeyFirst k1, KeySecond k2, CacheData & data) {
		return retrieveFromCache(std::make_pair(k1, k2), data);
	}

	/**
	 * @param key: kew we are looking for
	 * @param data: reference to the data
	 * @return true if found;
	 */
	bool retrieveFromCache(Key & key, CacheData & data) {
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
	 * @param key:
	 * @return true if @p key in cache
	 */
	bool inCache(Key & key) {
		bool inC = (this->_cache.find(key)) != this->_cache.end();
		if(inC) {
			++cacheHits;
		} else {
			++cacheMisses;
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
		std::cout << ".Cache size: " << size;
		if(this->cacheHits+this->cacheMisses != 0)
			std::cout << "(" << this->cacheHits << ":" << this->cacheMisses << ") -> "<< (this->cacheHits/(double)(this->cacheHits+this->cacheMisses)) <<"\n";
		else
			std::cout << "\n";
		return size;
	}
};

#endif