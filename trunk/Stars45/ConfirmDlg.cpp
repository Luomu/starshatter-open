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
    FILE:         ConfirmDlg.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    General-purpose confirmation dialog class
*/

#include "MemDebug.h"
#include "ConfirmDlg.h"
#include "MenuScreen.h"
#include "Starshatter.h"
#include "FormatUtil.h"

#include "Game.h"
#include "Keyboard.h"
#include "Button.h"

// +--------------------------------------------------------------------+
// DECLARE MAPPING FUNCTIONS:

DEF_MAP_CLIENT(ConfirmDlg, OnApply);
DEF_MAP_CLIENT(ConfirmDlg, OnCancel);

// +--------------------------------------------------------------------+

ConfirmDlg::ConfirmDlg(Screen* s, FormDef& def, MenuScreen* mgr)
: FormWindow(s,  0,  0, s->Width(), s->Height()), manager(mgr),
parent_control(0), btn_apply(0), btn_cancel(0)
{
    Init(def);
}

ConfirmDlg::~ConfirmDlg()
{
}

void
ConfirmDlg::RegisterControls()
{
    if (btn_apply)
    return;

    btn_apply   = (Button*) FindControl(1);
    REGISTER_CLIENT(EID_CLICK, btn_apply,  ConfirmDlg, OnApply);

    btn_cancel  = (Button*) FindControl(2);
    REGISTER_CLIENT(EID_CLICK, btn_cancel, ConfirmDlg, OnCancel);

    lbl_title   = FindControl(100);
    lbl_message = FindControl(101);
}

// +--------------------------------------------------------------------+

ActiveWindow*
ConfirmDlg::GetParentControl()
{
    return parent_control;
}

void
ConfirmDlg::SetParentControl(ActiveWindow* p)
{
    parent_control = p;
}

Text
ConfirmDlg::GetTitle()
{
    if (lbl_title)
    return lbl_title->GetText();

    return "";
}

void
ConfirmDlg::SetTitle(const char* t)
{
    if (lbl_title)
    lbl_title->SetText(t);
}

Text
ConfirmDlg::GetMessage()
{
    if (lbl_message)
    return lbl_message->GetText();

    return "";
}

void
ConfirmDlg::SetMessage(const char* m)
{
    if (lbl_message)
    lbl_message->SetText(m);
}

// +--------------------------------------------------------------------+

void
ConfirmDlg::ExecFrame()
{
    if (Keyboard::KeyDown(VK_RETURN)) {
        OnApply(0);
    }

    if (Keyboard::KeyDown(VK_ESCAPE)) {
        OnCancel(0);
    }
}

// +--------------------------------------------------------------------+

void
ConfirmDlg::Show()
{
    if (!IsShown()) {
        Button::PlaySound(Button::SND_CONFIRM);
    }

    FormWindow::Show();
    SetFocus();
}

// +--------------------------------------------------------------------+

void
ConfirmDlg::OnApply(AWEvent* event)
{
    manager->HideConfirmDlg();

    if (parent_control)
    parent_control->ClientEvent(EID_USER_1);
}

void
ConfirmDlg::OnCancel(AWEvent* event)
{
    manager->HideConfirmDlg();
}

// +--------------------------------------------------------------------+
