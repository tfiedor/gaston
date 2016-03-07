/*****************************************************************************
 *  gaston - We pay homage to Gaston, an Africa-born brown fur seal who
 *    escaped the Prague Zoo during the floods in 2002 and made a heroic
 *    journey for freedom of over 300km all the way to Dresden. There he
 *    was caught and subsequently died due to exhaustion and infection.
 *    Rest In Piece, brave soldier.
 *
 *  Copyright (c) 2016  Tomas Fiedor <ifiedortom@fit.vutbr.cz>
 *      Notable mentions:   Ondrej Lengal <ondra.lengal@gmail.com>
 *                              (author of VATA)
 *                          Petr Janku <ijanku@fit.vutbr.cz>
 *                              (MTBDD and automata optimizations)
 *
 *  Description:
 *      Generic cache and various hashing and comparison structures
 *****************************************************************************/
#ifndef __SYM_CACHE__H__
#define __SYM_CACHE__H__

#include <iomanip>
#include <map>
#include <vector>
#include <sparsehash/dense_hash_map>
#include <sparsehash/sparse_hash_map>
#include <unordered_map>
#include <typeinfo>
#include "StateSet.hh"
#include "../environment.hh"

#include <boost/functional/hash.hpp>

namespace Gaston {
	extern size_t hash_value(Term *);
	extern size_t hash_value(ZeroSymbol *);
}

template<class T>
inline size_t HashPointer(T* Ptr)
{
	size_t Value = (size_t)(Ptr);
	Value = ~Value + (Value << 15);
	Value = Value ^ (Value >> 12);
	Value = Value + (Value << 2);
	Value = Value ^ (Value >> 4);
	Value = Value * 2057;
	Value = Value ^ (Value >> 16);
	return Value;
}

struct ResultHashType {
	size_t operator()(std::pair<Term*, ZeroSymbol*> const& set) const {
#       if (OPT_USE_CUSTOM_PTR_HASH == true)
        size_t seed = HashPointer<Term>(set.first);
		boost::hash_combine(seed, HashPointer<ZeroSymbol>(set.second));
		return seed;
#       else
		size_t seed = Gaston::hash_value(set.first);
		boost::hash_combine(seed, Gaston::hash_value(set.second));
		return seed;
#       endif
	}
};

struct SubsumptionHashType {
	size_t operator()(std::pair<Term*, Term*> const& set) const {
		size_t seed = Gaston::hash_value(set.first);
		boost::hash_combine(seed, Gaston::hash_value(set.second));
		return seed;
	}
};

struct PreHashType {
	size_t operator()(std::pair<size_t, ZeroSymbol*> const& set) const {
		size_t seed = boost::hash_value(set.first);
		boost::hash_combine(seed, Gaston::hash_value(set.second));
		return seed;
	}
};

template<class Key>
struct PairCompare : public std::binary_function<Key, Key, bool>
{
	/**
     * @param lhs: left operand
     * @param rhs: right operand
     * @return true if lhs = rhs
     */
	bool operator()(Key const& lhs, Key const& rhs) const {
#       if (DEBUG_TERM_CACHE_COMPARISON == true)
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
#       else
		if(lhs.second == nullptr || rhs.second == nullptr) {
			if(lhs.first == nullptr || rhs.first == nullptr) {
				return lhs.first == rhs.first && lhs.second == rhs.second;
			} else {
				return (*lhs.first == *rhs.first) && lhs.second == rhs.second;
			}
		} else {
			if(lhs.first == nullptr || rhs.first == nullptr) {
				return lhs.first == rhs.first && (*lhs.second == *rhs.second);
			} else {
				return (*lhs.second == *rhs.second) && (*lhs.first == *rhs.first);
			}
		}
#       endif
	}
};

template<class Key>
struct PrePairCompare : public std::binary_function<Key, Key, bool>
{
	/**
     * @param lhs: left operand
     * @param rhs: right operand
     * @return true if lhs = rhs
     */
	bool operator()(Key const& lhs, Key const& rhs) const {
		return (lhs.first == rhs.first) && (*lhs.second == *rhs.second);
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
#   if (OPT_USE_DENSE_HASHMAP == true)
	typedef google::dense_hash_map<Key, CacheData, KeyHash, KeyCompare> KeyToValueMap;
#   else
	typedef std::unordered_map<Key, CacheData, KeyHash, KeyCompare> KeyToValueMap;
#   endif
	typedef typename KeyToValueMap::iterator iterator;
	typedef typename KeyToValueMap::const_iterator const_iterator;

	// < Private Members >
	KeyToValueMap _cache;
	unsigned int cacheHits = 0;
	unsigned int cacheMisses = 0;

public:
	BinaryCache() {
#       if (OPT_USE_DENSE_HASHMAP == true)
		this->_cache.set_empty_key(Key());
#       endif
	}
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
	void StoreIn(Key& key, const CacheData& data){
#       if (OPT_USE_DENSE_HASHMAP == true)
		this->_cache.insert(std::make_pair(key, data));
#       else
		this->_cache.emplace(key, data);
#       endif
	}

	/**
	 * @param key: key we are looking for
	 * @param data: reference to the data
	 * @return true if found;
	 */
	bool retrieveFromCache(Key& key, CacheData& data) {
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
#       if (MEASURE_CACHE_BUCKETS == true)
        size_t bucketNo = this->_cache.bucket_count();
		std::cout << "\t\t-> Buckets: " << bucketNo;
		if(bucketNo < 50) {
			std::cout << "{";
			for(int i = 0; i < bucketNo; ++i) {
				std::cout << this->_cache.bucket_size(i);
				if(i != bucketNo - 1) {
					std::cout << ", ";
				}
			}
			std::cout << "}";
		}
		size_t sum = 0;
		size_t usedCount = 0;
		size_t usedSum = 0;
		size_t max = 0;
		for(int i = 0; i < bucketNo; ++i) {
			sum += this->_cache.bucket_size(i);
			if(max == 0 || this->_cache.bucket_size(i) > max) {
				max = this->_cache.bucket_size(i);
			}
			if(this->_cache.bucket_size(i) != 0) {
				usedSum += this->_cache.bucket_size(i);
				++usedCount;
			}
		}
		std::cout << " avg: " << std::fixed << std::setprecision(2) << (sum / (double) bucketNo) << " ("
		          << (usedCount == 0 ? 0 : (usedSum / (double) usedCount)) << "), max: "
		          << max << ", " << usedCount << " buckets used";
		std::cout << "\n";

#       endif

#       if (DEBUG_CACHE_MEMBERS == true)
        if(size) {
#           if (DEBUG_CACHE_BUCKETS == true)
			std::cout << "{\n";
			for(int i = 0; i < this->_cache.bucket_count(); ++i) {
				if(this->_cache.bucket_size(i)) {
					std::cout << "\tbucket " << i << "{\n";
					for(auto it = this->_cache.begin(i); it != this->_cache.end(i); ++it) {
						std::cout << "\t\t";
#                       if (DEBUG_CACHE_MEMBERS_HASH == true)
                        auto hasher = this->_cache.hash_function();
						std::cout << "[#" << hasher(it->first) << "] ";
#                       endif
						KeyDump(it->first);
						std::cout << " : ";
						DataDump(it->second);
						std::cout << "\n";
					}
					std::cout << "\t}\n";
				};
			}
			std::cout << "}\n";
#           else
			std::cout << "{\n";
			for (auto it = this->_cache.begin(); it != this->_cache.end(); ++it) {
				std::cout << "\t";
				KeyDump(it->first);
				std::cout << " : ";
				DataDump(it->second);
				std::cout << "\n";
			}
			std::cout << "}\n";
#           endif
		}
#       endif

		return size;
	}

	inline const_iterator begin() const{
		return this->_cache.begin();
	}

	inline const_iterator end() const {
		return this->_cache.end();
	}
};

#endif