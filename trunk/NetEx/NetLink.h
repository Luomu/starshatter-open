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
     FILE:         NetLink.h
     AUTHOR:       John DiCamillo


     OVERVIEW
     ========
     Network Link for Remote Player
*/


#ifndef NetLink_h
#define NetLink_h

#include <windows.h>
#include "NetAddr.h"
#include "NetSock.h"
#include "List.h"
#include "ThreadSync.h"

// +-------------------------------------------------------------------+

class NetGram;
class NetMsg;
class NetPeer;

// +-------------------------------------------------------------------+

class NetLink
{
public:
    static const char* TYPENAME() { return "NetLink"; }

    NetLink();
    NetLink(NetAddr& a);
    virtual ~NetLink();

    int operator == (const NetLink& that) const { return this == &that; }

    const NetAddr&    GetAddress() const { return addr;  }

    DWORD             AddPeer(const char* a, WORD p=12345);
    DWORD             AddPeer(DWORD       a, WORD p=12345);
    DWORD             AddPeer(const NetAddr& a);

    bool              SendMessage(DWORD nid, void* d, int l, BYTE f=0);
    bool              SendMessage(DWORD nid, BYTE type, const char* text, int len, BYTE f=0);
    bool              SendMessage(NetMsg* msg);

    NetMsg*           GetMessage();
    NetMsg*           GetMessage(DWORD netid);

    virtual void      Shutdown();
    DWORD             DoSendRecv();

    DWORD             GetResendInterval()     const { return resend_time;   }
    void              SetResendInterval(DWORD t)    { resend_time = t;      }
    DWORD             GetTrafficInterval()    const { return traffic_time;  }
    void              SetTrafficInterval(DWORD t)   { traffic_time = t;     }

    DWORD             GetPacketsSent()        const { return packets_sent;  }
    DWORD             GetPacketsRecv()        const { return packets_recv;  }
    DWORD             GetBytesSent()          const { return bytes_sent;    }
    DWORD             GetBytesRecv()          const { return bytes_recv;    }
    DWORD             GetRetries()            const { return retries;       }
    DWORD             GetDrops()              const { return drops;         }
    DWORD             GetLag()                const { return lag;           }

    NetPeer*          FindPeer(const NetAddr& a);
    NetPeer*          FindPeer(DWORD netid);

protected:
    void              SendNetGram(NetGram* g);
    NetGram*          RecvNetGram();
    void              AckNetGram(NetGram* gram);
    void              ProcessAck(NetGram* gram);
    void              QueueNetGram(NetGram* gram);

    void              ReadPackets();
    void              SendPackets();
    void              DoRetries();

    NetAddr           addr;
    NetSock           sock;
    List<NetGram>     send_list;
    List<NetMsg>      recv_list;
    List<NetPeer>     peer_list;

    HANDLE            hnet;
    bool              shutdown;
    ThreadSync        sync;

    DWORD             resend_time;
    DWORD             traffic_time;

    DWORD             packets_sent;
    DWORD             packets_recv;
    DWORD             bytes_sent;
    DWORD             bytes_recv;
    DWORD             retries;
    DWORD             drops;
    DWORD             lag;

    DWORD             lag_samples[10];
    int               lag_index;
};


#endif NetLink_h