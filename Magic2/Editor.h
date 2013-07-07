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

    SUBSYSTEM:    Magic.exe
    FILE:         Editor.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Classes for rendering solid meshes of polygons
*/

#ifndef Editor_h
#define Editor_h

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

class Editor
{
public:
   Editor(MagicDoc* doc) : document(doc), model(0) { }

   // accessors / mutators
   void     UseModel(Model* m)   { model = m;      }
   Model*   GetModel()     const { return model;   }

   // operations

   enum { MAP_NONE, MAP_PLANAR, MAP_CYLINDRICAL, MAP_SPHERICAL, MAP_STRETCH };

   void     ApplyMaterial(Material* material, List<Poly>& polys,
                    int mapping, int axis, float scale_u, float scale_v,
                    int flip, int mirror, int rotate);

   void     ApplyMaterialCylindrical(Material* material, List<Poly>& polys,
                    int axis, float scale_u, float scale_v,
                    int flip, int mirror, int rotate);

   void     ApplyMaterialSpherical(Material* material, List<Poly>& polys,
                    int axis, float scale_u, float scale_v,
                    int flip, int mirror, int rotate);


   void     Resegment();

protected:
   MagicDoc*   document;
   Model*      model;
};

// +--------------------------------------------------------------------+

class EditCommand : public Command
{
public:
   EditCommand(const char* name, MagicDoc* doc);
   virtual ~EditCommand();

   virtual void   Do();
   virtual void   Undo();

private:
   Model*   model1;
   Model*   model2;
};

// +--------------------------------------------------------------------+

#endif Editor_h

