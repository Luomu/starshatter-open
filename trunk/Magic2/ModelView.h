/*  Project Magic 2.0
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Magic.exe
    FILE:         ModelView.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Interface of the ModelView class
*/


#ifndef ModelView_h
#define ModelView_h

#include "CameraView.h"
#include "Grid.h"

// +--------------------------------------------------------------------+

class ModelView : public CameraView
{
public:
   enum VIEW_MODE { VIEW_PLAN=1, VIEW_FRONT, VIEW_SIDE, VIEW_PROJECT };
   enum FILL_MODE { FILL_WIRE=1, FILL_SOLID, FILL_TEXTURE };

   ModelView(Window* c, Scene* s, DWORD m);
   virtual ~ModelView();

   virtual void RenderScene();
   virtual void Render(Graphic* g, DWORD flags);

   DWORD       GetViewMode()        const { return view_mode;  }
   void        SetViewMode(DWORD m);
   DWORD       GetFillMode()        const { return fill_mode;  }
   void        SetFillMode(DWORD m)       { fill_mode = m;     }

   void        MoveTo(Point origin);
   void        MoveBy(double dx,  double dy);
   void        SpinBy(double phi, double theta);

   void        UseGrid(Grid* g);
   void        RenderGrid();
   void        ZoomNormal();

   CPoint      ProjectPoint(Vec3& p);

   static ModelView* FindView(DWORD mode);

protected:
   Camera      cam;
   DWORD       view_mode;
   DWORD       fill_mode;
   double      az;
   double      el;
   Grid*       grid;
};

// +--------------------------------------------------------------------+

#endif ModelView_h
