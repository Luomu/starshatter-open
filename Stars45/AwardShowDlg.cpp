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
