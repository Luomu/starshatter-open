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
