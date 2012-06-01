/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         AwardDlg.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Main Menu Dialog Active Window class
*/

#include "MemDebug.h"
#include "AwardDlg.h"
#include "PlanScreen.h"
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
#include "Sound.h"

// +--------------------------------------------------------------------+
// DECLARE MAPPING FUNCTIONS:

DEF_MAP_CLIENT(AwardDlg, OnClose);

// +--------------------------------------------------------------------+

AwardDlg::AwardDlg(Screen* s, FormDef& def, PlanScreen* mgr)
: FormWindow(s, 0, 0, s->Width(), s->Height()), manager(mgr),
lbl_name(0), lbl_info(0), img_rank(0), btn_close(0), exit_latch(true)
{
	Init(def);
}

AwardDlg::~AwardDlg()
{
}

// +--------------------------------------------------------------------+

void
AwardDlg::RegisterControls()
{
	lbl_name     =              FindControl(203);
	lbl_info     =              FindControl(201);
	img_rank     = (ImageBox*)  FindControl(202);

	btn_close    = (Button*) FindControl(1);
	REGISTER_CLIENT(EID_CLICK, btn_close, AwardDlg, OnClose);
}

// +--------------------------------------------------------------------+

void
AwardDlg::Show()
{
	FormWindow::Show();
	ShowPlayer();

	exit_latch = true;
}

// +--------------------------------------------------------------------+

void
AwardDlg::ExecFrame()
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
AwardDlg::ShowPlayer()
{
	Player* p = Player::GetCurrentPlayer();

	if (p) {
		if (lbl_name) {
			lbl_name->SetText(p->AwardName());
		}

		if (lbl_info) {
			lbl_info->SetText(p->AwardDesc());
		}

		if (img_rank) {
			img_rank->SetPicture(*p->AwardImage());
			img_rank->Show();
		}

		Sound* congrats = p->AwardSound();
		if (congrats) {
			congrats->Play();
		}
	}
	else {
		if (lbl_info)        lbl_info->SetText("");
		if (img_rank)        img_rank->Hide();
	}
}

// +--------------------------------------------------------------------+

void
AwardDlg::OnClose(AWEvent* event)
{
	Player* player = Player::GetCurrentPlayer();
	if (player)
	player->ClearShowAward();

	Starshatter* stars = Starshatter::GetInstance();

	if (stars) {
		Mouse::Show(false);

		Campaign* campaign = Campaign::GetCampaign();
		if (campaign && campaign->GetCampaignId() < Campaign::SINGLE_MISSIONS)
		stars->SetGameMode(Starshatter::CMPN_MODE);
		else
		stars->SetGameMode(Starshatter::MENU_MODE);
	}

	else
	Game::Panic("AwardDlg::OnClose() - Game instance not found");
}
