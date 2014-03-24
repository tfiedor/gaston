#ifndef __CACHE__H__
#define __CACHE__H__

#include <map>

/**
 * Class representing cache for storing @p CacheData according to the
 * @p CacheKey
 *
 * CacheKey represents the key for lookup of CacheData
 * CacheData represents pure data that are stored inside cache
 */
template<class CacheKey, class CacheData>
class Cache {
private:
	// < Typedefs >
	typedef std::map<CacheKey, CacheData> CacheMap;

	// < Private Members >
	CacheMap _cache;
public:
	// < Public Methods >
	CacheData lookUp(CacheKey);
	void storeIn(CacheKey, CacheData);
};

template<class CacheKey, class CacheData, unsigned LEVELS>
class MultiLevelCache {
private:
	// < Private Members>
	Cache<CacheKey, CacheData> _mlCache[LEVELS];

public:
	// < Public Methods >
	CacheData lookUp(CacheKey, unsigned);
	void storeIn(CacheKey, CacheData, unsigned);
};

#endif
