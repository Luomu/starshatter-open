/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

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

