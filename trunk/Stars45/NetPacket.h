/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

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

