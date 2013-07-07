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
     FILE:         NetAddr.cpp
     AUTHOR:       John DiCamillo


     OVERVIEW
     ========
     Network Address
*/


#include "MemDebug.h"
#include "NetAddr.h"
#include "NetHost.h"
#include "NetLayer.h"
#include <ctype.h>

NetAddr::NetAddr(const char* host_name, WORD p)
    : addr(0), port(p)
{
    if (host_name && *host_name) {
        HOSTENT* h = 0;

        if (isdigit(*host_name)) {
            DWORD a = inet_addr(host_name);
            h = gethostbyaddr((const char*) &a, 4, AF_INET);   
        }
        else {
            h = gethostbyname(host_name);
        }

        if (h) {
            if (h->h_addr_list) {
                addr = **(DWORD**) (h->h_addr_list);
            }
        }
    }

    Init();
}

NetAddr::NetAddr(DWORD a, WORD p)
    : addr(a), port(p)
{
    Init();
}

NetAddr::NetAddr(const NetAddr& n)
    : addr(n.addr), port(n.port)
{
    Init();
}

// +--------------------------------------------------------------------+

void
NetAddr::Init()
{
    ZeroMemory(&sadr, sizeof(sadr));

    sadr.sin_family      = AF_INET;
    sadr.sin_port        = ::htons(port);
    sadr.sin_addr.s_addr = addr;
}

void
NetAddr::InitFromSockAddr()
{
    addr = sadr.sin_addr.s_addr;
    port = ::ntohs(sadr.sin_port);
}

// +--------------------------------------------------------------------+

sockaddr*
NetAddr::GetSockAddr() const
{
    return (sockaddr*) &sadr;
}

size_t
NetAddr::GetSockAddrLength() const
{
    return sizeof(sadr);
}

// +--------------------------------------------------------------------+

void
NetAddr::SetSockAddr(sockaddr* s, int size)
{
    if (s) {
        ZeroMemory(&sadr, sizeof(sadr));

        if (size > sizeof(sadr))
            CopyMemory(&sadr, s, sizeof(sadr));
        else
            CopyMemory(&sadr, s, size);

        InitFromSockAddr();
    }
}

