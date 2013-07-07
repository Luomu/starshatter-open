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
