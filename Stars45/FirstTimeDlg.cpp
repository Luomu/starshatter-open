/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         FirstTimeDlg.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Main Menu Dialog Active Window class
*/

#include "MemDebug.h"
#include "FirstTimeDlg.h"
#include "Player.h"
#include "MenuScreen.h"
#include "Ship.h"
#include "Starshatter.h"
#include "KeyMap.h"
#include "Random.h"

#include "DataLoader.h"
#include "Button.h"
#include "EditBox.h"
#include "ComboBox.h"
#include "Video.h"
#include "Keyboard.h"
#include "MachineInfo.h"

// +--------------------------------------------------------------------+
// DECLARE MAPPING FUNCTIONS:

DEF_MAP_CLIENT(FirstTimeDlg, OnApply);

// +--------------------------------------------------------------------+

FirstTimeDlg::FirstTimeDlg(Screen* s, FormDef& def, MenuScreen* mgr)
: FormWindow(s, 0, 0, s->Width(), s->Height()), manager(mgr)
{
	Init(def);
}

FirstTimeDlg::~FirstTimeDlg()
{
}

// +--------------------------------------------------------------------+

void
FirstTimeDlg::RegisterControls()
{
	edt_name       = (EditBox*)   FindControl(200);
	cmb_playstyle  = (ComboBox*)  FindControl(201);
	cmb_experience = (ComboBox*)  FindControl(202);

	btn_apply      = (Button*)    FindControl(1);
	REGISTER_CLIENT(EID_CLICK,  btn_apply,    FirstTimeDlg, OnApply);
}

// +--------------------------------------------------------------------+

void
FirstTimeDlg::Show()
{
	if (!IsShown())
	FormWindow::Show();

	if (edt_name)
	edt_name->SetText("Noobie");
}

// +--------------------------------------------------------------------+

void
FirstTimeDlg::ExecFrame()
{
}

// +--------------------------------------------------------------------+

void
FirstTimeDlg::OnApply(AWEvent* event)
{
	Starshatter*   stars    = Starshatter::GetInstance();
	Player*        player   = Player::GetCurrentPlayer();

	if (player) {
		if (edt_name) {
			char password[16];
			sprintf(password, "%08x", (DWORD) Random(0, 2e9));

			player->SetName(edt_name->GetText());
			player->SetPassword(password);
		}

		if (cmb_playstyle) {
			// ARCADE:
			if (cmb_playstyle->GetSelectedIndex() == 0) {
				player->SetFlightModel(2);
				player->SetLandingModel(1);
				player->SetHUDMode(0);
				player->SetGunsight(1);

				if (stars) {
					KeyMap&  keymap    = stars->GetKeyMap();

					keymap.Bind(KEY_CONTROL_MODEL, 1, 0);
					keymap.SaveKeyMap("key.cfg", 256);

					stars->MapKeys();
				}

				Ship::SetControlModel(1);
			}

			// HARDCORE:
			else {
				player->SetFlightModel(0);
				player->SetLandingModel(0);
				player->SetHUDMode(0);
				player->SetGunsight(0);

				if (stars) {
					KeyMap&  keymap    = stars->GetKeyMap();

					keymap.Bind(KEY_CONTROL_MODEL, 0, 0);
					keymap.SaveKeyMap("key.cfg", 256);

					stars->MapKeys();
				}

				Ship::SetControlModel(0);
			}
		}

		if (cmb_experience && cmb_experience->GetSelectedIndex() > 0) {
			player->SetRank(2);        // Lieutenant
			player->SetTrained(255);   // Fully Trained
		}

		Player::Save();
	}

	manager->ShowMenuDlg();
}
