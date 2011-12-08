/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         DropShipAI.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Drop Ship (orbit/surface and surface/orbit) AI class
*/

#ifndef DropShipAI_h
#define DropShipAI_h

#include "Types.h"
#include "Geometry.h"
#include "System.h"
#include "ShipAI.h"

// +--------------------------------------------------------------------+

class Ship;

// +--------------------------------------------------------------------+

class DropShipAI : public ShipAI
{
public:
   DropShipAI(Ship* s);
   virtual ~DropShipAI();

   enum { DIR_TYPE = 2001 };
   virtual int       Type() const { return DIR_TYPE; }

protected:
   // accumulate behaviors:
   virtual void      Navigator();
   virtual void      FindObjective();
};

// +--------------------------------------------------------------------+

#endif DropShipAI_h

