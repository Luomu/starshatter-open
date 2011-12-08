/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    NetEx.lib
    FILE:         NetLayer.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Wrapper for WinSock Library
*/


#include "MemDebug.h"
#include <windows.h>
#include "NetLayer.h"

#include <mmsystem.h>
#include <time.h>

// +-------------------------------------------------------------------+

static DWORD baseTime = timeGetTime();

// +-------------------------------------------------------------------+

NetLayer::NetLayer()
{
   fail = false;
   ZeroMemory(&info, sizeof(info));

   WORD ver = MAKEWORD(2,2);
   int  err = WSAStartup(ver, &info);

   if (err)
      fail = true;
}

NetLayer::~NetLayer()
{
   WSACleanup();
}

bool
NetLayer::OK() const
{
   return !fail;
}

const char*
NetLayer::Description() const
{
   return info.szDescription;
}

int
NetLayer::GetLastError()
{
   return WSAGetLastError();
}

DWORD
NetLayer::GetTime()
{
   DWORD msec = timeGetTime();

   if (msec >= baseTime) {
      return msec - baseTime;
   }

   else {
      DWORD extra = 0xffffffff;
      return msec + extra - baseTime;
   }
}

long
NetLayer::GetUTC()
{
   return (long) time(0);
}

Text
NetLayer::GetHostName()
{
   char hostname[256];
   ZeroMemory(hostname, sizeof(hostname));
   ::gethostname(hostname, sizeof(hostname));

   return hostname;
}
