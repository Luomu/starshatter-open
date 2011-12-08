/*  Project Magic 2.0
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Magic.exe
    FILE:         ModelFile3DS.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    File loader for 3DStudio MAX 3DS format models
*/

#ifndef ModelFile3DS_h
#define ModelFile3DS_h

#include "Solid.h"

// +--------------------------------------------------------------------+

class ModelFile3DS : public ModelFile
{
public:
   ModelFile3DS(const char* fname);
   virtual ~ModelFile3DS();

   virtual bool   Load(Model* m, double scale=1.0);
   virtual bool   Save(Model* m);

protected:
};

// +--------------------------------------------------------------------+

#endif ModelFile3DS_h

