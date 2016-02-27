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
	identifiers->sort();

	size_t idx = 0;
	Ident formal;
	ASTForm* restriction = nullptr;
	for (int i = 0; i < identSize; ++i) {
		val = identifiers->pop_front();
		if(symbolTable.lookupType(val) == MonaTypeTag::Varname1) {
			restriction = symbolTable.getDefault1Restriction(&formal);
		} else {
			restriction = symbolTable.getDefault2Restriction(&formal);
		}

		if(restriction != nullptr && formal == val) {
			continue;
		}

		(this->vttMap)[val] = idx;
		(this->ttvMap)[idx++] = val;
	}

#   if(DEBUG_VARMAP == true)
    this->dumpMap();
#   endif
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

uint VarToTrackMap::inverseGet(uint key) {
	assert(false && "Currently disabled functionality");
	return this->ttvMap[key];
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
