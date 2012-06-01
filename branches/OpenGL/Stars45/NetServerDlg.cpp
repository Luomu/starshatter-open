/*  Project Starshatter 5.0
	Destroyer Studios LLC
	Copyright © 1997-2007. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         NetServerDlg.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Main Menu Dialog Active Window class
*/

#include "MemDebug.h"
#include "NetServerDlg.h"
#include "NetServerConfig.h"
#include "MenuScreen.h"
#include "Starshatter.h"

#include "NetAddr.h"
#include "HttpClient.h"
#include "HttpServer.h"

#include "DataLoader.h"
#include "Button.h"
#include "EditBox.h"
#include "ComboBox.h"
#include "Video.h"
#include "Keyboard.h"
#include "MachineInfo.h"

// +--------------------------------------------------------------------+
// DECLARE MAPPING FUNCTIONS:

DEF_MAP_CLIENT(NetServerDlg, OnApply);
DEF_MAP_CLIENT(NetServerDlg, OnCancel);

// +--------------------------------------------------------------------+

NetServerDlg::NetServerDlg(Screen* s, FormDef& def, MenuScreen* mgr)
: FormWindow(s, 0, 0, s->Width(), s->Height()), manager(mgr)
{
	config = NetServerConfig::GetInstance();
	Init(def);
}

NetServerDlg::~NetServerDlg()
{
}

// +--------------------------------------------------------------------+

void
NetServerDlg::RegisterControls()
{
	edt_name       = (EditBox*)   FindControl(200);
	cmb_type       = (ComboBox*)  FindControl(201);
	edt_game_port  = (EditBox*)   FindControl(202);
	edt_admin_port = (EditBox*)   FindControl(203);
	edt_game_pass  = (EditBox*)   FindControl(204);
	edt_admin_name = (EditBox*)   FindControl(205);
	edt_admin_pass = (EditBox*)   FindControl(206);

	btn_apply      = (Button*) FindControl(1);
	btn_cancel     = (Button*) FindControl(2);

	REGISTER_CLIENT(EID_CLICK,  btn_apply,    NetServerDlg, OnApply);
	REGISTER_CLIENT(EID_CLICK,  btn_cancel,   NetServerDlg, OnCancel);
}

// +--------------------------------------------------------------------+

void
NetServerDlg::Show()
{
	if (!IsShown())
	FormWindow::Show();

	NetServerConfig::Initialize();
	config = NetServerConfig::GetInstance();

	if (config) {
		config->Load();

		char buff[32];

		if (edt_name) {
			edt_name->SetText(config->Name());
			edt_name->SetFocus();
		}

		if (cmb_type)
		cmb_type->SetSelection(config->GetGameType());

		if (edt_game_port) {
			sprintf_s(buff, "%d", config->GetLobbyPort());
			edt_game_port->SetText(buff);
		}

		if (edt_admin_port) {
			sprintf_s(buff, "%d", config->GetAdminPort());
			edt_admin_port->SetText(buff);
		}

		if (edt_game_pass)
		edt_game_pass->SetText(config->GetGamePass());

		if (edt_admin_name)
		edt_admin_name->SetText(config->GetAdminName());

		if (edt_admin_pass)
		edt_admin_pass->SetText(config->GetAdminPass());
	}
}

// +--------------------------------------------------------------------+

void
NetServerDlg::ExecFrame()
{
}

// +--------------------------------------------------------------------+

void
NetServerDlg::OnApply(AWEvent* event)
{
	if (config) {
		if (edt_name)
		config->SetName(edt_name->GetText());

		if (cmb_type)
		config->SetGameType(cmb_type->GetSelectedIndex());

		if (edt_game_port) {
			int port = 0;
			sscanf_s(edt_game_port->GetText(), "%d", &port);
			config->SetLobbyPort((WORD) port);
			config->SetGamePort((WORD) port+1);
		}

		if (edt_admin_port) {
			int port = 0;
			sscanf_s(edt_admin_port->GetText(), "%d", &port);
			config->SetAdminPort((WORD) port);
		}

		if (edt_game_pass)
		config->SetGamePass(edt_game_pass->GetText());

		if (edt_admin_name)
		config->SetAdminName(edt_admin_name->GetText());

		if (edt_admin_pass)
		config->SetAdminPass(edt_admin_pass->GetText());

		
		config->Save();
	}

	Starshatter* stars = Starshatter::GetInstance();

	if (stars) {
		::Print("\nSTART LOCAL SERVER\n\n");
		stars->SetLobbyMode(Starshatter::NET_LOBBY_SERVER);
		manager->ShowNetLobbyDlg();
	}
	else {
		manager->ShowMenuDlg();
	}
}

void
NetServerDlg::OnCancel(AWEvent* event)
{
	NetServerConfig::Close();
	manager->ShowNetClientDlg();
}
