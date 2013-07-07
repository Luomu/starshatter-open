/*  Starshatter OpenSource Distribution
    Copyright (c) 1997-2004, Destroyer Studios LLC.
    All Rights Reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.
    * Neither the name "Destroyer Studios" nor the names of its contributors
      may be used to endorse or promote products derived from this software
      without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

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
