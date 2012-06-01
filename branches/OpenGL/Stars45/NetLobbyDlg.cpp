/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         NetLobbyDlg.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Main Menu Dialog Active Window class
*/

#include "MemDebug.h"
#include "NetLobbyDlg.h"
#include "NetUnitDlg.h"
#include "NetClientConfig.h"
#include "MenuScreen.h"
#include "Starshatter.h"
#include "Ship.h"
#include "Player.h"
#include "Campaign.h"

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

DEF_MAP_CLIENT(NetLobbyDlg, OnCampaignSelect);
DEF_MAP_CLIENT(NetLobbyDlg, OnMissionSelect);
DEF_MAP_CLIENT(NetLobbyDlg, OnApply);
DEF_MAP_CLIENT(NetLobbyDlg, OnCancel);

// +--------------------------------------------------------------------+

NetLobbyDlg::NetLobbyDlg(Screen* s, FormDef& def, MenuScreen* mgr)
: FormWindow(s, 0, 0, s->Width(), s->Height()), manager(mgr),
net_lobby(0)
{
	selected_campaign = 0;
	selected_mission  = 0;
	last_chat         = 0;
	host_mode         = false;

	Init(def);
}

NetLobbyDlg::~NetLobbyDlg()
{
}

// +--------------------------------------------------------------------+

void
NetLobbyDlg::RegisterControls()
{
	lst_campaigns = (ComboBox*) FindControl(200);
	REGISTER_CLIENT(EID_SELECT, lst_campaigns, NetLobbyDlg, OnCampaignSelect);

	lst_missions = (ListBox*) FindControl(201);
	REGISTER_CLIENT(EID_SELECT, lst_missions, NetLobbyDlg, OnMissionSelect);

	txt_desc    =            FindControl(202);
	lst_players = (ListBox*) FindControl(210);
	lst_chat    = (ListBox*) FindControl(211);
	edt_chat    = (EditBox*) FindControl(212);

	if (edt_chat)
	edt_chat->SetText("");

	apply   = (Button*) FindControl(1);
	REGISTER_CLIENT(EID_CLICK, apply, NetLobbyDlg, OnApply);

	cancel  = (Button*) FindControl(2);
	REGISTER_CLIENT(EID_CLICK, cancel, NetLobbyDlg, OnCancel);
}

// +--------------------------------------------------------------------+

void
NetLobbyDlg::Show()
{
	if (!IsShown()) {
		// clear server data:
		if (lst_chat)        lst_chat->ClearItems();
		if (lst_campaigns)   lst_campaigns->ClearItems();
		if (lst_missions)    lst_missions->ClearItems();
		if (txt_desc)        txt_desc->SetText("");
		if (apply)           apply->SetEnabled(false);

		if (lst_missions) {
			lst_missions->SetSelectedStyle(ListBox::LIST_ITEM_STYLE_FILLED_BOX);
			lst_missions->SetLeading(2);
		}

		selected_campaign = 0;
		selected_mission  = 0;
		last_chat         = 0;

		FormWindow::Show();

		net_lobby = NetLobby::GetInstance();

		if (!net_lobby) {
			Starshatter* stars = Starshatter::GetInstance();
			if (stars)
			stars->StartLobby();

			net_lobby = NetLobby::GetInstance();
		}

		if (net_lobby) {
			if (net_lobby->IsServer()) {
				host_mode = true;

				NetUser* user = net_lobby->GetLocalUser();

				if (!user) {
					Player* player = Player::GetCurrentPlayer();
					if (player) {
						user = new(__FILE__,__LINE__) NetUser(player);
						user->SetHost(true);
					}
					else {
						::Print("NetLobbyDlg::Show() Host mode - no current player?\n");
					}
				}

				net_lobby->SetLocalUser(user);
			}

			SelectMission();
		}
	}
}

// +--------------------------------------------------------------------+

void
NetLobbyDlg::ExecFrame()
{
	ExecLobbyFrame();

	if (Keyboard::KeyDown(VK_RETURN)) {
		if (edt_chat && edt_chat->GetText().length() > 0) {
			SendChat(edt_chat->GetText());
			edt_chat->SetText("");
		}
	}

	GetPlayers();
	GetChat();

	if (lst_campaigns) {
		if (lst_campaigns->NumItems() < 1)
		GetMissions();
		else
		GetSelectedMission();
	}
}

// +--------------------------------------------------------------------+

void
NetLobbyDlg::ExecLobbyFrame()
{
	if (net_lobby && net_lobby->GetLastError() != 0) {
		if (net_lobby->IsClient()) {
			Starshatter* stars = Starshatter::GetInstance();
			if (stars)
			stars->StopLobby();

			net_lobby = 0;
			manager->ShowNetClientDlg();
		}
	}
}

// +--------------------------------------------------------------------+

void
NetLobbyDlg::GetPlayers()
{
	if (!lst_players) return;

	if (net_lobby) {
		lst_players->ClearItems();

		NetUser* u = net_lobby->GetLocalUser();
		if (u) {
			Text name = Player::RankAbrv(u->Rank());
			name += " ";
			name += u->Name();

			int count = lst_players->AddItem(u->IsHost() ? "*" : " ");
			lst_players->SetItemText(count-1, 1, name);
			host_mode = true;
		}

		ListIter<NetUser> iter = net_lobby->GetUsers();
		while (++iter) {
			NetUser* u = iter.value();
			int count = lst_players->AddItem(u->IsHost() ? "*" : " ");

			Text name = Player::RankAbrv(u->Rank());
			name += " ";
			name += u->Name();

			lst_players->SetItemText(count-1, 1, name);

			if (Player::GetCurrentPlayer()->Name() == u->Name())
			host_mode = u->IsHost();
		}
	}
}

// +--------------------------------------------------------------------+

void
NetLobbyDlg::GetChat()
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
NetLobbyDlg::SendChat(Text msg)
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
	net_lobby->AddChat(net_lobby->GetLocalUser(), msg);
}

// +--------------------------------------------------------------------+

void
NetLobbyDlg::GetMissions()
{
	if (!lst_campaigns || !lst_missions)
	return;

	if (net_lobby) {
		lst_campaigns->ClearItems();

		List<NetCampaignInfo>& campaigns = net_lobby->GetCampaigns();

		if (campaigns.size()) {
			ListIter<NetCampaignInfo> c_iter = campaigns;
			while (++c_iter) {
				NetCampaignInfo* c = c_iter.value();
				lst_campaigns->AddItem(c->name);
			}

			lst_campaigns->SetSelection(0);
			NetCampaignInfo* c = campaigns[0];

			lst_missions->ClearItems();

			ListIter<MissionInfo> m_iter = c->missions;
			while (++m_iter) {
				MissionInfo* m = m_iter.value();
				lst_missions->AddItem(m->name);
			}
		}
	}
}

// +--------------------------------------------------------------------+

void
NetLobbyDlg::GetSelectedMission()
{
	if (!lst_campaigns || !lst_missions) return;

	if (net_lobby) {
		if (net_lobby->GetSelectedMissionID()) {
			int id = net_lobby->GetSelectedMissionID();

			selected_campaign = id >> NET_CAMPAIGN_SHIFT;
			selected_mission  = id &  NET_MISSION_MASK;

			List<NetCampaignInfo>& campaigns = net_lobby->GetCampaigns();

			for (int i = 0; i < campaigns.size(); i++) {
				NetCampaignInfo* c = campaigns[i];

				if (c->id == selected_campaign) {
					lst_campaigns->SetSelection(i);
					OnCampaignSelect(0);

					for (int j = 0; j < c->missions.size(); j++) {
						MissionInfo* m = c->missions[j];

						if (m->id == selected_mission) {
							lst_missions->SetSelected(j);
							OnMissionSelect(0);
						}
					}
				}
			}
		}
		else if (selected_campaign) {
			selected_campaign = 0;
			selected_mission  = 0;
		}

		lst_campaigns->SetEnabled(selected_campaign == 0);
		lst_missions->SetEnabled(selected_mission   == 0);

		if (!host_mode)
		apply->SetEnabled(selected_mission != 0);
	}
}

// +--------------------------------------------------------------------+

void
NetLobbyDlg::SelectMission()
{
	if (!lst_campaigns || !lst_missions) return;

	bool selected = false;

	if (net_lobby) {
		int c_index = lst_campaigns->GetSelectedIndex();
		int m_index = lst_missions->GetSelection();

		List<NetCampaignInfo>& campaigns = net_lobby->GetCampaigns();

		if (c_index >= 0 && c_index < campaigns.size() &&  m_index >= 0) {
			NetCampaignInfo* c = campaigns[c_index];

			if (m_index < c->missions.size()) {
				MissionInfo* m = c->missions[m_index];

				DWORD id = (c->id << NET_CAMPAIGN_SHIFT) +
				(m->id &  NET_MISSION_MASK);

				net_lobby->SelectMission(id);
				selected = true;
			}
		}

		if (!selected)
		net_lobby->SelectMission(0);
	}
}

// +--------------------------------------------------------------------+

void
NetLobbyDlg::OnCampaignSelect(AWEvent* event)
{
	if (net_lobby) {
		List<NetCampaignInfo>& campaigns = net_lobby->GetCampaigns();

		if (lst_campaigns && lst_missions && campaigns.size()) {
			int index = lst_campaigns->GetSelectedIndex();

			if (index >= 0 && index < campaigns.size()) {
				NetCampaignInfo* c = campaigns[index];

				lst_missions->ClearItems();
				txt_desc->SetText("");

				ListIter<MissionInfo> iter = c->missions;
				while (++iter) {
					MissionInfo* m = iter.value();
					lst_missions->AddItem(m->name);
				}
			}
		}
	}
}

void
NetLobbyDlg::OnMissionSelect(AWEvent* event)
{
	if (net_lobby) {
		List<NetCampaignInfo>& campaigns = net_lobby->GetCampaigns();

		if (lst_campaigns && lst_missions && txt_desc && campaigns.size()) {
			txt_desc->SetText("");

			if (host_mode && apply)
			apply->SetEnabled(false);

			int c_index = lst_campaigns->GetSelectedIndex();
			int m_index = lst_missions->GetSelection();

			if (c_index >= 0 && c_index < campaigns.size()) {
				NetCampaignInfo* c = campaigns[c_index];

				if (m_index >= 0 && m_index < c->missions.size()) {
					MissionInfo* m = c->missions[m_index];
					txt_desc->SetText(m->description);

					if (host_mode && apply)
					apply->SetEnabled(true);
				}
			}
		}
	}
}

// +--------------------------------------------------------------------+

void
NetLobbyDlg::OnApply(AWEvent* event)
{
	if (host_mode)
	SelectMission();

	manager->ShowNetUnitDlg();

	NetUnitDlg* unit_dlg = manager->GetNetUnitDlg();
	if (unit_dlg)
	unit_dlg->SetHostMode(host_mode);
}

void
NetLobbyDlg::OnCancel(AWEvent* event)
{
	if (net_lobby) {
		net_lobby->SelectMission(0);
		net_lobby = 0;

		Starshatter* stars = Starshatter::GetInstance();
		if (stars)
		stars->StopLobby();
	}

	manager->ShowNetClientDlg();
}
