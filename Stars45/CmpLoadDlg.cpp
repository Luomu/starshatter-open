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
    FILE:         CmpLoadDlg.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
*/

#include "MemDebug.h"
#include "CmpLoadDlg.h"
#include "Campaign.h"
#include "Starshatter.h"
#include "FormatUtil.h"

#include "Game.h"
#include "DataLoader.h"
#include "Video.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "ImageBox.h"
#include "Slider.h"

// +--------------------------------------------------------------------+

CmpLoadDlg::CmpLoadDlg(Screen* s, FormDef& def)
: FormWindow(s, 0, 0, s->Width(), s->Height()),
lbl_progress(0), lbl_activity(0), lbl_title(0), img_title(0), show_time(0)
{
    Init(def);
}

CmpLoadDlg::~CmpLoadDlg()
{
}

void
CmpLoadDlg::RegisterControls()
{
    img_title    = (ImageBox*) FindControl(100);
    lbl_title    =             FindControl(200);
    lbl_activity =             FindControl(101);
    lbl_progress = (Slider*)   FindControl(102);
}

void
CmpLoadDlg::Show()
{
    FormWindow::Show();

    Campaign* campaign = Campaign::GetCampaign();

    if (campaign) {
        Bitmap* bmp = campaign->GetImage(3);
        if (img_title && bmp) {
            Rect tgt_rect;
            tgt_rect.w = img_title->Width();
            tgt_rect.h = img_title->Height();

            img_title->SetTargetRect(tgt_rect);
            img_title->SetPicture(*bmp);
        }

        if (lbl_title)
        lbl_title->SetText(campaign->Name());
    }

    show_time = Game::RealTime();
}

// +--------------------------------------------------------------------+

void
CmpLoadDlg::ExecFrame()
{
    Starshatter* stars = Starshatter::GetInstance();

    if (stars) {
        if (lbl_activity) lbl_activity->SetText(stars->GetLoadActivity());
        if (lbl_progress) lbl_progress->SetValue(stars->GetLoadProgress());
    }
}

// +--------------------------------------------------------------------+

void
CmpLoadDlg::MoveTo(const Rect& r)
{
    FormWindow::MoveTo(r);

    Campaign* campaign = Campaign::GetCampaign();

    if (campaign && img_title && campaign->GetImage(3)) {
        Bitmap* bmp = campaign->GetImage(3);

        Rect tgt_rect;
        tgt_rect.w = img_title->Width();
        tgt_rect.h = img_title->Height();

        img_title->SetTargetRect(tgt_rect);
        img_title->SetPicture(*bmp);
    }
}

// +--------------------------------------------------------------------+

bool
CmpLoadDlg::IsDone()
{
    if (Game::RealTime() - show_time < 5000)
    return false;

    return true;
}
