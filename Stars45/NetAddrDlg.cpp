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
    FILE:         NetAddrDlg.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Mission Select Dialog Active Window class
*/

#include "MemDebug.h"
#include "NetAddrDlg.h"
#include "MenuScreen.h"
#include "NetClientConfig.h"

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

DEF_MAP_CLIENT(NetAddrDlg, OnSave);
DEF_MAP_CLIENT(NetAddrDlg, OnCancel);

// +--------------------------------------------------------------------+

NetAddrDlg::NetAddrDlg(Screen* s, FormDef& def, MenuScreen* mgr)
: FormWindow(s, 0, 0, s->Width(), s->Height()), manager(mgr),
btn_save(0), btn_cancel(0), edt_name(0), edt_addr(0), edt_port(0), edt_pass(0)
{
    Init(def);
}

NetAddrDlg::~NetAddrDlg()
{
}

// +--------------------------------------------------------------------+

void
NetAddrDlg::RegisterControls()
{
    btn_save    = (Button*) FindControl(1);
    btn_cancel  = (Button*) FindControl(2);

    REGISTER_CLIENT(EID_CLICK, btn_save,   NetAddrDlg, OnSave);
    REGISTER_CLIENT(EID_CLICK, btn_cancel, NetAddrDlg, OnCancel);

    edt_name    = (EditBox*) FindControl(200);
    edt_addr    = (EditBox*) FindControl(201);
    edt_port    = (EditBox*) FindControl(202);
    edt_pass    = (EditBox*) FindControl(203);

    if (edt_name) edt_name->SetText("");
    if (edt_addr) edt_addr->SetText("");
    if (edt_port) edt_port->SetText("");
    if (edt_pass) edt_pass->SetText("");
}

// +--------------------------------------------------------------------+

void
NetAddrDlg::Show()
{
    if (!IsShown()) {
        FormWindow::Show();

        if (edt_name)  edt_name->SetText("");
        if (edt_addr)  edt_addr->SetText("");
        if (edt_port)  edt_port->SetText("");
        if (edt_pass)  edt_pass->SetText("");

        if (edt_name)  edt_name->SetFocus();
    }
}

// +--------------------------------------------------------------------+

static bool tab_latch = false;

void
NetAddrDlg::ExecFrame()
{
    if (Keyboard::KeyDown(VK_RETURN)) {
        OnSave(0);
    }
}

// +--------------------------------------------------------------------+

void
NetAddrDlg::OnSave(AWEvent* event)
{
    NetClientConfig* config = NetClientConfig::GetInstance();

    if (config   && 
            edt_addr && edt_addr->GetText().length() > 0 &&
            edt_port && edt_port->GetText().length() > 0)
    {
        Text name;
        Text addr;
        Text pass;
        int  port;

        sscanf_s(edt_port->GetText().data(), "%d", &port);

        if (edt_name && edt_name->GetText().length() < 250) {
            char buffer[256];
            strcpy_s(buffer, edt_name->GetText().data());
            char* p = strpbrk(buffer, "\n\r\t");
            if (p) *p = 0;

            name = SafeQuotes(buffer);
        }

        if (edt_pass && edt_pass->GetText().length() < 250) {
            char buffer[256];
            strcpy_s(buffer, edt_pass->GetText().data());
            char* p = strpbrk(buffer, "\n\r\t");
            if (p) *p = 0;

            pass = SafeQuotes(buffer);
        }

        if (edt_addr && edt_addr->GetText().length() < 250) {
            char buffer[256];
            strcpy_s(buffer, edt_addr->GetText().data());
            char* p = strpbrk(buffer, "\n\r\t");
            if (p) *p = 0;

            addr = SafeQuotes(buffer);
        }

        config->AddServer(name, addr, port, pass, true);
        config->Save();
    }

    if (manager)
    manager->ShowNetClientDlg();
}

void
NetAddrDlg::OnCancel(AWEvent* event)
{
    if (manager)
    manager->ShowNetClientDlg();
}
