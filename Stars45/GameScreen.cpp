/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars
    FILE:         GameScreen.cpp
    AUTHOR:       John DiCamillo

*/

#include "MemDebug.h"
#include "GameScreen.h"

#include "Sim.h"
#include "Ship.h"
#include "DisplayView.h"
#include "HUDView.h"
#include "HUDSounds.h"
#include "WepView.h"
#include "QuantumView.h"
#include "QuitView.h"
#include "RadioView.h"
#include "TacticalView.h"
#include "NavDlg.h"
#include "EngDlg.h"
#include "FltDlg.h"
#include "AudDlg.h"
#include "VidDlg.h"
#include "OptDlg.h"
#include "CtlDlg.h"
#include "KeyDlg.h"
#include "JoyDlg.h"
#include "ModDlg.h"
#include "ModInfoDlg.h"
#include "FormDef.h"
#include "Starshatter.h"
#include "CameraDirector.h"

#include "Game.h"
#include "Video.h"
#include "Screen.h"
#include "Window.h"
#include "ActiveWindow.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "MouseController.h"
#include "CameraView.h"
#include "FadeView.h"
#include "Color.h"
#include "Bitmap.h"
#include "Font.h"
#include "FontMgr.h"
#include "EventDispatch.h"
#include "DataLoader.h"
#include "Resource.h"

// +--------------------------------------------------------------------+

GameScreen* GameScreen::game_screen    = 0;

static MouseController* mouse_con      = 0;
static bool             mouse_active   = false;

// +--------------------------------------------------------------------+

GameScreen::GameScreen()
   : screen(0), gamewin(0), 
     navdlg(0), wep_view(0), engdlg(0), fltdlg(0),
     HUDfont(0), GUIfont(0), GUI_small_font(0), title_font(0), cam_view(0),
     isShown(false), disp_view(0), hud_view(0), tac_view(0), radio_view(0),
     quantum_view(0), quit_view(0), ctldlg(0), joydlg(0), keydlg(0), auddlg(0),
     viddlg(0), moddlg(0), modInfoDlg(0), 
     flare1(0), flare2(0), flare3(0), flare4(0),
     optdlg(0), cam_dir(0), sim(0)
{
   cam_dir  = new(__FILE__,__LINE__) CameraDirector;
   sim      = Sim::GetSim();
   loader   = DataLoader::GetLoader();

   // find the fonts:
   HUDfont        = FontMgr::Find("HUD");
   GUIfont        = FontMgr::Find("GUI");
   GUI_small_font = FontMgr::Find("GUIsmall");
   title_font     = FontMgr::Find("Title");

   loader->LoadTexture("flare0+.pcx", flare1, Bitmap::BMP_TRANSLUCENT);
   loader->LoadTexture("flare2.pcx",  flare2, Bitmap::BMP_TRANSLUCENT);
   loader->LoadTexture("flare3.pcx",  flare3, Bitmap::BMP_TRANSLUCENT);
   loader->LoadTexture("flare4.pcx",  flare4, Bitmap::BMP_TRANSLUCENT);

   mouse_con      = MouseController::GetInstance();
   game_screen    = this;
}

GameScreen::~GameScreen()
{
   TearDown();
   game_screen = 0;
}

// +--------------------------------------------------------------------+

void
GameScreen::Setup(Screen* s)
{
   if (!s)
      return;

   screen = s;

   loader->UseFileSystem(true);

   // create windows
   gamewin = new(__FILE__,__LINE__) Window(screen, 0, 0, screen->Width(), screen->Height());
   
   // attach views to windows (back to front)
   // fade in:
   gamewin->AddView(new(__FILE__,__LINE__) FadeView(gamewin, 1, 0, 0));

   // camera:
   cam_dir  = CameraDirector::GetInstance();
   cam_view = new(__FILE__,__LINE__) CameraView(gamewin, cam_dir->GetCamera(), 0);

   if (cam_view)
      gamewin->AddView(cam_view);
   
   // HUD:
   hud_view = new(__FILE__,__LINE__) HUDView(gamewin);
   gamewin->AddView(hud_view);

   wep_view = new(__FILE__,__LINE__) WepView(gamewin);
   gamewin->AddView(wep_view);

   quantum_view = new(__FILE__,__LINE__) QuantumView(gamewin);
   gamewin->AddView(quantum_view);

   radio_view = new(__FILE__,__LINE__) RadioView(gamewin);
   gamewin->AddView(radio_view);

   tac_view = new(__FILE__,__LINE__) TacticalView(gamewin, this);
   gamewin->AddView(tac_view);

   disp_view = DisplayView::GetInstance();

   // note: quit view must be last in chain 
   // so it can release window surface
   quit_view = new(__FILE__,__LINE__) QuitView(gamewin);
   gamewin->AddView(quit_view);

   Starshatter* stars = Starshatter::GetInstance();

   // initialize lens flare bitmaps:
   if (stars->LensFlare()) {
      cam_view->LensFlareElements(flare1, flare4, flare2, flare3);
      cam_view->LensFlare(true);
   }

   // if lens flare disabled, just create the corona:
   else if (stars->Corona()) {
      cam_view->LensFlareElements(flare1, 0, 0, 0);
      cam_view->LensFlare(true);
   }

   screen->AddWindow(gamewin);

   FormDef aud_def("AudDlg", 0);
   aud_def.Load("AudDlg");
   auddlg = new(__FILE__,__LINE__) AudDlg(screen, aud_def, this);

   FormDef ctl_def("CtlDlg", 0);
   ctl_def.Load("CtlDlg");
   ctldlg = new(__FILE__,__LINE__) CtlDlg(screen, ctl_def, this);

   FormDef opt_def("OptDlg", 0);
   opt_def.Load("OptDlg");
   optdlg = new(__FILE__,__LINE__) OptDlg(screen, opt_def, this);

   FormDef vid_def("VidDlg", 0);
   vid_def.Load("VidDlg");
   viddlg = new(__FILE__,__LINE__) VidDlg(screen, vid_def, this);

   FormDef mod_def("ModDlg", 0);
   mod_def.Load("ModDlg");
   moddlg = new(__FILE__,__LINE__) ModDlg(screen, mod_def, this);

   FormDef joy_def("JoyDlg", 0);
   joy_def.Load("JoyDlg");
   joydlg = new(__FILE__,__LINE__) JoyDlg(screen, joy_def, this);

   FormDef key_def("KeyDlg", 0);
   key_def.Load("KeyDlg");
   keydlg = new(__FILE__,__LINE__) KeyDlg(screen, key_def, this);

   FormDef mod_info_def("ModInfoDlg", 0);
   mod_info_def.Load("ModInfoDlg");
   modInfoDlg = new(__FILE__,__LINE__) ModInfoDlg(screen, mod_info_def, this);

   FormDef nav_def("NavDlg", 0);
   nav_def.Load("NavDlg");
   navdlg = new(__FILE__,__LINE__) NavDlg(screen, nav_def, this);

   FormDef eng_def("EngDlg", 0);
   eng_def.Load("EngDlg");
   engdlg = new(__FILE__,__LINE__) EngDlg(screen, eng_def, this);

   FormDef flt_def("FltDlg", 0);
   flt_def.Load("FltDlg");
   fltdlg = new(__FILE__,__LINE__) FltDlg(screen, flt_def, this);

   if (engdlg)  engdlg->Hide();
   if (fltdlg)  fltdlg->Hide();
   if (navdlg)  navdlg->Hide();
   if (auddlg)  auddlg->Hide();
   if (viddlg)  viddlg->Hide();
   if (optdlg)  optdlg->Hide();
   if (ctldlg)  ctldlg->Hide();
   if (keydlg)  keydlg->Hide();
   if (joydlg)  joydlg->Hide();
   if (moddlg)  moddlg->Hide();
   if (modInfoDlg) modInfoDlg->Hide();

   screen->SetBackgroundColor(Color::Black);

   frame_rate = 0;

   loader->UseFileSystem(Starshatter::UseFileSystem());
}

// +--------------------------------------------------------------------+

void
GameScreen::TearDown()
{
   if (gamewin && disp_view)
      gamewin->DelView(disp_view);

   if (screen) {
      screen->DelWindow(engdlg);
      screen->DelWindow(fltdlg);
      screen->DelWindow(navdlg);
      screen->DelWindow(modInfoDlg);
      screen->DelWindow(auddlg);
      screen->DelWindow(viddlg);
      screen->DelWindow(optdlg);
      screen->DelWindow(moddlg);
      screen->DelWindow(ctldlg);
      screen->DelWindow(keydlg);
      screen->DelWindow(joydlg);
      screen->DelWindow(gamewin);
   }

   delete engdlg;
   delete fltdlg;
   delete navdlg;
   delete modInfoDlg;
   delete auddlg;
   delete viddlg;
   delete optdlg;
   delete moddlg;
   delete ctldlg;
   delete keydlg;
   delete joydlg;
   delete gamewin;
   delete cam_dir;

   engdlg      = 0;
   fltdlg      = 0;
   navdlg      = 0;
   modInfoDlg  = 0;
   auddlg      = 0;
   viddlg      = 0;
   optdlg      = 0;
   moddlg      = 0;
   ctldlg      = 0;
   keydlg      = 0;
   joydlg      = 0;
   gamewin     = 0;
   screen      = 0;
   cam_dir     = 0;
   cam_view    = 0;
   disp_view   = 0;
}

// +--------------------------------------------------------------------+

void
GameScreen::FrameRate(double f)
{
   frame_rate = f;
}

// +--------------------------------------------------------------------+

void
GameScreen::SetFieldOfView(double fov)
{
   cam_view->SetFieldOfView(fov);
}

// +--------------------------------------------------------------------+

double
GameScreen::GetFieldOfView() const
{
   return cam_view->GetFieldOfView();
}

// +--------------------------------------------------------------------+

Bitmap*
GameScreen::GetLensFlare(int index)
{
   switch (index) {
   case 0:  return flare1;
   case 1:  return flare2;
   case 2:  return flare3;
   case 3:  return flare4;
   }

   return 0;
}

// +--------------------------------------------------------------------+

void
GameScreen::ExecFrame()
{
   sim = Sim::GetSim();

   if (sim) {
      cam_view->UseCamera(CameraDirector::GetInstance()->GetCamera());
      cam_view->UseScene(sim->GetScene());

      Ship* player = sim->GetPlayerShip();

      if (player) {
         bool dialog_showing = false;

         if (hud_view) {
            hud_view->UseCameraView(cam_view);
            hud_view->ExecFrame();
         }

         if (quit_view && quit_view->IsMenuShown()) {
            quit_view->ExecFrame();
            dialog_showing = true;
         }

         if (navdlg && navdlg->IsShown()) {
            navdlg->SetShip(player);
            navdlg->ExecFrame();
            dialog_showing = true;
         }

         if (engdlg && engdlg->IsShown()) {
            engdlg->SetShip(player);
            engdlg->ExecFrame();
            dialog_showing = true;
         }

         if (fltdlg && fltdlg->IsShown()) {
            fltdlg->SetShip(player);
            fltdlg->ExecFrame();
            dialog_showing = true;
         }

         if (auddlg && auddlg->IsShown()) {
            auddlg->ExecFrame();
            dialog_showing = true;
         }

         if (viddlg && viddlg->IsShown()) {
            viddlg->ExecFrame();
            dialog_showing = true;
         }

         if (optdlg && optdlg->IsShown()) {
            optdlg->ExecFrame();
            dialog_showing = true;
         }

         if (ctldlg && ctldlg->IsShown()) {
            ctldlg->ExecFrame();
            dialog_showing = true;
         }

         if (keydlg && keydlg->IsShown()) {
            keydlg->ExecFrame();
            dialog_showing = true;
         }

         if (joydlg && joydlg->IsShown()) {
            joydlg->ExecFrame();
            dialog_showing = true;
         }

         if (moddlg && moddlg->IsShown()) {
            moddlg->ExecFrame();
            dialog_showing = true;
         }

         if (quantum_view && !dialog_showing) {
            quantum_view->ExecFrame();
         }

         if (radio_view && !dialog_showing) {
            radio_view->ExecFrame();
         }

         if (wep_view && !dialog_showing) {
            wep_view->ExecFrame();
         }

         if (tac_view && !dialog_showing) {
            if (cam_view)
               tac_view->UseProjector(cam_view->GetProjector());
            tac_view->ExecFrame();
         }
      }

      if (disp_view) {
         disp_view->ExecFrame();
      }
   }

   Starshatter* stars = Starshatter::GetInstance();

   if (stars) {
      if (stars->LensFlare()) {
         cam_view->LensFlareElements(flare1, flare4, flare2, flare3);
         cam_view->LensFlare(true);
      }

      else if (stars->Corona()) {
         cam_view->LensFlareElements(flare1, 0, 0, 0);
         cam_view->LensFlare(true);
      }

      else {
         cam_view->LensFlare(false);
      }
   }
}

// +--------------------------------------------------------------------+

void
GameScreen::CycleMFDMode(int mfd)
{
   if (hud_view)
      hud_view->CycleMFDMode(mfd);
}

void
GameScreen::CycleHUDMode()
{
   if (hud_view)
      hud_view->CycleHUDMode();
}

void
GameScreen::CycleHUDColor()
{
   if (hud_view)
      hud_view->CycleHUDColor();
}

void
GameScreen::CycleHUDWarn()
{
   if (hud_view)
      hud_view->CycleHUDWarn();
}

// +--------------------------------------------------------------------+

bool
GameScreen::CloseTopmost()
{
   bool processed = false;

   if (!gamewin) return processed;

   if (navdlg && navdlg->IsShown()) {
      HideNavDlg();
      processed = true;
   }

   else if (engdlg && engdlg->IsShown()) {
      HideEngDlg();
      processed = true;
   }

   else if (fltdlg && fltdlg->IsShown()) {
      HideFltDlg();
      processed = true;
   }

   else if (modInfoDlg && modInfoDlg->IsShown()) {
      HideModInfoDlg();
      processed = true;
   }

   else if (keydlg && keydlg->IsShown()) {
      ShowCtlDlg();
      processed = true;
   }

   else if (joydlg && joydlg->IsShown()) {
      ShowCtlDlg();
      processed = true;
   }

   else if (auddlg && auddlg->IsShown()) {
      CancelOptions();
      processed = true;
   }

   else if (viddlg && viddlg->IsShown()) {
      CancelOptions();
      processed = true;
   }

   else if (optdlg && optdlg->IsShown()) {
      CancelOptions();
      processed = true;
   }

   else if (moddlg && moddlg->IsShown()) {
      CancelOptions();
      processed = true;
   }

   else if (ctldlg && ctldlg->IsShown()) {
      CancelOptions();
      processed = true;
   }

   else if (quantum_view && quantum_view->IsMenuShown()) {
      quantum_view->CloseMenu();
      processed = true;
   }

   else if (quit_view && quit_view->IsMenuShown()) {
      quit_view->CloseMenu();
      processed = true;
   }

   else if (radio_view && radio_view->IsMenuShown()) {
      radio_view->CloseMenu();
      processed = true;
   }

   return processed;
}

static Window* old_disp_win = 0;

void
GameScreen::Show()
{
   if (!isShown) {
      screen->AddWindow(gamewin);
      isShown = true;

      if (disp_view) {
         old_disp_win = disp_view->GetWindow();

         disp_view->SetWindow(gamewin);
         gamewin->AddView(disp_view);
      }
   }
}

void
GameScreen::Hide()
{
   if (isShown) {
      HideAll();

      if (disp_view && gamewin) {
         gamewin->DelView(disp_view);
         disp_view->SetWindow(old_disp_win);
      }

      if (engdlg) engdlg->SetShip(0);
      if (fltdlg) fltdlg->SetShip(0);
      if (navdlg) navdlg->SetShip(0);

      HUDSounds::StopSound(HUDSounds::SND_RED_ALERT);

      screen->DelWindow(gamewin);
      isShown = false;
   }
}

// +--------------------------------------------------------------------+

bool
GameScreen::IsFormShown() const
{
   bool form_shown = false;

   if (navdlg && navdlg->IsShown())
      form_shown = true;

   else if (engdlg && engdlg->IsShown())
      form_shown = true;

   else if (fltdlg && fltdlg->IsShown())
      form_shown = true;

   else if (auddlg && auddlg->IsShown())
      form_shown = true;

   else if (viddlg && viddlg->IsShown())
      form_shown = true;

   else if (optdlg && optdlg->IsShown())
      form_shown = true;

   else if (moddlg && moddlg->IsShown())
      form_shown = true;

   else if (ctldlg && ctldlg->IsShown())
      form_shown = true;

   else if (keydlg && keydlg->IsShown())
      form_shown = true;

   else if (joydlg && joydlg->IsShown())
      form_shown = true;

   return form_shown;
}

// +--------------------------------------------------------------------+

void
GameScreen::ShowExternal()
{
   if (!gamewin) return;

   if ((navdlg && navdlg->IsShown()) || 
       (engdlg && engdlg->IsShown()) || 
       (fltdlg && fltdlg->IsShown()) ||
       (auddlg && auddlg->IsShown()) ||
       (viddlg && viddlg->IsShown()) ||
       (optdlg && optdlg->IsShown()) ||
       (moddlg && moddlg->IsShown()) ||
       (ctldlg && ctldlg->IsShown()) ||
       (keydlg && keydlg->IsShown()) ||
       (joydlg && joydlg->IsShown()))
      return;

   gamewin->MoveTo(Rect(0, 0, screen->Width(), screen->Height()));
   screen->AddWindow(gamewin);
}

// +--------------------------------------------------------------------+

void
GameScreen::ShowNavDlg()
{
   if (!gamewin) return;

   if (navdlg && !navdlg->IsShown()) {
      HideAll();

      navdlg->SetSystem(sim->GetStarSystem());
      navdlg->SetShip(sim->GetPlayerShip());
      navdlg->Show();

      if (mouse_con) {
         mouse_active = mouse_con->Active();
         mouse_con->SetActive(false);
      }

      Mouse::Show(true);
   }
   else {
      HideNavDlg();
   }
}

// +--------------------------------------------------------------------+

void
GameScreen::HideNavDlg()
{
   if (!gamewin) return;

   if (navdlg && navdlg->IsShown()) {
      navdlg->Hide();

      if (mouse_con)
         mouse_con->SetActive(mouse_active);

      Mouse::Show(false);
      screen->AddWindow(gamewin);
   }
}

// +--------------------------------------------------------------------+

bool
GameScreen::IsNavShown()
{
   return gamewin && navdlg && navdlg->IsShown();
}

// +--------------------------------------------------------------------+

void
GameScreen::ShowEngDlg()
{
   if (!gamewin) return;

   if (engdlg && !engdlg->IsShown()) {
      HideAll();

      engdlg->SetShip(sim->GetPlayerShip());
      engdlg->Show();

      if (mouse_con) {
         mouse_active = mouse_con->Active();
         mouse_con->SetActive(false);
      }

      Mouse::Show(true);
   }
   else {
      HideEngDlg();
   }
}

// +--------------------------------------------------------------------+

void
GameScreen::HideEngDlg()
{
   if (!gamewin) return;

   if (engdlg && engdlg->IsShown()) {
      engdlg->Hide();

      if (mouse_con)
         mouse_con->SetActive(mouse_active);

      Mouse::Show(false);
      screen->AddWindow(gamewin);
   }
}

// +--------------------------------------------------------------------+

bool
GameScreen::IsEngShown()
{
   return gamewin && engdlg && engdlg->IsShown();
}

// +--------------------------------------------------------------------+

void
GameScreen::ShowFltDlg()
{
   if (!gamewin) return;

   if (fltdlg && !fltdlg->IsShown()) {
      HideAll();

      fltdlg->SetShip(sim->GetPlayerShip());
      fltdlg->Show();

      if (mouse_con) {
         mouse_active = mouse_con->Active();
         mouse_con->SetActive(false);
      }

      Mouse::Show(true);
   }
   else {
      HideFltDlg();
   }
}

// +--------------------------------------------------------------------+

void
GameScreen::HideFltDlg()
{
   if (!gamewin) return;

   if (fltdlg && fltdlg->IsShown()) {
      fltdlg->Hide();

      if (mouse_con)
         mouse_con->SetActive(mouse_active);

      Mouse::Show(false);
      screen->AddWindow(gamewin);
   }
}

// +--------------------------------------------------------------------+

bool
GameScreen::IsFltShown()
{
   return gamewin && fltdlg && fltdlg->IsShown();
}

// +--------------------------------------------------------------------+

void
GameScreen::ShowAudDlg()
{
   if (auddlg) {
      if (quit_view) {
         quit_view->CloseMenu();
         Starshatter::GetInstance()->Pause(true);
      }

      HideAll();

      auddlg->Show();
      auddlg->SetTopMost(true);

      if (mouse_con) {
         mouse_active = mouse_con->Active();
         mouse_con->SetActive(false);
      }

      Mouse::Show(true);
   }
}

// +--------------------------------------------------------------------+

void
GameScreen::HideAudDlg()
{
   if (auddlg && auddlg->IsShown()) {
      auddlg->Hide();
      Mouse::Show(false);
      screen->AddWindow(gamewin);

      if (quit_view)
         quit_view->ShowMenu();
   }
}

// +--------------------------------------------------------------------+

bool
GameScreen::IsAudShown()
{
   return auddlg && auddlg->IsShown();
}

// +--------------------------------------------------------------------+

void
GameScreen::ShowVidDlg()
{
   if (viddlg) {
      if (quit_view) {
         quit_view->CloseMenu();
         Starshatter::GetInstance()->Pause(true);
      }

      HideAll();

      viddlg->Show();
      viddlg->SetTopMost(true);

      if (mouse_con) {
         mouse_active = mouse_con->Active();
         mouse_con->SetActive(false);
      }

      Mouse::Show(true);
   }
}

// +--------------------------------------------------------------------+

void
GameScreen::HideVidDlg()
{
   if (viddlg && viddlg->IsShown()) {
      viddlg->Hide();
      Mouse::Show(false);
      screen->AddWindow(gamewin);

      if (quit_view)
         quit_view->ShowMenu();
   }
}

// +--------------------------------------------------------------------+

bool
GameScreen::IsVidShown()
{
   return viddlg && viddlg->IsShown();
}

// +--------------------------------------------------------------------+

void
GameScreen::ShowOptDlg()
{
   if (optdlg) {
      if (quit_view) {
         quit_view->CloseMenu();
         Starshatter::GetInstance()->Pause(true);
      }

      HideAll();

      optdlg->Show();
      optdlg->SetTopMost(true);

      if (mouse_con) {
         mouse_active = mouse_con->Active();
         mouse_con->SetActive(false);
      }

      Mouse::Show(true);
   }
}

// +--------------------------------------------------------------------+

void
GameScreen::HideOptDlg()
{
   if (optdlg && optdlg->IsShown()) {
      optdlg->Hide();
      Mouse::Show(false);
      screen->AddWindow(gamewin);

      if (quit_view)
         quit_view->ShowMenu();
   }
}

// +--------------------------------------------------------------------+

bool
GameScreen::IsOptShown()
{
   return optdlg && optdlg->IsShown();
}

// +--------------------------------------------------------------------+

void
GameScreen::ShowModDlg()
{
   if (moddlg) {
      if (quit_view) {
         quit_view->CloseMenu();
         Starshatter::GetInstance()->Pause(true);
      }

      HideAll();

      moddlg->Show();
      moddlg->SetTopMost(true);

      if (mouse_con) {
         mouse_active = mouse_con->Active();
         mouse_con->SetActive(false);
      }

      Mouse::Show(true);
   }
}

// +--------------------------------------------------------------------+

void
GameScreen::HideModDlg()
{
   if (moddlg && moddlg->IsShown()) {
      moddlg->Hide();
      Mouse::Show(false);
      screen->AddWindow(gamewin);

      if (quit_view)
         quit_view->ShowMenu();
   }
}

// +--------------------------------------------------------------------+

bool
GameScreen::IsModShown()
{
   return moddlg && moddlg->IsShown();
}

// +--------------------------------------------------------------------+

void
GameScreen::ShowModInfoDlg()
{
   if (moddlg && modInfoDlg) {
      if (quit_view) {
         quit_view->CloseMenu();
         Starshatter::GetInstance()->Pause(true);
      }

      HideAll();

      moddlg->Show();
      moddlg->SetTopMost(false);

      modInfoDlg->Show();
      modInfoDlg->SetTopMost(true);

      if (mouse_con) {
         mouse_active = mouse_con->Active();
         mouse_con->SetActive(false);
      }

      Mouse::Show(true);
   }
}

// +--------------------------------------------------------------------+

void
GameScreen::HideModInfoDlg()
{
   ShowModDlg();
}

// +--------------------------------------------------------------------+

bool
GameScreen::IsModInfoShown()
{
   return modInfoDlg && modInfoDlg->IsShown();
}

// +--------------------------------------------------------------------+

void
GameScreen::ShowCtlDlg()
{
   if (ctldlg) {
      if (quit_view) {
         quit_view->CloseMenu();
         Starshatter::GetInstance()->Pause(true);
      }

      HideAll();

      ctldlg->Show();
      ctldlg->SetTopMost(true);

      if (mouse_con) {
         mouse_active = mouse_con->Active();
         mouse_con->SetActive(false);
      }

      Mouse::Show(true);
   }
}

// +--------------------------------------------------------------------+

void
GameScreen::HideCtlDlg()
{
   if (ctldlg && ctldlg->IsShown()) {
      ctldlg->Hide();
      Mouse::Show(false);
      screen->AddWindow(gamewin);

      if (quit_view)
         quit_view->ShowMenu();
   }
}

// +--------------------------------------------------------------------+

bool
GameScreen::IsCtlShown()
{
   return ctldlg && ctldlg->IsShown();
}

// +--------------------------------------------------------------------+

void
GameScreen::ShowKeyDlg()
{
   if (keydlg) {
      if (quit_view) {
         quit_view->CloseMenu();
         Starshatter::GetInstance()->Pause(true);
      }

      HideAll();

      if (ctldlg)  {
         ctldlg->Show();
         ctldlg->SetTopMost(false);
      }

      if (keydlg)  keydlg->Show();

      if (mouse_con) {
         mouse_con->SetActive(false);
      }

      Mouse::Show(true);
   }
}

// +--------------------------------------------------------------------+

bool
GameScreen::IsKeyShown()
{
   return keydlg && keydlg->IsShown();
}

// +--------------------------------------------------------------------+

void
GameScreen::ShowJoyDlg()
{
   if (joydlg) {
      if (quit_view) {
         quit_view->CloseMenu();
         Starshatter::GetInstance()->Pause(true);
      }

      HideAll();

      if (ctldlg)  {
         ctldlg->Show();
         ctldlg->SetTopMost(false);
      }

      if (joydlg)  joydlg->Show();

      if (mouse_con) {
         mouse_con->SetActive(false);
      }

      Mouse::Show(true);
   }
}

// +--------------------------------------------------------------------+

bool
GameScreen::IsJoyShown()
{
   return joydlg && joydlg->IsShown();
}

// +--------------------------------------------------------------------+

void
GameScreen::ShowWeaponsOverlay()
{
   if (wep_view)
      wep_view->CycleOverlayMode();
}

void
GameScreen::HideWeaponsOverlay()
{
   if (wep_view)
      wep_view->SetOverlayMode(0);
}

// +--------------------------------------------------------------------+

void
GameScreen::HideAll()
{
   screen->DelWindow(gamewin);

   if (engdlg)       engdlg->Hide();
   if (fltdlg)       fltdlg->Hide();
   if (navdlg)       navdlg->Hide();
   if (auddlg)       auddlg->Hide();
   if (viddlg)       viddlg->Hide();
   if (optdlg)       optdlg->Hide();
   if (moddlg)       moddlg->Hide();
   if (modInfoDlg)   modInfoDlg->Hide();
   if (ctldlg)       ctldlg->Hide();
   if (keydlg)       keydlg->Hide();
   if (joydlg)       joydlg->Hide();
}

// +--------------------------------------------------------------------+

void
GameScreen::ApplyOptions()
{
   if (ctldlg)  ctldlg->Apply();
   if (optdlg)  optdlg->Apply();
   if (auddlg)  auddlg->Apply();
   if (viddlg)  viddlg->Apply();

   if (engdlg)  engdlg->Hide();
   if (fltdlg)  fltdlg->Hide();
   if (navdlg)  navdlg->Hide();
   if (ctldlg)  ctldlg->Hide();
   if (auddlg)  auddlg->Hide();
   if (viddlg)  viddlg->Hide();
   if (optdlg)  optdlg->Hide();
   if (moddlg)  moddlg->Hide();
   if (keydlg)  keydlg->Hide();
   if (joydlg)  joydlg->Hide();

   Mouse::Show(false);
   screen->AddWindow(gamewin);
   Starshatter::GetInstance()->Pause(false);
}

void
GameScreen::CancelOptions()
{
   if (ctldlg)  ctldlg->Cancel();
   if (optdlg)  optdlg->Cancel();
   if (auddlg)  auddlg->Cancel();
   if (viddlg)  viddlg->Cancel();

   if (engdlg)  engdlg->Hide();
   if (fltdlg)  fltdlg->Hide();
   if (navdlg)  navdlg->Hide();
   if (ctldlg)  ctldlg->Hide();
   if (auddlg)  auddlg->Hide();
   if (viddlg)  viddlg->Hide();
   if (optdlg)  optdlg->Hide();
   if (moddlg)  moddlg->Hide();
   if (keydlg)  keydlg->Hide();
   if (joydlg)  joydlg->Hide();

   Mouse::Show(false);
   screen->AddWindow(gamewin);
   Starshatter::GetInstance()->Pause(false);
}
