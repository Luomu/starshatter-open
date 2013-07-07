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
    FILE:         ExitDlg.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
*/

#include "MemDebug.h"
#include "ExitDlg.h"
#include "MenuScreen.h"
#include "MusicDirector.h"
#include "Starshatter.h"
#include "FormatUtil.h"

#include "Game.h"
#include "Keyboard.h"
#include "Button.h"
#include "RichTextBox.h"
#include "DataLoader.h"

// +--------------------------------------------------------------------+
// DECLARE MAPPING FUNCTIONS:

DEF_MAP_CLIENT(ExitDlg, OnApply);
DEF_MAP_CLIENT(ExitDlg, OnCancel);

// +--------------------------------------------------------------------+

ExitDlg::ExitDlg(Screen* s, FormDef& def, MenuScreen* mgr)
: FormWindow(s,  0,  0, s->Width(), s->Height()), 
manager(mgr), exit_latch(false),
credits(0), apply(0), cancel(0),
def_rect(def.GetRect())
{
    Init(def);
}

ExitDlg::~ExitDlg()
{
}

void
ExitDlg::RegisterControls()
{
    if (apply)
    return;

    credits = (RichTextBox*) FindControl(201);

    apply   = (Button*) FindControl(1);
    REGISTER_CLIENT(EID_CLICK, apply, ExitDlg, OnApply);

    cancel  = (Button*) FindControl(2);
    REGISTER_CLIENT(EID_CLICK, cancel, ExitDlg, OnCancel);
}

// +--------------------------------------------------------------------+

void
ExitDlg::ExecFrame()
{
    if (credits && credits->GetLineCount() > 0) {
        credits->SmoothScroll(ScrollWindow::SCROLL_DOWN, Game::GUITime());

        if (credits->GetTopIndex() >= credits->GetLineCount()-1) {
            credits->ScrollTo(0);
        }
    }

    if (Keyboard::KeyDown(VK_RETURN)) {
        OnApply(0);
    }

    if (Keyboard::KeyDown(VK_ESCAPE)) {
        if (!exit_latch)
        OnCancel(0);
    }
    else {
        exit_latch = false;
    }
}

// +--------------------------------------------------------------------+

void
ExitDlg::Show()
{
    if (!IsShown()) {
        Rect r = def_rect;

        if (r.w > screen->Width()) {
            int extra = r.w - screen->Width();
            r.w -= extra;
        }

        if (r.h > screen->Height()) {
            int extra = r.h - screen->Height();
            r.h -= extra;
        }

        r.x = (screen->Width()  - r.w) / 2;
        r.y = (screen->Height() - r.h) / 2;

        MoveTo(r);

        exit_latch = true;
        Button::PlaySound(Button::SND_CONFIRM);
        MusicDirector::SetMode(MusicDirector::CREDITS);

        DataLoader* loader = DataLoader::GetLoader();
        BYTE*       block    = 0;

        loader->SetDataPath(0);
        loader->LoadBuffer("credits.txt", block, true);

        if (block && credits) {
            credits->SetText((const char*) block);
        }

        loader->ReleaseBuffer(block);
    }

    FormWindow::Show();
}

// +--------------------------------------------------------------------+

void
ExitDlg::OnApply(AWEvent* event)
{
    Starshatter* stars = Starshatter::GetInstance();

    if (stars) {
        ::Print("Exit Confirmed.\n");
        stars->Exit();
    }
}

void
ExitDlg::OnCancel(AWEvent* event)
{
    manager->ShowMenuDlg();
    MusicDirector::SetMode(MusicDirector::MENU);
}

// +--------------------------------------------------------------------+
