/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         Callsign.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Package Callsign catalog class
*/

#ifndef Callsign_h
#define Callsign_h

#include "Types.h"

// +--------------------------------------------------------------------+

class Callsign
{
public:
	static const char*   GetCallsign(int IFF=1);
};

#endif Callsign_h

