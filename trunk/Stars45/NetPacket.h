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
    FILE:         NetPacket.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Wrapper for low-level datagram class
*/

#ifndef NetPacket_h
#define NetPacket_h

#include "Types.h"
#include "Geometry.h"
#include "NetData.h"

// +--------------------------------------------------------------------+

class NetLink;
class NetMsg;

// +--------------------------------------------------------------------+

class NetPacket
{
public:
    static const char* TYPENAME() { return "NetPacket"; }

    NetPacket(NetMsg* g);
    NetPacket(DWORD netid, BYTE type);
    ~NetPacket();

    bool Send(NetLink& link);

    // various accessors:
    DWORD       NetID()  const;
    BYTE        Type()   const;

    DWORD       GetPingSequence();
    void        SetPingSequence(DWORD seq);
    DWORD       GetNetID();
    void        SetNetID(DWORD id);
    Point       GetShipLocation();
    void        SetShipLocation(const Point& loc);
    Point       GetShipVelocity();
    void        SetShipVelocity(const Point& vel);
    Point       GetShipOrientation();
    void        SetShipOrientation(const Point& rpy);
    double      GetThrottle();
    void        SetThrottle(double t);
    const char* GetName();
    void        SetName(const char* name);
    const char* GetDesign();
    void        SetDesign(const char* design);
    const char* GetRegion();
    void        SetRegion(const char* rgn_name);
    bool        GetTrigger(int i);
    void        SetTrigger(int i, bool trigger);


protected:
    NetMsg* msg;
};


// +--------------------------------------------------------------------+

#endif NetPacket_h

