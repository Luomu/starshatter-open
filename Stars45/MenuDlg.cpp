/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         MenuDlg.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Main Menu Dialog Active Window class
*/

#include "MemDebug.h"
#include "MenuDlg.h"
#include "MenuScreen.h"
#include "Starshatter.h"
#include "Campaign.h"

#include "Game.h"
#include "DataLoader.h"
#include "Button.h"
#include "ListBox.h"
#include "Slider.h"
#include "Video.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "ParseUtil.h"

// +--------------------------------------------------------------------+
// DECLARE MAPPING FUNCTIONS:

DEF_MAP_CLIENT(MenuDlg, OnStart);
DEF_MAP_CLIENT(MenuDlg, OnCampaign);
DEF_MAP_CLIENT(MenuDlg, OnMission);
DEF_MAP_CLIENT(MenuDlg, OnPlayer);
DEF_MAP_CLIENT(MenuDlg, OnMultiplayer);
DEF_MAP_CLIENT(MenuDlg, OnMod);
DEF_MAP_CLIENT(MenuDlg, OnTacReference);
DEF_MAP_CLIENT(MenuDlg, OnVideo);
DEF_MAP_CLIENT(MenuDlg, OnOptions);
DEF_MAP_CLIENT(MenuDlg, OnControls);
DEF_MAP_CLIENT(MenuDlg, OnQuit);
DEF_MAP_CLIENT(MenuDlg, OnButtonEnter);
DEF_MAP_CLIENT(MenuDlg, OnButtonExit);

extern const char* versionInfo;

// +--------------------------------------------------------------------+

MenuDlg::MenuDlg(Screen* s, FormDef& def, MenuScreen* mgr)
   : FormWindow(s,  0,  0, s->Width(), s->Height()), 
     manager(mgr), btn_start(0),
     btn_campaign(0), btn_mission(0), btn_player(0), btn_multi(0),
     btn_mod(0), btn_tac(0),
     btn_options(0), btn_controls(0), btn_quit(0),
     version(0), description(0), stars(0), campaign(0)
{
   stars    = Starshatter::GetInstance();
   campaign = Campaign::GetCampaign();

   Init(def);
}

MenuDlg::~MenuDlg()
{
}

// +--------------------------------------------------------------------+

void
MenuDlg::RegisterControls()
{
   if (btn_start)
      return;

   btn_start      = (Button*) FindControl(120);
   btn_campaign   = (Button*) FindControl(101);
   btn_mission    = (Button*) FindControl(102);
   btn_player     = (Button*) FindControl(103);
   btn_multi      = (Button*) FindControl(104);
   btn_video      = (Button*) FindControl(111);
   btn_options    = (Button*) FindControl(112);
   btn_controls   = (Button*) FindControl(113);
   btn_quit       = (Button*) FindControl(114);
   btn_mod        = (Button*) FindControl(115);
   btn_tac        = (Button*) FindControl(116);

   if (btn_start) {
      REGISTER_CLIENT(EID_CLICK,       btn_start, MenuDlg, OnStart);
      REGISTER_CLIENT(EID_MOUSE_ENTER, btn_start, MenuDlg, OnButtonEnter);
      REGISTER_CLIENT(EID_MOUSE_EXIT,  btn_start, MenuDlg, OnButtonExit);
   }

   if (btn_campaign) {
      REGISTER_CLIENT(EID_CLICK,       btn_campaign, MenuDlg, OnCampaign);
      REGISTER_CLIENT(EID_MOUSE_ENTER, btn_campaign, MenuDlg, OnButtonEnter);
      REGISTER_CLIENT(EID_MOUSE_EXIT,  btn_campaign, MenuDlg, OnButtonExit);
   }

   if (btn_mission) {
      REGISTER_CLIENT(EID_CLICK,       btn_mission, MenuDlg, OnMission);
      REGISTER_CLIENT(EID_MOUSE_ENTER, btn_mission, MenuDlg, OnButtonEnter);
      REGISTER_CLIENT(EID_MOUSE_EXIT,  btn_mission, MenuDlg, OnButtonExit);
   }

   if (btn_player) {
      REGISTER_CLIENT(EID_CLICK,       btn_player, MenuDlg, OnPlayer);
      REGISTER_CLIENT(EID_MOUSE_ENTER, btn_player, MenuDlg, OnButtonEnter);
      REGISTER_CLIENT(EID_MOUSE_EXIT,  btn_player, MenuDlg, OnButtonExit);
   }

   if (btn_multi) {
      REGISTER_CLIENT(EID_CLICK,       btn_multi,  MenuDlg, OnMultiplayer);
      REGISTER_CLIENT(EID_MOUSE_ENTER, btn_multi,  MenuDlg, OnButtonEnter);
      REGISTER_CLIENT(EID_MOUSE_EXIT,  btn_multi,  MenuDlg, OnButtonExit);
   }

   if (btn_video) {
      REGISTER_CLIENT(EID_CLICK,       btn_video, MenuDlg, OnVideo);
      REGISTER_CLIENT(EID_MOUSE_ENTER, btn_video, MenuDlg, OnButtonEnter);
      REGISTER_CLIENT(EID_MOUSE_EXIT,  btn_video, MenuDlg, OnButtonExit);
   }

   if (btn_options) {
      REGISTER_CLIENT(EID_CLICK,       btn_options, MenuDlg, OnOptions);
      REGISTER_CLIENT(EID_MOUSE_ENTER, btn_options, MenuDlg, OnButtonEnter);
      REGISTER_CLIENT(EID_MOUSE_EXIT,  btn_options, MenuDlg, OnButtonExit);
   }

   if (btn_controls) {
      REGISTER_CLIENT(EID_CLICK,       btn_controls, MenuDlg, OnControls);
      REGISTER_CLIENT(EID_MOUSE_ENTER, btn_controls, MenuDlg, OnButtonEnter);
      REGISTER_CLIENT(EID_MOUSE_EXIT,  btn_controls, MenuDlg, OnButtonExit);
   }

   if (btn_mod) {
      REGISTER_CLIENT(EID_CLICK,       btn_mod, MenuDlg, OnMod);
      REGISTER_CLIENT(EID_MOUSE_ENTER, btn_mod, MenuDlg, OnButtonEnter);
      REGISTER_CLIENT(EID_MOUSE_EXIT,  btn_mod, MenuDlg, OnButtonExit);
   }

   if (btn_tac) {
      REGISTER_CLIENT(EID_CLICK,       btn_tac, MenuDlg, OnTacReference);
      REGISTER_CLIENT(EID_MOUSE_ENTER, btn_tac, MenuDlg, OnButtonEnter);
      REGISTER_CLIENT(EID_MOUSE_EXIT,  btn_tac, MenuDlg, OnButtonExit);
   }

   if (btn_quit) {
      REGISTER_CLIENT(EID_CLICK,       btn_quit, MenuDlg, OnQuit);
      REGISTER_CLIENT(EID_MOUSE_ENTER, btn_quit, MenuDlg, OnButtonEnter);
      REGISTER_CLIENT(EID_MOUSE_EXIT,  btn_quit, MenuDlg, OnButtonExit);
   }

   version     = FindControl(100);

   if (version) {
      version->SetText(versionInfo);
   }

   description = FindControl(202);
}

// +--------------------------------------------------------------------+

void
MenuDlg::Show()
{
   FormWindow::Show();

   if (btn_multi && Starshatter::UseFileSystem())
      btn_multi->SetEnabled(false);
}

void
MenuDlg::ExecFrame()
{
}

// +--------------------------------------------------------------------+

void
MenuDlg::OnStart(AWEvent* event)
{
   if (description) description->SetText("");
   stars->StartOrResumeGame();
}

void
MenuDlg::OnCampaign(AWEvent* event)
{
   if (description) description->SetText("");
   manager->ShowCmpSelectDlg();
}

void
MenuDlg::OnMission(AWEvent* event)
{
   if (description) description->SetText("");
   manager->ShowMsnSelectDlg();
}

void
MenuDlg::OnPlayer(AWEvent* event)
{
   if (description) description->SetText("");
   manager->ShowPlayerDlg();
}

void
MenuDlg::OnMultiplayer(AWEvent* event)
{
   if (description) description->SetText("");
   manager->ShowNetClientDlg();
}

void
MenuDlg::OnVideo(AWEvent* event)
{
   if (description) description->SetText("");
   manager->ShowVidDlg();
}

void
MenuDlg::OnOptions(AWEvent* event)
{
   if (description) description->SetText("");
   manager->ShowOptDlg();
}

void
MenuDlg::OnControls(AWEvent* event)
{
   if (description) description->SetText("");
   manager->ShowCtlDlg();
}

void
MenuDlg::OnMod(AWEvent* event)
{
   if (description) description->SetText("");
   manager->ShowModDlg();
}

void
MenuDlg::OnTacReference(AWEvent* event)
{
   if (description) description->SetText("");
   stars->OpenTacticalReference();
}

void
MenuDlg::OnQuit(AWEvent* event)
{
   if (description) description->SetText("");
   manager->ShowExitDlg();
}

// +--------------------------------------------------------------------+

void
MenuDlg::OnButtonEnter(AWEvent* event)
{
   ActiveWindow* src = event->window;

   if (src && description)
      description->SetText(src->GetAltText());
}

void
MenuDlg::OnButtonExit(AWEvent* event)
{
   if (description)
      description->SetText("");
}
