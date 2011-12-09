/*  Project nGen
	John DiCamillo
	Copyright © 1997-2002. All Rights Reserved.

	SUBSYSTEM:    nGen.lib
	FILE:         Universe.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Abstract Universe class
*/

#ifndef Universe_h
#define Universe_h

#include "Types.h"

// +--------------------------------------------------------------------+

class Universe
{
public:
	Universe()           { }
	virtual ~Universe()  { }

	virtual void   ExecFrame(double seconds) { }
};

#endif Universe_h

