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
     FILE:         NetServer.cpp
     AUTHOR:       John DiCamillo


     OVERVIEW
     ========
     Network Server Pump for HTTP Server
*/


#include "MemDebug.h"
#include "NetServer.h"
#include "NetHost.h"
#include "NetLayer.h"

#include <stdlib.h>
#include <stdio.h>
#include <mmsystem.h>

// +-------------------------------------------------------------------+

DWORD WINAPI NetServerListenerProc(LPVOID link);
DWORD WINAPI NetServerReaderProc(LPVOID link);

struct PoolItem { NetServer* server; int thread_index; };

// +-------------------------------------------------------------------+

NetServer::NetServer(WORD port, int nthreads)
    : sock(true), pool(0), conn(0), poolsize(nthreads), err(0),
      server_shutdown(false), hreader(0)
{
    NetHost host;
    addr = NetAddr(host.Address().IPAddr(), port);

    sock.bind(addr);
    sock.listen(3);

    if (poolsize < 1) poolsize = 1;

    pool = new(__FILE__,__LINE__) HANDLE[poolsize];
    conn = new(__FILE__,__LINE__) NetSock*[poolsize];
    clients = new(__FILE__,__LINE__) NetAddr[poolsize];

    if (pool && conn && clients) {
        ZeroMemory(pool, poolsize * sizeof(HANDLE));
        ZeroMemory(conn, poolsize * sizeof(NetSock*));

        DWORD thread_id = 0;

        for (int i = 0; i < poolsize; i++) {
            thread_id = 0;
            PoolItem* item = new PoolItem;
            item->server = this;
            item->thread_index = i;

            pool[i] = CreateThread(0, 4096, NetServerReaderProc, (LPVOID) item, 0, &thread_id);
        }

        thread_id = 0;
        hreader = CreateThread(0, 4096, NetServerListenerProc, (LPVOID) this, 0, &thread_id);
    }
}

NetServer::~NetServer()
{
    if (!server_shutdown) {
        server_shutdown = true;
        sock.close();
    }

    if (hreader) {
        WaitForSingleObject(hreader, 1000);
        CloseHandle(hreader);
    }

    if (pool && poolsize) {
        for (int i = 0; i < poolsize; i++) {
            WaitForSingleObject(pool[i], 1000);
            CloseHandle(pool[i]);
            delete conn[i];
            conn[i] = 0;
        }

        delete [] pool;
        delete [] conn;
        delete [] clients;
    }
}

// +--------------------------------------------------------------------+

void
NetServer::Shutdown()
{
    server_shutdown = true;
}

// +--------------------------------------------------------------------+

DWORD WINAPI NetServerListenerProc(LPVOID link)
{
    NetServer* net_server = (NetServer*) link;

    if (net_server)
        return net_server->Listener();

    return (DWORD) E_POINTER;
}

DWORD
NetServer::Listener()
{
    while (!server_shutdown) {
        NetSock* s = sock.accept(&client_addr);

        while (s) {
            sync.acquire();

            for (int i = 0; i < poolsize; i++) {
                if (conn[i] == 0) {
                    conn[i] = s;
                    clients[i] = client_addr;
                    s = 0;
                    break;
                }
            }

            sync.release();

            // wait for a thread to become not busy
            if (s)
                Sleep(10);
        }
    }

    return 0;
}

// +--------------------------------------------------------------------+

DWORD WINAPI NetServerReaderProc(LPVOID link)
{
    if (!link) return (DWORD) E_POINTER;

    PoolItem*  item       = (PoolItem*) link;
    NetServer* net_server = item->server;
    int        index      = item->thread_index;

    delete item;

    if (net_server)
        return net_server->Reader(index);

    return (DWORD) E_POINTER;
}

DWORD
NetServer::Reader(int index)
{
    // init random seed for this thread:
    srand(timeGetTime());

    while (!server_shutdown) {
        sync.acquire();
        NetSock* s = conn[index];
        sync.release();

        if (s) {
            const int MAX_REQUEST = 4096;
            Text request;

            /***
             *** NOT SURE WHY, BUT THIS DOESN'T WORK FOR SHIT
             ***
             *** Setting the socket timeout to 2 seconds caused it
             *** to wait for two seconds, read nothing, and give up
             *** with a WSAETIMEDOUT error.  Meanwhile, the client
             *** immediately registered a failure (during the 2 sec
             *** delay) and aborted the request.
             ***

            s->set_timeout(2000);
            Text msg = s->recv();

            while (msg.length() > 0 && request.length() < MAX_REQUEST) {
                request += msg;
                msg = s->recv();
            }

             ***/

            request = s->recv();

            if (request.length() > 0 && !s->is_closed()) {
                Text response = ProcessRequest(request, clients[index]);
                err = s->send(response);
                if (err < 0) {
                    err = NetLayer::GetLastError();
                }
            }

            sync.acquire();
            delete conn[index];
            conn[index] = 0;
            sync.release();
        }
        else {
            Sleep(5);
        }
    }

    return 0;
}

// +--------------------------------------------------------------------+

Text
NetServer::ProcessRequest(Text msg, const NetAddr& addr)
{
    if (msg.indexOf("GET ") == 0)
        return DefaultResponse();
    
    return ErrorResponse();
}

Text
NetServer::DefaultResponse()
{
    Text response = 
        "HTTP/1.0 200 OK\nServer: Generic NetServer 1.0\nMIME-Version: 1.0\nContent-type: text/html\n\n";

    response += "<html><head><title>Generic NetServer 1.0</title></head>\n\n";
    response += "<body bgcolor=\"black\" text=\"white\">\n";
    response += "<h1>Generic NetServer 1.0</h1>\n";
    response += "<p>Didn't think I could do it, did ya?\n";
    response += "</body></html>\n\n";

    return response;
}

Text
NetServer::ErrorResponse()
{
    Text response = 
        "HTTP/1.0 501 Not Implemented\nServer: Generic NetServer 1.0\nMIME-Version: 1.0\nContent-type: text/html\n\n";

    response += "<html><head><title>Generic NetServer 1.0</title></head>\n\n";
    response += "<body bgcolor=\"black\" text=\"white\">\n";
    response += "<h1>Generic NetServer 1.0</h1>\n";
    response += "<p>Sorry charlie...  I'm not a magician.\n";
    response += "</body></html>\n\n";

    return response;
}
