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
