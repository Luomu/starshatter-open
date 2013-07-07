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
    FILE:         Selection.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Classes for rendering solid meshes of polygons
*/

#ifndef Selection_h
#define Selection_h

#include <vector>
#include "Polygon.h"
#include "Graphic.h"
#include "Video.h"
#include "List.h"

// +--------------------------------------------------------------------+

class Selection;
class Solid;
class Model;
class ModelView;
class Surface;
class Segment;

// +--------------------------------------------------------------------+

class Selection : public Graphic
{
public:
   static const char* TYPENAME() { return "Selection"; }

   Selection();
   virtual ~Selection();

   // operations
   virtual void   Render(Video* video, DWORD flags);
   virtual bool   CheckVisibility(Projector& projector) { return true; }

   // accessors / mutators
   void           UseModel(Model* m)   { model = m;      }
   void           UseView(ModelView* v){ model_view = v; }
   Model*         GetModel()     const { return model;   }
   List<Poly>&    GetPolys()           { return polys;   }
   std::vector<DWORD>&     GetVerts()  { return verts;   }

   virtual void   Clear()              { polys.clear(); 
                                         verts.clear();  }

   void           AddPoly(Poly* p);
   void           AddVert(WORD s, WORD v);
   void           RemovePoly(Poly* p);
   void           RemoveVert(WORD s, WORD v);
   bool           Contains(Poly* p)          const;
   bool           Contains(WORD s, WORD v)   const;

protected:
   Model*         model;
   ModelView*     model_view;
   List<Poly>     polys;
   std::vector<DWORD> verts;
};

// +--------------------------------------------------------------------+

#endif Selection_h

