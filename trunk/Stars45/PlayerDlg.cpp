/*  Project Starshatter 5.0
	Destroyer Studios LLC
	Copyright © 1997-2007. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         PlayerDlg.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Main Menu Dialog Active Window class
*/

#include "MemDebug.h"
#include "PlayerDlg.h"
#include "AwardShowDlg.h"
#include "ConfirmDlg.h"
#include "MenuScreen.h"
#include "Player.h"

#include "FormatUtil.h"

#include "Game.h"
#include "DataLoader.h"
#include "Button.h"
#include "ListBox.h"
#include "EditBox.h"
#include "ImageBox.h"
#include "Slider.h"
#include "Video.h"
#include "Keyboard.h"
#include "MachineInfo.h"

// +--------------------------------------------------------------------+
// DECLARE MAPPING FUNCTIONS:

DEF_MAP_CLIENT(PlayerDlg, OnApply);
DEF_MAP_CLIENT(PlayerDlg, OnCancel);
DEF_MAP_CLIENT(PlayerDlg, OnSelectPlayer);
DEF_MAP_CLIENT(PlayerDlg, OnAdd);
DEF_MAP_CLIENT(PlayerDlg, OnDel);
DEF_MAP_CLIENT(PlayerDlg, OnDelConfirm);
DEF_MAP_CLIENT(PlayerDlg, OnRank);
DEF_MAP_CLIENT(PlayerDlg, OnMedal);

// +--------------------------------------------------------------------+

PlayerDlg::PlayerDlg(Screen* s, FormDef& def, MenuScreen* mgr)
: FormWindow(s,  0,  0, s->Width(), s->Height()), manager(mgr),
lst_roster(0), btn_add(0), btn_del(0),
txt_name(0), txt_password(0), txt_squadron(0), txt_signature(0),
img_rank(0)
{
	Init(def);
}

PlayerDlg::~PlayerDlg()
{
}

// +--------------------------------------------------------------------+

void
PlayerDlg::RegisterControls()
{
	lst_roster     = (ListBox*) FindControl(200);
	btn_add        = (Button*)  FindControl(101);
	btn_del        = (Button*)  FindControl(102);
	txt_name       = (EditBox*) FindControl(201);
	txt_password   = (EditBox*) FindControl(202);
	txt_squadron   = (EditBox*) FindControl(203);
	txt_signature  = (EditBox*) FindControl(204);

	lbl_createdate =            FindControl(205);
	lbl_rank =                  FindControl(206);
	lbl_flighttime =            FindControl(207);
	lbl_missions =              FindControl(208);
	lbl_kills    =              FindControl(209);
	lbl_losses   =              FindControl(210);
	lbl_points   =              FindControl(211);

	img_rank     = (ImageBox*)  FindControl(220);
	REGISTER_CLIENT(EID_CLICK,       img_rank, PlayerDlg, OnRank);

	for (int i = 0; i < 15; i++) {
		medals[i] = -1;
		img_medals[i] = (ImageBox*) FindControl(230 + i);
		if (img_medals[i])
		REGISTER_CLIENT(EID_CLICK, img_medals[i], PlayerDlg, OnMedal);
	}

	for (int i = 0; i < 10; i++) {
		txt_chat[i] = (EditBox*) FindControl(300 + i);
	}

	REGISTER_CLIENT(EID_SELECT, lst_roster, PlayerDlg, OnSelectPlayer);
	REGISTER_CLIENT(EID_CLICK,  btn_add,    PlayerDlg, OnAdd);
	REGISTER_CLIENT(EID_CLICK,  btn_del,    PlayerDlg, OnDel);
	REGISTER_CLIENT(EID_USER_1, btn_del,    PlayerDlg, OnDelConfirm);

	apply   = (Button*) FindControl(1);
	REGISTER_CLIENT(EID_CLICK, apply, PlayerDlg, OnApply);

	cancel  = (Button*) FindControl(2);
	REGISTER_CLIENT(EID_CLICK, cancel, PlayerDlg, OnCancel);
}

// +--------------------------------------------------------------------+

void
PlayerDlg::Show()
{
	FormWindow::Show();

	if (!lst_roster) return;
	lst_roster->ClearItems();
	lst_roster->SetSelectedStyle(ListBox::LIST_ITEM_STYLE_FILLED_BOX);
	lst_roster->SetLeading(2);

	int current_index = 0;

	List<Player>& roster = Player::GetRoster();
	for (int i = 0; i < roster.size(); i++) {
		Player* p = roster[i];
		lst_roster->AddItem(p->Name());
		if (p == Player::GetCurrentPlayer())
		current_index = i;
	}

	lst_roster->SetSelected(current_index);
	ShowPlayer();

	apply->SetEnabled(roster.size() > 0);
}

// +--------------------------------------------------------------------+

void
PlayerDlg::ExecFrame()
{
	if (Keyboard::KeyDown(VK_RETURN)) {
		OnApply(0);
	}
}

// +--------------------------------------------------------------------+

void
PlayerDlg::ShowPlayer()
{
	Player* p = Player::GetCurrentPlayer();

	if (p) {
		if (txt_name)        txt_name->SetText(p->Name());
		if (txt_password)    txt_password->SetText(p->Password());
		if (txt_squadron)    txt_squadron->SetText(p->Squadron());
		if (txt_signature)   txt_signature->SetText(p->Signature());

		char flight_time[64], missions[16], kills[16], losses[16], points[16];
		FormatTime(flight_time, p->FlightTime());
		sprintf_s(missions, "%d", p->Missions());
		sprintf_s(kills,    "%d", p->Kills());
		sprintf_s(losses,   "%d", p->Losses());
		sprintf_s(points,   "%d", p->Points());

		if (lbl_createdate)  lbl_createdate->SetText(FormatTimeString(p->CreateDate()));
		if (lbl_rank)        lbl_rank->SetText(Player::RankName(p->Rank()));
		if (lbl_flighttime)  lbl_flighttime->SetText(flight_time);
		if (lbl_missions)    lbl_missions->SetText(missions);
		if (lbl_kills)       lbl_kills->SetText(kills);
		if (lbl_losses)      lbl_losses->SetText(losses);
		if (lbl_points)      lbl_points->SetText(points);

		if (img_rank) {
			img_rank->SetPicture(*Player::RankInsignia(p->Rank(), 0));
			img_rank->Show();
		}

		for (int i = 0; i < 15; i++) {
			if (img_medals[i]) {
				int medal = p->Medal(i);
				if (medal) {
					medals[i] = medal;
					img_medals[i]->SetPicture(*Player::MedalInsignia(medal, 0));
					img_medals[i]->Show();
				}
				else {
					medals[i] = -1;
					img_medals[i]->Hide();
				}
			}
		}

		for (int i = 0; i < 10; i++) {
			if (txt_chat[i])
			txt_chat[i]->SetText(p->ChatMacro(i));
		}
	}
	else {
		if (txt_name)        txt_name->SetText("");
		if (txt_password)    txt_password->SetText("");
		if (txt_squadron)    txt_squadron->SetText("");
		if (txt_signature)   txt_signature->SetText("");

		if (lbl_createdate)  lbl_createdate->SetText("");
		if (lbl_rank)        lbl_rank->SetText("");
		if (lbl_flighttime)  lbl_flighttime->SetText("");
		if (lbl_missions)    lbl_missions->SetText("");
		if (lbl_kills)       lbl_kills->SetText("");
		if (lbl_losses)      lbl_losses->SetText("");
		if (lbl_points)      lbl_points->SetText("");

		if (img_rank)        img_rank->Hide();

		for (int i = 0; i < 15; i++) {
			medals[i] = -1;
			if (img_medals[i])
			img_medals[i]->Hide();
		}

		for (int i = 0; i < 10; i++) {
			if (txt_chat[i])
			txt_chat[i]->SetText("");
		}
	}
}

// +--------------------------------------------------------------------+

void
PlayerDlg::UpdatePlayer()
{
	Player* p = Player::GetCurrentPlayer();

	if (p) {
		if (txt_name)        p->SetName(txt_name->GetText());
		if (txt_password)    p->SetPassword(txt_password->GetText());
		if (txt_squadron)    p->SetSquadron(txt_squadron->GetText());
		if (txt_signature)   p->SetSignature(txt_signature->GetText());

		for (int i = 0; i < 10; i++) {
			if (txt_chat[i])
			p->SetChatMacro(i, txt_chat[i]->GetText());
		}
	}
}

// +--------------------------------------------------------------------+

void
PlayerDlg::OnSelectPlayer(AWEvent* event)
{
	if (!lst_roster) return;

	UpdatePlayer();

	int index = lst_roster->GetSelection();

	List<Player>& roster = Player::GetRoster();
	if (index >= 0 && index < roster.size()) {
		Player::SelectPlayer(roster.at(index));
	}

	ShowPlayer();

	apply->SetEnabled(roster.size() > 0);
}

void
PlayerDlg::OnAdd(AWEvent* event)
{
	Player::Create("Pilot");
	ShowPlayer();

	if (!lst_roster) return;
	lst_roster->ClearItems();

	List<Player>& roster = Player::GetRoster();
	for (int i = 0; i < roster.size(); i++) {
		Player* p = roster[i];
		lst_roster->AddItem(p->Name());
		lst_roster->SetSelected(i, (p == Player::GetCurrentPlayer()));
	}

	apply->SetEnabled(roster.size() > 0);
}

void
PlayerDlg::OnDel(AWEvent* event)
{
	if (!Player::GetCurrentPlayer())
	return;

	ConfirmDlg* confirm = manager->GetConfirmDlg();
	if (confirm) {
		char msg[256];
		sprintf_s(msg, Game::GetText("PlayerDlg.are-you-sure").data(),
		Player::GetCurrentPlayer()->Name().data());
		confirm->SetMessage(msg);
		confirm->SetTitle(Game::GetText("PlayerDlg.confirm-delete"));
		confirm->SetParentControl(btn_del);

		manager->ShowConfirmDlg();
	}

	else {
		OnDelConfirm(event);
	}
}

void
PlayerDlg::OnDelConfirm(AWEvent* event)
{
	Player::Destroy(Player::GetCurrentPlayer());
	ShowPlayer();

	if (!lst_roster) return;
	lst_roster->ClearItems();

	List<Player>& roster = Player::GetRoster();
	for (int i = 0; i < roster.size(); i++) {
		Player* p = roster[i];
		lst_roster->AddItem(p->Name());
		lst_roster->SetSelected(i, (p == Player::GetCurrentPlayer()));
	}

	apply->SetEnabled(roster.size() > 0);
}

// +--------------------------------------------------------------------+

void
PlayerDlg::OnRank(AWEvent* event)
{
	Player*        p           = Player::GetCurrentPlayer();
	AwardShowDlg*  award_dlg   = manager->GetAwardDlg();

	if (p && award_dlg) {
		award_dlg->SetRank(p->Rank());
		manager->ShowAwardDlg();
	}
}

void
PlayerDlg::OnMedal(AWEvent* event)
{
	Player*        p           = Player::GetCurrentPlayer();
	AwardShowDlg*  award_dlg   = manager->GetAwardDlg();

	if (p && award_dlg) {
		int m = -1, i;

		for (i = 0; i < 15; i++) {
			if (event->window == img_medals[i]) {
				m = i;
				break;
			}
		}

		if (m >= 0) {
			award_dlg->SetMedal(medals[i]);
			manager->ShowAwardDlg();
		}
	}
}

// +--------------------------------------------------------------------+

void
PlayerDlg::OnApply(AWEvent* event)
{
	Player* player = Player::GetCurrentPlayer();
	if (player) {
		UpdatePlayer();
		Player::Save();
	}

	FlushKeys();
	manager->ShowMenuDlg();
}

void
PlayerDlg::OnCancel(AWEvent* event)
{
	Player::Load();
	FlushKeys();
	manager->ShowMenuDlg();
}
