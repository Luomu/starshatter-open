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
     FILE:         NetSock.cpp
     AUTHOR:       John DiCamillo


     OVERVIEW
     ========
     Network (IP) Socket Wrapper Implementation
*/


// WINSOCK2.H MUST COME FIRST!!
#include <winsock2.h>

#include "MemDebug.h"
#include "NetSock.h"
#include "NetLayer.h"

// +-------------------------------------------------------------------+

/**
 * Server-side socket constructor
 */
NetSock::NetSock(bool str)
    : stream(str), closed(false), stat(0), current_timeout(9999)
{
    if (stream)
        s = ::socket(AF_INET, SOCK_STREAM, 0);
    else
        s = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
}

/**
 * Server-side socket constructor
 *
 * PRIVATE: used only by the accept call to build a socket for
 * a client connection to this server
 */
NetSock::NetSock(SOCKET sock, bool str)
    : s(sock), stream(str), closed(false), stat(0), current_timeout(9999)
{ }

/**
 * Client-side socket constructor
 *
 * Will connect to server at "addr"
 */
NetSock::NetSock(const NetAddr& addr, bool str)
    : stream(str), closed(false), stat(0), current_timeout(9999)
{
    if (stream)
        s = ::socket(AF_INET, SOCK_STREAM, 0);
    else
        s = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

    connect(addr);
}

NetSock::~NetSock()
{
    close();
}

// +--------------------------------------------------------------------+

int
NetSock::bind(const NetAddr& addr)
{
    if (closed || s == INVALID_SOCKET) return INVALID_SOCKET;
    return stat = ::bind(s, addr.GetSockAddr(), addr.GetSockAddrLength());
}

int
NetSock::connect(const NetAddr& addr)
{
    if (closed || s == INVALID_SOCKET) return INVALID_SOCKET;
    return stat = ::connect(s, addr.GetSockAddr(), addr.GetSockAddrLength());
}

int
NetSock::listen(int max_connections)
{
    if (closed || s == INVALID_SOCKET) return INVALID_SOCKET;
    return stat = ::listen(s, max_connections);
}

NetSock*
NetSock::accept(NetAddr* addr)
{
    if (closed || s == INVALID_SOCKET) return 0;

    SOCKET conn = INVALID_SOCKET;

    if (addr) {
        sockaddr a;
        int      asize = sizeof(a);
    
        conn = ::accept(s, &a, &asize);

        if (conn != INVALID_SOCKET && asize > 0) {
            addr->SetSockAddr(&a, asize);
        }
    }
    else {
        conn = ::accept(s, 0, 0);
    }

    if (conn == INVALID_SOCKET)
        return 0;

    return new(__FILE__,__LINE__) NetSock(conn, stream);
}

// +--------------------------------------------------------------------+
 
int
NetSock::available()
{
    if (closed || s == INVALID_SOCKET) return 0;
 
    DWORD nbytes = 0;
    if (::ioctlsocket(s, FIONREAD, &nbytes) == 0)
        return (int) nbytes;

    return 0;
}
 
// +--------------------------------------------------------------------+

int
NetSock::send(Text msg)
{
    if (closed || s == INVALID_SOCKET) return INVALID_SOCKET;
    return stat = ::send(s, msg.data(), msg.length(), 0);
}

Text
NetSock::recv()
{
    if (closed || s == INVALID_SOCKET) return "";

    static char rbuf[8192];
    int         rlen = -1;

    if (stream) {
        rlen = ::recv(s, rbuf, sizeof(rbuf), 0);
    }
    else {
        rlen = ::recvfrom(s, rbuf, sizeof(rbuf), 0, 0, 0);
    }

    if (rlen < 0) {
        stat = NetLayer::GetLastError();

        switch (stat) {
        case WSAENETDOWN:
        case WSAENETRESET:
        case WSAEINTR:
        case WSAESHUTDOWN:
        case WSAECONNABORTED:
        case WSAECONNRESET:
        case WSAETIMEDOUT:
                close();
                break;

        case WSAEWOULDBLOCK:
                stat = WSAEWOULDBLOCK;
                break;
        }

        return Text();
    }

    else if (rlen == 0) {
        return Text();
    }

    return Text(rbuf, rlen);
}


// +--------------------------------------------------------------------+

int
NetSock::sendto(Text msg, const NetAddr& dest)
{
    if (closed || s == INVALID_SOCKET) return INVALID_SOCKET;
    return stat = ::sendto(s, msg.data(), msg.length(),
                           0, dest.GetSockAddr(), dest.GetSockAddrLength());
}

Text
NetSock::recvfrom(NetAddr* a)
{
    if (closed || s == INVALID_SOCKET) return "";

    static char rbuf[4096];
    int rlen = 0;

    if (a) {
        int addrlen = a->GetSockAddrLength();
        rlen = ::recvfrom(s, rbuf, sizeof(rbuf), 0, a->GetSockAddr(), &addrlen);
        a->InitFromSockAddr();
    }
    else {
        rlen = ::recvfrom(s, rbuf, sizeof(rbuf), 0, 0, 0);
    }

    if (rlen < 0) {
        stat = NetLayer::GetLastError();
        return Text();
    }

    else if (rlen == 0) {
        return Text();
    }

    return Text(rbuf, rlen);
}

// +--------------------------------------------------------------------+

int
NetSock::select(SELECT_TYPE t, long seconds, long microseconds)
{
    if (closed || s == INVALID_SOCKET) return INVALID_SOCKET;

    FD_SET fd;
    ZeroMemory(&fd, sizeof(fd));

    FD_SET(s, &fd);
    TIMEVAL  timeval = {seconds, microseconds};
    TIMEVAL* timeout = &timeval;

    if (t == SELECT_READ)
        return stat = ::select(1, &fd, 0, 0, timeout);

    else if (t == SELECT_WRITE)
        return stat = ::select(1, 0, &fd, 0, timeout);

    else if (t == (SELECT_READ|SELECT_WRITE))
        return stat = ::select(1, &fd, &fd, 0, timeout);

    return 0;
}

// +--------------------------------------------------------------------+

int
NetSock::set_timeout(int msecs)
{
    if (closed || s == INVALID_SOCKET) return 0;
    if (msecs == current_timeout)      return 1;
 
    // zero timeout means non-blocking
    if (msecs == 0) {
        u_long nonblocking = 1;
        if (::ioctlsocket(s, FIONBIO, &nonblocking) == SOCKET_ERROR) {
            stat = NetLayer::GetLastError();
            return stat;
        }
    }
 
    // non-zero timeout means blocking
    else {
        if (current_timeout == 0) {
            u_long nonblocking = 0;   // disable non-blocking
            if (::ioctlsocket(s, FIONBIO, &nonblocking) == SOCKET_ERROR) {
                stat = NetLayer::GetLastError();
                return stat;
            }
        }

        // max timeout means infinite wait
        if (msecs >= NET_MAX_TIMEOUT) {
            int maxto = 0;
            ::setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char*) &maxto, sizeof(maxto));
            ::setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, (char*) &maxto, sizeof(maxto));
        }
 
        // otherwise, set the timeout
        else {
            ::setsockopt(s, SOL_SOCKET, SO_RCVTIMEO, (char*) &msecs, sizeof(msecs));
            ::setsockopt(s, SOL_SOCKET, SO_SNDTIMEO, (char*) &msecs, sizeof(msecs));
        }
    }
 
    current_timeout = msecs;
    return 1;
}

// +--------------------------------------------------------------------+

int
NetSock::shutdown_input()
{
    if (closed || s == INVALID_SOCKET) return INVALID_SOCKET;
    ::shutdown(s, SD_RECEIVE);
    return 0;
}

// +--------------------------------------------------------------------+

int
NetSock::shutdown_output()
{
    if (closed || s == INVALID_SOCKET) return INVALID_SOCKET;
    ::shutdown(s, SD_SEND);
    return 0;
}

// +--------------------------------------------------------------------+

int
NetSock::close()
{
    if (s != INVALID_SOCKET && !closed) {
        ::shutdown(s, SD_BOTH);
        ::closesocket(s);

        closed = true;
    }

    return 0;
}

// +--------------------------------------------------------------------+

DWORD
NetSock::max_packet_size() const
{
    DWORD size = 0;
    int   len  = sizeof(size);

    ::getsockopt(s, SOL_SOCKET, 0x2003, (char*) &size, &len);

    return size;
}
