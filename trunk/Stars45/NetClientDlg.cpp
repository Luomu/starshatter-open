/*  Project Starshatter 5.0
	Destroyer Studios LLC
	Copyright © 1997-2007. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         NetClientDlg.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Main Menu Dialog Active Window class
*/

#include "MemDebug.h"
#include "NetClientDlg.h"
#include "NetClientConfig.h"
#include "NetLobbyClient.h"
#include "MenuScreen.h"
#include "Starshatter.h"
#include "Ship.h"
#include "HUDView.h"

#include "NetAddr.h"

#include "DataLoader.h"
#include "Button.h"
#include "ListBox.h"
#include "Slider.h"
#include "Video.h"
#include "Keyboard.h"
#include "MachineInfo.h"

// +--------------------------------------------------------------------+
// DECLARE MAPPING FUNCTIONS:

DEF_MAP_CLIENT(NetClientDlg, OnSelect);
DEF_MAP_CLIENT(NetClientDlg, OnAdd);
DEF_MAP_CLIENT(NetClientDlg, OnDel);
DEF_MAP_CLIENT(NetClientDlg, OnServer);
DEF_MAP_CLIENT(NetClientDlg, OnHost);
DEF_MAP_CLIENT(NetClientDlg, OnJoin);
DEF_MAP_CLIENT(NetClientDlg, OnCancel);

// +--------------------------------------------------------------------+

NetClientDlg::NetClientDlg(Screen* s, FormDef& def, MenuScreen* mgr)
: FormWindow(s, 0, 0, s->Width(), s->Height()), manager(mgr),
server_index(-1), ping_index(0), hnet(0)
{
	config = NetClientConfig::GetInstance();
	Init(def);
}

NetClientDlg::~NetClientDlg()
{
	StopNetProc();
}

void
NetClientDlg::StopNetProc()
{
	if (hnet != 0) {
		WaitForSingleObject(hnet, 500);
		CloseHandle(hnet);
		hnet = 0;
	}
}

// +--------------------------------------------------------------------+

void
NetClientDlg::RegisterControls()
{
	btn_add     = (Button*) FindControl(101);
	btn_del     = (Button*) FindControl(102);
	lst_servers = (ListBox*) FindControl(200);
	lbl_info    =           FindControl(210);
	btn_server  = (Button*) FindControl(301);
	btn_host    = (Button*) FindControl(302);
	btn_join    = (Button*) FindControl(303);
	btn_cancel  = (Button*) FindControl(2);

	REGISTER_CLIENT(EID_CLICK,  btn_add,      NetClientDlg, OnAdd);
	REGISTER_CLIENT(EID_CLICK,  btn_del,      NetClientDlg, OnDel);
	REGISTER_CLIENT(EID_SELECT, lst_servers,  NetClientDlg, OnSelect);
	REGISTER_CLIENT(EID_CLICK,  btn_server,   NetClientDlg, OnServer);
	REGISTER_CLIENT(EID_CLICK,  btn_host,     NetClientDlg, OnHost);
	REGISTER_CLIENT(EID_CLICK,  btn_join,     NetClientDlg, OnJoin);
	REGISTER_CLIENT(EID_CLICK,  btn_cancel,   NetClientDlg, OnCancel);
}

// +--------------------------------------------------------------------+

void
NetClientDlg::Show()
{
	int selected_index = -1;

	if (!IsShown()) {
		FormWindow::Show();

		// try to retrieve list of active servers from web broker
		if (config && lst_servers) {
			selected_index = config->GetServerIndex();
			lst_servers->ClearItems();
			config->Download();
		}
	}

	if (config && lst_servers) {
		if (lst_servers->NumItems() != config->GetServerList().size())
		ShowServers();
		else
		UpdateServers();

		if (selected_index >= 0 && selected_index < lst_servers->NumItems()) {
			config->SetServerIndex(selected_index);
			lst_servers->SetSelected(selected_index);
			OnSelect(0);
		}
	}
}

// +--------------------------------------------------------------------+

void
NetClientDlg::ExecFrame()
{
	if (!config || !lst_servers) return;

	if (lst_servers->NumItems() != config->GetServerList().size())
	ShowServers();
	else
	UpdateServers();

	NetServerInfo* info = config->GetServerInfo(server_index);

	bool del_enabled   = info != 0;
	bool join_enabled  = info != 0 && info->status > NetServerInfo::OFFLINE;
	bool host_enabled  = join_enabled && info->hosted == 0;

	if (btn_host)
	btn_host->SetEnabled(host_enabled);

	if (btn_join)
	btn_join->SetEnabled(join_enabled);

	if (btn_del)
	btn_del->SetEnabled(del_enabled);
}

// +--------------------------------------------------------------------+

void
NetClientDlg::ShowServers()
{
	if (!config || !lst_servers) return;

	lst_servers->ClearItems();
	lst_servers->SetSelectedStyle(ListBox::LIST_ITEM_STYLE_FILLED_BOX);
	lst_servers->SetLeading(2);

	int i = 0;
	ListIter<NetServerInfo> iter = config->GetServerList();
	while (++iter) {
		NetServerInfo* info = iter.value();

		lst_servers->AddItemWithData(info->name, (DWORD) i);
		lst_servers->SetItemText(i, 1, info->type);
		lst_servers->SetItemText(i, 2, info->password);
		lst_servers->SetItemText(i, 3, Game::GetText("NetClientDlg.offline"));
		lst_servers->SetItemText(i, 4, "0");
		lst_servers->SetItemText(i, 5, Game::GetText("NetClientDlg.not-avail"));

		i++;
	}

	ping_index   = 0;
	server_index = -1;

	if (btn_host)  btn_host->SetEnabled(false);
	if (btn_join)  btn_join->SetEnabled(false);
	if (btn_del)   btn_del->SetEnabled(false);
}

void
NetClientDlg::UpdateServers()
{
	if (!config || !lst_servers || lst_servers->NumItems() < 1) return;

	if (!PingComplete())
	return;

	PingServer(ping_index);

	for (int i = 0; i < lst_servers->NumItems(); i++) {
		int n = lst_servers->GetItemData(i);

		NetServerInfo* info = config->GetServerList().at(n);
		lst_servers->SetItemText(i, 0, info->name);

		Text status = Game::GetText("NetClientDlg.offline");

		if (info->ping_time > 0 && info->ping_time < 10000) {
			char buffer[32];
			sprintf_s(buffer, "%d ms", info->ping_time);
			lst_servers->SetItemText(i, 5, buffer);

			switch (info->status) {
			default:
			case NetServerInfo::OFFLINE:     status = Game::GetText("NetClientDlg.offline");     break;
			case NetServerInfo::LOBBY:       status = Game::GetText("NetClientDlg.lobby");       break;
			case NetServerInfo::BRIEFING:    status = Game::GetText("NetClientDlg.briefing");    break;
			case NetServerInfo::ACTIVE:      status = Game::GetText("NetClientDlg.active");      break;
			case NetServerInfo::DEBRIEFING:  status = Game::GetText("NetClientDlg.debriefing");  break;
			case NetServerInfo::PERSISTENT:  status = Game::GetText("NetClientDlg.persistent");  break;
			}
		}
		else {
			lst_servers->SetItemText(i, 5, Game::GetText("NetClientDlg.not-avail"));
		}

		lst_servers->SetItemText(i, 3, status);

		char num_users[16];
		sprintf_s(num_users, "%d", info->nplayers);
		lst_servers->SetItemText(i, 4, num_users);
	}
}

// +--------------------------------------------------------------------+

DWORD WINAPI NetClientPingProc(LPVOID link);

void
NetClientDlg::PingServer(int n)
{
	if (hnet == 0) {
		NetClientConfig* config = NetClientConfig::GetInstance();
		if (!config) return;

		NetServerInfo* info = config->GetServerInfo(n);
		if (!info) return;

		// copy info from server list
		ping_info = *info;

		DWORD thread_id = 0;
		hnet = CreateThread(0, 4096, NetClientPingProc, (LPVOID) &ping_info, 0, &thread_id);

		if (hnet == 0) {
			static int report = 10;
			if (report > 0) {
				::Print("WARNING: NetClientDlg() failed to create PING thread for server '%s' (err=%08x)\n", info->name.data(), GetLastError());
				report--;

				if (report == 0)
				::Print("         Further warnings of this type will be supressed.\n");
			}
		}
	}
}

bool
NetClientDlg::PingComplete()
{
	if (hnet != 0) {
		DWORD result = 0;
		GetExitCodeThread(hnet, &result);

		if (result != STILL_ACTIVE) {
			CloseHandle(hnet);
			hnet = 0;

			NetClientConfig* config = NetClientConfig::GetInstance();
			if (config) {
				NetServerInfo* info = config->GetServerInfo(ping_index);
				if (info) {
					// copy result back into server list
					info->machine_info = ping_info.machine_info;
					info->gameport     = ping_info.gameport;
					info->status       = ping_info.status;
					info->nplayers     = ping_info.nplayers;
					info->hosted       = ping_info.hosted;
					info->ping_time    = ping_info.ping_time;
				}
			}

			if (lst_servers && ping_index >= lst_servers->NumItems()-1)
			ping_index = 0;
			else
			ping_index++;
		}
	}

	return !hnet;
}

// +--------------------------------------------------------------------+

DWORD WINAPI NetClientPingProc(LPVOID link)
{
	NetServerInfo* info = (NetServerInfo*) link;
	if (!info) {
		Print("NetClientPingProc() no link\n");
		Sleep(200);
		return 1;
	}

	NetAddr addr = info->addr;
	if (!addr.IPAddr()) {
		Sleep(200);
		return 1;
	}

	NetLobbyClient conn(addr);

	if (conn.Ping()) {
		info->machine_info = conn.GetMachineInfo();
		info->gameport     = conn.GetGamePort();
		info->status       = conn.GetStatus();
		info->nplayers     = conn.NumUsers();
		info->hosted       = conn.HasHost();
		info->ping_time    = conn.GetLag();
	}

	else {
		info->machine_info = Text();
		info->nplayers     = 0;
		info->hosted       = 0;
		info->status       = NetServerInfo::OFFLINE;
		info->ping_time    = 0;
	}

	return 0;
}

// +--------------------------------------------------------------------+

void
NetClientDlg::OnAdd(AWEvent* event)
{
	manager->ShowNetAddrDlg();
}

void
NetClientDlg::OnDel(AWEvent* event)
{
	if (config && server_index >= 0)
	config->DelServer(server_index);
}

void
NetClientDlg::OnSelect(AWEvent* event)
{
	if (lst_servers) {
		server_index = lst_servers->GetSelection();
		NetServerInfo* info = config->GetServerInfo(server_index);

		if (lbl_info) {
			if (info)
			lbl_info->SetText(info->machine_info);
			else
			lbl_info->SetText("");
		}

		if (btn_host) {
			btn_host->SetEnabled(info && info->status > NetServerInfo::OFFLINE && info->hosted == 0);
		}

		if (btn_join) {
			btn_join->SetEnabled(info && info->status > NetServerInfo::OFFLINE);
		}
	}
}

// +--------------------------------------------------------------------+

void
NetClientDlg::OnServer(AWEvent* event)
{
	manager->ShowNetServerDlg();
}

void
NetClientDlg::OnHost(AWEvent* event)
{
	if (config) {
		config->SetServerIndex(server_index);
		config->SetHostRequest(true);
		config->Save();

		NetServerInfo* info = config->GetServerInfo(server_index);
		if (info && info->password == "Yes") {
			manager->ShowNetPassDlg();
		}
		else {
			manager->ShowNetLobbyDlg();
		}
	}
}

void
NetClientDlg::OnJoin(AWEvent* event)
{
	if (config) {
		config->SetServerIndex(server_index);
		config->SetHostRequest(false);
		config->Save();

		NetServerInfo* info = config->GetServerInfo(server_index);
		if (info && info->password == "Yes") {
			manager->ShowNetPassDlg();
		}
		else {
			manager->ShowNetLobbyDlg();
		}
	}
}

void
NetClientDlg::OnCancel(AWEvent* event)
{
	if (config) {
		config->Save();
		config->SetServerIndex(-1);
	}

	manager->ShowMenuDlg();
}
