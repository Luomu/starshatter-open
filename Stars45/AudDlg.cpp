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
    FILE:         AudDlg.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Main Menu Dialog Active Window class
*/

#include "MemDebug.h"
#include "AudDlg.h"
#include "MenuScreen.h"
#include "Starshatter.h"
#include "AudioConfig.h"

#include "DataLoader.h"
#include "Button.h"
#include "ListBox.h"
#include "Slider.h"
#include "Video.h"
#include "Keyboard.h"
#include "MachineInfo.h"

// +--------------------------------------------------------------------+
// DECLARE MAPPING FUNCTIONS:

DEF_MAP_CLIENT(AudDlg, OnApply);
DEF_MAP_CLIENT(AudDlg, OnCancel);
DEF_MAP_CLIENT(AudDlg, OnAudio);
DEF_MAP_CLIENT(AudDlg, OnVideo);
DEF_MAP_CLIENT(AudDlg, OnOptions);
DEF_MAP_CLIENT(AudDlg, OnControls);
DEF_MAP_CLIENT(AudDlg, OnMod);

// +--------------------------------------------------------------------+

AudDlg::AudDlg(Screen* s, FormDef& def, BaseScreen* mgr)
    : FormWindow(s, 0, 0, s->Width(), s->Height()), manager(mgr),
       apply(0), cancel(0), vid_btn(0), aud_btn(0), ctl_btn(0), opt_btn(0),
       mod_btn(0), closed(true)
{
    Init(def);
}

AudDlg::~AudDlg()
{
}

// +--------------------------------------------------------------------+

void
AudDlg::RegisterControls()
{
    if (apply)
    return;

    efx_volume_slider = (Slider*) FindControl(201);
    gui_volume_slider = (Slider*) FindControl(202);
    wrn_volume_slider = (Slider*) FindControl(203);
    vox_volume_slider = (Slider*) FindControl(204);

    menu_music_slider = (Slider*) FindControl(205);
    game_music_slider = (Slider*) FindControl(206);

    apply   = (Button*) FindControl(1);
    REGISTER_CLIENT(EID_CLICK, apply, AudDlg, OnApply);

    cancel  = (Button*) FindControl(2);
    REGISTER_CLIENT(EID_CLICK, cancel, AudDlg, OnCancel);

    vid_btn = (Button*) FindControl(901);
    REGISTER_CLIENT(EID_CLICK, vid_btn, AudDlg, OnVideo);

    aud_btn = (Button*) FindControl(902);
    REGISTER_CLIENT(EID_CLICK, aud_btn, AudDlg, OnAudio);

    ctl_btn = (Button*) FindControl(903);
    REGISTER_CLIENT(EID_CLICK, ctl_btn, AudDlg, OnControls);

    opt_btn = (Button*) FindControl(904);
    REGISTER_CLIENT(EID_CLICK, opt_btn, AudDlg, OnOptions);

    mod_btn = (Button*) FindControl(905);
    if (mod_btn) {
        REGISTER_CLIENT(EID_CLICK, mod_btn, AudDlg, OnMod);
    }
}

// +--------------------------------------------------------------------+

void
AudDlg::Show()
{
    FormWindow::Show();

    if (closed && AudioConfig::GetInstance()) {
        AudioConfig* audio = AudioConfig::GetInstance();

        if (efx_volume_slider)
        efx_volume_slider->SetValue(audio->GetEfxVolume());

        if (gui_volume_slider)
        gui_volume_slider->SetValue(audio->GetGuiVolume());

        if (wrn_volume_slider)
        wrn_volume_slider->SetValue(audio->GetWrnVolume());

        if (vox_volume_slider)
        vox_volume_slider->SetValue(audio->GetVoxVolume());

        if (menu_music_slider)
        menu_music_slider->SetValue(audio->GetMenuMusic());

        if (game_music_slider)
        game_music_slider->SetValue(audio->GetGameMusic());
    }

    if (vid_btn)   vid_btn->SetButtonState(0);
    if (aud_btn)   aud_btn->SetButtonState(1);
    if (ctl_btn)   ctl_btn->SetButtonState(0);
    if (opt_btn)   opt_btn->SetButtonState(0);
    if (mod_btn)   mod_btn->SetButtonState(0);

    closed = false;
}

// +--------------------------------------------------------------------+

void
AudDlg::ExecFrame()
{
    if (Keyboard::KeyDown(VK_RETURN)) {
        OnApply(0);
    }
}

// +--------------------------------------------------------------------+

void AudDlg::OnAudio(AWEvent* event)      { manager->ShowAudDlg();  }
void AudDlg::OnVideo(AWEvent* event)      { manager->ShowVidDlg();  }
void AudDlg::OnOptions(AWEvent* event)    { manager->ShowOptDlg();  }
void AudDlg::OnControls(AWEvent* event)   { manager->ShowCtlDlg();  }
void AudDlg::OnMod(AWEvent* event)        { manager->ShowModDlg();  }

// +--------------------------------------------------------------------+

void
AudDlg::OnApply(AWEvent* event)
{
    if (manager)
    manager->ApplyOptions();
}

void
AudDlg::OnCancel(AWEvent* event)
{
    manager->CancelOptions();
}

// +--------------------------------------------------------------------+

void
AudDlg::Apply()
{
    if (!closed && AudioConfig::GetInstance()) {
        AudioConfig* audio = AudioConfig::GetInstance();

        if (efx_volume_slider)
        audio->SetEfxVolume(efx_volume_slider->GetValue());

        if (gui_volume_slider)
        audio->SetGuiVolume(gui_volume_slider->GetValue());

        if (wrn_volume_slider)
        audio->SetWrnVolume(wrn_volume_slider->GetValue());

        if (vox_volume_slider)
        audio->SetVoxVolume(vox_volume_slider->GetValue());

        if (menu_music_slider)
        audio->SetMenuMusic(menu_music_slider->GetValue());

        if (game_music_slider)
        audio->SetGameMusic(game_music_slider->GetValue());

        audio->Save();
    }

    closed = true;
}

void
AudDlg::Cancel()
{
    closed = true;
}
