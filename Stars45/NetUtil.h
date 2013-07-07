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
    FILE:         NetUtil.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Utility class to simplify sending NetData messages.
*/


#ifndef NetUtil_h
#define NetUtil_h

#include "Types.h"
#include "Geometry.h"

// +-------------------------------------------------------------------+

class Element;
class Instruction;
class SimObject;
class Ship;
class Shot;
class System;
class Weapon;

// +-------------------------------------------------------------------+

class NetUtil
{
public:
    static void    SendObjDamage(SimObject* obj, double damage, Shot* shot=0);
    static void    SendObjKill(Ship* obj, const Ship* killer, int type=0, int deck=0);
    static void    SendObjHyper(Ship* obj, const char* rgn, const Point& loc, const Ship* fc1=0, const Ship* fc2=0, int ttype=0);
    static void    SendObjTarget(Ship* obj);
    static void    SendObjEmcon(Ship* obj);
    static void    SendSysDamage(Ship* obj, System* sys, double damage);
    static void    SendSysStatus(Ship* obj, System* sys);
    static void    SendWepTrigger(Weapon* wep, int count=1);
    static void    SendWepRelease(Weapon* wep, Shot* shot);
    static void    SendWepDestroy(Shot* shot);
    static void    SendChat(DWORD dst, const char* name, const char* text);
    static void    SendElemRequest(const char* name);
    static void    SendElemCreate(Element* elem, int squadron, int* slots, bool alert, bool in_flight=false);
    static void    SendShipLaunch(Ship* carrier, int squadron, int slot);
    static void    SendNavData(bool add, Element* elem, int index, Instruction* navpt);
    static void    SendNavDelete(Element* elem, int index);
    static void    SendSelfDestruct(Ship* ship, double damage);
};

#endif NetUtil_h