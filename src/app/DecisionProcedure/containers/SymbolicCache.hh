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
#include <unordered_map>
#include <typeinfo>
#include <bitset>
#include "../environment.hh"
#include "../../Frontend/ast.h"
#include "../../Frontend/symboltable.h"

#if (OPT_USE_DENSE_HASHMAP == true)
#include <sparsehash/dense_hash_map>
#include <sparsehash/sparse_hash_map>
#endif

#include <boost/functional/hash.hpp>

inline size_t hash64shift(size_t key)
{
	key = (~key) + (key << 21); // key = (key << 21) - key - 1;
	key = key ^ (key >> 24);
	key = (key + (key << 3)) + (key << 8); // key * 265
	key = key ^ (key >> 14);
	key = (key + (key << 2)) + (key << 4); // key * 21
	key = key ^ (key >> 28);
	key = key + (key << 31);
	return key;
}

extern SymbolTable symbolTable;

namespace Gaston {
	extern size_t hash_value(Term*);
	extern size_t hash_value_no_ptr(Term*);
	extern size_t hash_value(ZeroSymbol*);
	extern size_t hash_value_no_ptr(ZeroSymbol*);
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
#		if (OPT_SHUFFLE_HASHES == true)
        return hash64shift(seed);
#		else
		return seed;
#		endif
#       else
#		if (OPT_SHUFFLE_HASHES == true)
		size_t seed = hash64shift(Gaston::hash_value(set.first));
#		else
		size_t seed = Gaston::hash_value(set.first);
#		endif
		boost::hash_combine(seed, Gaston::hash_value(set.second));
#		if (OPT_SHUFFLE_HASHES == true)
		return hash64shift(seed);
#		else
		return seed;
#		endif
#       endif
	}
};

struct ResultLevelHashType {
	size_t operator()(std::tuple<Term*, ZeroSymbol*, size_t, char> const& set) const {
		size_t seed = Gaston::hash_value(std::get<0>(set));
		boost::hash_combine(seed, Gaston::hash_value(std::get<1>(set)));
		boost::hash_combine(seed, boost::hash_value(std::get<2>(set)));
		boost::hash_combine(seed, boost::hash_value(std::get<3>(set)));
		return seed;
	}
};

struct SubsumptionHashType {
	size_t operator()(std::pair<Term*, Term*> const& set) const {
#		if (OPT_SHUFFLE_HASHES == true)
        size_t seed = hash64shift(Gaston::hash_value(set.first));
#		else
		size_t seed = Gaston::hash_value(set.first);
#		endif
		boost::hash_combine(seed, Gaston::hash_value(set.second));
#		if (OPT_SHUFFLE_HASHES == true)
        return hash64shift(seed);
#		else
		return seed;
#		endif
	}
};


struct PreHashType {
	size_t operator()(std::pair<size_t, ZeroSymbol*> const& set) const {
#		if (OPT_SHUFFLE_HASHES == true)
		size_t seed = hash64shift(boost::hash_value(set.first));
#		else
        size_t seed = boost::hash_value(set.first);
#		endif
		boost::hash_combine(seed, Gaston::hash_value(set.second));
#		if (OPT_SHUFFLE_HASHES == true)
		return hash64shift(seed);
#		else
		return seed;
#		endif
	}
};

struct DagHashType {
	size_t operator()(ASTForm* const& f) const {
		size_t seed = boost::hash_value(f->kind);
#		if(OPT_SHUFFLE_HASHES == true)
        return hash64shift(seed);
#		else
		return seed;
#		endif
	}
};

struct TermHash {
	size_t operator()(Gaston::Term_raw const& t) const {
#		if (OPT_SHUFFLE_HASHES == true)
    	return hash64shift(boost::hash_value(t));
#		else
    	return boost::hash_value(t);
#		endif
	}
};

struct TermCompare : public std::binary_function<Gaston::Term_raw, Gaston::Term_raw, bool> {
	bool operator()(Gaston::Term_raw const& lhs, Gaston::Term_raw const& rhs) const {
		return (lhs == rhs);
	}
};

using TermAt_Key = std::pair<Gaston::Term_ptr, size_t>;
struct TermAtHash {
	size_t operator()(TermAt_Key const& k) const {
		size_t seed = Gaston::hash_value(k.first);
		boost::hash_combine(seed, boost::hash_value(k.second));
		return seed;
	}
};

struct TermAtCompare : public std::binary_function<TermAt_Key, TermAt_Key, bool> {
	bool operator()(TermAt_Key const& lhs, TermAt_Key const& rhs) const {
		return (lhs.first == rhs.first) && (lhs.second == rhs.second);
	}
};

using RLC_key = std::tuple<Gaston::Term_ptr, Gaston::Symbol_ptr, size_t, char>;
struct ResultLevelCompare : public std::binary_function<RLC_key, RLC_key, bool> {
	bool operator()(RLC_key const& lhs, RLC_key const& rhs) const {
		return std::get<0>(lhs) == std::get<0>(rhs) &&
			   std::get<1>(lhs) == std::get<1>(rhs) &&
			   std::get<2>(lhs) == std::get<2>(rhs) &&
			   std::get<3>(lhs) == std::get<3>(rhs);
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

template<class Key>
struct DagCompare : public std::binary_function<Key, Key, bool> {
	bool operator()(Key const& lhs, Key const& rhs) const {
		std::fill(AST::temporalMapping.begin(), AST::temporalMapping.end(), 0);
		bool result = lhs->StructuralCompare(rhs);
		return result;
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
#	define LAST_QUERIES_SIZE 3
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

#	if (OPT_CACHE_LAST_QUERIES == true)
	Key _lastKey[LAST_QUERIES_SIZE];
	CacheData _lastData[LAST_QUERIES_SIZE];
	int pos = 0;
	int size = 0;
#	endif
public:
	BinaryCache() {
#       if (OPT_USE_DENSE_HASHMAP == true)
		this->_cache.set_empty_key(Key());
#       endif
        this->_cache.max_load_factor(0.25);

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
	void StoreIn(const Key& key, const CacheData& data){
#       if (OPT_USE_DENSE_HASHMAP == true)
		this->_cache.insert(std::make_pair(key, data));
#       else
#		if (OPT_CACHE_LAST_QUERIES == true)
        this->pos = (this->pos + 1) % LAST_QUERIES_SIZE;
		this->_lastData[this->pos] = data;
		this->_lastKey[this->pos] = key;
		this->size = std::min(this->size+1, LAST_QUERIES_SIZE);
#		endif
		this->_cache.emplace(key, data);
#       endif
	}

	/**
	 * @param key: key we are looking for
	 * @param data: reference to the data
	 * @return true if found;
	 */
	bool retrieveFromCache(const Key& key, CacheData& data) {
#		if (OPT_CACHE_LAST_QUERIES == true)
		// Check the last queries first
		for(size_t i = this->pos, j = 0; j < this->size; ++j) {
			if(this->_lastKey[i] == key) {
				++cacheHits;
				data = this->_lastData[i];
				return true;
			}
			i = (i == 0) ? LAST_QUERIES_SIZE - 1 : i - 1;
		}
#		endif

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
			for(unsigned int i = 0; i < this->_cache.bucket_count(); ++i) {
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

	int GetSize() {
		return this->_cache.size();
	}

	inline const_iterator begin() const{
		return this->_cache.begin();
	}

	inline const_iterator end() const {
		return this->_cache.end();
	}
};

#endif