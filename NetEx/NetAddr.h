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
     FILE:         NetAddr.h
     AUTHOR:       John DiCamillo


     OVERVIEW
     ========
     Network Address (specifically, Internet Protocol)
*/


#ifndef NetAddr_h
#define NetAddr_h

#include <windows.h>

// +-------------------------------------------------------------------+

class NetAddr
{
public:
    static const char* TYPENAME() { return "NetAddr"; }

    NetAddr(const char* a, WORD p=0);
    NetAddr(DWORD a=0, WORD p=0);
    NetAddr(const NetAddr& n);

    int operator == (const NetAddr& a) const { return addr==a.addr && port==a.port; }

    DWORD          IPAddr() const { return addr; }
    BYTE           B4()     const { return (BYTE) ((addr & 0xff000000) >> 24); }
    BYTE           B3()     const { return (BYTE) ((addr & 0x00ff0000) >> 16); }
    BYTE           B2()     const { return (BYTE) ((addr & 0x0000ff00) >>  8); }
    BYTE           B1()     const { return (BYTE) ((addr & 0x000000ff)      ); }

    WORD           Port()   const    { return port; }
    void           SetPort(WORD p)   { port = p;    }

    sockaddr*      GetSockAddr()       const;
    size_t         GetSockAddrLength() const;

    void           SetSockAddr(sockaddr* s, int size);
    void           InitFromSockAddr();

private:
    void        Init();

    DWORD       addr; // IP addr in host byte order
    WORD        port; // IP port in host byte order
    sockaddr_in sadr;
};


#endif NetAddr_h