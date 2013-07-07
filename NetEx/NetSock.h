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
     FILE:         NetSock.h
     AUTHOR:       John DiCamillo


     OVERVIEW
     ========
     Network (IP) Socket
*/

#ifndef NetSock_h
#define NetSock_h

#include <windows.h>
#include "NetAddr.h"
#include "Text.h"

// +-------------------------------------------------------------------+

#define NET_MAX_TIMEOUT 1e9

class NetSock
{
public:
    static const char* TYPENAME() { return "NetSock"; }

    enum SELECT_TYPE {
        SELECT_READ  = 1,
        SELECT_WRITE = 2
    };

    NetSock(bool stream=false);
    NetSock(const NetAddr& addr, bool stream=false);
    ~NetSock();

    int      bind(const NetAddr& addr);
    int      connect(const NetAddr& addr);
    int      listen(int max_connections=5);
    NetSock* accept(NetAddr* addr=0);
    int      available();
    int      send(Text msg);
    Text     recv();
    int      sendto(Text msg, const NetAddr& dest);
    Text     recvfrom(NetAddr* a=0);
    int      select(SELECT_TYPE t=SELECT_READ,
                         long seconds=0, long microseconds=0);

    int      shutdown_input();
    int      shutdown_output();
    int      set_timeout(int msecs);
    int      close();

    DWORD    max_packet_size() const;
    bool     is_stream()       const { return stream; }
    bool     is_closed()       const { return closed; }
    int      status()          const { return stat;   }

private:
    NetSock(SOCKET s, bool stream);

    SOCKET   s;
    bool     stream;
    bool     closed;
    int      stat;
    int      current_timeout;
};


#endif NetSock_h