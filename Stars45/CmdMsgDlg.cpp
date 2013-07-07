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
    FILE:         CmdMsgDlg.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
*/

#include "MemDebug.h"
#include "CmdMsgDlg.h"
#include "CmpnScreen.h"
#include "Starshatter.h"

#include "Game.h"
#include "ListBox.h"
#include "ComboBox.h"
#include "Button.h"
#include "Keyboard.h"
#include "FormatUtil.h"

// +--------------------------------------------------------------------+
// DECLARE MAPPING FUNCTIONS:

DEF_MAP_CLIENT(CmdMsgDlg, OnApply);

// +--------------------------------------------------------------------+

CmdMsgDlg::CmdMsgDlg(Screen* s, FormDef& def, CmpnScreen* mgr)
    : FormWindow(s, 0, 0, s->Width(), s->Height()), manager(mgr),
      exit_latch(false)
{
    Init(def);
}

CmdMsgDlg::~CmdMsgDlg()
{
}

void
CmdMsgDlg::RegisterControls()
{
    title   = FindControl(100);
    message = FindControl(101);

    apply   = (Button*) FindControl(1);
    REGISTER_CLIENT(EID_CLICK, apply, CmdMsgDlg, OnApply);
}

// +--------------------------------------------------------------------+

void
CmdMsgDlg::ExecFrame()
{
    if (Keyboard::KeyDown(VK_RETURN)) {
        OnApply(0);
    }

    if (Keyboard::KeyDown(VK_ESCAPE)) {
        if (!exit_latch)
        OnApply(0);

        exit_latch = true;
    }
    else {
        exit_latch = false;
    }
}

// +--------------------------------------------------------------------+

void
CmdMsgDlg::Show()
{
    FormWindow::Show();
    SetFocus();
}

// +--------------------------------------------------------------------+

void
CmdMsgDlg::OnApply(AWEvent* event)
{
    if (manager)
    manager->CloseTopmost();
}

// +--------------------------------------------------------------------+
