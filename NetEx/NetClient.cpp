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
     FILE:         NetClient.cpp
     AUTHOR:       John DiCamillo


     OVERVIEW
     ========
     Network Server Pump for HTTP Server
*/


#include "MemDebug.h"
#include "NetClient.h"
#include "NetHost.h"
#include "NetLayer.h"
#include <mmsystem.h>

// +-------------------------------------------------------------------+

NetClient::NetClient(const NetAddr& server_addr)
    : addr(server_addr), sock(0), delta(0), time(0), err(0)
{
}

NetClient::~NetClient()
{
    delete sock;
}

// +--------------------------------------------------------------------+

bool
NetClient::Send(Text msg)
{
    if (msg.length() > 0) {
        if (sock)
            delete sock;

        sock  = new(__FILE__,__LINE__) NetSock(addr, true);
        delta = 0;
        time  = timeGetTime();

        if (!sock) {
            err = ERR_NOBUFS;
            return false;
        }

        err = sock->send(msg);
        if (err < 0) {
            err = NetLayer::GetLastError();
            return false;
        }

        err = sock->shutdown_output();
        if (err < 0) {
            err = NetLayer::GetLastError();
            return false;
        }

        return true;
    }

    else {
        delete sock;
        sock = 0;
    }

    return false;
}

Text
NetClient::Recv()
{
    Text response;

    if (sock) {
        int ready = sock->select();

        while (!ready && timeGetTime() - time < 2000) {
            Sleep(5);
            ready = sock->select();
        }

        if (ready) {
            Text msg = sock->recv();

            while (msg.length() > 0) {
                response += msg;
                msg = sock->recv();
            }

            delta = timeGetTime() - time;
        }

        delete sock;
        sock = 0;
    }

    return response;
}

Text
NetClient::SendRecv(Text msg)
{
    Text response;

    if (msg.length() > 0 && Send(msg)) {
        response = Recv();
    }

    return response;
}
