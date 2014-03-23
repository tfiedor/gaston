#ifndef __VAR_TO_TRACK_MAP__
#define __VAR_TO_TRACK_MAP__

#include <map>

#include "../../Frontend/ident.h"

class VarToTrackMap {
private:
	// < Typedefs >
	typedef unsigned int uint;
	typedef std::map<uint, uint> varMap;

	// < Members >
	varMap vttMap;

	// < Methods >
	void addIdentifiers(IdentList* );

public:
	// < Methods >
	uint TrackLength();
	uint operator[](uint);
	void dumpMap();

	VarToTrackMap() : vttMap() {};

	void initializeFromList(IdentList*);
	void initializeFromLists(IdentList*, IdentList*);
};

#endif
