/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         CmpCompleteDlg.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
*/

#include "MemDebug.h"
#include "CmpCompleteDlg.h"
#include "CmpnScreen.h"
#include "Campaign.h"
#include "CombatEvent.h"
#include "Starshatter.h"

#include "Game.h"
#include "DataLoader.h"
#include "Video.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "ImageBox.h"
#include "Button.h"
#include "FormatUtil.h"

// +--------------------------------------------------------------------+
// DECLARE MAPPING FUNCTIONS:

DEF_MAP_CLIENT(CmpCompleteDlg, OnClose);

// +--------------------------------------------------------------------+

CmpCompleteDlg::CmpCompleteDlg(Screen* s, FormDef& def, CmpnScreen* mgr)
: FormWindow(s, 0, 0, s->Width(), s->Height()), manager(mgr),
lbl_info(0), img_title(0), btn_close(0)
{
	Init(def);
}

CmpCompleteDlg::~CmpCompleteDlg()
{
}

void
CmpCompleteDlg::RegisterControls()
{
	img_title    = (ImageBox*) FindControl(100);
	lbl_info     =             FindControl(101);
	btn_close    = (Button*)   FindControl(1);

	REGISTER_CLIENT(EID_CLICK, btn_close, CmpCompleteDlg, OnClose);
}

void
CmpCompleteDlg::Show()
{
	FormWindow::Show();

	Campaign* c = Campaign::GetCampaign();

	if (img_title && c) {
		DataLoader*    loader = DataLoader::GetLoader();
		Starshatter*   stars  = Starshatter::GetInstance();
		CombatEvent*   event  = c->GetLastEvent();
		char           img_name[256];

		if (event) {
			strcpy_s(img_name, event->ImageFile());

			if (!strstr(img_name, ".pcx")) {
				strcat_s(img_name, ".pcx");
			}

			if (loader) {
				loader->SetDataPath(c->Path());
				loader->LoadBitmap(img_name, banner);
				loader->SetDataPath(0);

				Rect tgt_rect;
				tgt_rect.w = img_title->Width();
				tgt_rect.h = img_title->Height();

				img_title->SetTargetRect(tgt_rect);
				img_title->SetPicture(banner);
			}
		}
	}
}

// +--------------------------------------------------------------------+

void
CmpCompleteDlg::ExecFrame()
{
}

// +--------------------------------------------------------------------+

void
CmpCompleteDlg::OnClose(AWEvent* event)
{
	if (manager)
	manager->ShowCmdDlg();
}
