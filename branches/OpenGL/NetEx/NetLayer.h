/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    NetEx.lib
    FILE:         NetLayer.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Wrapper for WinSock Library
*/

#ifndef NetLayer_h
#define NetLayer_h

#include <windows.h>
#include "Text.h"

// +-------------------------------------------------------------------+

class NetLayer
{
public:
   static const char* TYPENAME() { return "NetLayer"; }

   NetLayer();
   ~NetLayer();

   bool           OK()           const;
   const char*    Description()  const;

   static int     GetLastError();
   static DWORD   GetTime();
   static long    GetUTC();
   static Text    GetHostName();

private:
   WSADATA  info;
   bool     fail;
};


#endif NetLayer_h