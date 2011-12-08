/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         CmdIntelDlg.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Operational Command Dialog (Intel/Newsfeed Tab)
*/

#include "MemDebug.h"
#include "CmdIntelDlg.h"
#include "CmpnScreen.h"
#include "Starshatter.h"
#include "Campaign.h"
#include "Combatant.h"
#include "CombatEvent.h"
#include "CombatGroup.h"
#include "CombatUnit.h"
#include "ShipDesign.h"
#include "Starshatter.h"
#include "Sim.h"
#include "CameraDirector.h"

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

DEF_MAP_CLIENT(CmdIntelDlg, OnMode);
DEF_MAP_CLIENT(CmdIntelDlg, OnSave);
DEF_MAP_CLIENT(CmdIntelDlg, OnExit);
DEF_MAP_CLIENT(CmdIntelDlg, OnNews);
DEF_MAP_CLIENT(CmdIntelDlg, OnPlay);

// +--------------------------------------------------------------------+

CmdIntelDlg::CmdIntelDlg(Screen* s, FormDef& def, CmpnScreen* mgr)
   : FormWindow(s, 0, 0, s->Width(), s->Height()), CmdDlg(mgr), manager(mgr),
     stars(0), campaign(0), update_time(0), start_scene(0),
     cam_view(0), dsp_view(0)
{
   stars    = Starshatter::GetInstance();
   campaign = Campaign::GetCampaign();

   if (campaign)
      update_time = campaign->GetUpdateTime();

   Init(def);
}

CmdIntelDlg::~CmdIntelDlg()
{
}

// +--------------------------------------------------------------------+

void
CmdIntelDlg::RegisterControls()
{
   lst_news = (ListBox*)      FindControl(401);
   txt_news = (RichTextBox*)  FindControl(402);
   img_news = (ImageBox*)     FindControl(403);
   mov_news =                 FindControl(404);
   btn_play = (Button*)       FindControl(405);

   RegisterCmdControls(this);

   if (btn_save)
      REGISTER_CLIENT(EID_CLICK,  btn_save,    CmdIntelDlg, OnSave);

   if (btn_exit)
      REGISTER_CLIENT(EID_CLICK,  btn_exit,    CmdIntelDlg, OnExit);

   if (btn_play)
      REGISTER_CLIENT(EID_CLICK,  btn_play,    CmdIntelDlg, OnPlay);

   for (int i = 0; i < 5; i++) {
      if (btn_mode[i])
      REGISTER_CLIENT(EID_CLICK,  btn_mode[i], CmdIntelDlg, OnMode);
   }

   if (lst_news) {
      REGISTER_CLIENT(EID_SELECT, lst_news,    CmdIntelDlg, OnNews);
   }

   if (img_news) {
      img_news->GetPicture(bmp_default);
   }

   if (mov_news) {
      CameraDirector* cam_dir = CameraDirector::GetInstance();
      cam_view = new(__FILE__,__LINE__) CameraView(mov_news, cam_dir->GetCamera(), 0);
      if (cam_view)
         mov_news->AddView(cam_view);

      dsp_view = DisplayView::GetInstance();
      if (dsp_view) {
         dsp_view->SetWindow(mov_news);
         mov_news->AddView(dsp_view);
      }

      mov_news->Hide();
   }
}

// +--------------------------------------------------------------------+

void
CmdIntelDlg::Show()
{
   mode = MODE_INTEL;

   FormWindow::Show();
   ShowCmdDlg();

   if (btn_play)
      btn_play->Hide();

   if (mov_news)
      mov_news->Hide();
}

// +--------------------------------------------------------------------+

void
CmdIntelDlg::ExecFrame()
{
   CmdDlg::ExecFrame();

   if (campaign != Campaign::GetCampaign() || campaign->GetUpdateTime() != update_time) {
      campaign    = Campaign::GetCampaign();
      update_time = campaign->GetUpdateTime();

      lst_news->ClearItems();
      txt_news->SetText("");

      if (img_news)
         img_news->SetPicture(bmp_default);
   }

   if (campaign) {
      List<CombatEvent>& events      = campaign->GetEvents();
      bool               auto_scroll = false;

      if (events.size() > lst_news->NumItems()) {
         while (events.size() > lst_news->NumItems()) {
            CombatEvent* info = events[lst_news->NumItems()];

            const char* unread = info->Visited() ? " " : "*";
            int i = lst_news->AddItemWithData(unread, (DWORD) info) - 1;

            char dateline[32];
            FormatDayTime(dateline, info->Time());
            lst_news->SetItemText(i, 1, dateline);
            lst_news->SetItemText(i, 2, info->Title());
            lst_news->SetItemText(i, 3, info->Region());
            lst_news->SetItemText(i, 4, Game::GetText(info->SourceName()));

            if (!info->Visited())
               auto_scroll = true;
         }

         if (lst_news->GetSortColumn() > 0)
            lst_news->SortItems();
      }

      else if (events.size() < lst_news->NumItems()) {
         lst_news->ClearItems();

         for (int i = 0; i < events.size(); i++) {
            CombatEvent* info = events[i];

            const char* unread = info->Visited() ? " " : "*";
            int j = lst_news->AddItemWithData(unread, (DWORD) info) - 1;

            char dateline[32];
            FormatDayTime(dateline, info->Time());
            lst_news->SetItemText(j, 1, dateline);
            lst_news->SetItemText(j, 2, info->Title());
            lst_news->SetItemText(j, 3, info->Region());
            lst_news->SetItemText(j, 4, Game::GetText(info->SourceName()));

            if (!info->Visited())
               auto_scroll = true;
         }

         if (lst_news->GetSortColumn() > 0)
            lst_news->SortItems();

         txt_news->SetText("");

         if (img_news)
            img_news->SetPicture(bmp_default);
      }

      if (auto_scroll) {
         int first_unread = -1;

         for (int i = 0; i < lst_news->NumItems(); i++) {
            if (lst_news->GetItemText(i, 0) == "*") {
               first_unread = i;
               break;
            }
         }

         if (first_unread >= 0)
            lst_news->ScrollTo(first_unread);
      }
   }

   Starshatter* stars = Starshatter::GetInstance();

   if (start_scene > 0) {
      ShowMovie();

      start_scene--;

      if (start_scene == 0) {
         if (stars && campaign) {
            stars->ExecCutscene(event_scene, campaign->Path());

            if (stars->InCutscene()) {
               Sim* sim = Sim::GetSim();

               if (sim) {
                  cam_view->UseCamera(CameraDirector::GetInstance()->GetCamera());
                  cam_view->UseScene(sim->GetScene());
               }
            }
         }

         event_scene = "";
      }
   }
   else {
      if (dsp_view)
         dsp_view->ExecFrame();

      if (stars->InCutscene())
         ShowMovie();
      else
         HideMovie();
   }
}

// +--------------------------------------------------------------------+

void
CmdIntelDlg::OnSave(AWEvent* event)
{
   CmdDlg::OnSave(event);
}

void
CmdIntelDlg::OnExit(AWEvent* event)
{
   CmdDlg::OnExit(event);
}

void
CmdIntelDlg::OnMode(AWEvent* event)
{
   CmdDlg::OnMode(event);
}


// +--------------------------------------------------------------------+

void
CmdIntelDlg::OnNews(AWEvent* e)
{
   CombatEvent* event = 0;
   int          index = lst_news->GetSelection();

   if (index >= 0) {
      event = (CombatEvent*) lst_news->GetItemData(index, 0);
   }

   if (event) {
      Text info("<font Limerick12><color ffff80>");
      info += event->Title();
      info += "<font Verdana><color ffffff>\n\n";
      info += event->Information();

      txt_news->SetText(info);
      txt_news->EnsureVisible(0);

      lst_news->SetItemText(index, 0, " ");

      if (img_news) {
         if (event->Image().Width() >= 64)
            img_news->SetPicture(event->Image());
         else
            img_news->SetPicture(bmp_default);
      }

      if (btn_play) {
         if (event->SceneFile() && *event->SceneFile())
            btn_play->Show();
         else
            btn_play->Hide();
      }

      if (!event->Visited() && btn_play->IsEnabled())
         OnPlay(0);

      event->SetVisited(true);
   }
   else {
      txt_news->SetText("");

      if (img_news)
         img_news->SetPicture(bmp_default);

      if (btn_play)
         btn_play->Hide();
   }
}

// +--------------------------------------------------------------------+

void
CmdIntelDlg::OnPlay(AWEvent* e)
{
   CombatEvent* event = 0;
   int          index = lst_news->GetSelection();

   if (index >= 0) {
      event = (CombatEvent*) lst_news->GetItemData(index, 0);
   }

   if (mov_news && cam_view && event && event->SceneFile() && *event->SceneFile()) {
      event_scene = event->SceneFile();
      start_scene = 2;
      ShowMovie();
   }
}

void
CmdIntelDlg::ShowMovie()
{
   if (mov_news) {
      mov_news->Show();
      dsp_view->SetWindow(mov_news);

      if (img_news) img_news->Hide();
      if (txt_news) txt_news->Hide();
      if (btn_play) btn_play->Hide();
   }
}

void
CmdIntelDlg::HideMovie()
{
   CombatEvent* event = 0;
   int          index = lst_news->GetSelection();
   bool         play  = false;

   if (index >= 0) {
      event = (CombatEvent*) lst_news->GetItemData(index, 0);

      if (event && event->SceneFile() && *event->SceneFile())
         play = true;
   }

   if (mov_news) {
      mov_news->Hide();

      if (img_news) img_news->Show();
      if (txt_news) txt_news->Show();
      if (btn_play) {
         if (play)
            btn_play->Show();
         else
            btn_play->Hide();
      }
   }
}
