/*  Project nGen
    John DiCamillo
    Copyright © 1997-2002. All Rights Reserved.

    SUBSYSTEM:    nGen.lib
    FILE:         Director.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Abstract Director (AI or Human Input) for Physical Objects
*/

#ifndef Director_h
#define Director_h

#include "Types.h"
#include "Geometry.h"

// +--------------------------------------------------------------------+

class Physical;

// +--------------------------------------------------------------------+

class Director
{
public:
   Director()           { } 
   virtual ~Director()  { }

   // accessors:
   virtual int       Type()      const { return 0; }
   virtual int       Subframe()  const { return 0; }

   // operations
   virtual void      ExecFrame(double factor) {    }
};

// +--------------------------------------------------------------------+

#endif Director_h

