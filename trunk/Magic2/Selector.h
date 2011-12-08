/*  Project Magic 2.0
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Magic.exe
    FILE:         Selector.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Class definition for Selector (free-form selection tool)
*/

#ifndef Selector_h
#define Selector_h

#include "Polygon.h"
#include "Graphic.h"
#include "Video.h"

class  ModelView;
class  Model;
class  Selection;
class  Surface;

// +----------------------------------------------------------------------+

class Selector : public Graphic
{
public:
   Selector(Selection* s=0);
   virtual ~Selector();

   enum SELECT_MODE { SELECT_REMOVE=-1, SELECT_REPLACE=0, SELECT_APPEND=1 };

   // Operations
   virtual void   Render(Video* video, DWORD flags);
   virtual bool   CheckVisibility(Projector& projector) { return true; }

   void           Clear();
   void           Begin(Model* m, int mode, int select_mode = SELECT_REPLACE);
   void           AddMark(CPoint& p);
   void           End();

   bool           IsActive()     const { return view_mode ? true : false; }
   int            GetViewMode()  const { return view_mode; }
   Selection*     GetSelection() const { return selection; }

   void           UseModel(Model* m);
   void           SelectAll(int select_mode = SELECT_REPLACE);
   void           SelectInverse();
   void           SelectSurface(Surface* s, int select_mode = SELECT_REPLACE);
   void           SelectVert(Surface* s, int   v, int select_mode = SELECT_REPLACE);
   void           SelectPoly(Poly* p, int select_mode = SELECT_REPLACE);
   void           SelectMaterial(Material* m, int select_mode = SELECT_REPLACE);

   void           Reselect();

protected:
   enum { MAX_MARK = 4096 };

   int            view_mode;
   int            nmarks;
   CPoint         marks[MAX_MARK];
   int            select_mode;

   bool           own_selection;
   Selection*     selection;
   Model*         model;
};


// +----------------------------------------------------------------------+

#endif Selector_h
