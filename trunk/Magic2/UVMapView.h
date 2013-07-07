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
    FILE:         UVMapView.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Interface of the UVMapView class
*/


#ifndef UVMapView_h
#define UVMapView_h

#include <vector>
#include "View.h"
#include "Polygon.h"
#include "List.h"

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

   std::vector<DWORD>   selverts;
};

// +--------------------------------------------------------------------+

#endif UVMapView_h
