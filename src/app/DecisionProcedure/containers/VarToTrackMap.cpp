#include "VarToTrackMap.hh"

#include <iostream>

/**
 * Private Methods
 */

/**
 * Adds variables from list to the map
 *
 * @param identifiers: list of variables
 */
void VarToTrackMap::addIdentifiers(IdentList* identifiers) {
	uint index = this->vttMap.size();
	uint val;
	uint identSize = identifiers->size();

	for (int i = 0; i < identSize; ++i) {
		val = identifiers->pop_front();
		(this->vttMap)[val] = index++;
	}
}

/**
 * Public Methods
 */

/**
 * @return: Lenght of the track
 */
uint VarToTrackMap::TrackLength() {
	return this->vttMap.size();
}

/**
 * @param[in] val: value of the variable we are looking up
 * @return: track number associanted to variable val
 */
uint VarToTrackMap::operator[](uint val) {
	return this->vttMap[val];
}

/**
 * Prints var -> trackNo map
 */
void VarToTrackMap::dumpMap() {
	for (auto it = this->vttMap.begin(); it != vttMap.end(); ++it) {
		std::cout << it->first << " -> " << it->second << '\n';
	}
}

/**
 * Constructs a mapping from variables to track according to the list of
 * variables
 *
 * @param usedVar: list of variables used for track
 */
void VarToTrackMap::initializeFromList(IdentList* usedVar) {
	this->addIdentifiers(usedVar);
}

/**
 * Constructs a mapping from variables to track according to the list of
 * variables encountering in prefix and int matrix
 */
void VarToTrackMap::initializeFromLists(IdentList* prefixVars, IdentList* matrixVars) {
	this->addIdentifiers(prefixVars);
	this->addIdentifiers(matrixVars);
}
