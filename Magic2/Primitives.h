/*  Project Magic 2.0
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Magic.exe
    FILE:         Primitives.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Commands for adding basic geometric primitives to a mesh
*/

#ifndef Primitives_h
#define Primitives_h

#include "MagicDoc.h"
#include "Command.h"
#include "Polygon.h"
#include "Solid.h"
#include "Video.h"
#include "List.h"

// +--------------------------------------------------------------------+

class Selection;
class ModelView;

// +--------------------------------------------------------------------+

class CreatePolyCommand : public Command
{
public:
   CreatePolyCommand(MagicDoc*   doc, 
                     int         nsides,
                     double      lx,
                     double      ly,
                     double      lz);
   virtual ~CreatePolyCommand();

   virtual void   Do();
   virtual void   Undo();

private:
   int      nsides;
   double   lx;
   double   ly;
   double   lz;
};

// +--------------------------------------------------------------------+

#endif Primitives_h

