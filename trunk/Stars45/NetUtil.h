/*  Project STARSHATTER
    John DiCamillo
    Copyright © 1997-2002. All Rights Reserved.

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