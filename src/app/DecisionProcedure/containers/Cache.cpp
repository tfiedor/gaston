#include "Cache.hh"

/**
 * Class: Cache
 * < Public Methods >
 */

/**
 * Looks for data specified by @key in Cache. If no data is found for key,
 * then default value is returned
 *
 * @param key: looked-up key
 * @return: data for key
 */
CacheData lookUp(CacheKey key) {
	return this->_cache[key];
}

/**
 * Stores data for @p key in Cache
 *
 * @param key: key for which we are storing data
 * @param data: stored data
 */
void Cache::storeIn(CacheKey key, CacheData data) {
	this->_cache[key] = data;
}

/**
 * Class: Multi Leveled Cache
 * <Public Methods >
 */

/**
 * Looks up data for appropriate level and key
 *
 * @param key: key we are looking the data up
 * @param level: level of the cache
 * @return: looked up data
 */
CacheData MultiLevelCache::lookUp(CacheKey key, unsigned level) {
	return this->_mlCache[level].lookUp(key);
}

/**
 * Stores data in cache
 *
 * @param key: key we are storing for
 * @param data: data we are storing
 * @param level: level where we are storing
 */
void MultiLevelCache::storeIn(CacheKey key, CacheData data, unsigned level) {
	this->_mlCache[level].storeIn(key, data);
}

