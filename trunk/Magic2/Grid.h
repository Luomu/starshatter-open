/*  Project Magic 2.0
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Magic.exe
    FILE:         Grid.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Interface of the Grid class
*/


#ifndef Grid_h
#define Grid_h

#include "Bitmap.h"
#include "Graphic.h"

// +--------------------------------------------------------------------+

class Grid : public Graphic
{
public:
   enum PLANE { GRID_XY, GRID_XZ, GRID_YZ };

   Grid();
   virtual ~Grid();

   CPoint& Snap(CPoint& p);

   bool IsSnap()                    const { return snap;       }
   bool IsShow()                    const { return show;       }
   void SetSnap(bool s)                   { snap = s;          }
   void SetShow(bool s)                   { show = s;          }
   void SetSize(int x, int y = 0);
   void SetSkip(int x, int y = 0);
   void ShowMinor(bool show)              { minor = show;      }
   void ShowPlane(int p)                  { plane = p;         }
   void ShowReference(bool show)          { show_ref = show;   }

   const char* GetReferencePlan()   const;
   void        SetReferencePlan(const char* fname);
   const char* GetReferenceFront()  const;
   void        SetReferenceFront(const char* fname);
   const char* GetReferenceSide()   const;
   void        SetReferenceSide(const char* fname);
   
   int  GetSize()                   const { return x_size;     }

   // operations
   virtual void      Render(Video* video, DWORD flags);
   virtual void      RenderReference(Video* video);

protected:
   int snapto(int i, int dim);

   bool     show;
   bool     show_ref;
   bool     snap;
   bool     minor;

   int      x_size, y_size;
   int      x_skip, y_skip;
   int      plane;

   Vec3*    x_major;
   Vec3*    x_minor;
   Vec3*    y_major;
   Vec3*    y_minor;

   Bitmap*  bmp_plan;
   Bitmap*  bmp_front;
   Bitmap*  bmp_side;
};

// +--------------------------------------------------------------------+

#endif Grid_h
