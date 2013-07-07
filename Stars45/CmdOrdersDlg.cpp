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

