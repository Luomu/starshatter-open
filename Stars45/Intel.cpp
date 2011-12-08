/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         Intel.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    An element in the dynamic campaign
*/

#include "MemDebug.h"
#include "Intel.h"

#include "Game.h"
#include "DataLoader.h"

// +--------------------------------------------------------------------+

static const char* intel_name[] = {
   "",
   "Reserve",
   "Secret",
   "Known",
   "Located",
   "Tracked",
};

const char*
Intel::NameFromIntel(int intel)
{
   return intel_name[intel];
}

int
Intel::IntelFromName(const char* type_name)
{
   for (int i = RESERVE; i <= TRACKED; i++)
      if (!stricmp(type_name, intel_name[i]))
         return i;

   return 0;
}
