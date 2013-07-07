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
    FILE:         CmdMissionsDlg.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Operational Command Dialog (Mission List Tab)
*/

#include "MemDebug.h"
#include "CmdMissionsDlg.h"
#include "CmpnScreen.h"
#include "Starshatter.h"
#include "Campaign.h"
#include "Combatant.h"
#include "CombatAssignment.h"
#include "CombatGroup.h"
#include "CombatUnit.h"
#include "CombatZone.h"
#include "Mission.h"
#include "ShipDesign.h"
#include "Player.h"

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

DEF_MAP_CLIENT(CmdMissionsDlg, OnMode);
DEF_MAP_CLIENT(CmdMissionsDlg, OnSave);
DEF_MAP_CLIENT(CmdMissionsDlg, OnExit);
DEF_MAP_CLIENT(CmdMissionsDlg, OnMission);
DEF_MAP_CLIENT(CmdMissionsDlg, OnAccept);

// +--------------------------------------------------------------------+

CmdMissionsDlg::CmdMissionsDlg(Screen* s, FormDef& def, CmpnScreen* mgr)
    : FormWindow(s, 0, 0, s->Width(), s->Height()), CmdDlg(mgr), manager(mgr),
      lst_missions(0), txt_desc(0), btn_accept(0),
      stars(0), campaign(0), mission(0)
{
    stars    = Starshatter::GetInstance();
    campaign = Campaign::GetCampaign();

    Init(def);
}

CmdMissionsDlg::~CmdMissionsDlg()
{
}

// +--------------------------------------------------------------------+

void
CmdMissionsDlg::RegisterControls()
{
    lst_missions   = (ListBox*)   FindControl(401);
    txt_desc       =              FindControl(402);
    btn_accept     = (Button*)    FindControl(403);

    RegisterCmdControls(this);

    if (btn_save)
    REGISTER_CLIENT(EID_CLICK,  btn_save,    CmdMissionsDlg, OnSave);

    if (btn_exit)
    REGISTER_CLIENT(EID_CLICK,  btn_exit,    CmdMissionsDlg, OnExit);

    for (int i = 0; i < 5; i++) {
        if (btn_mode[i])
        REGISTER_CLIENT(EID_CLICK,  btn_mode[i], CmdMissionsDlg, OnMode);
    }

    if (lst_missions) {
        REGISTER_CLIENT(EID_SELECT, lst_missions, CmdMissionsDlg, OnMission);
    }

    if (btn_accept) {
        btn_accept->SetEnabled(false);
        REGISTER_CLIENT(EID_CLICK, btn_accept, CmdMissionsDlg, OnAccept);
    }
}

// +--------------------------------------------------------------------+

void
CmdMissionsDlg::Show()
{
    mode = MODE_MISSIONS;

    FormWindow::Show();
    ShowCmdDlg();

    campaign = Campaign::GetCampaign();

    if (campaign) {
        if (lst_missions) {
            lst_missions->ClearItems();

            Player*            player   = Player::GetCurrentPlayer();
            List<MissionInfo>& missions = campaign->GetMissionList();
            for (int i = 0; i < missions.size(); i++) {
                MissionInfo* info = missions[i];
                lst_missions->AddItemWithData(info->name, info->id);

                Mission* m = info->mission;
                if (m) {
                    if (m->Type() == Mission::TRAINING && player->HasTrained(m->Identity())) {
                        lst_missions->SetItemText(i, 1, Game::GetText("CmdMissionsDlg.training"));
                    }
                    else {
                        lst_missions->SetItemText(i, 1, m->TypeName());
                    }
                }

                char start_time[64];
                FormatDayTime(start_time, info->start);
                lst_missions->SetItemText(i, 2, start_time);
            }
        }
    }
}

void
CmdMissionsDlg::ExecFrame()
{
    CmdDlg::ExecFrame();

    if (campaign) {
        List<MissionInfo>& missions = campaign->GetMissionList();
        Player*            player   = Player::GetCurrentPlayer();

        if (missions.size() > lst_missions->NumItems()) {
            while (missions.size() > lst_missions->NumItems()) {
                MissionInfo* info = missions[lst_missions->NumItems()];
                int i = lst_missions->AddItemWithData(info->name, info->id) - 1;

                Mission* m = info->mission;
                if (m) {
                    if (m->Type() == Mission::TRAINING && player->HasTrained(m->Identity())) {
                        lst_missions->SetItemText(i, 1, Game::GetText("CmdMissionsDlg.training"));
                    }
                    else {
                        lst_missions->SetItemText(i, 1, m->TypeName());
                    }
                }

                char start_time[64];
                FormatDayTime(start_time, info->start);
                lst_missions->SetItemText(i, 2, start_time);
            }
        }

        else if (missions.size() < lst_missions->NumItems()) {
            lst_missions->ClearItems();

            for (int i = 0; i < missions.size(); i++) {
                MissionInfo* info = missions[i];
                lst_missions->AddItemWithData(info->name, info->id);

                Mission* m = info->mission;
                if (m) {
                    if (m->Type() == Mission::TRAINING && player->HasTrained(m->Identity())) {
                        lst_missions->SetItemText(i, 1, Game::GetText("CmdMissionsDlg.training"));
                    }
                    else {
                        lst_missions->SetItemText(i, 1, m->TypeName());
                    }
                }

                char start_time[64];
                FormatDayTime(start_time, info->start);
                lst_missions->SetItemText(i, 2, start_time);
            }
        }

        else if (missions.size() > 0 && lst_missions->NumItems() > 0) {
            int id = lst_missions->GetItemData(0);
            MissionInfo* info = campaign->GetMissionInfo(id);

            if (!info) {
                int seln    = -1;
                int seln_id = 0;

                for (int i = 0; i < lst_missions->NumItems(); i++)
                if (lst_missions->IsSelected(i))
                seln = i;
                
                if (seln >= 0)
                seln_id = lst_missions->GetItemData(seln);

                lst_missions->ClearItems();
                seln = -1;

                for (int i = 0; i < missions.size(); i++) {
                    MissionInfo* info = missions[i];
                    lst_missions->AddItemWithData(info->name, info->id);

                    Mission* m = info->mission;
                    if (m) {
                        if (m->Type() == Mission::TRAINING && player->HasTrained(m->Identity())) {
                            lst_missions->SetItemText(i, 1, Game::GetText("CmdMissionsDlg.training"));
                        }
                        else {
                            lst_missions->SetItemText(i, 1, m->TypeName());
                        }
                    }

                    char start_time[64];
                    FormatDayTime(start_time, info->start);
                    lst_missions->SetItemText(i, 2, start_time);

                    if (info->id == seln_id)
                    seln = i;
                }

                if (seln >= 0)
                lst_missions->SetSelected(seln);
            }
        }

        bool found = false;

        for (int i = 0; i < missions.size() && !found; i++) {
            MissionInfo* info = missions[i];
            if (info->mission == mission)
            found = true;
        }

        if (!found) {
            mission = 0;
            txt_desc->SetText("");
            btn_accept->SetEnabled(false);
        }
    }
}

// +--------------------------------------------------------------------+

void
CmdMissionsDlg::OnSave(AWEvent* event)
{
    CmdDlg::OnSave(event);
}

void
CmdMissionsDlg::OnExit(AWEvent* event)
{
    CmdDlg::OnExit(event);
}

void
CmdMissionsDlg::OnMode(AWEvent* event)
{
    CmdDlg::OnMode(event);
}


// +--------------------------------------------------------------------+

void
CmdMissionsDlg::OnMission(AWEvent* event)
{
    if (campaign && lst_missions) {
        MissionInfo* info = 0;
        mission = 0;

        for (int i = 0; i < lst_missions->NumItems(); i++) {
            if (lst_missions->IsSelected(i)) {
                int id   = lst_missions->GetItemData(i);
                info     = campaign->GetMissionInfo(id);
            }
        }

        btn_accept->SetEnabled((info != 0) ? true : false);

        if (info) {
            Text desc("<font Limerick12><color ffff80>");
            desc += info->name;
            desc += "<font Verdana><color ffffff>\n\n";
            desc += info->player_info;
            desc += "\n\n";
            desc += info->description;

            txt_desc->SetText(desc);
            mission = info->mission;
        }
        else {
            txt_desc->SetText(" ");
        }
    }
}

// +--------------------------------------------------------------------+

void
CmdMissionsDlg::OnAccept(AWEvent* event)
{
    if (!campaign || !mission) {
        ::Print("  ERROR CMD::Accept campaign=0x%08x mission=0x%08x\n", campaign, mission);
        return;
    }

    ::Print("  CMD::Accept Mission %d\n", mission->Identity());

    Mouse::Show(false);
    campaign->SetMissionId(mission->Identity());
    campaign->StartMission();
    stars->SetGameMode(Starshatter::PREP_MODE);
}
