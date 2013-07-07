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
    FILE:         HUDView.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    View class for Heads Up Display
*/

#ifndef HUDView_h
#define HUDView_h

#include "Types.h"
#include "View.h"
#include "Bitmap.h"
#include "Font.h"
#include "System.h"
#include "SimObject.h"
#include "Text.h"

// +--------------------------------------------------------------------+

class Graphic;
class Sprite;
class Solid;
class Ship;
class Contact;
class Physical;
class OrbitalBody;
class OrbitalRegion;
class Instruction;
class CameraView;
class Projector;
class MFD;

// +--------------------------------------------------------------------+

class HUDView : public View,
public SimObserver
{
public:
    HUDView(Window* c);
    virtual ~HUDView();

    enum HUDModes { HUD_MODE_OFF, HUD_MODE_TAC, HUD_MODE_NAV, HUD_MODE_ILS };

    // Operations:
    virtual void      Refresh();
    virtual void      OnWindowMove();
    virtual void      ExecFrame();
    virtual void      UseCameraView(CameraView* v);

    virtual Ship*     GetShip()      const { return ship;   }
    virtual SimObject* GetTarget()   const { return target; }
    virtual void      SetShip(Ship* s);
    virtual void      SetTarget(SimObject* t);
    virtual MFD*      GetMFD(int n)  const;

    virtual void      HideAll();
    virtual void      DrawBars();
    virtual void      DrawNav();
    virtual void      DrawILS();
    virtual void      DrawObjective();
    virtual void      DrawNavInfo();
    virtual void      DrawNavPoint(Instruction& navpt, int index, int next);
    virtual void      DrawContactMarkers();
    virtual void      DrawContact(Contact* c, int index);
    virtual void      DrawTrack(Contact* c);
    virtual void      DrawTrackSegment(Point& t1, Point& t2, Color c);
    virtual void      DrawRect(SimObject* targ);
    virtual void      DrawTarget();
    virtual void      DrawSight();
    virtual void      DrawLCOS(SimObject* targ, double dist);
    virtual void      DrawDesignators();
    virtual void      DrawFPM();
    virtual void      DrawHPM();
    virtual void      DrawCompass();
    virtual void      HideCompass();
    virtual void      DrawPitchLadder();
    virtual void      DrawStarSystem();

    virtual void      DrawMFDs();
    virtual void      DrawWarningPanel();
    virtual void      DrawInstructions();
    virtual void      DrawMessages();

    virtual void      MouseFrame();

    virtual int       GetHUDMode()      const { return mode;     }
    virtual int       GetTacticalMode() const { return tactical; }
    virtual void      SetTacticalMode(int mode=1);
    virtual int       GetOverlayMode()  const { return overlay;  }
    virtual void      SetOverlayMode(int mode=1);

    virtual void      SetHUDMode(int mode);
    virtual void      CycleHUDMode();
    virtual Color     CycleHUDColor();
    virtual void      SetHUDColorSet(int c);
    virtual int       GetHUDColorSet()  const { return color;     }
    virtual Color     GetHUDColor()     const { return hud_color; }
    virtual Color     GetTextColor()    const { return txt_color; }
    virtual Color     Ambient()         const;
    virtual void      ShowHUDWarn();
    virtual void      ShowHUDInst();
    virtual void      HideHUDWarn();
    virtual void      HideHUDInst();
    virtual void      CycleHUDWarn();
    virtual void      CycleHUDInst();
    virtual void      CycleMFDMode(int mfd);
    virtual void      CycleInstructions(int direction);
    virtual void      RestoreHUD();

    virtual void      TargetOff() { target = 0; }
    static  Color     MarkerColor(Contact* targ);

    static bool       IsNameCrowded(int x, int y);
    static bool       IsMouseLatched();
    static HUDView*   GetInstance() { return hud_view; }
    static void       Message(const char* fmt, ...);
    static void       ClearMessages();
    static void       PrepareBitmap(const char* name, Bitmap& img, BYTE*& shades);
    static void       TransferBitmap(const Bitmap& src, Bitmap& img, BYTE*& shades);
    static void       ColorizeBitmap(Bitmap& img, BYTE* shades, Color color, bool force_alpha=false);

    static int        GetGunsight()              { return gunsight;      }
    static void       SetGunsight(int s)         { gunsight = s;         }
    static bool       IsArcade()                 { return arcade;        }
    static void       SetArcade(bool a)          { arcade = a;           }
    static int        DefaultColorSet()          { return def_color_set; }
    static void       SetDefaultColorSet(int c)  { def_color_set = c;    }
    static Color      GetStatusColor(System::STATUS status);
    static bool       ShowFPS()                  { return show_fps;      }
    static void       ShowFPS(bool f)            { show_fps = f;         }

    virtual bool         Update(SimObject* obj);
    virtual const char*  GetObserverName() const;

protected:
    const char*       FormatInstruction(Text instr);
    void              SetStatusColor(System::STATUS status);

    enum HUD_CASE { HUD_MIXED_CASE, HUD_UPPER_CASE };

    void              DrawDiamond(int x, int y, int r, Color c);
    void              DrawHUDText(int index, const char* txt, Rect& rect, int align, int upcase=HUD_UPPER_CASE, bool box=false);
    void              HideHUDText(int index);

    void              DrawOrbitalBody(OrbitalBody* body);

    Projector*  projector;
    CameraView* camview;

    int         width, height, aw, ah;
    double      xcenter, ycenter;

    Sim*        sim;
    Ship*       ship;
    SimObject*  target;

    SimRegion*  active_region;

    Bitmap*     cockpit_hud_texture;

    Color       hud_color;
    Color       txt_color;
    Color       status_color;

    bool        show_warn;
    bool        show_inst;
    int         inst_page;
    int         threat;

    int         mode;
    int         color;
    int         tactical;
    int         overlay;
    int         transition;
    int         docking;

    MFD*        mfd[3];

    Sprite*     pitch_ladder[31];
    Sprite*     hud_sprite[32];

    Solid*      az_ring;
    Solid*      az_pointer;
    Solid*      el_ring;
    Solid*      el_pointer;
    double      compass_scale;

    enum { MAX_MSG = 6 };
    Text        msg_text[MAX_MSG];
    double      msg_time[MAX_MSG];

    static HUDView*   hud_view;
    static bool       arcade;
    static bool       show_fps;
    static int        gunsight;
    static int        def_color_set;
};


// +--------------------------------------------------------------------+

struct HUDText {
    Font*    font;
    Color    color;
    Rect     rect;
    bool     hidden;
};

#endif HUDView_h

