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
