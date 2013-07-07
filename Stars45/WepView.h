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

    SUBSYSTEM:    Stars.exe
    FILE:         WepView.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    View class for Tactical HUD Overlay
*/

#ifndef WepView_h
#define WepView_h

#include "Types.h"
#include "View.h"
#include "Projector.h"
#include "Bitmap.h"
#include "Font.h"
#include "System.h"
#include "SimObject.h"

// +--------------------------------------------------------------------+

class Graphic;
class Sprite;
class Ship;
class Contact;
class HUDView;

// +--------------------------------------------------------------------+

class WepView : public View,
public SimObserver
{
public:
    WepView(Window* c);
    virtual ~WepView();

    // Operations:
    virtual void      Refresh();
    virtual void      OnWindowMove();
    virtual void      ExecFrame();
    virtual void      SetOverlayMode(int mode);
    virtual int       GetOverlayMode()     const { return mode; }
    virtual void      CycleOverlayMode();

    virtual void      RestoreOverlay();

    virtual bool         Update(SimObject* obj);
    virtual const char*  GetObserverName() const;

    static WepView*   GetInstance() { return wep_view; }
    static void       SetColor(Color c);

    static bool       IsMouseLatched();

protected:
    void              DrawOverlay();

    void              DoMouseFrame();
    bool              CheckButton(int index, int x, int y);
    void              CycleSubTarget(int direction);

    int         mode;
    int         transition;
    int         mouse_down;
    int         width, height, aw, ah;
    double      xcenter, ycenter;

    Sim*        sim;
    Ship*       ship;
    SimObject*  target;
    HUDView*    hud;

    enum { MAX_WEP = 4, MAX_BTN = 16 };
    Rect        btn_rect[MAX_BTN];

    SimRegion*  active_region;

    static WepView*   wep_view;
};

#endif WepView_h

