/*  Project nGenEx
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    nGenEx.lib
	FILE:         Res.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Abstract Resource class
*/

#include "MemDebug.h"
#include "Res.h"

// +--------------------------------------------------------------------+

static int RESOURCE_KEY = 1;

Resource::Resource()
: id((HANDLE) RESOURCE_KEY++)
{ }

Resource::~Resource()
{ }

