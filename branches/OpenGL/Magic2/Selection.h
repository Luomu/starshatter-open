/*  Project Magic 2.0
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

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

