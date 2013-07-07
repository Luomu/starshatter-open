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
    FILE:         NetPassDlg.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Network Server Password Dialog Active Window class
*/

#include "MemDebug.h"
#include "NetPassDlg.h"
#include "MenuScreen.h"
#include "NetClientConfig.h"
#include "NetLobby.h"

#include "Game.h"
#include "DataLoader.h"
#include "Button.h"
#include "EditBox.h"
#include "ListBox.h"
#include "Slider.h"
#include "Video.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "ParseUtil.h"
#include "FormatUtil.h"

// +--------------------------------------------------------------------+
// DECLARE MAPPING FUNCTIONS:

DEF_MAP_CLIENT(NetPassDlg, OnApply);
DEF_MAP_CLIENT(NetPassDlg, OnCancel);

// +--------------------------------------------------------------------+

NetPassDlg::NetPassDlg(Screen* s, FormDef& def, MenuScreen* mgr)
: FormWindow(s, 0, 0, s->Width(), s->Height()), manager(mgr),
btn_apply(0), btn_cancel(0), edt_pass(0), lbl_name(0)
{
    Init(def);
}

NetPassDlg::~NetPassDlg()
{
}

// +--------------------------------------------------------------------+

void
NetPassDlg::RegisterControls()
{
    btn_apply   = (Button*) FindControl(1);
    btn_cancel  = (Button*) FindControl(2);

    REGISTER_CLIENT(EID_CLICK, btn_apply,  NetPassDlg, OnApply);
    REGISTER_CLIENT(EID_CLICK, btn_cancel, NetPassDlg, OnCancel);

    lbl_name    =            FindControl(110);
    edt_pass    = (EditBox*) FindControl(200);

    if (edt_pass)
    edt_pass->SetText("");
}

// +--------------------------------------------------------------------+

void
NetPassDlg::Show()
{
    if (!IsShown()) {
        FormWindow::Show();

        NetClientConfig* config = NetClientConfig::GetInstance();

        if (config && lbl_name) {
            NetServerInfo* info = config->GetSelectedServer();

            if (info)
            lbl_name->SetText(info->name);
        }

        if (edt_pass) {
            edt_pass->SetText("");
            edt_pass->SetFocus();
        }
    }
}

// +--------------------------------------------------------------------+

static bool tab_latch = false;

void
NetPassDlg::ExecFrame()
{
    if (Keyboard::KeyDown(VK_RETURN)) {
        OnApply(0);
    }
}

// +--------------------------------------------------------------------+

void
NetPassDlg::OnApply(AWEvent* event)
{
    NetClientConfig* config = NetClientConfig::GetInstance();

    if (config && edt_pass) {
        NetServerInfo* info = config->GetSelectedServer();

        if (info && edt_pass->GetText().length() < 250) {
            char buffer[256];
            strcpy_s(buffer, edt_pass->GetText().data());

            // trim from first occurrence of invalid character
            char* p = strpbrk(buffer, "\n\r\t");
            if (p) *p = 0;

            info->password = SafeQuotes(buffer);

            if (manager) {
                manager->ShowNetLobbyDlg();
                return;
            }
        }
    }

    if (manager) {
        manager->ShowNetClientDlg();
    }
}

void
NetPassDlg::OnCancel(AWEvent* event)
{
    if (manager)
    manager->ShowNetClientDlg();
}
