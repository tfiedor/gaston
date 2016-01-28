/*****************************************************************************
 *  dWiNA - Deciding WSkS using non-deterministic automata
 *
 *  Copyright (c) 2014  Tomas Fiedor <xfiedo01@stud.fit.vutbr.cz>
 *
 *  Description:
 *    Implementation of Var to Track Map
 *
 *****************************************************************************/

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
	// Fixme: Refactor this
	return this->vttMap[val];
	#if (OPT_SMARTER_MONA_CONVERSION == true)
	return this->vttMap[val];
	#else
	unsigned int mapSize = this->vttMap.size();
	return mapSize - 1 - this->vttMap[val];
	#endif
}

/**
 * Prints var -> trackNo map
 */
void VarToTrackMap::dumpMap() {
	unsigned int mapSize = this->vttMap.size();
	for (auto it = this->vttMap.begin(); it != vttMap.end(); ++it) {
		std::cout << it->first << " -> " << (mapSize - 1 - it->second) << '\n';
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
