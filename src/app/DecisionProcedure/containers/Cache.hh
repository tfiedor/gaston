/*****************************************************************************
 *  dWiNA - Deciding WSkS using non-deterministic automata
 *
 *  Copyright (c) 2014  Tomas Fiedor <xfiedo01@stud.fit.vutbr.cz>
 *
 *  Description:
 *    Implementation of generic cache
 *
 *****************************************************************************/

#ifndef __CACHE__H__
#define __CACHE__H__

#include <map>
#include <vector>
#include <unordered_map>
#include "StateSet.hh"

/**
 * Structure used for comparing two states so we can use it for unordered map
 */
struct SetCompare : public std::binary_function<MacroStateSet*, MacroStateSet*, bool>
{
	/**
	 * @param lhs: left operand
	 * @param rhs: right operand
	 * @return true if lhs = rhs
	 */
    bool operator()(MacroStateSet* lhs, MacroStateSet* rhs) const
    {
        return lhs->DoCompare(rhs);
    }
};

/**
 * Structure for computing hash of set - returns the number of states in
 * the macro-state
 */
struct MacroStateSet_Hash{
	/**
	 * @param set: set we are computing hash of
	 * @return hash of @p set
	 */
	int operator()(MacroStateSet * set) const {
		return set->getMacroStates().size();
	}
};

/**
 * Class representing cache for storing @p CacheData according to the
 * @p CacheKey
 *
 * CacheKey represents the key for lookup of CacheData
 * CacheData represents pure data that are stored inside cache
 */
template<class CacheData>
class MCache {
private:
	// < Typedefs >
	typedef typename std::unordered_map<MacroStateSet*, CacheData, MacroStateSet_Hash, SetCompare> CacheMap;
	typedef typename std::unordered_map<MacroStateSet*, CacheData, MacroStateSet_Hash, SetCompare>::const_iterator ConstInterator_CacheMap;

	// < Private Members >
	CacheMap _cache;
	unsigned int cacheHits = 0;
	unsigned int cacheMisses = 0;
public:
	// < Public Methods >
	/**
	 * @param key: key of the looked up macro state
	 * @return: data corresponding to the @p key
	 */
	CacheData lookUp(MacroStateSet* key){
		ConstInterator_CacheMap search = this->_cache.find(key);
		if (search != this->_cache.end()) {
			return search->second;
		}
	}

	/**
	 * @param key: key we are storing to
	 * @param data: data we are storing
	 */
	void storeIn(MacroStateSet* key, const CacheData & data){
		auto itBoolPair = _cache.insert(std::make_pair(key, data));
		if (!itBoolPair.second)
		{
			(itBoolPair.first)->second = data;
		}
	}

	/**
	 * @param key: kew we are looking for
	 * @param data: reference to the data
	 * @return true if found;
	 */
	bool retrieveFromCache(MacroStateSet* key, CacheData & data) {
		ConstInterator_CacheMap search = this->_cache.find(key);
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
	bool inCache(MacroStateSet* key) {
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
	unsigned int dumpStats(unsigned int level) {
		unsigned int size = this->_cache.size();
		std::cout << "Level " << level << " cache: " << size;
		if(this->cacheHits+this->cacheMisses != 0)
			std::cout << "(" << this->cacheHits << ":" << this->cacheMisses << ") -> "<< (this->cacheHits/(double)(this->cacheHits+this->cacheMisses)) <<"\n";
		else
			std::cout << "\n";
		return size;
	}
};

template<class CacheData>
class MultiLevelMCache {
private:
	// < Private Members>
	std::vector<MCache<CacheData>> _mlCache;
	unsigned levels;

public:
	MultiLevelMCache() {}
	MultiLevelMCache(unsigned levels) : _mlCache(levels) { this->levels = levels;}

	// < Public Methods >
	/**
	 * @param key: looked up key
	 * @param level: level of looked up key
	 * @return looked up data
	 */
	CacheData lookUp(MacroStateSet* key, unsigned level) {
		return this->_mlCache[level].lookUp(key);
	}

	/**
	 * @param key: key we are storing
	 * @param data: reference to data we are storing
	 * @param level: level where we are storing
	 */
	void storeIn(MacroStateSet* key, const CacheData & data, unsigned level) {
		this->_mlCache[level].storeIn(key, data);
	}

	/**
	 * @param key: key we are looking for
	 * @param data: reference to lookedup data
	 * @param level: level of looking up data
	 * @return true if @p key on @p level found
	 */
	bool retrieveFromCache(MacroStateSet* key, CacheData & data, unsigned level) {
		return this->_mlCache[level].retrieveFromCache(key, data);
	}

	/**
	 * Extends cache up to @p level
	 *
	 * @param level: level we are extending to
	 */
	void extend(unsigned level) {
		this->_mlCache.resize(level+1);
	}

	/**
	 * @param key: key we are looking for
	 * @param level: level of the key
	 * @return true if @p key on level @level is in cache
	 */
	bool inCache(MacroStateSet* key, unsigned level) {
		return this->_mlCache[level].inCache(key);
	}

	/**
	 * Clears the cache
	 */
	void clear() {
		for(auto it = this->_mlCache.begin(); it != this->_mlCache.end(); ++it) {
			it->clear();
		}
	}

	/**
	 * Dumps statistic of all cache levels
	 */
	void dumpStats() {
		unsigned int count = 0;
		unsigned int index = 0;
		for(auto it = this->_mlCache.begin(); it != this->_mlCache.end(); ++it) {
			count += it->dumpStats(index++);
		}
		std::cout << "Overall number of states in cache: " << count << "\n";
	}
};

#endif
