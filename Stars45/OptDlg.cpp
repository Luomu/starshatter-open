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
    FILE:         OptDlg.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Main Menu Dialog Active Window class
*/

#include "MemDebug.h"
#include "OptDlg.h"
#include "MenuScreen.h"
#include "Starshatter.h"
#include "Ship.h"
#include "HUDView.h"
#include "Player.h"

#include "DataLoader.h"
#include "Button.h"
#include "ListBox.h"
#include "Slider.h"
#include "Video.h"
#include "Keyboard.h"
#include "MachineInfo.h"

// +--------------------------------------------------------------------+
// DECLARE MAPPING FUNCTIONS:

DEF_MAP_CLIENT(OptDlg, OnApply);
DEF_MAP_CLIENT(OptDlg, OnCancel);
DEF_MAP_CLIENT(OptDlg, OnEnter);
DEF_MAP_CLIENT(OptDlg, OnExit);
DEF_MAP_CLIENT(OptDlg, OnAudio);
DEF_MAP_CLIENT(OptDlg, OnVideo);
DEF_MAP_CLIENT(OptDlg, OnOptions);
DEF_MAP_CLIENT(OptDlg, OnControls);
DEF_MAP_CLIENT(OptDlg, OnMod);

// +--------------------------------------------------------------------+

OptDlg::OptDlg(Screen* s, FormDef& def, BaseScreen* mgr)
: FormWindow(s,  0,  0, s->Width(), s->Height()), manager(mgr),
apply(0), cancel(0), vid_btn(0), aud_btn(0), ctl_btn(0), opt_btn(0), mod_btn(0),
closed(true)
{
    Init(def);
}

OptDlg::~OptDlg()
{
}

// +--------------------------------------------------------------------+

void
OptDlg::RegisterControls()
{
    if (apply)
    return;

    flight_model   = (ComboBox*) FindControl(201);
    flying_start   = (ComboBox*) FindControl(211);
    landings       = (ComboBox*) FindControl(202);
    ai_difficulty  = (ComboBox*) FindControl(203);
    hud_mode       = (ComboBox*) FindControl(204);
    hud_color      = (ComboBox*) FindControl(205);
    ff_mode        = (ComboBox*) FindControl(206);
    grid_mode      = (ComboBox*) FindControl(207);
    gunsight       = (ComboBox*) FindControl(208);
    description    =             FindControl(500);
    apply          = (Button*) FindControl(1);
    cancel         = (Button*) FindControl(2);
    vid_btn        = (Button*) FindControl(901);
    aud_btn        = (Button*) FindControl(902);
    ctl_btn        = (Button*) FindControl(903);
    opt_btn        = (Button*) FindControl(904);
    mod_btn        = (Button*) FindControl(905);

    if (flight_model) {
        REGISTER_CLIENT(EID_MOUSE_ENTER, flight_model,  OptDlg, OnEnter);
        REGISTER_CLIENT(EID_MOUSE_EXIT,  flight_model,  OptDlg, OnExit);
    }

    if (flying_start) {
        REGISTER_CLIENT(EID_MOUSE_ENTER, flying_start,  OptDlg, OnEnter);
        REGISTER_CLIENT(EID_MOUSE_EXIT,  flying_start,  OptDlg, OnExit);
    }

    if (landings) {
        REGISTER_CLIENT(EID_MOUSE_ENTER, landings,      OptDlg, OnEnter);
        REGISTER_CLIENT(EID_MOUSE_EXIT,  landings,      OptDlg, OnExit);
    }

    if (ai_difficulty) {
        REGISTER_CLIENT(EID_MOUSE_ENTER, ai_difficulty, OptDlg, OnEnter);
        REGISTER_CLIENT(EID_MOUSE_EXIT,  ai_difficulty, OptDlg, OnExit);
    }

    if (hud_mode) {
        REGISTER_CLIENT(EID_MOUSE_ENTER, hud_mode,      OptDlg, OnEnter);
        REGISTER_CLIENT(EID_MOUSE_EXIT,  hud_mode,      OptDlg, OnExit);
    }

    if (hud_color) {
        REGISTER_CLIENT(EID_MOUSE_ENTER, hud_color,     OptDlg, OnEnter);
        REGISTER_CLIENT(EID_MOUSE_EXIT,  hud_color,     OptDlg, OnExit);
    }

    if (ff_mode) {
        REGISTER_CLIENT(EID_MOUSE_ENTER, ff_mode,       OptDlg, OnEnter);
        REGISTER_CLIENT(EID_MOUSE_EXIT,  ff_mode,       OptDlg, OnExit);
    }

    if (grid_mode) {
        REGISTER_CLIENT(EID_MOUSE_ENTER, grid_mode,     OptDlg, OnEnter);
        REGISTER_CLIENT(EID_MOUSE_EXIT,  grid_mode,     OptDlg, OnExit);
    }

    if (gunsight) {
        REGISTER_CLIENT(EID_MOUSE_ENTER, gunsight,      OptDlg, OnEnter);
        REGISTER_CLIENT(EID_MOUSE_EXIT,  gunsight,      OptDlg, OnExit);
    }

    if (apply)
    REGISTER_CLIENT(EID_CLICK, apply, OptDlg, OnApply);

    if (cancel)
    REGISTER_CLIENT(EID_CLICK, cancel, OptDlg, OnCancel);

    if (vid_btn)
    REGISTER_CLIENT(EID_CLICK, vid_btn, OptDlg, OnVideo);

    if (aud_btn)
    REGISTER_CLIENT(EID_CLICK, aud_btn, OptDlg, OnAudio);

    if (ctl_btn)
    REGISTER_CLIENT(EID_CLICK, ctl_btn, OptDlg, OnControls);

    if (opt_btn)
    REGISTER_CLIENT(EID_CLICK, opt_btn, OptDlg, OnOptions);

    if (mod_btn)
    REGISTER_CLIENT(EID_CLICK, mod_btn, OptDlg, OnMod);
}

// +--------------------------------------------------------------------+

void
OptDlg::Show()
{
    FormWindow::Show();

    if (closed) {
        Starshatter*   stars = Starshatter::GetInstance();

        if (stars) {
            if (flight_model)
            flight_model->SetSelection(Ship::GetFlightModel());

            if (landings)
            landings->SetSelection(Ship::GetLandingModel());

            if (hud_mode)
            hud_mode->SetSelection(HUDView::IsArcade() ? 1 : 0);

            if (hud_color)
            hud_color->SetSelection(HUDView::DefaultColorSet());

            if (ff_mode)
            ff_mode->SetSelection((int) (Ship::GetFriendlyFireLevel() * 4));
        }

        Player* player = Player::GetCurrentPlayer();
        if (player) {
            if (flying_start)
            flying_start->SetSelection(player->FlyingStart());

            if (ai_difficulty)
            ai_difficulty->SetSelection(ai_difficulty->NumItems() - player->AILevel() - 1);

            if (grid_mode)
            grid_mode->SetSelection(player->GridMode());

            if (gunsight)
            gunsight->SetSelection(player->Gunsight());
        }
    }

    if (vid_btn)   vid_btn->SetButtonState(0);
    if (aud_btn)   aud_btn->SetButtonState(0);
    if (ctl_btn)   ctl_btn->SetButtonState(0);
    if (opt_btn)   opt_btn->SetButtonState(1);
    if (mod_btn)   mod_btn->SetButtonState(0);

    closed = false;
}

// +--------------------------------------------------------------------+

void
OptDlg::ExecFrame()
{
    if (Keyboard::KeyDown(VK_RETURN)) {
        OnApply(0);
    }
}

// +--------------------------------------------------------------------+

void OptDlg::OnAudio(AWEvent* event)      { manager->ShowAudDlg();  }
void OptDlg::OnVideo(AWEvent* event)      { manager->ShowVidDlg();  }
void OptDlg::OnOptions(AWEvent* event)    { manager->ShowOptDlg();  }
void OptDlg::OnControls(AWEvent* event)   { manager->ShowCtlDlg();  }
void OptDlg::OnMod(AWEvent* event)        { manager->ShowModDlg();  }

// +--------------------------------------------------------------------+

void
OptDlg::OnApply(AWEvent* event)
{
    manager->ApplyOptions();
}

void
OptDlg::OnCancel(AWEvent* event)
{
    manager->CancelOptions();
}

// +--------------------------------------------------------------------+

void
OptDlg::OnEnter(AWEvent* event)
{
    ActiveWindow* src = event->window;

    if (src && description)
    description->SetText(src->GetAltText());
}

void
OptDlg::OnExit(AWEvent* event)
{
    ComboBox* cmb = (ComboBox*) event->window;

    if (!cmb || cmb->IsListShowing())
    return;

    if (description)
    description->SetText("");
}

// +--------------------------------------------------------------------+

void
OptDlg::Apply()
{
    if (closed) return;

    Player* player = Player::GetCurrentPlayer();
    if (player) {
        if (flight_model)
        player->SetFlightModel(flight_model->GetSelectedIndex());

        if (flying_start)
        player->SetFlyingStart(flying_start->GetSelectedIndex());

        if (landings)
        player->SetLandingModel(landings->GetSelectedIndex());

        if (ai_difficulty)
        player->SetAILevel(ai_difficulty->NumItems() - ai_difficulty->GetSelectedIndex() - 1);

        if (hud_mode)
        player->SetHUDMode(hud_mode->GetSelectedIndex());

        if (hud_color)
        player->SetHUDColor(hud_color->GetSelectedIndex());

        if (ff_mode)
        player->SetFriendlyFire(ff_mode->GetSelectedIndex());

        if (grid_mode)
        player->SetGridMode(grid_mode->GetSelectedIndex());

        if (gunsight)
        player->SetGunsight(gunsight->GetSelectedIndex());

        Player::Save();
    }

    if (flight_model)
    Ship::SetFlightModel(flight_model->GetSelectedIndex());

    if (landings)
    Ship::SetLandingModel(landings->GetSelectedIndex());

    if (hud_mode)
    HUDView::SetArcade(hud_mode->GetSelectedIndex() > 0);

    if (hud_color)
    HUDView::SetDefaultColorSet(hud_color->GetSelectedIndex());

    if (ff_mode)
    Ship::SetFriendlyFireLevel(ff_mode->GetSelectedIndex() / 4.0);

    HUDView* hud = HUDView::GetInstance();
    if (hud) hud->SetHUDColorSet(hud_color->GetSelectedIndex());

    closed = true;
}

void
OptDlg::Cancel()
{
    closed = true;
}
