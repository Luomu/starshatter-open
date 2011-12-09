/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         NetPassDlg.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Network Server Password Dialog Active Window class
*/

#include "MemDebug.h"
#include "NetPassDlg.h"
#include "MenuScreen.h"
#include "NetClientConfig.h"
#include "NetLobby.h"

#include "Game.h"
#include "DataLoader.h"
#include "Button.h"
#include "EditBox.h"
#include "ListBox.h"
#include "Slider.h"
#include "Video.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "ParseUtil.h"
#include "FormatUtil.h"

// +--------------------------------------------------------------------+
// DECLARE MAPPING FUNCTIONS:

DEF_MAP_CLIENT(NetPassDlg, OnApply);
DEF_MAP_CLIENT(NetPassDlg, OnCancel);

// +--------------------------------------------------------------------+

NetPassDlg::NetPassDlg(Screen* s, FormDef& def, MenuScreen* mgr)
: FormWindow(s, 0, 0, s->Width(), s->Height()), manager(mgr),
btn_apply(0), btn_cancel(0), edt_pass(0), lbl_name(0)
{
	Init(def);
}

NetPassDlg::~NetPassDlg()
{
}

// +--------------------------------------------------------------------+

void
NetPassDlg::RegisterControls()
{
	btn_apply   = (Button*) FindControl(1);
	btn_cancel  = (Button*) FindControl(2);

	REGISTER_CLIENT(EID_CLICK, btn_apply,  NetPassDlg, OnApply);
	REGISTER_CLIENT(EID_CLICK, btn_cancel, NetPassDlg, OnCancel);

	lbl_name    =            FindControl(110);
	edt_pass    = (EditBox*) FindControl(200);

	if (edt_pass)
	edt_pass->SetText("");
}

// +--------------------------------------------------------------------+

void
NetPassDlg::Show()
{
	if (!IsShown()) {
		FormWindow::Show();

		NetClientConfig* config = NetClientConfig::GetInstance();

		if (config && lbl_name) {
			NetServerInfo* info = config->GetSelectedServer();

			if (info)
			lbl_name->SetText(info->name);
		}

		if (edt_pass) {
			edt_pass->SetText("");
			edt_pass->SetFocus();
		}
	}
}

// +--------------------------------------------------------------------+

static bool tab_latch = false;

void
NetPassDlg::ExecFrame()
{
	if (Keyboard::KeyDown(VK_RETURN)) {
		OnApply(0);
	}
}

// +--------------------------------------------------------------------+

void
NetPassDlg::OnApply(AWEvent* event)
{
	NetClientConfig* config = NetClientConfig::GetInstance();

	if (config && edt_pass) {
		NetServerInfo* info = config->GetSelectedServer();

		if (info && edt_pass->GetText().length() < 250) {
			char buffer[256];
			strcpy_s(buffer, edt_pass->GetText().data());

			// trim from first occurrence of invalid character
			char* p = strpbrk(buffer, "\n\r\t");
			if (p) *p = 0;

			info->password = SafeQuotes(buffer);

			if (manager) {
				manager->ShowNetLobbyDlg();
				return;
			}
		}
	}

	if (manager) {
		manager->ShowNetClientDlg();
	}
}

void
NetPassDlg::OnCancel(AWEvent* event)
{
	if (manager)
	manager->ShowNetClientDlg();
}
