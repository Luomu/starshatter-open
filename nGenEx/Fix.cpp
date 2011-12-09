/*  Project nGenEx
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    nGenEx.lib
	FILE:         Fix.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Fixed point number class with 16 bits of fractional precision
*/

#include "MemDebug.h"
#include "Fix.h"

// +--------------------------------------------------------------------+

const fix    fix::one         = fix(1);
const fix    fix::two         = fix(2);
const fix    fix::three       = fix(3);
const fix    fix::five        = fix(5);
const fix    fix::ten         = fix(10);
