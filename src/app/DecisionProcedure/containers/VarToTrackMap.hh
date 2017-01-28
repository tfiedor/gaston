/*****************************************************************************
 *  dWiNA - Deciding WSkS using non-deterministic automata
 *
 *  Copyright (c) 2014  Tomas Fiedor <xfiedo01@stud.fit.vutbr.cz>
 *
 *  Description:
 *    Implementation of Var to Track Map
 *
 *****************************************************************************/

#ifndef __VAR_TO_TRACK_MAP__
#define __VAR_TO_TRACK_MAP__

#include <map>

#include "../../Frontend/ident.h"
#include "../environment.hh"

class VarToTrackMap {
private:
	// < Typedefs >
	typedef std::map<unsigned int, unsigned int> varMap;

	// < Private Members >
	varMap vttMap;
	varMap ttvMap;

	// < Private Methods >
	void addIdentifiers(IdentList* );

public:
	// < Public Methods >
	unsigned int TrackLength();
	unsigned int operator[](unsigned int);
	unsigned int inverseGet(unsigned int);
	void dumpMap();
	bool IsIn(unsigned int);

	VarToTrackMap() : vttMap() {};

	void initializeFromList(IdentList*);
	void initializeFromLists(IdentList*, IdentList*);
};

#endif
