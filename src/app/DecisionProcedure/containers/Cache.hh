#ifndef __CACHE__H__
#define __CACHE__H__

#include <map>
#include <vector>
#include <unordered_map>
#include "StateSet.hh"

struct SetCompare : public std::binary_function<MacroStateSet*, MacroStateSet*, bool>
{
    bool operator()(MacroStateSet* lhs, MacroStateSet* rhs) const
    {
        return lhs->DoCompare(rhs);
    }
};

struct MacroStateSet_Hash{
	int operator()(MacroStateSet * set) const {
		//StateSetList macroStates = set->getMacroStates();
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
	CacheData lookUp(MacroStateSet* key){
		//return this->_cache[key];
		ConstInterator_CacheMap search = this->_cache.find(key);
		if (search != this->_cache.end()) {
			return search->second;
		}
	}

	void storeIn(MacroStateSet* key, const CacheData & data){
		//this->_cache[key] = data;
		auto itBoolPair = _cache.insert(std::make_pair(key, data));
		if (!itBoolPair.second)
		{
			(itBoolPair.first)->second = data;
		}
	}

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

	bool inCache(MacroStateSet* key) {
		bool inC = (this->_cache.find(key)) != this->_cache.end();
		if(inC) {
			++cacheHits;
		} else {
			++cacheMisses;
		}
	}

	void clear() {
		for(auto itPair = this->_cache.begin(); itPair != this->_cache.end(); ++itPair) {
			delete itPair->first;
			itPair = this->_cache.erase(itPair);
		}
	}

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
	CacheData lookUp(MacroStateSet* key, unsigned level) {
		return this->_mlCache[level].lookUp(key);
	}

	void storeIn(MacroStateSet* key, const CacheData & data, unsigned level) {
		this->_mlCache[level].storeIn(key, data);
	}

	bool retrieveFromCache(MacroStateSet* key, CacheData & data, unsigned level) {
		return this->_mlCache[level].retrieveFromCache(key, data);
	}

	void extend(unsigned level) {
		this->_mlCache.resize(level+1);
	}

	bool inCache(MacroStateSet* key, unsigned level) {
		return this->_mlCache[level].inCache(key);
	}

	void clear() {
		for(auto it = this->_mlCache.begin(); it != this->_mlCache.end(); ++it) {
			it->clear();
		}
	}

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
