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
	typedef unsigned int uint;
	typedef std::map<uint, uint> varMap;

	// < Private Members >
	varMap vttMap;

	// < Private Methods >
	void addIdentifiers(IdentList* );

public:
	// < Public Methods >
	uint TrackLength();
	uint operator[](uint);
	void dumpMap();

	VarToTrackMap() : vttMap() {};

	void initializeFromList(IdentList*);
	void initializeFromLists(IdentList*, IdentList*);
};

#endif
