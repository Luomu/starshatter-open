/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         CmpFileDlg.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Mission Select Dialog Active Window class
*/

#include "MemDebug.h"
#include "CmpFileDlg.h"
#include "CmpnScreen.h"
#include "Starshatter.h"
#include "Campaign.h"
#include "CampaignSaveGame.h"
#include "CombatGroup.h"

#include "Game.h"
#include "DataLoader.h"
#include "Button.h"
#include "EditBox.h"
#include "ListBox.h"
#include "Slider.h"
#include "Video.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "FormatUtil.h"
#include "ParseUtil.h"

// +--------------------------------------------------------------------+
// DECLARE MAPPING FUNCTIONS:

DEF_MAP_CLIENT(CmpFileDlg, OnSave);
DEF_MAP_CLIENT(CmpFileDlg, OnCancel);
DEF_MAP_CLIENT(CmpFileDlg, OnCampaign);

// +--------------------------------------------------------------------+

CmpFileDlg::CmpFileDlg(Screen* s, FormDef& def, CmpnScreen* mgr)
   : FormWindow(s, 0, 0, s->Width(), s->Height()), manager(mgr),
     exit_latch(false), btn_save(0), btn_cancel(0), edt_name(0), lst_campaigns(0)
{
   Init(def);
}

CmpFileDlg::~CmpFileDlg()
{
}

// +--------------------------------------------------------------------+

void
CmpFileDlg::RegisterControls()
{
   btn_save    = (Button*) FindControl(1);
   btn_cancel  = (Button*) FindControl(2);

   if (btn_save)
      REGISTER_CLIENT(EID_CLICK, btn_save,   CmpFileDlg, OnSave);

   if (btn_cancel)
      REGISTER_CLIENT(EID_CLICK, btn_cancel, CmpFileDlg, OnCancel);

   edt_name    = (EditBox*) FindControl(200);
   
   if (edt_name)
      edt_name->SetText("");

   lst_campaigns = (ListBox*) FindControl(201);

   if (lst_campaigns)
      REGISTER_CLIENT(EID_SELECT, lst_campaigns, CmpFileDlg, OnCampaign);
}

// +--------------------------------------------------------------------+

void
CmpFileDlg::Show()
{
   FormWindow::Show();

   if (lst_campaigns) {
      lst_campaigns->ClearItems();
      lst_campaigns->SetLineHeight(12);

      List<Text>  save_list;

      CampaignSaveGame::GetSaveGameList(save_list);
      save_list.sort();

      for (int i = 0; i < save_list.size(); i++)
         lst_campaigns->AddItem(*save_list[i]);

      save_list.destroy();
   }

   if (edt_name) {
      char save_name[256];
      save_name[0] = 0;

      campaign = Campaign::GetCampaign();
      if (campaign && campaign->GetPlayerGroup()) {
         const char*  op_name = campaign->Name();
         char         day[32];
         CombatGroup* group   = campaign->GetPlayerGroup();

         if (strstr(op_name, "Operation "))
            op_name += 10;

         FormatDay(day, campaign->GetTime());

         sprintf(save_name, "%s %s (%s)",
            op_name,
            day,
            group->GetRegion().data());
      }

      edt_name->SetText(save_name);
      edt_name->SetFocus();
   }
}

// +--------------------------------------------------------------------+

void
CmpFileDlg::ExecFrame()
{
   if (Keyboard::KeyDown(VK_RETURN)) {
      OnSave(0);
   }

   if (Keyboard::KeyDown(VK_ESCAPE)) {
      if (!exit_latch)
         OnCancel(0);

      exit_latch = true;
   }
   else {
      exit_latch = false;
   }
}

// +--------------------------------------------------------------------+

void
CmpFileDlg::OnSave(AWEvent* event)
{
   if (edt_name && edt_name->GetText().length() > 0) {
      campaign = Campaign::GetCampaign();
      CampaignSaveGame save(campaign);

      char filename[256];
      strcpy(filename, edt_name->GetText());
      char* newline = strchr(filename, '\n');
      if (newline)
         *newline = 0;

      save.Save(filename);
      save.SaveAuto();

      if (manager)
         manager->HideCmpFileDlg();
   }
}

void
CmpFileDlg::OnCancel(AWEvent* event)
{
   if (manager)
      manager->HideCmpFileDlg();
}

// +--------------------------------------------------------------------+

void
CmpFileDlg::OnCampaign(AWEvent* event)
{
   int n = lst_campaigns->GetSelection();

   if (n >= 0) {
      Text cmpn = lst_campaigns->GetItemText(n);

      if (edt_name)
         edt_name->SetText(cmpn);
   }
}