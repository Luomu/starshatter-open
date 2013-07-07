/*  Starshatter OpenSource Distribution
     Copyright (c) 1997-2004, Destroyer Studios LLC.
     All Rights Reserved.

     Redistribution and use in source and binary forms, with or without
     modification, are permitted provided that the following conditions are met:

     * Redistributions of source code must retain the above copyright notice,
        this list of conditions and the following disclaimer.
     * Redistributions in binary form must reproduce the above copyright notice,
        this list of conditions and the following disclaimer in the documentation
        and/or other materials provided with the distribution.
     * Neither the name "Destroyer Studios" nor the names of its contributors
        may be used to endorse or promote products derived from this software
        without specific prior written permission.

     THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
     AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
     IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
     ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
     LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
     CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
     SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
     INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
     CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
     ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
     POSSIBILITY OF SUCH DAMAGE.

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
