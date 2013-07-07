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
    FILE:         FirstTimeDlg.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Main Menu Dialog Active Window class
*/

#include "MemDebug.h"
#include "FirstTimeDlg.h"
#include "Player.h"
#include "MenuScreen.h"
#include "Ship.h"
#include "Starshatter.h"
#include "KeyMap.h"
#include "Random.h"

#include "DataLoader.h"
#include "Button.h"
#include "EditBox.h"
#include "ComboBox.h"
#include "Video.h"
#include "Keyboard.h"
#include "MachineInfo.h"

// +--------------------------------------------------------------------+
// DECLARE MAPPING FUNCTIONS:

DEF_MAP_CLIENT(FirstTimeDlg, OnApply);

// +--------------------------------------------------------------------+

FirstTimeDlg::FirstTimeDlg(Screen* s, FormDef& def, MenuScreen* mgr)
: FormWindow(s, 0, 0, s->Width(), s->Height()), manager(mgr)
{
    Init(def);
}

FirstTimeDlg::~FirstTimeDlg()
{
}

// +--------------------------------------------------------------------+

void
FirstTimeDlg::RegisterControls()
{
    edt_name       = (EditBox*)   FindControl(200);
    cmb_playstyle  = (ComboBox*)  FindControl(201);
    cmb_experience = (ComboBox*)  FindControl(202);

    btn_apply      = (Button*)    FindControl(1);
    REGISTER_CLIENT(EID_CLICK,  btn_apply,    FirstTimeDlg, OnApply);
}

// +--------------------------------------------------------------------+

void
FirstTimeDlg::Show()
{
    if (!IsShown())
    FormWindow::Show();

    if (edt_name)
    edt_name->SetText("Noobie");
}

// +--------------------------------------------------------------------+

void
FirstTimeDlg::ExecFrame()
{
}

// +--------------------------------------------------------------------+

void
FirstTimeDlg::OnApply(AWEvent* event)
{
    Starshatter*   stars    = Starshatter::GetInstance();
    Player*        player   = Player::GetCurrentPlayer();

    if (player) {
        if (edt_name) {
            char password[16];
            sprintf_s(password, "%08x", (DWORD) Random(0, 2e9));

            player->SetName(edt_name->GetText());
            player->SetPassword(password);
        }

        if (cmb_playstyle) {
            // ARCADE:
            if (cmb_playstyle->GetSelectedIndex() == 0) {
                player->SetFlightModel(2);
                player->SetLandingModel(1);
                player->SetHUDMode(0);
                player->SetGunsight(1);

                if (stars) {
                    KeyMap&  keymap    = stars->GetKeyMap();

                    keymap.Bind(KEY_CONTROL_MODEL, 1, 0);
                    keymap.SaveKeyMap("key.cfg", 256);

                    stars->MapKeys();
                }

                Ship::SetControlModel(1);
            }

            // HARDCORE:
            else {
                player->SetFlightModel(0);
                player->SetLandingModel(0);
                player->SetHUDMode(0);
                player->SetGunsight(0);

                if (stars) {
                    KeyMap&  keymap    = stars->GetKeyMap();

                    keymap.Bind(KEY_CONTROL_MODEL, 0, 0);
                    keymap.SaveKeyMap("key.cfg", 256);

                    stars->MapKeys();
                }

                Ship::SetControlModel(0);
            }
        }

        if (cmb_experience && cmb_experience->GetSelectedIndex() > 0) {
            player->SetRank(2);        // Lieutenant
            player->SetTrained(255);   // Fully Trained
        }

        Player::Save();
    }

    manager->ShowMenuDlg();
}
