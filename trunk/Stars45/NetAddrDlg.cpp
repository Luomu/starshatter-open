/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         NetAddrDlg.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Mission Select Dialog Active Window class
*/

#include "MemDebug.h"
#include "NetAddrDlg.h"
#include "MenuScreen.h"
#include "NetClientConfig.h"

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

DEF_MAP_CLIENT(NetAddrDlg, OnSave);
DEF_MAP_CLIENT(NetAddrDlg, OnCancel);

// +--------------------------------------------------------------------+

NetAddrDlg::NetAddrDlg(Screen* s, FormDef& def, MenuScreen* mgr)
: FormWindow(s, 0, 0, s->Width(), s->Height()), manager(mgr),
btn_save(0), btn_cancel(0), edt_name(0), edt_addr(0), edt_port(0), edt_pass(0)
{
	Init(def);
}

NetAddrDlg::~NetAddrDlg()
{
}

// +--------------------------------------------------------------------+

void
NetAddrDlg::RegisterControls()
{
	btn_save    = (Button*) FindControl(1);
	btn_cancel  = (Button*) FindControl(2);

	REGISTER_CLIENT(EID_CLICK, btn_save,   NetAddrDlg, OnSave);
	REGISTER_CLIENT(EID_CLICK, btn_cancel, NetAddrDlg, OnCancel);

	edt_name    = (EditBox*) FindControl(200);
	edt_addr    = (EditBox*) FindControl(201);
	edt_port    = (EditBox*) FindControl(202);
	edt_pass    = (EditBox*) FindControl(203);

	if (edt_name) edt_name->SetText("");
	if (edt_addr) edt_addr->SetText("");
	if (edt_port) edt_port->SetText("");
	if (edt_pass) edt_pass->SetText("");
}

// +--------------------------------------------------------------------+

void
NetAddrDlg::Show()
{
	if (!IsShown()) {
		FormWindow::Show();

		if (edt_name)  edt_name->SetText("");
		if (edt_addr)  edt_addr->SetText("");
		if (edt_port)  edt_port->SetText("");
		if (edt_pass)  edt_pass->SetText("");

		if (edt_name)  edt_name->SetFocus();
	}
}

// +--------------------------------------------------------------------+

static bool tab_latch = false;

void
NetAddrDlg::ExecFrame()
{
	if (Keyboard::KeyDown(VK_RETURN)) {
		OnSave(0);
	}
}

// +--------------------------------------------------------------------+

void
NetAddrDlg::OnSave(AWEvent* event)
{
	NetClientConfig* config = NetClientConfig::GetInstance();

	if (config   && 
			edt_addr && edt_addr->GetText().length() > 0 &&
			edt_port && edt_port->GetText().length() > 0)
	{
		Text name;
		Text addr;
		Text pass;
		int  port;

		sscanf(edt_port->GetText().data(), "%d", &port);

		if (edt_name && edt_name->GetText().length() < 250) {
			char buffer[256];
			strcpy(buffer, edt_name->GetText().data());
			char* p = strpbrk(buffer, "\n\r\t");
			if (p) *p = 0;

			name = SafeQuotes(buffer);
		}

		if (edt_pass && edt_pass->GetText().length() < 250) {
			char buffer[256];
			strcpy(buffer, edt_pass->GetText().data());
			char* p = strpbrk(buffer, "\n\r\t");
			if (p) *p = 0;

			pass = SafeQuotes(buffer);
		}

		if (edt_addr && edt_addr->GetText().length() < 250) {
			char buffer[256];
			strcpy(buffer, edt_addr->GetText().data());
			char* p = strpbrk(buffer, "\n\r\t");
			if (p) *p = 0;

			addr = SafeQuotes(buffer);
		}

		config->AddServer(name, addr, port, pass, true);
		config->Save();
	}

	if (manager)
	manager->ShowNetClientDlg();
}

void
NetAddrDlg::OnCancel(AWEvent* event)
{
	if (manager)
	manager->ShowNetClientDlg();
}
