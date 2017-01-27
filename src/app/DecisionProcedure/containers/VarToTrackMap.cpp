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
extern Ident allPosVar;

/**
 * Private Methods
 */

/**
 * Adds variables from list to the map
 *
 * @param identifiers: list of variables
 */
void VarToTrackMap::addIdentifiers(IdentList* identifiers) {
	uint identSize = identifiers->size();
	identifiers->sort();

	size_t idx = 0;
	Ident formal;
	ASTForm* restriction = nullptr;
	for (unsigned int i = 0; i < identSize; ++i) {
		// Fixme: there should be some shit with restrictions
		int val = identifiers->pop_front();
		if(symbolTable.lookupType(val) == MonaTypeTag::Varname1) {
			restriction = symbolTable.getDefault1Restriction(&formal);
		} else if (symbolTable.lookupType(val) == MonaTypeTag::Varname2) {
			restriction = symbolTable.getDefault2Restriction(&formal);
		} else {
			restriction = nullptr;
		}

		// Fixme: Refactor this later
		if(restriction != nullptr && formal == val) {
			continue;
		} else if(allPosVar != -1)  {
			restriction = symbolTable.getRestriction(allPosVar, &formal);
			IdentList free, bound;
			restriction->freeVars(&free, &bound);
			bool skip = false;
			for(auto it = bound.begin(); it != bound.end(); ++it) {
				if(*it == val) {
					skip = true;
					break;
				}
			}
			if(skip)
				continue;
		}

		(this->vttMap)[val] = idx;
		(this->ttvMap)[idx++] = val;
	}

#   if(DEBUG_VARMAP == true)
	std::cout << "[*] Initialized VarMap\n";
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
	assert(this->IsIn(val));
	return this->vttMap[val];
}

bool VarToTrackMap::IsIn(uint val) {
	return this->vttMap.find(val) != this->vttMap.end();
}

uint VarToTrackMap::inverseGet(uint key) {
	return this->ttvMap[key];
}

/**
 * Prints var -> trackNo map
 */
void VarToTrackMap::dumpMap() {
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
