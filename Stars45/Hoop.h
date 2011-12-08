/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         Hoop.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    ILS Hoop (HUD display) class
*/

#ifndef Hoop_h
#define Hoop_h

#include "Types.h"
#include "Solid.h"
#include "Geometry.h"

// +--------------------------------------------------------------------+

class Hoop : public Solid
{
public:
   Hoop();
   virtual ~Hoop();

   virtual void      Update();
   static  void      SetColor(Color c);

protected:
   virtual void      CreatePolys();

   Bitmap*           hoop_texture;
   Material*         mtl;
   int               width;
   int               height;
};

// +--------------------------------------------------------------------+

#endif Hoop_h

