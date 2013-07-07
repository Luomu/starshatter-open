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
     FILE:         NetClient.h
     AUTHOR:       John DiCamillo


     OVERVIEW
     ========
     Stream-oriented network client class
*/


#ifndef NetClient_h
#define NetClient_h

#include <windows.h>
#include "NetAddr.h"
#include "NetGram.h"
#include "NetSock.h"
#include "List.h"

// +-------------------------------------------------------------------+

class NetClient
{
public:
    static const char* TYPENAME() { return "NetClient"; }

    NetClient(const NetAddr& server_addr);
    virtual ~NetClient();

    int operator == (const NetClient& c)   const { return this == &c; }

    NetAddr           GetServerAddr()      const { return addr;       }

    bool              Send(Text msg);
    Text              Recv();
    Text              SendRecv(Text msg);

    int               GetLastError()       const { return err;        }
    DWORD             GetTime()            const { return delta;      }

protected:
    NetAddr           addr;
    NetSock*          sock;
    DWORD             delta;
    DWORD             time;
    int               err;

public:
    enum ERRS {
        ERR_INTR                = 10004,
        ERR_BADF                = 10009,
        ERR_ACCES               = 10013,
        ERR_FAULT               = 10014,
        ERR_INVAL               = 10022,
        ERR_MFILE               = 10024,

        ERR_WOULDBLOCK          = 10035,
        ERR_INPROGRESS          = 10036,
        ERR_ALREADY             = 10037,
        ERR_NOTSOCK             = 10038,
        ERR_DESTADDRREQ         = 10039,
        ERR_MSGSIZE             = 10040,
        ERR_PROTOTYPE           = 10041,
        ERR_NOPROTOOPT          = 10042,
        ERR_PROTONOSUPPORT      = 10043,
        ERR_SOCKTNOSUPPORT      = 10044,
        ERR_OPNOTSUPP           = 10045,
        ERR_PFNOSUPPORT         = 10046,
        ERR_AFNOSUPPORT         = 10047,
        ERR_ADDRINUSE           = 10048,
        ERR_ADDRNOTAVAIL        = 10049,
        ERR_NETDOWN             = 10050,
        ERR_NETUNREACH          = 10051,
        ERR_NETRESET            = 10052,
        ERR_CONNABORTED         = 10053,
        ERR_CONNRESET           = 10054,
        ERR_NOBUFS              = 10055,
        ERR_ISCONN              = 10056,
        ERR_NOTCONN             = 10057,
        ERR_SHUTDOWN            = 10058,
        ERR_TOOMANYREFS         = 10059,
        ERR_TIMEDOUT            = 10060,
        ERR_CONNREFUSED         = 10061,
        ERR_LOOP                = 10062,
        ERR_NAMETOOLONG         = 10063,
        ERR_HOSTDOWN            = 10064,
        ERR_HOSTUNREACH         = 10065,
        ERR_NOTEMPTY            = 10066,
        ERR_PROCLIM             = 10067,
        ERR_USERS               = 10068,
        ERR_DQUOT               = 10069,
        ERR_STALE               = 10070,
        ERR_REMOTE              = 10071
    };
};


#endif NetClient_h