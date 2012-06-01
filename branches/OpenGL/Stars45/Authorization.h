/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         Authorization.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Authorization Sprite class
*/

#ifndef Authorization_h
#define Authorization_h

#include "Types.h"

// +--------------------------------------------------------------------+

class Authorization
{
public:
	static bool          IsUserAuthorized();
	static const char*   GetSerialNumber();
};

#endif Authorization_h

