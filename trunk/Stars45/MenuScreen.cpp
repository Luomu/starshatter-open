/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         main.cpp
    AUTHOR:       John DiCamillo

*/

#include "MemDebug.h"
#include "MenuScreen.h"

#include "FormDef.h"
#include "MenuDlg.h"

#include "ExitDlg.h"
#include "AudDlg.h"
#include "VidDlg.h"
#include "OptDlg.h"
#include "CtlDlg.h"
#include "JoyDlg.h"
#include "KeyDlg.h"
#include "ConfirmDlg.h"
#include "PlayerDlg.h"
#include "ModDlg.h"
#include "ModInfoDlg.h"
#include "MsnSelectDlg.h"
#include "MsnEditDlg.h"
#include "MsnElemDlg.h"
#include "MsnEventDlg.h"
#include "MsnEditNavDlg.h"
#include "FirstTimeDlg.h"
#include "AwardShowDlg.h"
#include "LoadDlg.h"
#include "TacRefDlg.h"

#include "CmpSelectDlg.h"
#include "NetClientDlg.h"
#include "NetLobbyDlg.h"
#include "NetServerDlg.h"
#include "NetUnitDlg.h"
#include "NetAddrDlg.h"
#include "NetPassDlg.h"

#include "Starshatter.h"
#include "Player.h"
#include "NetLobby.h"
#include "NetClientConfig.h"
#include "NetServerConfig.h"

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

MenuScreen::MenuScreen()
   : screen(0), menudlg(0), 
     fadewin(0), fadeview(0), exitdlg(0),
     auddlg(0), viddlg(0), optdlg(0), ctldlg(0), joydlg(0), keydlg(0),
     playdlg(0), confirmdlg(0), firstdlg(0), awarddlg(0), cmpSelectDlg(0), 
     msnSelectDlg(0), modDlg(0), modInfoDlg(0),
     msnEditDlg(0), msnElemDlg(0), msnEventDlg(0), msnEditNavDlg(0),
     netClientDlg(0), netAddrDlg(0), netPassDlg(0), netLobbyDlg(0), netServerDlg(0),
     netUnitDlg(0), loadDlg(0), tacRefDlg(0), current_dlg(0), isShown(false)
{
   loader   = DataLoader::GetLoader();
}

MenuScreen::~MenuScreen()
{
   TearDown();
}

// +--------------------------------------------------------------------+

void
MenuScreen::Setup(Screen* s)
{
   if (!s)
      return;

   screen = s;

   Color::SetFade(0);

   // create windows

   loader->UseFileSystem(true);

   FormDef menu_def("MenuDlg", 0);
   menu_def.Load("MenuDlg");
   menudlg = new(__FILE__,__LINE__) MenuDlg(screen, menu_def, this);

   FormDef exit_def("ExitDlg", 0);
   exit_def.Load("ExitDlg");
   exitdlg = new(__FILE__,__LINE__) ExitDlg(screen, exit_def, this);

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
   modDlg = new(__FILE__,__LINE__) ModDlg(screen, mod_def, this);

   FormDef tac_ref_def("TacRefDlg", 0);
   tac_ref_def.Load("TacRefDlg");
   tacRefDlg = new(__FILE__,__LINE__) TacRefDlg(screen, tac_ref_def, this);

   FormDef cmp_sel_def("CmpSelectDlg", 0);
   cmp_sel_def.Load("CmpSelectDlg");
   cmpSelectDlg = new(__FILE__,__LINE__) CmpSelectDlg(screen, cmp_sel_def, this);

   FormDef net_lobby_def("NetLobbyDlg", 0);
   net_lobby_def.Load("NetLobbyDlg");
   netLobbyDlg = new(__FILE__,__LINE__) NetLobbyDlg(screen, net_lobby_def, this);

   FormDef net_client_def("NetClientDlg", 0);
   net_client_def.Load("NetClientDlg");
   netClientDlg = new(__FILE__,__LINE__) NetClientDlg(screen, net_client_def, this);

   FormDef net_server_def("NetServerDlg", 0);
   net_server_def.Load("NetServerDlg");
   netServerDlg = new(__FILE__,__LINE__) NetServerDlg(screen, net_server_def, this);

   FormDef net_unit_def("NetUnitDlg", 0);
   net_unit_def.Load("NetUnitDlg");
   netUnitDlg = new(__FILE__,__LINE__) NetUnitDlg(screen, net_unit_def, this);

   FormDef net_addr_def("NetAddrDlg", 0);
   net_addr_def.Load("NetAddrDlg");
   netAddrDlg = new(__FILE__,__LINE__) NetAddrDlg(screen, net_addr_def, this);

   FormDef net_pass_def("NetPassDlg", 0);
   net_pass_def.Load("NetPassDlg");
   netPassDlg = new(__FILE__,__LINE__) NetPassDlg(screen, net_pass_def, this);

   FormDef msn_edit_def("MsnEditDlg", 0);
   msn_edit_def.Load("MsnEditDlg");
   msnEditDlg = new(__FILE__,__LINE__) MsnEditDlg(screen, msn_edit_def, this);

   FormDef msn_nav_def("MsnEditNavDlg", 0);
   msn_nav_def.Load("MsnEditNavDlg");
   msnEditNavDlg = new(__FILE__,__LINE__) MsnEditNavDlg(screen, msn_nav_def, this);

   FormDef msn_elem_def("MsnElemDlg", 0);
   msn_elem_def.Load("MsnElemDlg");
   msnElemDlg = new(__FILE__,__LINE__) MsnElemDlg(screen, msn_elem_def, this);

   FormDef msn_event_def("MsnEventDlg", 0);
   msn_event_def.Load("MsnEventDlg");
   msnEventDlg = new(__FILE__,__LINE__) MsnEventDlg(screen, msn_event_def, this);

   FormDef msn_def("MsnSelectDlg", 0);
   msn_def.Load("MsnSelectDlg");
   msnSelectDlg = new(__FILE__,__LINE__) MsnSelectDlg(screen, msn_def, this);

   FormDef player_def("PlayerDlg", 0);
   player_def.Load("PlayerDlg");
   playdlg = new(__FILE__,__LINE__) PlayerDlg(screen, player_def, this);

   FormDef award_def("AwardDlg", 0);
   award_def.Load("AwardDlg");
   awarddlg = new(__FILE__,__LINE__) AwardShowDlg(screen, award_def, this);

   FormDef joy_def("JoyDlg", 0);
   joy_def.Load("JoyDlg");
   joydlg = new(__FILE__,__LINE__) JoyDlg(screen, joy_def, this);

   FormDef key_def("KeyDlg", 0);
   key_def.Load("KeyDlg");
   keydlg = new(__FILE__,__LINE__) KeyDlg(screen, key_def, this);

   FormDef first_def("FirstTimeDlg", 0);
   first_def.Load("FirstTimeDlg");
   firstdlg = new(__FILE__,__LINE__) FirstTimeDlg(screen, first_def, this);

   FormDef mod_info_def("ModInfoDlg", 0);
   mod_info_def.Load("ModInfoDlg");
   modInfoDlg = new(__FILE__,__LINE__) ModInfoDlg(screen, mod_info_def, this);

   FormDef confirm_def("ConfirmDlg", 0);
   confirm_def.Load("ConfirmDlg");
   confirmdlg = new(__FILE__,__LINE__) ConfirmDlg(screen, confirm_def, this);

   FormDef load_def("LoadDlg", 0);
   load_def.Load("LoadDlg");
   loadDlg = new(__FILE__,__LINE__) LoadDlg(screen, load_def);

   fadewin  = new(__FILE__,__LINE__) Window(screen, 0, 0, 1, 1);
   fadeview = new(__FILE__,__LINE__) FadeView(fadewin, 2, 0, 0);
   fadewin->AddView(fadeview);
   screen->AddWindow(fadewin);

   loader->UseFileSystem(Starshatter::UseFileSystem());

   ShowMenuDlg();
}

// +--------------------------------------------------------------------+

void
MenuScreen::TearDown()
{
   if (screen) {
      if (menudlg)         screen->DelWindow(menudlg);
      if (netClientDlg)    screen->DelWindow(netClientDlg);
      if (netAddrDlg)      screen->DelWindow(netAddrDlg);
      if (netPassDlg)      screen->DelWindow(netPassDlg);
      if (netLobbyDlg)     screen->DelWindow(netLobbyDlg);
      if (netServerDlg)    screen->DelWindow(netServerDlg);
      if (netUnitDlg)      screen->DelWindow(netUnitDlg);

      if (cmpSelectDlg)    screen->DelWindow(cmpSelectDlg);
      if (awarddlg)        screen->DelWindow(awarddlg);
      if (firstdlg)        screen->DelWindow(firstdlg);
      if (msnSelectDlg)    screen->DelWindow(msnSelectDlg);
      if (msnEditDlg)      screen->DelWindow(msnEditDlg);
      if (msnElemDlg)      screen->DelWindow(msnElemDlg);
      if (msnEventDlg)     screen->DelWindow(msnEventDlg);
      if (msnEditNavDlg)   screen->DelWindow(msnEditNavDlg);
      if (tacRefDlg)       screen->DelWindow(tacRefDlg);
      if (loadDlg)         screen->DelWindow(loadDlg);

      if (auddlg)          screen->DelWindow(auddlg);
      if (viddlg)          screen->DelWindow(viddlg);
      if (optdlg)          screen->DelWindow(optdlg);
      if (ctldlg)          screen->DelWindow(ctldlg);
      if (modDlg)          screen->DelWindow(modDlg);
      if (modInfoDlg)      screen->DelWindow(modInfoDlg);
      if (joydlg)          screen->DelWindow(joydlg);
      if (keydlg)          screen->DelWindow(keydlg);
      if (exitdlg)         screen->DelWindow(exitdlg);
      if (playdlg)         screen->DelWindow(playdlg);
      if (confirmdlg)      screen->DelWindow(confirmdlg);
      if (fadewin)         screen->DelWindow(fadewin);
   }

   delete menudlg;
   delete fadewin;
   delete exitdlg;
   delete auddlg;
   delete viddlg;
   delete optdlg;
   delete ctldlg;
   delete modDlg;
   delete modInfoDlg;
   delete joydlg;
   delete keydlg;
   delete playdlg;
   delete confirmdlg;

   delete netClientDlg;
   delete netAddrDlg;
   delete netPassDlg;
   delete netLobbyDlg;
   delete netServerDlg;
   delete netUnitDlg;
   delete msnSelectDlg;
   delete msnEditDlg;
   delete msnElemDlg;
   delete msnEventDlg;
   delete msnEditNavDlg;
   delete tacRefDlg;
   delete loadDlg;
   delete firstdlg;
   delete awarddlg;
   delete cmpSelectDlg;

   screen         = 0;
   fadewin        = 0;
   fadeview       = 0;
   menudlg        = 0;
   exitdlg        = 0;
   playdlg        = 0;
   confirmdlg     = 0;
   msnSelectDlg   = 0;
   msnEditDlg     = 0;
   msnElemDlg     = 0;
   msnEventDlg    = 0;
   msnEditNavDlg  = 0;
   cmpSelectDlg   = 0;
   awarddlg       = 0;
   firstdlg       = 0;
   netClientDlg   = 0;
   netAddrDlg     = 0;
   netPassDlg     = 0;
   netLobbyDlg    = 0;
   netServerDlg   = 0;
   netUnitDlg     = 0;
   loadDlg        = 0;
   tacRefDlg      = 0;

   auddlg         = 0;
   viddlg         = 0;
   optdlg         = 0;
   ctldlg         = 0;
   modDlg         = 0;
   modInfoDlg     = 0;
   joydlg         = 0;
   keydlg         = 0;

   screen         = 0;
}

// +--------------------------------------------------------------------+

void
MenuScreen::ExecFrame()
{
   Game::SetScreenColor(Color::Black);

   if (menudlg && menudlg->IsShown())
      menudlg->ExecFrame();

   if (exitdlg && exitdlg->IsShown())
      exitdlg->ExecFrame();

   if (joydlg && joydlg->IsShown())
      joydlg->ExecFrame();

   if (keydlg && keydlg->IsShown())
      keydlg->ExecFrame();

   if (ctldlg && ctldlg->IsShown())
      ctldlg->ExecFrame();

   if (optdlg && optdlg->IsShown())
      optdlg->ExecFrame();

   if (auddlg && auddlg->IsShown())
      auddlg->ExecFrame();

   if (viddlg && viddlg->IsShown())
      viddlg->ExecFrame();

   if (confirmdlg && confirmdlg->IsShown())
      confirmdlg->ExecFrame();

   if (playdlg && playdlg->IsShown())
      playdlg->ExecFrame();

   if (msnSelectDlg && msnSelectDlg->IsShown())
      msnSelectDlg->ExecFrame();

   if (msnEditNavDlg && msnEditNavDlg->IsShown())
      msnEditNavDlg->ExecFrame();

   if (firstdlg && firstdlg->IsShown())
      firstdlg->ExecFrame();

   if (awarddlg && awarddlg->IsShown())
      awarddlg->ExecFrame();

   if (cmpSelectDlg && cmpSelectDlg->IsShown())
      cmpSelectDlg->ExecFrame();

   if (netClientDlg && netClientDlg->IsShown())
      netClientDlg->ExecFrame();

   if (netAddrDlg && netAddrDlg->IsShown())
      netAddrDlg->ExecFrame();

   if (netPassDlg && netPassDlg->IsShown())
      netPassDlg->ExecFrame();

   if (netLobbyDlg && netLobbyDlg->IsShown())
      netLobbyDlg->ExecFrame();

   if (netServerDlg && netServerDlg->IsShown())
      netServerDlg->ExecFrame();

   if (netUnitDlg && netUnitDlg->IsShown())
      netUnitDlg->ExecFrame();

   if (loadDlg && loadDlg->IsShown())
      loadDlg->ExecFrame();

   if (tacRefDlg && tacRefDlg->IsShown())
      tacRefDlg->ExecFrame();
}

// +--------------------------------------------------------------------+

bool
MenuScreen::CloseTopmost()
{
   bool processed = false;
   if (joydlg && joydlg->IsShown()) {
      ShowCtlDlg();
      processed = true;
   }

   else if (keydlg && keydlg->IsShown()) {
      ShowCtlDlg();
      processed = true;
   }

   else if (msnElemDlg && msnElemDlg->IsShown()) {
      HideMsnElemDlg();
      processed = true;
   }

   else if (msnEventDlg && msnEventDlg->IsShown()) {
      HideMsnEventDlg();
      processed = true;
   }

   else if (netAddrDlg && netAddrDlg->IsShown()) {
      ShowNetClientDlg();
      processed = true;
   }

   else if (netPassDlg && netPassDlg->IsShown()) {
      ShowNetClientDlg();
      processed = true;
   }

   else if (netServerDlg && netServerDlg->IsShown()) {
      ShowNetClientDlg();
      processed = true;
   }

   else if (netUnitDlg && netUnitDlg->IsShown()) {
      netUnitDlg->OnCancel(0);
      processed = true;
   }

   else if (netLobbyDlg && netLobbyDlg->IsShown()) {
      netLobbyDlg->OnCancel(0);
      processed = true;
   }

   else if (netClientDlg && netClientDlg->IsShown()) {
      netClientDlg->OnCancel(0);
      processed = true;
   }

   else if (exitdlg && exitdlg->IsShown()) {
      // key_exit is handled in the exit dlg...
   }

   else if (cmpSelectDlg && cmpSelectDlg->IsShown()) {
      if (cmpSelectDlg->CanClose())
         ShowMenuDlg();

      processed = true;
   }

   else if (menudlg && !menudlg->IsShown()) {
      ShowMenuDlg();
      processed = true;
   }

   return processed;
}

void
MenuScreen::Show()
{
   if (!isShown) {
      Starshatter*   stars = Starshatter::GetInstance();
      NetLobby*      lobby = NetLobby::GetInstance();

      if (lobby) {
         ShowNetLobbyDlg();
      }
      else if (current_dlg == msnSelectDlg) {
         ShowMsnSelectDlg();
      }
      else {
         if (Player::ConfigExists()) {
            ShowMenuDlg();
         }

         else {
            ShowFirstTimeDlg();
         }
      }

      isShown = true;
   }
}

void
MenuScreen::Hide()
{
   if (isShown) {
      HideAll();
      isShown = false;
   }
}

// +--------------------------------------------------------------------+

void
MenuScreen::ShowMenuDlg()
{
   HideAll();
   if (menudlg) {
      menudlg->Show();
      menudlg->SetTopMost(true);
   }
   Mouse::Show(true);
}

// +--------------------------------------------------------------------+

void
MenuScreen::ShowCmpSelectDlg()
{
   HideAll();
   current_dlg = cmpSelectDlg;
   if (cmpSelectDlg)
      cmpSelectDlg->Show();
   Mouse::Show(true);
}

// +--------------------------------------------------------------------+

void
MenuScreen::ShowMsnSelectDlg()
{
   HideAll();
   current_dlg = msnSelectDlg;

   if (msnSelectDlg)
      msnSelectDlg->Show();

   if (msnEditNavDlg)
      msnEditNavDlg->SetMission(0);

   Mouse::Show(true);
}

// +--------------------------------------------------------------------+

void
MenuScreen::ShowModDlg()
{
   if (modDlg) {
      HideAll();
      modDlg->Show();
      modDlg->SetTopMost(true);
      Mouse::Show(true);
   }
   else {
      ShowMsnSelectDlg();
   }
}

// +--------------------------------------------------------------------+

void
MenuScreen::ShowModInfoDlg()
{
   if (modDlg && modInfoDlg) {
      HideAll();
      modDlg->Show();
      modDlg->SetTopMost(false);
      modInfoDlg->Show();
      Mouse::Show(true);
   }
   else {
      ShowMsnSelectDlg();
   }
}

// +--------------------------------------------------------------------+

void
MenuScreen::ShowMsnEditDlg()
{
   if (msnEditDlg) {
      bool nav_shown = false;
      if (msnEditNavDlg && msnEditNavDlg->IsShown())
         nav_shown = true;

      HideAll();

      if (nav_shown) {
         msnEditNavDlg->Show();
         msnEditNavDlg->SetTopMost(true);
      }
      else {
         msnEditDlg->Show();
         msnEditDlg->SetTopMost(true);
      }

      Mouse::Show(true);
   }
   else {
      ShowMsnSelectDlg();
   }
}

// +--------------------------------------------------------------------+

void
MenuScreen::ShowMsnElemDlg()
{
   if (msnElemDlg) {
      if (msnEditDlg && msnEditDlg->IsShown())
         msnEditDlg->SetTopMost(false);

      if (msnEditNavDlg && msnEditNavDlg->IsShown())
         msnEditNavDlg->SetTopMost(false);

      msnElemDlg->Show();
      msnElemDlg->SetTopMost(true);
      Mouse::Show(true);
   }
}

// +--------------------------------------------------------------------+

void
MenuScreen::ShowMsnEventDlg()
{
   if (msnEventDlg) {
      if (msnEditDlg && msnEditDlg->IsShown())
         msnEditDlg->SetTopMost(false);

      if (msnEditNavDlg && msnEditNavDlg->IsShown())
         msnEditNavDlg->SetTopMost(false);

      msnEventDlg->Show();
      msnEventDlg->SetTopMost(true);
      Mouse::Show(true);
   }
}

// +--------------------------------------------------------------------+

void
MenuScreen::ShowNavDlg()
{
   if (msnEditNavDlg && !msnEditNavDlg->IsShown()) {
      HideAll();
      msnEditNavDlg->Show();
      Mouse::Show(true);
   }
}

// +--------------------------------------------------------------------+

void
MenuScreen::ShowFirstTimeDlg()
{
   HideAll();
   if (menudlg && firstdlg) {
      menudlg->Show();
      menudlg->SetTopMost(false);

      firstdlg->Show();
      firstdlg->SetTopMost(true);
   }

   Mouse::Show(true);
}

// +--------------------------------------------------------------------+

void
MenuScreen::ShowPlayerDlg()
{
   if (playdlg) {
      HideAll();
      playdlg->Show();
   }
   Mouse::Show(true);
}

// +--------------------------------------------------------------------+

void
MenuScreen::ShowAwardDlg()
{
   if (awarddlg) {
      HideAll();
      awarddlg->Show();
   }
   Mouse::Show(true);
}

// +--------------------------------------------------------------------+

void
MenuScreen::ShowTacRefDlg()
{
   if (tacRefDlg) {
      HideAll();
      tacRefDlg->Show();
   }
   Mouse::Show(true);
}

// +--------------------------------------------------------------------+

void
MenuScreen::ShowNetClientDlg()
{
   if (netClientDlg) {
      HideAll();
      netClientDlg->Show();
      netClientDlg->SetTopMost(true);
   }

   Mouse::Show(true);
}

// +--------------------------------------------------------------------+

void
MenuScreen::ShowNetAddrDlg()
{
   if (netAddrDlg) {
      if (netClientDlg) {
         netClientDlg->Show();
         netClientDlg->SetTopMost(false);
      }

      netAddrDlg->Show();
   }

   Mouse::Show(true);
}

// +--------------------------------------------------------------------+

void
MenuScreen::ShowNetPassDlg()
{
   if (netPassDlg) {
      ShowNetClientDlg();
      if (netClientDlg)
         netClientDlg->SetTopMost(false);

      netPassDlg->Show();
   }

   Mouse::Show(true);
}

// +--------------------------------------------------------------------+

void
MenuScreen::ShowNetLobbyDlg()
{
   if (netLobbyDlg) {
      HideAll();
      netLobbyDlg->Show();
      netLobbyDlg->SetTopMost(true);
   }

   Mouse::Show(true);
}

// +--------------------------------------------------------------------+

void
MenuScreen::ShowNetServerDlg()
{
   if (netServerDlg) {
      netServerDlg->Show();
      netServerDlg->SetTopMost(true);
   }

   Mouse::Show(true);
}

// +--------------------------------------------------------------------+

void
MenuScreen::ShowNetUnitDlg()
{
   if (netUnitDlg) {
      HideAll();
      netUnitDlg->Show();
      netUnitDlg->SetTopMost(true);
   }

   Mouse::Show(true);
}

// +--------------------------------------------------------------------+

void
MenuScreen::ShowAudDlg()
{
   HideAll();
   if (auddlg)
      auddlg->Show();
   Mouse::Show(true);
}

// +--------------------------------------------------------------------+

void
MenuScreen::ShowVidDlg()
{
   HideAll();
   if (viddlg)
      viddlg->Show();
   Mouse::Show(true);
}

// +--------------------------------------------------------------------+

void
MenuScreen::ShowOptDlg()
{
   HideAll();
   if (optdlg)
      optdlg->Show();
   Mouse::Show(true);
}

// +--------------------------------------------------------------------+

void
MenuScreen::ShowCtlDlg()
{
   HideAll();
   if (ctldlg) {
      ctldlg->Show();
      ctldlg->SetTopMost(true);
   }
   Mouse::Show(true);
}

// +--------------------------------------------------------------------+

void
MenuScreen::ShowJoyDlg()
{
   HideAll();
   if (ctldlg) {
      ctldlg->Show();
      ctldlg->SetTopMost(false);
   }

   if (joydlg)
      joydlg->Show();
   Mouse::Show(true);
}

// +--------------------------------------------------------------------+

void
MenuScreen::ShowKeyDlg()
{
   HideAll();

   if (ctldlg) {
      ctldlg->Show();
      ctldlg->SetTopMost(false);
   }

   if (keydlg)
      keydlg->Show();
   Mouse::Show(true);
}

// +--------------------------------------------------------------------+

void
MenuScreen::ShowConfirmDlg()
{
   if (confirmdlg) {
      if (msnSelectDlg && msnSelectDlg->IsShown())
         msnSelectDlg->SetTopMost(false);

      if (playdlg && playdlg->IsShown())
         playdlg->SetTopMost(false);

      confirmdlg->Show();
      confirmdlg->SetTopMost(true);
      Mouse::Show(true);
   }
}

void
MenuScreen::HideConfirmDlg()
{
   if (confirmdlg)
      confirmdlg->Hide();

   if (msnSelectDlg && msnSelectDlg->IsShown())
      msnSelectDlg->SetTopMost(true);

   if (playdlg && playdlg->IsShown())
      playdlg->SetTopMost(true);
}

// +--------------------------------------------------------------------+

void
MenuScreen::ShowLoadDlg()
{
   if (loadDlg) {
      if (menudlg && menudlg->IsShown())
         menudlg->SetTopMost(false);

      loadDlg->Show();
      loadDlg->SetTopMost(true);
      Mouse::Show(true);
   }
}

void
MenuScreen::HideLoadDlg()
{
   if (loadDlg)
      loadDlg->Hide();

   if (menudlg && menudlg->IsShown())
      menudlg->SetTopMost(true);
}

// +--------------------------------------------------------------------+

void
MenuScreen::ShowExitDlg()
{
   HideAll();

   if (menudlg) {
      menudlg->Show();
      menudlg->SetTopMost(false);
   }

   if (exitdlg)
      exitdlg->Show();
   else
      Starshatter::GetInstance()->Exit();

   Mouse::Show(true);
}

// +--------------------------------------------------------------------+

void
MenuScreen::HideNavDlg()
{
   if (msnEditNavDlg)
      msnEditNavDlg->Hide();
}

// +--------------------------------------------------------------------+

void
MenuScreen::HideMsnElemDlg()
{
   if (msnElemDlg)
      msnElemDlg->Hide();

   if (msnEditDlg && msnEditDlg->IsShown())
      msnEditDlg->SetTopMost(true);

   if (msnEditNavDlg && msnEditNavDlg->IsShown())
      msnEditNavDlg->SetTopMost(true);
}

// +--------------------------------------------------------------------+

void
MenuScreen::HideMsnEventDlg()
{
   if (msnEventDlg)
      msnEventDlg->Hide();

   if (msnEditDlg && msnEditDlg->IsShown())
      msnEditDlg->SetTopMost(true);

   if (msnEditNavDlg && msnEditNavDlg->IsShown())
      msnEditNavDlg->SetTopMost(true);
}

// +--------------------------------------------------------------------+

bool
MenuScreen::IsNavShown()
{
   return msnEditNavDlg && msnEditNavDlg->IsShown();
}

// +--------------------------------------------------------------------+

void
MenuScreen::HideAll()
{
   Keyboard::FlushKeys();

   current_dlg = 0;

   if (menudlg)         menudlg->Hide();
   if (exitdlg)         exitdlg->Hide();
   if (auddlg)          auddlg->Hide();
   if (viddlg)          viddlg->Hide();
   if (ctldlg)          ctldlg->Hide();
   if (optdlg)          optdlg->Hide();
   if (joydlg)          joydlg->Hide();
   if (keydlg)          keydlg->Hide();
   if (playdlg)         playdlg->Hide();
   if (confirmdlg)      confirmdlg->Hide();
   if (modDlg)          modDlg->Hide();
   if (modInfoDlg)      modInfoDlg->Hide();
   if (msnSelectDlg)    msnSelectDlg->Hide();
   if (msnEditDlg)      msnEditDlg->Hide();
   if (msnElemDlg)      msnElemDlg->Hide();
   if (msnEventDlg)     msnEventDlg->Hide();
   if (msnEditNavDlg)   msnEditNavDlg->Hide();
   if (netClientDlg)    netClientDlg->Hide();
   if (netAddrDlg)      netAddrDlg->Hide();
   if (netPassDlg)      netPassDlg->Hide();
   if (netLobbyDlg)     netLobbyDlg->Hide();
   if (netServerDlg)    netServerDlg->Hide();
   if (netUnitDlg)      netUnitDlg->Hide();
   if (firstdlg)        firstdlg->Hide();
   if (awarddlg)        awarddlg->Hide();
   if (cmpSelectDlg)    cmpSelectDlg->Hide();
   if (tacRefDlg)       tacRefDlg->Hide();
   if (loadDlg)         loadDlg->Hide();
}

// +--------------------------------------------------------------------+

void
MenuScreen::ApplyOptions()
{
   if (ctldlg)          ctldlg->Apply();
   if (optdlg)          optdlg->Apply();
   if (auddlg)          auddlg->Apply();
   if (viddlg)          viddlg->Apply();
   if (modDlg)          modDlg->Apply();

   ShowMenuDlg();
}

void
MenuScreen::CancelOptions()
{
   if (ctldlg)          ctldlg->Cancel();
   if (optdlg)          optdlg->Cancel();
   if (auddlg)          auddlg->Cancel();
   if (viddlg)          viddlg->Cancel();
   if (modDlg)          modDlg->Cancel();

   ShowMenuDlg();
}
