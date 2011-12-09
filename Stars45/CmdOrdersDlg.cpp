/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         CmdOrdersDlg.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Operational Command Dialog (Campaign Orders Tab)
*/

#include "MemDebug.h"
#include "CmdOrdersDlg.h"
#include "CmdDlg.h"
#include "CmpnScreen.h"
#include "Starshatter.h"
#include "Campaign.h"
#include "Combatant.h"
#include "CombatGroup.h"
#include "CombatUnit.h"
#include "ShipDesign.h"

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

DEF_MAP_CLIENT(CmdOrdersDlg, OnMode);
DEF_MAP_CLIENT(CmdOrdersDlg, OnSave);
DEF_MAP_CLIENT(CmdOrdersDlg, OnExit);

// +--------------------------------------------------------------------+

CmdOrdersDlg::CmdOrdersDlg(Screen* s, FormDef& def, CmpnScreen* mgr)
: FormWindow(s, 0, 0, s->Width(), s->Height()), CmdDlg(mgr), manager(mgr),
lbl_orders(0), stars(0), campaign(0)
{
	stars    = Starshatter::GetInstance();
	campaign = Campaign::GetCampaign();

	Init(def);
}

CmdOrdersDlg::~CmdOrdersDlg()
{
}

// +--------------------------------------------------------------------+

void
CmdOrdersDlg::RegisterControls()
{
	lbl_orders = FindControl(400);

	RegisterCmdControls(this);

	if (btn_save)
	REGISTER_CLIENT(EID_CLICK,  btn_save,    CmdOrdersDlg, OnSave);

	if (btn_exit)
	REGISTER_CLIENT(EID_CLICK,  btn_exit,    CmdOrdersDlg, OnExit);

	for (int i = 0; i < 5; i++) {
		if (btn_mode[i])
		REGISTER_CLIENT(EID_CLICK,  btn_mode[i], CmdOrdersDlg, OnMode);
	}
}

// +--------------------------------------------------------------------+

void
CmdOrdersDlg::Show()
{
	mode = MODE_ORDERS;

	FormWindow::Show();
	ShowCmdDlg();

	campaign = Campaign::GetCampaign();

	if (campaign && lbl_orders) {
		Text orders("<font Limerick12><color ffff80>");
		orders += Game::GetText("CmdOrdersDlg.situation");
		orders += "\n<font Verdana><color ffffff>";
		if (*campaign->Situation())
		orders += campaign->Situation();
		else
		orders += campaign->Description();

		orders += "\n\n<font Limerick12><color ffff80>";
		orders += Game::GetText("CmdOrdersDlg.orders");
		orders += "\n<font Verdana><color ffffff>";
		orders += campaign->Orders();

		lbl_orders->SetText(orders);
	}
}

// +--------------------------------------------------------------------+

void
CmdOrdersDlg::ExecFrame()
{
	CmdDlg::ExecFrame();
}

// +--------------------------------------------------------------------+

void
CmdOrdersDlg::OnSave(AWEvent* event)
{
	CmdDlg::OnSave(event);
}

void
CmdOrdersDlg::OnExit(AWEvent* event)
{
	CmdDlg::OnExit(event);
}

void
CmdOrdersDlg::OnMode(AWEvent* event)
{
	CmdDlg::OnMode(event);
}

