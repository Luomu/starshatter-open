/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         AwardShowDlg.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Main Menu Dialog Active Window class
*/

#include "MemDebug.h"
#include "AwardShowDlg.h"
#include "MenuScreen.h"
#include "Starshatter.h"
#include "Ship.h"
#include "Player.h"
#include "Campaign.h"

#include "Game.h"
#include "DataLoader.h"
#include "Button.h"
#include "ListBox.h"
#include "EditBox.h"
#include "ImageBox.h"
#include "FormatUtil.h"
#include "Video.h"
#include "Keyboard.h"
#include "Mouse.h"

// +--------------------------------------------------------------------+
// DECLARE MAPPING FUNCTIONS:

DEF_MAP_CLIENT(AwardShowDlg, OnClose);

// +--------------------------------------------------------------------+

AwardShowDlg::AwardShowDlg(Screen* s, FormDef& def, MenuScreen* mgr)
: FormWindow(s, 0, 0, s->Width(), s->Height()), manager(mgr),
lbl_name(0), lbl_info(0), img_rank(0), btn_close(0), exit_latch(true),
rank(-1), medal(-1)
{
	Init(def);
}

AwardShowDlg::~AwardShowDlg()
{
}

// +--------------------------------------------------------------------+

void
AwardShowDlg::RegisterControls()
{
	lbl_name     =              FindControl(203);
	lbl_info     =              FindControl(201);
	img_rank     = (ImageBox*)  FindControl(202);

	btn_close    = (Button*) FindControl(1);
	REGISTER_CLIENT(EID_CLICK, btn_close, AwardShowDlg, OnClose);
}

// +--------------------------------------------------------------------+

void
AwardShowDlg::Show()
{
	FormWindow::Show();
	ShowAward();
}

// +--------------------------------------------------------------------+

void
AwardShowDlg::ExecFrame()
{
	if (Keyboard::KeyDown(VK_RETURN)) {
		if (!exit_latch)
		OnClose(0);
	}

	else if (Keyboard::KeyDown(VK_ESCAPE)) {
		if (!exit_latch)
		OnClose(0);
	}

	else {
		exit_latch = false;
	}
}

// +--------------------------------------------------------------------+

void
AwardShowDlg::SetRank(int r)
{
	rank = r;
	medal = -1;
}

void
AwardShowDlg::SetMedal(int m)
{
	rank = -1;
	medal = m;
}

// +--------------------------------------------------------------------+

void
AwardShowDlg::ShowAward()
{
	if (rank >= 0) {
		if (lbl_name) {
			lbl_name->SetText(Text("Rank of ") + Player::RankName(rank));
		}

		if (lbl_info) {
			lbl_info->SetText(Player::RankDescription(rank));
		}

		if (img_rank) {
			img_rank->SetPicture(*Player::RankInsignia(rank, 1));
			img_rank->Show();
		}
	}

	else if (medal >= 0) {
		if (lbl_name) {
			lbl_name->SetText(Player::MedalName(medal));
		}

		if (lbl_info) {
			lbl_info->SetText(Player::MedalDescription(medal));
		}

		if (img_rank) {
			img_rank->SetPicture(*Player::MedalInsignia(medal, 1));
			img_rank->Show();
		}
	}

	else {
		if (lbl_name)        lbl_name->SetText("");
		if (lbl_info)        lbl_info->SetText("");
		if (img_rank)        img_rank->Hide();
	}
}

// +--------------------------------------------------------------------+

void
AwardShowDlg::OnClose(AWEvent* event)
{
	manager->ShowPlayerDlg();
}
