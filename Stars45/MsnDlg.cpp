/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         MsnDlg.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Mission Briefing Dialog Active Window class
*/

#include "MemDebug.h"
#include "MsnDlg.h"
#include "PlanScreen.h"
#include "Starshatter.h"
#include "Campaign.h"
#include "Mission.h"
#include "Instruction.h"
#include "Ship.h"
#include "ShipDesign.h"
#include "StarSystem.h"

#include "NetLobby.h"

#include "Game.h"
#include "FormWindow.h"
#include "FormatUtil.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "Button.h"
#include "ListBox.h"
#include "Slider.h"
#include "ParseUtil.h"

// +--------------------------------------------------------------------+

MsnDlg::MsnDlg(PlanScreen* mgr)
   : plan_screen(mgr),
     commit(0), cancel(0), campaign(0), mission(0),
     pkg_index(-1), info(0)
{
   campaign = Campaign::GetCampaign();

   if (campaign)
      mission = campaign->GetMission();

   mission_name               = 0;
   mission_system             = 0;
   mission_sector             = 0;
   mission_time_start         = 0;
   mission_time_target        = 0;
   mission_time_target_label  = 0;

   sit_button                 = 0;
   pkg_button                 = 0;
   nav_button                 = 0;
   wep_button                 = 0;
   commit                     = 0;
   cancel                     = 0;
}

MsnDlg::~MsnDlg()
{
}

// +--------------------------------------------------------------------+

void
MsnDlg::RegisterMsnControls(FormWindow* win)
{
   mission_name               = win->FindControl(200);
   mission_system             = win->FindControl(202);
   mission_sector             = win->FindControl(204);
   mission_time_start         = win->FindControl(206);
   mission_time_target        = win->FindControl(208);
   mission_time_target_label  = win->FindControl(207);

   sit_button                 = (Button*) win->FindControl(900);
   pkg_button                 = (Button*) win->FindControl(901);
   nav_button                 = (Button*) win->FindControl(902);
   wep_button                 = (Button*) win->FindControl(903);
   commit                     = (Button*) win->FindControl(1);
   cancel                     = (Button*) win->FindControl(2);
}

// +--------------------------------------------------------------------+

void
MsnDlg::ShowMsnDlg()
{
   campaign = Campaign::GetCampaign();

   mission   = 0;
   pkg_index = -1;

   if (campaign) {
      mission = campaign->GetMission();

      if (!mission)
         ::Print("ERROR - MsnDlg::Show() no mission.\n");
      else
         ::Print("MsnDlg::Show() mission id = %d name = '%s'\n",
                  mission->Identity(),
                  mission->Name());
   }
   else {
      ::Print("ERROR - MsnDlg::Show() no campaign.\n");
   }

   if (mission_name) {
      if (mission)
         mission_name->SetText(mission->Name());
      else
         mission_name->SetText(Game::GetText("MsnDlg.no-mission"));
   }

   if (mission_system) {
      mission_system->SetText("");

      if (mission) {
         StarSystem* sys = mission->GetStarSystem();

         if (sys)
            mission_system->SetText(sys->Name());
      }
   }

   if (mission_sector) {
      mission_sector->SetText("");

      if (mission) {
         mission_sector->SetText(mission->GetRegion());
      }
   }

   if (mission_time_start) {
      if (mission) {
         char txt[32];
         FormatDayTime(txt, mission->Start());
         mission_time_start->SetText(txt);
      }
   }

   if (mission_time_target) {
      int time_on_target = CalcTimeOnTarget();

      if (time_on_target) {
         char txt[32];
         FormatDayTime(txt, time_on_target);
         mission_time_target->SetText(txt);
         mission_time_target_label->SetText(Game::GetText("MsnDlg.target"));
      }
      else {
         mission_time_target->SetText("");
         mission_time_target_label->SetText("");
      }
   }


   if (sit_button) {
      sit_button->SetButtonState(plan_screen->IsMsnObjShown());
      sit_button->SetEnabled(true);
   }

   if (pkg_button) {
      pkg_button->SetButtonState(plan_screen->IsMsnPkgShown());
      pkg_button->SetEnabled(true);
   }

   if (nav_button) {
      nav_button->SetButtonState(plan_screen->IsNavShown());
      nav_button->SetEnabled(true);
   }

   if (wep_button) {
      wep_button->SetButtonState(plan_screen->IsMsnWepShown());
      wep_button->SetEnabled(true);
   }

   bool mission_ok = true;

   if (!mission || !mission->IsOK()) {
      mission_ok = false;

      if (sit_button) sit_button->SetEnabled(false);
      if (pkg_button) pkg_button->SetEnabled(false);
      if (nav_button) nav_button->SetEnabled(false);
      if (wep_button) wep_button->SetEnabled(false);
   }
   else {
      MissionElement* player_elem = mission->GetPlayer();

      if (wep_button && player_elem)
         wep_button->SetEnabled(player_elem->Loadouts().size() > 0);
   }

   if (wep_button && NetLobby::GetInstance())
      wep_button->SetEnabled(false);

   commit->SetEnabled(mission_ok);
   cancel->SetEnabled(true);
}

// +--------------------------------------------------------------------+

int
MsnDlg::CalcTimeOnTarget()
{
   if (mission) {
      MissionElement* element = mission->GetElements()[0];
      if (element) {
         Point    loc = element->Location();
         loc.SwapYZ();  // navpts use Z for altitude, element loc uses Y for altitude.

         int mission_time = mission->Start();

         int i = 0;
         ListIter<Instruction> navpt = element->NavList();
         while (++navpt) {
            int action = navpt->Action();

            double dist = Point(loc - navpt->Location()).length();

            int etr     = 0;
            
            if (navpt->Speed() > 0)
               etr = (int) (dist / navpt->Speed());
            else
               etr = (int) (dist / 500);

            mission_time += etr;

            loc = navpt->Location();
            i++;

            if (action >= Instruction::ESCORT) {   // this is the target!
               return mission_time;
            }
         }
      }
   }

   return 0;
}

// +--------------------------------------------------------------------+

void
MsnDlg::OnTabButton(AWEvent* event)
{
   if (event->window == sit_button) {
      plan_screen->ShowMsnObjDlg();
   }

   if (event->window == pkg_button) {
      plan_screen->ShowMsnPkgDlg();
   }

   if (event->window == nav_button) {
      plan_screen->ShowNavDlg();
   }

   if (event->window == wep_button) {
      plan_screen->ShowMsnWepDlg();
   }
}

// +--------------------------------------------------------------------+

void
MsnDlg::OnCommit(AWEvent* event)
{
   Starshatter* stars = Starshatter::GetInstance();

   if (stars) {
      Mouse::Show(false);
      stars->SetGameMode(Starshatter::LOAD_MODE);
   }

   else
      Game::Panic("MsnDlg::OnCommit() - Game instance not found");
}

void
MsnDlg::OnCancel(AWEvent* event)
{
   Starshatter* stars = Starshatter::GetInstance();

   if (stars) {
      Mouse::Show(false);

      if (campaign && (campaign->IsDynamic() || campaign->IsTraining()))
         stars->SetGameMode(Starshatter::CMPN_MODE);
      else
         stars->SetGameMode(Starshatter::MENU_MODE);
   }

   else
      Game::Panic("MsnDlg::OnCancel() - Game instance not found");
}
