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
    FILE:         GameScreen.h
    AUTHOR:       John DiCamillo

*/

#ifndef GameScreen_h
#define GameScreen_h

#include "Types.h"
#include "Bitmap.h"
#include "Screen.h"
#include "BaseScreen.h"

// +--------------------------------------------------------------------+

class Screen;
class Sim;
class Window;
class Font;

class NavDlg;
class EngDlg;
class FltDlg;
class CtlDlg;
class JoyDlg;
class KeyDlg;
class ModDlg;
class ModInfoDlg;

class CameraDirector;
class DisplayView;
class HUDView;
class WepView;
class QuantumView;
class QuitView;
class RadioView;
class TacticalView;
class CameraView;
class PolyRender;
class Bitmap;
class DataLoader;
class Video;
class VideoFactory;

// +--------------------------------------------------------------------+

class GameScreen : public BaseScreen
{
public:
    GameScreen();
    virtual ~GameScreen();

    virtual void         Setup(Screen* screen);
    virtual void         TearDown();
    virtual bool         CloseTopmost();

    virtual bool         IsShown()         const { return isShown; }
    virtual void         Show();
    virtual void         Hide();

    virtual bool         IsFormShown()     const;
    virtual void         ShowExternal();

    virtual void         ShowNavDlg();
    virtual void         HideNavDlg();
    virtual bool         IsNavShown();
    virtual NavDlg*      GetNavDlg()             { return navdlg; }

    virtual void         ShowEngDlg();
    virtual void         HideEngDlg();
    virtual bool         IsEngShown();
    virtual EngDlg*      GetEngDlg()             { return engdlg; }

    virtual void         ShowFltDlg();
    virtual void         HideFltDlg();
    virtual bool         IsFltShown();
    virtual FltDlg*      GetFltDlg()             { return fltdlg; }

    virtual void         ShowCtlDlg();
    virtual void         HideCtlDlg();
    virtual bool         IsCtlShown();

    virtual void         ShowJoyDlg();
    virtual bool         IsJoyShown();

    virtual void         ShowKeyDlg();
    virtual bool         IsKeyShown();


    virtual AudDlg*      GetAudDlg() const { return auddlg; }
    virtual VidDlg*      GetVidDlg() const { return viddlg; }
    virtual OptDlg*      GetOptDlg() const { return optdlg; }
    virtual CtlDlg*      GetCtlDlg() const { return ctldlg; }
    virtual JoyDlg*      GetJoyDlg() const { return joydlg; }
    virtual KeyDlg*      GetKeyDlg() const { return keydlg; }
    virtual ModDlg*      GetModDlg() const { return moddlg; }
    virtual ModInfoDlg*  GetModInfoDlg() const { return modInfoDlg; }

    virtual void         ShowAudDlg();
    virtual void         HideAudDlg();
    virtual bool         IsAudShown();

    virtual void         ShowVidDlg();
    virtual void         HideVidDlg();
    virtual bool         IsVidShown();

    virtual void         ShowOptDlg();
    virtual void         HideOptDlg();
    virtual bool         IsOptShown();

    virtual void         ShowModDlg();
    virtual void         HideModDlg();
    virtual bool         IsModShown();

    virtual void         ShowModInfoDlg();
    virtual void         HideModInfoDlg();
    virtual bool         IsModInfoShown();

    virtual void         ApplyOptions();
    virtual void         CancelOptions();

    virtual void         ShowWeaponsOverlay();
    virtual void         HideWeaponsOverlay();

    void                 SetFieldOfView(double fov);
    double               GetFieldOfView() const;
    void                 CycleMFDMode(int mfd);
    void                 CycleHUDMode();
    void                 CycleHUDColor();
    void                 CycleHUDWarn();
    void                 FrameRate(double f);
    void                 ExecFrame();

    static GameScreen*   GetInstance()           { return game_screen; }
    CameraView*          GetCameraView()   const { return cam_view; }
    Bitmap*              GetLensFlare(int index);

private:
    void                 HideAll();

    Sim*                 sim;
    Screen*              screen;

    Window*              gamewin;
    NavDlg*              navdlg;
    EngDlg*              engdlg;
    FltDlg*              fltdlg;
    CtlDlg*              ctldlg;
    KeyDlg*              keydlg;
    JoyDlg*              joydlg;
    AudDlg*              auddlg;
    VidDlg*              viddlg;
    OptDlg*              optdlg;
    ModDlg*              moddlg;
    ModInfoDlg*          modInfoDlg;

    Font*                HUDfont;
    Font*                GUIfont;
    Font*                GUI_small_font;
    Font*                title_font;

    Bitmap*              flare1;
    Bitmap*              flare2;
    Bitmap*              flare3;
    Bitmap*              flare4;

    CameraDirector*      cam_dir;
    DisplayView*         disp_view;
    HUDView*             hud_view;
    WepView*             wep_view;
    QuantumView*         quantum_view;
    QuitView*            quit_view;
    TacticalView*        tac_view;
    RadioView*           radio_view;
    CameraView*          cam_view;
    DataLoader*          loader;

    double               frame_rate;
    bool                 isShown;

    static GameScreen*   game_screen;
};

// +--------------------------------------------------------------------+

#endif GameScreen_h

