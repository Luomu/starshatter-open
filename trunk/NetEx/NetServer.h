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
     FILE:         NetServer.h
     AUTHOR:       John DiCamillo


     OVERVIEW
     ========
     Network Server Pump for HTTP Server
*/


#ifndef NetServer_h
#define NetServer_h

#include <windows.h>
#include "NetAddr.h"
#include "NetGram.h"
#include "NetSock.h"
#include "List.h"

// +-------------------------------------------------------------------+

class NetServer
{
public:
    static const char* TYPENAME() { return "NetServer"; }

    NetServer(WORD port, int poolsize=1);
    virtual ~NetServer();

    int operator == (const NetServer& l) const { return addr == l.addr; }

    virtual void      Shutdown();
    virtual DWORD     Listener();
    virtual DWORD     Reader(int index);

    virtual Text      ProcessRequest(Text request, const NetAddr& addr);
    virtual Text      DefaultResponse();
    virtual Text      ErrorResponse();

    const NetAddr&    GetAddress() const { return addr; }
    int               GetLastError() const { return err; }

protected:
    NetAddr           addr;
    NetSock           sock;
    NetAddr           client_addr;

    int               poolsize;
    HANDLE            hreader;
    HANDLE*           pool;
    NetSock**         conn;
    NetAddr*          clients;
    int               err;
    bool              server_shutdown;

    ThreadSync        sync;
};


#endif NetServer_h