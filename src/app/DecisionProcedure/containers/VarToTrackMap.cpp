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
#include "../../Frontend/symboltable.h"
#include "../../Frontend/offsets.h"

#include <iostream>

extern SymbolTable symbolTable;
extern Offsets offsets;

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

	auto it = identifiers->begin();
	size_t min = (*it);
	for(; it != identifiers->end(); ++it) {
		min = (min > (*it)) ? (*it) : min;
	}

	for (int i = 0; i < identSize; ++i) {
		val = identifiers->pop_front();
		(this->vttMap)[val] = val - min;
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
}

/**
 * Prints var -> trackNo map
 */
void VarToTrackMap::dumpMap() {
	unsigned int mapSize = this->vttMap.size();
	for (auto it = this->vttMap.begin(); it != vttMap.end(); ++it) {
		std::cout << it->first << "(" << symbolTable.lookupSymbol(it->first) << ") -> " << it->second << '\n';
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
	assert(false);
	this->addIdentifiers(prefixVars);
	this->addIdentifiers(matrixVars);
}
