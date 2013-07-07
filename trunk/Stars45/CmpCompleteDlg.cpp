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
    FILE:         CmpCompleteDlg.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
*/

#include "MemDebug.h"
#include "CmpCompleteDlg.h"
#include "CmpnScreen.h"
#include "Campaign.h"
#include "CombatEvent.h"
#include "Starshatter.h"

#include "Game.h"
#include "DataLoader.h"
#include "Video.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "ImageBox.h"
#include "Button.h"
#include "FormatUtil.h"

// +--------------------------------------------------------------------+
// DECLARE MAPPING FUNCTIONS:

DEF_MAP_CLIENT(CmpCompleteDlg, OnClose);

// +--------------------------------------------------------------------+

CmpCompleteDlg::CmpCompleteDlg(Screen* s, FormDef& def, CmpnScreen* mgr)
: FormWindow(s, 0, 0, s->Width(), s->Height()), manager(mgr),
lbl_info(0), img_title(0), btn_close(0)
{
    Init(def);
}

CmpCompleteDlg::~CmpCompleteDlg()
{
}

void
CmpCompleteDlg::RegisterControls()
{
    img_title    = (ImageBox*) FindControl(100);
    lbl_info     =             FindControl(101);
    btn_close    = (Button*)   FindControl(1);

    REGISTER_CLIENT(EID_CLICK, btn_close, CmpCompleteDlg, OnClose);
}

void
CmpCompleteDlg::Show()
{
    FormWindow::Show();

    Campaign* c = Campaign::GetCampaign();

    if (img_title && c) {
        DataLoader*    loader = DataLoader::GetLoader();
        Starshatter*   stars  = Starshatter::GetInstance();
        CombatEvent*   event  = c->GetLastEvent();
        char           img_name[256];

        if (event) {
            strcpy_s(img_name, event->ImageFile());

            if (!strstr(img_name, ".pcx")) {
                strcat_s(img_name, ".pcx");
            }

            if (loader) {
                loader->SetDataPath(c->Path());
                loader->LoadBitmap(img_name, banner);
                loader->SetDataPath(0);

                Rect tgt_rect;
                tgt_rect.w = img_title->Width();
                tgt_rect.h = img_title->Height();

                img_title->SetTargetRect(tgt_rect);
                img_title->SetPicture(banner);
            }
        }
    }
}

// +--------------------------------------------------------------------+

void
CmpCompleteDlg::ExecFrame()
{
}

// +--------------------------------------------------------------------+

void
CmpCompleteDlg::OnClose(AWEvent* event)
{
    if (manager)
    manager->ShowCmdDlg();
}
