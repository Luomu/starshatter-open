/*  Project Magic 2.0
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Magic.exe
    FILE:         ModelFileMAG.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    File loader for MAG format models
*/

#ifndef ModelFileMAG_h
#define ModelFileMAG_h

#include "Solid.h"

// +--------------------------------------------------------------------+

class ModelFileMAG : public ModelFile
{
public:
   ModelFileMAG(const char* fname);
   virtual ~ModelFileMAG();

   virtual bool   Load(Model* m, double scale=1.0);
   virtual bool   Save(Model* m);

protected:
   virtual bool   LoadMag5(FILE* fp, Model* m, double scale);
   virtual bool   LoadMag6(FILE* fp, Model* m, double scale);
};

// +--------------------------------------------------------------------+

#endif ModelFileMAG_h

