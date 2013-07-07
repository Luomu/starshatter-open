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

    SUBSYSTEM:    Stars.exe
    FILE:         NetGame.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Network Game Manager and Player classes
*/

#include "MemDebug.h"
#include "NetPacket.h"
#include "NetLink.h"
#include "NetMsg.h"
#include "Ship.h"

#include "Game.h"

// +--------------------------------------------------------------------+

const int PING_SIZE        =   8;
const int SHIP_LOC_SIZE    =  36;
const int JOIN_REQ_SIZE    = 116;
const int JOIN_ANN_SIZE    = 116;
const int QUIT_ANN_SIZE    =   8;

// +--------------------------------------------------------------------+

NetPacket::NetPacket(NetMsg* g)
: msg(g)
{ }

NetPacket::NetPacket(DWORD netid, BYTE type)
{
    int   len = 0;
    char  buf[256];
    ZeroMemory(buf, 256);

    switch (type) {
    case NET_PING:          len = PING_SIZE;     break;
    case NET_PONG:          len = PING_SIZE;     break;
    case NET_OBJ_LOC:       len = SHIP_LOC_SIZE; break;

    case NET_JOIN_REQUEST:  len = JOIN_REQ_SIZE; break;
    case NET_JOIN_ANNOUNCE: len = JOIN_ANN_SIZE; break;
    case NET_QUIT_ANNOUNCE: len = QUIT_ANN_SIZE; break;

    default:                len = JOIN_REQ_SIZE; break;
    }

    msg = new(__FILE__,__LINE__) NetMsg(netid, type, buf, len);
}

NetPacket::~NetPacket()
{
    delete msg;
}

bool
NetPacket::Send(NetLink& link)
{
    bool sent = false;

    if (msg)
    sent = link.SendMessage(msg);

    msg = 0;
    return sent;
}

// +--------------------------------------------------------------------+

DWORD
NetPacket::NetID() const
{
    if (msg)
    return msg->NetID();

    return 0;
}

BYTE
NetPacket::Type() const
{
    if (msg)
    return msg->Type();

    return 0;
}

// +--------------------------------------------------------------------+

DWORD
NetPacket::GetPingSequence()
{
    if (msg && msg->Length() >= PING_SIZE) {
        DWORD* data = (DWORD*) (msg->Data()+4);
        return *data;
    }

    return 0;
}

void
NetPacket::SetPingSequence(DWORD seq)
{
    if (msg && msg->Length() >= PING_SIZE) {
        DWORD* data = (DWORD*) (msg->Data()+4);
        *data = seq;
    }
}

// +--------------------------------------------------------------------+

DWORD
NetPacket::GetNetID()
{
    if (msg && msg->Length() >= PING_SIZE) {
        DWORD* data = (DWORD*) (msg->Data()+4);
        return *data;
    }

    return 0;
}

void
NetPacket::SetNetID(DWORD id)
{
    if (msg && msg->Length() >= PING_SIZE) {
        DWORD* data = (DWORD*) (msg->Data()+4);
        *data = id;
    }
}

// +--------------------------------------------------------------------+

Point
NetPacket::GetShipLocation()
{
    if (msg && msg->Length() >= SHIP_LOC_SIZE) {
        long* data = (long*) (msg->Data()+8);
        long  x    = *(data + 0);
        long  y    = *(data + 1);
        long  z    = *(data + 2);

        return Point(x/100.0, y/100.0, z/100.0);
    }

    return Point();
}

void
NetPacket::SetShipLocation(const Point& loc)
{
    if (msg && msg->Length() >= SHIP_LOC_SIZE) {
        long x = (long) (loc.x * 100);
        long y = (long) (loc.y * 100);
        long z = (long) (loc.z * 100);

        long* data = (long*) (msg->Data()+8);

        *(data + 0) = x;
        *(data + 1) = y;
        *(data + 2) = z;
    }
}

// +--------------------------------------------------------------------+

Point
NetPacket::GetShipVelocity()
{
    if (msg && msg->Length() >= SHIP_LOC_SIZE) {
        short* data = (short*) (msg->Data()+20);

        short  dx   = *(data + 0);
        short  dy   = *(data + 1);
        short  dz   = *(data + 2);

        return Point(dx, dy, dz);
    }

    return Point();
}

void
NetPacket::SetShipVelocity(const Point& vel)
{
    if (msg && msg->Length() >= SHIP_LOC_SIZE) {
        short* data = (short*) (msg->Data()+20);

        *(data + 0) = (short) vel.x;
        *(data + 1) = (short) vel.y;
        *(data + 2) = (short) vel.z;
    }
}

// +--------------------------------------------------------------------+

Point
NetPacket::GetShipOrientation()
{
    if (msg && msg->Length() >= SHIP_LOC_SIZE) {
        short* data = (short*) (msg->Data()+26);

        short  r    = *(data + 0);
        short  p    = *(data + 1);
        short  y    = *(data + 2);

        return Point(2*PI*r/32767, 2*PI*p/32767, 2*PI*y/32767);
    }

    return Point();
}

void
NetPacket::SetShipOrientation(const Point& rpy)
{
    if (msg && msg->Length() >= SHIP_LOC_SIZE) {
        short* data = (short*) (msg->Data()+26);

        *(data + 0) = (short) (32767*rpy.x/(2*PI));
        *(data + 1) = (short) (32767*rpy.y/(2*PI));
        *(data + 2) = (short) (32767*rpy.z/(2*PI));
    }
}

// +--------------------------------------------------------------------+

double
NetPacket::GetThrottle()
{
    if (msg && msg->Length() >= SHIP_LOC_SIZE) {
        BYTE* data = (BYTE*) msg->Data()+32;

        return (double) *data;
    }

    return 0;
}

void
NetPacket::SetThrottle(double t)
{
    if (msg && msg->Length() >= SHIP_LOC_SIZE) {
        BYTE* data = (BYTE*) msg->Data()+32;

        *data = (BYTE) t;
    }
}

// +--------------------------------------------------------------------+

bool
NetPacket::GetTrigger(int i)
{
    if (i >= 0 && i < 8 && msg && msg->Length() >= SHIP_LOC_SIZE) {
        BYTE* data = (BYTE*) msg->Data()+33;

        BYTE select = 1 << i;
        return (*data & select)?true:false;
    }

    return false;
}

void
NetPacket::SetTrigger(int i, bool trigger)
{
    if (i >= 0 && i < 8 && msg && msg->Length() >= SHIP_LOC_SIZE) {
        BYTE* data = (BYTE*) msg->Data()+33;

        BYTE select = 1 << i;

        if (trigger)
        *data = *data | select;
        else
        *data = *data & ~select;
    }
}

// +--------------------------------------------------------------------+

const char*
NetPacket::GetName()
{
    if (msg && msg->Length() >= JOIN_REQ_SIZE) {
        BYTE* data = (BYTE*) msg->Data()+20;

        return (const char*) data;
    }

    return 0;
}

void
NetPacket::SetName(const char* name)
{
    if (msg && msg->Length() >= JOIN_REQ_SIZE) {
        BYTE* data = (BYTE*) msg->Data()+20;
        strncpy((char*) data, name, 32);
    }
}

// +--------------------------------------------------------------------+

const char*
NetPacket::GetDesign()
{
    if (msg && msg->Length() >= JOIN_REQ_SIZE) {
        BYTE* data = (BYTE*) msg->Data()+52;

        return (const char*) data;
    }

    return 0;
}

void
NetPacket::SetDesign(const char* design)
{
    if (msg && msg->Length() >= JOIN_REQ_SIZE) {
        BYTE* data = (BYTE*) msg->Data()+52;
        strncpy((char*) data, design, 32);
    }
}

// +--------------------------------------------------------------------+

const char*
NetPacket::GetRegion()
{
    if (msg && msg->Length() >= JOIN_REQ_SIZE) {
        BYTE* data = (BYTE*) msg->Data()+84;

        return (const char*) data;
    }

    return 0;
}

void
NetPacket::SetRegion(const char* rgn_name)
{
    if (msg && msg->Length() >= JOIN_REQ_SIZE) {
        BYTE* data = (BYTE*) msg->Data()+84;
        strncpy((char*) data, rgn_name, 32);
    }
}

// +--------------------------------------------------------------------+

