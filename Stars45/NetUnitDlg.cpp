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
    FILE:         NetUnitDlg.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Main Menu Dialog Active Window class
*/

#include "MemDebug.h"
#include "NetUnitDlg.h"
#include "NetClientConfig.h"
#include "ConfirmDlg.h"
#include "MenuScreen.h"
#include "Starshatter.h"
#include "Campaign.h"
#include "Mission.h"
#include "Ship.h"
#include "Player.h"
#include "Campaign.h"
#include "ShipDesign.h"

#include "NetAddr.h"
#include "NetLobbyClient.h"
#include "NetLobbyServer.h"
#include "NetUser.h"
#include "NetChat.h"

#include "DataLoader.h"
#include "Video.h"
#include "Keyboard.h"
#include "MachineInfo.h"

// +--------------------------------------------------------------------+
// DECLARE MAPPING FUNCTIONS:

DEF_MAP_CLIENT(NetUnitDlg, OnSelect);
DEF_MAP_CLIENT(NetUnitDlg, OnUnit);
DEF_MAP_CLIENT(NetUnitDlg, OnMap);
DEF_MAP_CLIENT(NetUnitDlg, OnUnMap);
DEF_MAP_CLIENT(NetUnitDlg, OnBan);
DEF_MAP_CLIENT(NetUnitDlg, OnBanConfirm);
DEF_MAP_CLIENT(NetUnitDlg, OnApply);
DEF_MAP_CLIENT(NetUnitDlg, OnCancel);

// +--------------------------------------------------------------------+

NetUnitDlg::NetUnitDlg(Screen* s, FormDef& def, MenuScreen* mgr)
: FormWindow(s, 0, 0, s->Width(), s->Height()), manager(mgr),
net_lobby(0), unit_index(-1)
{
    last_chat         = 0;
    host_mode         = false;

    Init(def);
}

NetUnitDlg::~NetUnitDlg()
{
}

// +--------------------------------------------------------------------+

void
NetUnitDlg::RegisterControls()
{
    lst_players = (ListBox*) FindControl(201);
    lst_units   = (ListBox*) FindControl(202);
    lst_chat    = (ListBox*) FindControl(211);
    edt_chat    = (EditBox*) FindControl(212);

    REGISTER_CLIENT(EID_SELECT, lst_units, NetUnitDlg, OnUnit);

    if (edt_chat)
    edt_chat->SetText("");

    btn_select  = (Button*) FindControl(206);
    REGISTER_CLIENT(EID_CLICK, btn_select, NetUnitDlg, OnSelect);

    btn_map     = (Button*) FindControl(203);
    REGISTER_CLIENT(EID_CLICK, btn_map, NetUnitDlg, OnMap);

    btn_unmap   = (Button*) FindControl(204);
    REGISTER_CLIENT(EID_CLICK, btn_unmap, NetUnitDlg, OnUnMap);

    btn_ban     = (Button*) FindControl(205);

    if (btn_ban) {
        REGISTER_CLIENT(EID_CLICK,    btn_ban, NetUnitDlg, OnBan);
        REGISTER_CLIENT(EID_USER_1,   btn_ban, NetUnitDlg, OnBanConfirm);
    }

    btn_apply   = (Button*) FindControl(1);
    REGISTER_CLIENT(EID_CLICK, btn_apply, NetUnitDlg, OnApply);

    btn_cancel  = (Button*) FindControl(2);
    REGISTER_CLIENT(EID_CLICK, btn_cancel, NetUnitDlg, OnCancel);
}

// +--------------------------------------------------------------------+

void
NetUnitDlg::Show()
{
    if (!IsShown()) {
        FormWindow::Show();

        // clear server data:
        if (lst_players) {
            lst_players->ClearItems();
            lst_players->SetSelectedStyle(ListBox::LIST_ITEM_STYLE_FILLED_BOX);
            lst_players->SetLeading(2);
        }

        if (lst_units) {
            lst_units->ClearItems();
            lst_units->SetSelectedStyle(ListBox::LIST_ITEM_STYLE_FILLED_BOX);
            lst_units->SetLeading(2);
        }

        if (lst_chat)  lst_chat->ClearItems();
        last_chat = 0;

        if (btn_apply)
        btn_apply->SetEnabled(false);

        net_lobby = NetLobby::GetInstance();
        host_mode = false;

        if (net_lobby) {
            host_mode = net_lobby->IsHost();
        }

        if (host_mode) {
            btn_select->Hide();
            btn_map->Show();
            btn_unmap->Show();
            btn_ban->Show();
        }
        else {
            btn_select->Show();
            btn_map->Hide();
            btn_unmap->Hide();
            btn_ban->Hide();
        }
    }
}

// +--------------------------------------------------------------------+

void
NetUnitDlg::ExecFrame()
{
    ExecLobbyFrame();

    if (!net_lobby)
    return;

    Text player_name;

    if (Player::GetCurrentPlayer())
    player_name = Player::GetCurrentPlayer()->Name();

    if (btn_select) {
        bool enable = false;

        if (lst_players && lst_units && btn_select->IsVisible()) {
            int sel_unit = lst_units->GetSelection();

            enable = sel_unit >= 0 && 
            lst_units->GetItemText(sel_unit).length() == 0;
        }

        btn_select->SetEnabled(enable);
    }

    if (btn_map) {
        bool enable = false;

        if (lst_players && lst_units && btn_map->IsVisible()) {
            int sel_play = lst_players->GetSelection();
            int sel_unit = lst_units->GetSelection();

            enable = sel_unit >= 0 && sel_play >= 0 && 
            lst_units->GetItemText(sel_unit).length() == 0;

            if (enable && !host_mode) {
                NetUser* u = (NetUser*) lst_players->GetItemData(sel_play);
                if (!u || u->Name() != player_name)
                enable = false;
            }
        }

        btn_map->SetEnabled(enable);
    }

    if (btn_unmap) {
        bool enable = false;

        if (lst_players && lst_units && btn_unmap->IsVisible()) {
            int sel_play = lst_players->GetSelection();
            int sel_unit = lst_units->GetSelection();

            enable = sel_unit >= 0 && lst_units->GetItemText(sel_unit).length() > 0;

            if (enable && !host_mode) {
                NetUser* u = (NetUser*) lst_units->GetItemData(sel_unit);
                if (!u || u->Name() != Player::GetCurrentPlayer()->Name())
                enable = false;
            }
        }

        btn_unmap->SetEnabled(enable);
    }

    if (btn_ban) {
        bool enable = false;

        if (lst_players && lst_units && host_mode && btn_ban->IsVisible()) {
            int sel_play = lst_players->GetSelection();
            int sel_unit = lst_units->GetSelection();

            enable = sel_play >= 0 && sel_unit < 0;

            if (enable) {
                NetUser* u = (NetUser*) lst_players->GetItemData(sel_play);
                if (u && u->Name() == player_name)
                enable = false;
            }
        }

        btn_ban->SetEnabled(enable);
    }

    if (btn_apply) {
        bool ok_to_start = net_lobby->IsMapped(player_name);

        NetUser* host = net_lobby->GetHost();
        if (host && !net_lobby->IsMapped(host->Name()))
        ok_to_start = false;

        btn_apply->SetEnabled(ok_to_start);
    }

    if (Keyboard::KeyDown(VK_RETURN)) {
        if (edt_chat && edt_chat->GetText().length() > 0) {
            SendChat(edt_chat->GetText());
            edt_chat->SetText("");
        }
    }

    CheckUnitMapping();
    GetChat();
    GetUnits();
    GetAvailable();
}

// +--------------------------------------------------------------------+

void
NetUnitDlg::ExecLobbyFrame()
{
    Starshatter* stars = Starshatter::GetInstance();

    if (!net_lobby) {
        manager->ShowNetClientDlg();
    }

    else if (net_lobby->GetLastError() != 0) {
        if (net_lobby->IsClient()) {
            if (stars)
            stars->StopLobby();

            net_lobby = 0;
            manager->ShowNetClientDlg();
        }
    }
}

// +--------------------------------------------------------------------+

static bool assignment_change = false;
static int  num_users = 0;

void
NetUnitDlg::CheckUnitMapping()
{
    if (net_lobby && lst_units) {
        ListIter<NetUnitEntry>  iter  = net_lobby->GetUnitMap();
        List<NetUnitEntry>&     units = iter.container();
        List<NetUser>&          users = net_lobby->GetUsers();

        if (users.size() != num_users) {
            assignment_change = true;
            num_users = users.size();
        }

        if (units.size() != lst_units->NumItems()) {
            assignment_change = true;
        }

        else if (lst_units->NumItems()) {
            for (int i = 0; i < units.size(); i++) {
                NetUnitEntry* e = units.at(i);
                Text user_name = e->GetUserName();
                NetUser* u = net_lobby->FindUserByName(user_name);

                if (lst_units->GetItemData(i) != (DWORD) u)
                assignment_change = true;
            }
        }
    }
}

void
NetUnitDlg::GetAvailable()
{
    if (!lst_players) return;

    Text player_name;

    if (Player::GetCurrentPlayer())
    player_name = Player::GetCurrentPlayer()->Name();

    if (net_lobby) {
        List<NetUser> available_users;

        NetUser* u = net_lobby->GetLocalUser();
        if (u) {
            bool assigned = false;
            ListIter<NetUnitEntry> iter = net_lobby->GetUnitMap();
            while (++iter) {
                NetUnitEntry* unit = iter.value();
                if (unit->GetUserName() == u->Name())
                assigned = true;
            }

            if (!assigned)
            available_users.append(u);
        }

        ListIter<NetUser> iter = net_lobby->GetUsers();
        while (++iter) {
            NetUser* u = iter.value();
            bool assigned = false;
            ListIter<NetUnitEntry> iter = net_lobby->GetUnitMap();
            while (++iter) {
                NetUnitEntry* unit = iter.value();
                if (unit->GetUserName() == u->Name())
                assigned = true;
            }

            if (!assigned) {
                available_users.append(u);
            }
        }

        if (available_users.size() != lst_players->NumItems()) {
            assignment_change = true;
            lst_players->ClearItems();

            for (int i = 0; i < available_users.size(); i++) {
                NetUser* u = available_users[i];

                Text name = Player::RankAbrv(u->Rank());
                name += " ";
                name += u->Name();

                lst_players->AddItemWithData(name.data(), (DWORD) u);

                if (!host_mode && u->Name() == player_name) {
                    lst_players->SetSelected(lst_players->NumItems()-1);
                }
            }
        }
    }
}

// +--------------------------------------------------------------------+

void
NetUnitDlg::GetUnits()
{
    if (!lst_units) return;

    if (net_lobby) {
        ListIter<NetUnitEntry>  iter  = net_lobby->GetUnitMap();
        List<NetUnitEntry>&     units = iter.container();
        List<NetUser>&          users = net_lobby->GetUsers();

        if (assignment_change) {
            lst_units->ClearItems();

            for (int i = 0; i < units.size(); i++) {
                NetUnitEntry* e = units.at(i);

                char name[64];
                char team[16];

                if (e->GetIndex())
                sprintf_s(name, "%s %d", e->GetElemName().data(), e->GetIndex());
                else
                strcpy_s(name, e->GetElemName().data());

                sprintf_s(team, "%d", e->GetIFF());

                Text user_name = e->GetUserName();

                NetUser* u = net_lobby->FindUserByName(user_name);
                if (u) {
                    user_name = Player::RankAbrv(u->Rank());
                    user_name += " ";
                    user_name += u->Name();
                }

                int count = lst_units->AddItemWithData(user_name, (DWORD) u);
                lst_units->SetItemText(count-1, 1, name);
                lst_units->SetItemText(count-1, 2, e->GetDesign());

                if (lst_units->NumColumns() > 4) {
                    lst_units->SetItemText(count-1, 3, Mission::RoleName(e->GetMissionRole()));
                    lst_units->SetItemText(count-1, 4, team);
                }
                else if (lst_units->NumColumns() > 3) {
                    lst_units->SetItemText(count-1, 3, team);
                }
            }
        }
    }

    assignment_change = false;
}

// +--------------------------------------------------------------------+

void
NetUnitDlg::GetChat()
{
    if (!lst_chat) return;

    if (net_lobby) {
        int  last_item = lst_chat->NumItems() - 1;
        int  count = 0;
        bool added = false;

        ListIter<NetChatEntry> iter = net_lobby->GetChat();
        while (++iter) {
            NetChatEntry* c = iter.value();

            if (count++ > last_item) {
                int n = lst_chat->AddItem(c->GetUser());
                lst_chat->SetItemText(n-1, 1, c->GetMessage());
                added = true;
            }
        }

        if (added)
        lst_chat->EnsureVisible(lst_chat->NumItems()+1);
    }
}


void
NetUnitDlg::SendChat(Text msg)
{
    if (msg.length() < 1) return;

    Player* player = Player::GetCurrentPlayer();

    if (msg[0] >= '0' && msg[0] <= '9') {
        if (player) {
            Text macro = player->ChatMacro(msg[0] - '0');

            if (macro.length())
            msg = macro;
        }
    }

    if (net_lobby)
    net_lobby->AddChat(0, msg);
}

// +--------------------------------------------------------------------+

void
NetUnitDlg::OnUnit(AWEvent* event)
{
    if (!lst_units || host_mode) return;

    static DWORD unit_click_time = 0;

    int list_index    = lst_units->GetListIndex();

    // double-click:
    if (list_index == unit_index && Game::RealTime() - unit_click_time < 350) {
        OnSelect(0);
    }

    unit_click_time   = Game::RealTime();
    unit_index        = list_index;
}

// +--------------------------------------------------------------------+

void
NetUnitDlg::OnSelect(AWEvent* event)
{
    if (!lst_players || !lst_units) return;

    Text player_name;

    if (Player::GetCurrentPlayer())
    player_name = Player::GetCurrentPlayer()->Name();

    int sel_unit   = lst_units->GetSelection();

    if (net_lobby) {
        net_lobby->MapUnit(sel_unit, player_name, host_mode);
        lst_units->ClearItems();
    }

    assignment_change = true;

    GetUnits();
    GetAvailable();
}

// +--------------------------------------------------------------------+

void
NetUnitDlg::OnMap(AWEvent* event)
{
    if (!lst_players || !lst_units) return;

    int sel_player = lst_players->GetSelection();
    int sel_unit   = lst_units->GetSelection();

    if (net_lobby) {
        NetUser* u = (NetUser*) lst_players->GetItemData(sel_player);
        net_lobby->MapUnit(sel_unit, u->Name(), host_mode);
        lst_units->ClearItems();
    }

    assignment_change = true;

    GetUnits();
    GetAvailable();
}

void
NetUnitDlg::OnUnMap(AWEvent* event)
{
    if (!lst_players || !lst_units) return;

    if (net_lobby) {
        net_lobby->MapUnit(lst_units->GetSelection(), 0, host_mode);
        lst_units->ClearItems();
    }

    assignment_change = true;

    GetUnits();
    GetAvailable();
}

void
NetUnitDlg::OnBan(AWEvent* event)
{
    if (!lst_players) return;

    int sel_player = lst_players->GetSelection();

    if (net_lobby) {
        NetUser* u = (NetUser*) lst_players->GetItemData(sel_player);

        ConfirmDlg* confirm = manager->GetConfirmDlg();
        if (confirm) {
            char msg[512];
            sprintf_s(msg, Game::GetText("NetUnitDlg.are-you-sure").data(), u->Name().data());
            confirm->SetMessage(msg);
            confirm->SetTitle(Game::GetText("NetUnitDlg.confirm-ban"));
            confirm->SetParentControl(btn_ban);

            manager->ShowConfirmDlg();
        }

        else {
            OnBanConfirm(event);
        }
    }
}

void
NetUnitDlg::OnBanConfirm(AWEvent* event)
{
    if (!lst_players) return;

    int sel_player = lst_players->GetSelection();

    if (net_lobby) {
        NetUser* u = (NetUser*) lst_players->GetItemData(sel_player);
        net_lobby->BanUser(u);
        lst_units->ClearItems();
    }

    GetUnits();
    GetAvailable();
}

void
NetUnitDlg::OnApply(AWEvent* event)
{
    bool ok = false;

    if (net_lobby) {
        Mission*  mission = net_lobby->GetSelectedMission();

        if (mission) {
            net_lobby->GameStart();
            ok = true;
        }
    }

    if (!ok) OnCancel(0);
}

void
NetUnitDlg::OnCancel(AWEvent* event)
{
    if (net_lobby) {
        net_lobby->SelectMission(0);
        net_lobby = 0;
    }

    manager->ShowNetLobbyDlg();
}
