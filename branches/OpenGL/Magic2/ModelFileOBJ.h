/*  Project Magic 2.0
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Magic.exe
    FILE:         ModelFileOBJ.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    File loader for Wavefront/OBJ format models
*/

#ifndef ModelFileOBJ_h
#define ModelFileOBJ_h

#include "Solid.h"

// +--------------------------------------------------------------------+

class ModelFileOBJ : public ModelFile
{
public:
   ModelFileOBJ(const char* fname);
   virtual ~ModelFileOBJ();

   virtual bool   Load(Model* m, double scale=1.0);
   virtual bool   Save(Model* m);

protected:
};

// +--------------------------------------------------------------------+

#endif ModelFileOBJ_h

