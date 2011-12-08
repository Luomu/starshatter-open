/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars
    FILE:         PlanScreen.cpp
    AUTHOR:       John DiCamillo

*/

#include "MemDebug.h"
#include "PlanScreen.h"
#include "FormDef.h"
#include "MsnObjDlg.h"
#include "MsnPkgDlg.h"
#include "MsnWepDlg.h"
#include "MsnNavDlg.h"
#include "DebriefDlg.h"
#include "AwardDlg.h"
#include "Campaign.h"
#include "Mission.h"
#include "Sim.h"
#include "Starshatter.h"
#include "StarSystem.h"

#include "Game.h"
#include "Video.h"
#include "Screen.h"
#include "ActiveWindow.h"
#include "Mouse.h"
#include "Keyboard.h"
#include "FadeView.h"
#include "Color.h"
#include "Bitmap.h"
#include "Font.h"
#include "FontMgr.h"
#include "EventDispatch.h"
#include "DataLoader.h"
#include "Resource.h"

// +--------------------------------------------------------------------+

PlanScreen::PlanScreen()
   : screen(0), navdlg(0), award_dlg(0), debrief_dlg(0),
     objdlg(0), pkgdlg(0), wepdlg(0), isShown(false)
{
   loader   = DataLoader::GetLoader();
}

PlanScreen::~PlanScreen()
{
   TearDown();
}

// +--------------------------------------------------------------------+

void
PlanScreen::Setup(Screen* s)
{
   if (!s)
      return;

   screen        = s;

   // create windows
   loader->UseFileSystem(true);

   FormDef msn_obj_def("MsnObjDlg", 0);
   msn_obj_def.Load("MsnObjDlg");
   objdlg = new(__FILE__,__LINE__) MsnObjDlg(screen, msn_obj_def, this);

   FormDef msn_pkg_def("MsnPkgDlg", 0);
   msn_pkg_def.Load("MsnPkgDlg");
   pkgdlg = new(__FILE__,__LINE__) MsnPkgDlg(screen, msn_pkg_def, this);

   FormDef msn_nav_def("MsnNavDlg", 0);
   msn_nav_def.Load("MsnNavDlg");
   navdlg = new(__FILE__,__LINE__) MsnNavDlg(screen, msn_nav_def, this);

   FormDef msn_wep_def("MsnWepDlg", 0);
   msn_wep_def.Load("MsnWepDlg");
   wepdlg = new(__FILE__,__LINE__) MsnWepDlg(screen, msn_wep_def, this);

   FormDef award_def("AwardDlg", 0);
   award_def.Load("AwardDlg");
   award_dlg = new(__FILE__,__LINE__) AwardDlg(screen, award_def, this);

   FormDef debrief_def("DebriefDlg", 0);
   debrief_def.Load("DebriefDlg");
   debrief_dlg = new(__FILE__,__LINE__) DebriefDlg(screen, debrief_def, this);

   loader->UseFileSystem(Starshatter::UseFileSystem());
   ShowMsnDlg();
}

// +--------------------------------------------------------------------+

void
PlanScreen::TearDown()
{
   if (screen) {
      screen->DelWindow(objdlg);
      screen->DelWindow(pkgdlg);
      screen->DelWindow(wepdlg);
      screen->DelWindow(navdlg);
      screen->DelWindow(debrief_dlg);
      screen->DelWindow(award_dlg);
   }

   delete objdlg;
   delete pkgdlg;
   delete wepdlg;
   delete navdlg;
   delete debrief_dlg;
   delete award_dlg;

   objdlg           = 0;
   pkgdlg           = 0;
   wepdlg           = 0;
   navdlg           = 0;
   debrief_dlg      = 0;
   award_dlg        = 0;
   screen           = 0;
}

// +--------------------------------------------------------------------+

void
PlanScreen::ExecFrame()
{
   Game::SetScreenColor(Color::Black);

   Mission*    mission  = 0;
   Campaign*   campaign = Campaign::GetCampaign();

   if (campaign)
      mission = campaign->GetMission();

   if (navdlg) {
      navdlg->SetMission(mission);

      if (navdlg->IsShown())
         navdlg->ExecFrame();
   }

   if (objdlg && objdlg->IsShown()) {
      objdlg->ExecFrame();
   }

   if (pkgdlg && pkgdlg->IsShown()) {
      pkgdlg->ExecFrame();
   }

   if (wepdlg && wepdlg->IsShown()) {
      wepdlg->ExecFrame();
   }

   if (award_dlg && award_dlg->IsShown()) {
      award_dlg->ExecFrame();
   }

   if (debrief_dlg && debrief_dlg->IsShown()) {
      debrief_dlg->ExecFrame();
   }
}

// +--------------------------------------------------------------------+

bool
PlanScreen::CloseTopmost()
{
   if (debrief_dlg->IsShown()) {
      debrief_dlg->OnClose(0);
   }

   if (award_dlg->IsShown()) {
      return true;
   }

   return false;
}

void
PlanScreen::Show()
{
   if (!isShown) {
      ShowMsnDlg();
      isShown = true;
   }
}

void
PlanScreen::Hide()
{
   HideAll();
   isShown = false;
}

// +--------------------------------------------------------------------+

void
PlanScreen::ShowMsnDlg()
{
   HideAll();
   Mouse::Show(true);
   objdlg->Show();
}

// +--------------------------------------------------------------------+

void
PlanScreen::HideMsnDlg()
{
   HideAll();
   Mouse::Show(true);
   objdlg->Show();
}

bool
PlanScreen::IsMsnShown()
{
   return IsMsnObjShown() || IsMsnPkgShown() || IsMsnWepShown();
}

// +--------------------------------------------------------------------+

void
PlanScreen::ShowMsnObjDlg()
{
   HideAll();
   Mouse::Show(true);
   objdlg->Show();
}

// +--------------------------------------------------------------------+

void
PlanScreen::HideMsnObjDlg()
{
   HideAll();
   Mouse::Show(true);
}

// +--------------------------------------------------------------------+

bool
PlanScreen::IsMsnObjShown()
{
   return objdlg && objdlg->IsShown();
}

// +--------------------------------------------------------------------+

void
PlanScreen::ShowMsnPkgDlg()
{
   HideAll();
   Mouse::Show(true);
   pkgdlg->Show();
}

// +--------------------------------------------------------------------+

void
PlanScreen::HideMsnPkgDlg()
{
   HideAll();
   Mouse::Show(true);
}

// +--------------------------------------------------------------------+

bool
PlanScreen::IsMsnPkgShown()
{
   return pkgdlg && pkgdlg->IsShown();
}

// +--------------------------------------------------------------------+

void
PlanScreen::ShowMsnWepDlg()
{
   HideAll();
   Mouse::Show(true);
   wepdlg->Show();
}

// +--------------------------------------------------------------------+

void
PlanScreen::HideMsnWepDlg()
{
   HideAll();
   Mouse::Show(true);
}

// +--------------------------------------------------------------------+

bool
PlanScreen::IsMsnWepShown()
{
   return wepdlg && wepdlg->IsShown();
}

// +--------------------------------------------------------------------+

void
PlanScreen::ShowNavDlg()
{
   if (navdlg && !navdlg->IsShown()) {
      HideAll();
      Mouse::Show(true);
      navdlg->Show();
   }
}

// +--------------------------------------------------------------------+

void
PlanScreen::HideNavDlg()
{
   if (navdlg && navdlg->IsShown()) {
      HideAll();
      Mouse::Show(true);
   }
}

// +--------------------------------------------------------------------+

bool
PlanScreen::IsNavShown()
{
   return navdlg && navdlg->IsShown();
}

// +--------------------------------------------------------------------+

void
PlanScreen::ShowDebriefDlg()
{
   HideAll();
   Mouse::Show(true);
   debrief_dlg->Show();
}

void
PlanScreen::HideDebriefDlg()
{
   HideAll();
   Mouse::Show(true);
}

bool
PlanScreen::IsDebriefShown()
{
   return debrief_dlg && debrief_dlg->IsShown();
}

// +--------------------------------------------------------------------+

void
PlanScreen::ShowAwardDlg()
{
   HideAll();
   Mouse::Show(true);
   award_dlg->Show();
}

void
PlanScreen::HideAwardDlg()
{
   HideAll();
   Mouse::Show(true);
}

bool
PlanScreen::IsAwardShown()
{
   return award_dlg && award_dlg->IsShown();
}


// +--------------------------------------------------------------------+

void
PlanScreen::HideAll()
{
   if (objdlg)       objdlg->Hide();
   if (pkgdlg)       pkgdlg->Hide();
   if (wepdlg)       wepdlg->Hide();
   if (navdlg)       navdlg->Hide();
   if (award_dlg)    award_dlg->Hide();
   if (debrief_dlg)  debrief_dlg->Hide();
}