/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         CmdTheaterDlg.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Operational Command Dialog (Theater Map Tab)
*/

#include "MemDebug.h"
#include "CmdTheaterDlg.h"
#include "CmdDlg.h"
#include "CmpnScreen.h"
#include "Galaxy.h"
#include "Starshatter.h"
#include "StarSystem.h"
#include "Campaign.h"
#include "Combatant.h"
#include "CombatGroup.h"
#include "CombatUnit.h"
#include "ShipDesign.h"

#include "Game.h"
#include "DataLoader.h"
#include "Button.h"
#include "ComboBox.h"
#include "ListBox.h"
#include "Slider.h"
#include "Video.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "ParseUtil.h"
#include "FormatUtil.h"

// +--------------------------------------------------------------------+
// DECLARE MAPPING FUNCTIONS:

DEF_MAP_CLIENT(CmdTheaterDlg, OnMode);
DEF_MAP_CLIENT(CmdTheaterDlg, OnSave);
DEF_MAP_CLIENT(CmdTheaterDlg, OnExit);
DEF_MAP_CLIENT(CmdTheaterDlg, OnView);

// Supported Selection Modes:

const int SELECT_NONE      = -1;
const int SELECT_SYSTEM    =  0;
const int SELECT_PLANET    =  1;
const int SELECT_REGION    =  2;
const int SELECT_STATION   =  3;
const int SELECT_STARSHIP  =  4;
const int SELECT_FIGHTER   =  5;

const int VIEW_GALAXY      =  0;
const int VIEW_SYSTEM      =  1;
const int VIEW_REGION      =  2;

// +--------------------------------------------------------------------+

CmdTheaterDlg::CmdTheaterDlg(Screen* s, FormDef& def, CmpnScreen* mgr)
   : FormWindow(s, 0, 0, s->Width(), s->Height()), CmdDlg(mgr), manager(mgr),
     map_theater(0), map_view(0), stars(0), campaign(0)
{
   stars    = Starshatter::GetInstance();
   campaign = Campaign::GetCampaign();

   Init(def);
}

CmdTheaterDlg::~CmdTheaterDlg()
{
}

// +--------------------------------------------------------------------+

void
CmdTheaterDlg::RegisterControls()
{
   map_theater = FindControl(400);

   RegisterCmdControls(this);

   if (btn_save)
      REGISTER_CLIENT(EID_CLICK,  btn_save,    CmdTheaterDlg, OnSave);

   if (btn_exit)
      REGISTER_CLIENT(EID_CLICK,  btn_exit,    CmdTheaterDlg, OnExit);

   for (int i = 0; i < 5; i++) {
      if (btn_mode[i])
      REGISTER_CLIENT(EID_CLICK,  btn_mode[i], CmdTheaterDlg, OnMode);
   }

   if (map_theater)
      map_view = new(__FILE__,__LINE__) MapView(map_theater);

   for (i = 0; i < 3; i++) {
      view_btn[i] = (Button*) FindControl(401 + i);
      REGISTER_CLIENT(EID_CLICK, view_btn[i], CmdTheaterDlg, OnView);
   }

   zoom_in_btn  = (Button*) FindControl(410);
   zoom_out_btn = (Button*) FindControl(411);
}

// +--------------------------------------------------------------------+

void
CmdTheaterDlg::Show()
{
   mode = MODE_THEATER;

   FormWindow::Show();
   ShowCmdDlg();

   campaign = Campaign::GetCampaign();

   if (campaign && map_theater) {
      map_view->SetCampaign(campaign);
   }
}

// +--------------------------------------------------------------------+

void
CmdTheaterDlg::ExecFrame()
{
   CmdDlg::ExecFrame();

   if (!map_view)
      return;

   if (Keyboard::KeyDown(VK_ADD) ||
      (zoom_in_btn && zoom_in_btn->GetButtonState() > 0)) {
      map_view->ZoomIn();
   }
   else if (Keyboard::KeyDown(VK_SUBTRACT) || 
           (zoom_out_btn && zoom_out_btn->GetButtonState() > 0)) {
      map_view->ZoomOut();
   }

   else if (Mouse::Wheel() > 0) {
      map_view->ZoomIn();
      map_view->ZoomIn();
      map_view->ZoomIn();
   }

   else if (Mouse::Wheel() < 0) {
      map_view->ZoomOut();
      map_view->ZoomOut();
      map_view->ZoomOut();
   }
}

// +--------------------------------------------------------------------+

void
CmdTheaterDlg::OnSave(AWEvent* event)
{
   CmdDlg::OnSave(event);
}

void
CmdTheaterDlg::OnExit(AWEvent* event)
{
   CmdDlg::OnExit(event);
}

void
CmdTheaterDlg::OnMode(AWEvent* event)
{
   CmdDlg::OnMode(event);
}

// +--------------------------------------------------------------------+

void
CmdTheaterDlg::OnView(AWEvent* event)
{
   int use_filter_mode = -1;

   view_btn[VIEW_GALAXY]->SetButtonState(0);
   view_btn[VIEW_SYSTEM]->SetButtonState(0);
   view_btn[VIEW_REGION]->SetButtonState(0);

   if (view_btn[0] == event->window) {
      if (map_view) map_view->SetViewMode(VIEW_GALAXY);
      view_btn[VIEW_GALAXY]->SetButtonState(1);
      use_filter_mode = SELECT_SYSTEM;
   }
   
   else if (view_btn[VIEW_SYSTEM] == event->window) {
      if (map_view) map_view->SetViewMode(VIEW_SYSTEM);
      view_btn[VIEW_SYSTEM]->SetButtonState(1);
      use_filter_mode = SELECT_REGION;
   }
   
   else if (view_btn[VIEW_REGION] == event->window) {
      if (map_view) map_view->SetViewMode(VIEW_REGION);
      view_btn[VIEW_REGION]->SetButtonState(1);
      use_filter_mode = SELECT_STARSHIP;
   }

   if (use_filter_mode >= 0) {
      if (map_view) map_view->SetSelectionMode(use_filter_mode);
   }
}

// +--------------------------------------------------------------------+



