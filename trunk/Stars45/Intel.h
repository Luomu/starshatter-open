/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         Intel.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
*/

#ifndef Intel_h
#define Intel_h

#include "Types.h"

// +--------------------------------------------------------------------+

class Intel
{
public:
	enum INTEL_TYPE {
		RESERVE = 1,   // out-system reserve: this group is not even here
		SECRET,        // enemy is completely unaware of this group
		KNOWN,         // enemy knows this group is in the system
		LOCATED,       // enemy has located at least the lead ship
		TRACKED        // enemy is tracking all elements
	};

	static int           IntelFromName(const char* name);
	static const char*   NameFromIntel(int intel);
};

#endif Intel_h

