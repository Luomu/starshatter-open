/*  Project Magic 2.0
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Magic.exe
    FILE:         UVMapView.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Interface of the UVMapView class
*/


#ifndef UVMapView_h
#define UVMapView_h

#include "View.h"
#include "Polygon.h"
#include "List.h"
#include "ArrayList.h"

// +--------------------------------------------------------------------+

class Video;

// +--------------------------------------------------------------------+

class UVMapView : public View
{
public:
   UVMapView(Window* c);
   virtual ~UVMapView();

   enum SELECT_MODE { SELECT_REMOVE=-1, SELECT_REPLACE=0, SELECT_APPEND=1 };

   virtual void Refresh();

   void        UseMaterial(Material* m);
   void        UsePolys(List<Poly>& p);

   void        MoveBy(double dx, double dy);
   void        DragBy(double dx, double dy);
   void        ZoomIn()    { zoom *= 1.15; }
   void        ZoomOut()   { zoom *= 0.85; }

   void        Clear();
   void        Begin(int select_mode = SELECT_REPLACE);
   void        AddMark(CPoint& p);
   void        End();

   bool        IsActive() const { return active; }
   void        SelectAll();
   void        SelectNone();
   void        SelectInverse();

   bool        IsSelected(Poly* p, WORD v);
   bool        WillSelect(CPoint& p);

protected:
   enum { MAX_MARK = 4096 };

   Material*   material;
   List<Poly>  polys;
   Video*      video;

   double      zoom;
   double      x_offset;
   double      y_offset;

   int         nmarks;
   CPoint      marks[MAX_MARK];
   int         select_mode;
   bool        active;

   ArrayList   selverts;
};

// +--------------------------------------------------------------------+

#endif UVMapView_h
