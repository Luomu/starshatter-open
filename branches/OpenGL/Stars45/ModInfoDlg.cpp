/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         ModInfoDlg.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Mod Config Dialog Active Window class
*/

#include "MemDebug.h"
#include "ModInfoDlg.h"
#include "BaseScreen.h"
#include "ModConfig.h"
#include "ModInfo.h"

#include "Game.h"
#include "DataLoader.h"
#include "Button.h"
#include "ListBox.h"
#include "ImageBox.h"
#include "Video.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "ParseUtil.h"

// +--------------------------------------------------------------------+
// DECLARE MAPPING FUNCTIONS:

DEF_MAP_CLIENT(ModInfoDlg, OnAccept);

// +--------------------------------------------------------------------+

ModInfoDlg::ModInfoDlg(Screen* s, FormDef& def, BaseScreen* mgr)
: FormWindow(s,  0,  0, s->Width(), s->Height()), manager(mgr),
btn_accept(0), mod_info(0)
{
	Init(def);
}

ModInfoDlg::~ModInfoDlg()
{
}

// +--------------------------------------------------------------------+

void
ModInfoDlg::RegisterControls()
{
	btn_accept     = (Button*) FindControl(  1);

	if (btn_accept)
	REGISTER_CLIENT(EID_CLICK,    btn_accept,    ModInfoDlg, OnAccept);

	lbl_name       = FindControl(101);
	lbl_desc       = FindControl(102);
	lbl_copy       = FindControl(103);

	img_logo       = (ImageBox*) FindControl(200);

	if (img_logo) {
		img_logo->GetPicture(bmp_default);
		img_logo->SetBlendMode(Video::BLEND_SOLID);
	}
}

// +--------------------------------------------------------------------+

void
ModInfoDlg::Show()
{
	FormWindow::Show();
}

// +--------------------------------------------------------------------+

void
ModInfoDlg::ExecFrame()
{
	if (Keyboard::KeyDown(VK_RETURN)) {
		if (btn_accept && btn_accept->IsEnabled())
		OnAccept(0);
	}
}

// +--------------------------------------------------------------------+

void
ModInfoDlg::SetModInfo(ModInfo* info)
{
	mod_info = info;

	if (mod_info) {
		if (lbl_name)  lbl_name->SetText(mod_info->Name());
		if (lbl_desc)  lbl_desc->SetText(mod_info->Description());
		if (lbl_copy)  lbl_copy->SetText(mod_info->Copyright());

		if (img_logo && mod_info->LogoImage() && mod_info->LogoImage()->Width() > 32)
		img_logo->SetPicture(*mod_info->LogoImage());
		else if (img_logo)
		img_logo->SetPicture(bmp_default);
	}
}

// +--------------------------------------------------------------------+

void
ModInfoDlg::OnAccept(AWEvent* event)
{
	manager->ShowModDlg();
}
