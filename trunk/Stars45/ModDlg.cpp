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
    FILE:         ModDlg.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Mod Config Dialog Active Window class
*/

#include "MemDebug.h"
#include "ModDlg.h"
#include "ModInfoDlg.h"
#include "BaseScreen.h"
#include "ModConfig.h"

#include "Game.h"
#include "DataLoader.h"
#include "Button.h"
#include "ListBox.h"
#include "Slider.h"
#include "Video.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "ParseUtil.h"

// +--------------------------------------------------------------------+
// DECLARE MAPPING FUNCTIONS:

DEF_MAP_CLIENT(ModDlg, OnIncrease);
DEF_MAP_CLIENT(ModDlg, OnDecrease);
DEF_MAP_CLIENT(ModDlg, OnEnable);
DEF_MAP_CLIENT(ModDlg, OnDisable);
DEF_MAP_CLIENT(ModDlg, OnSelectEnabled);
DEF_MAP_CLIENT(ModDlg, OnSelectDisabled);
DEF_MAP_CLIENT(ModDlg, OnAccept);
DEF_MAP_CLIENT(ModDlg, OnCancel);
DEF_MAP_CLIENT(ModDlg, OnAudio);
DEF_MAP_CLIENT(ModDlg, OnVideo);
DEF_MAP_CLIENT(ModDlg, OnOptions);
DEF_MAP_CLIENT(ModDlg, OnControls);
DEF_MAP_CLIENT(ModDlg, OnMod);

// +--------------------------------------------------------------------+

ModDlg::ModDlg(Screen* s, FormDef& def, BaseScreen* mgr)
: FormWindow(s,  0,  0, s->Width(), s->Height()), manager(mgr),
lst_disabled(0), lst_enabled(0), btn_enable(0), btn_disable(0),
btn_increase(0), btn_decrease(0), btn_accept(0), btn_cancel(0),
aud_btn(0), vid_btn(0), ctl_btn(0), opt_btn(0), mod_btn(0),
config(0), changed(false)
{
    config = ModConfig::GetInstance();
    Init(def);
}

ModDlg::~ModDlg()
{
}

// +--------------------------------------------------------------------+

void
ModDlg::RegisterControls()
{
    btn_accept     = (Button*) FindControl(  1);
    btn_cancel     = (Button*) FindControl(  2);
    btn_enable     = (Button*) FindControl(301);
    btn_disable    = (Button*) FindControl(302);
    btn_increase   = (Button*) FindControl(303);
    btn_decrease   = (Button*) FindControl(304);

    lst_disabled   = (ListBox*) FindControl(201);
    lst_enabled    = (ListBox*) FindControl(202);

    if (btn_accept)
    REGISTER_CLIENT(EID_CLICK,    btn_accept,    ModDlg, OnAccept);

    if (btn_cancel)
    REGISTER_CLIENT(EID_CLICK,    btn_cancel,    ModDlg, OnCancel);

    if (lst_enabled)
    REGISTER_CLIENT(EID_SELECT,   lst_enabled,   ModDlg, OnSelectEnabled);

    if (lst_disabled)
    REGISTER_CLIENT(EID_SELECT,   lst_disabled,  ModDlg, OnSelectDisabled);

    if (btn_enable) {
        REGISTER_CLIENT(EID_CLICK,    btn_enable,    ModDlg, OnEnable);
        btn_enable->SetEnabled(false);
    }

    if (btn_disable) {
        REGISTER_CLIENT(EID_CLICK,    btn_disable,   ModDlg, OnDisable);
        btn_disable->SetEnabled(false);
    }

    if (btn_increase) {
        char up_arrow[2];
        up_arrow[0] = Font::ARROW_UP;
        up_arrow[1] = 0;
        btn_increase->SetText(up_arrow);
        btn_increase->SetEnabled(false);
        REGISTER_CLIENT(EID_CLICK,    btn_increase,  ModDlg, OnIncrease);
    }

    if (btn_decrease) {
        char dn_arrow[2];
        dn_arrow[0] = Font::ARROW_DOWN;
        dn_arrow[1] = 0;
        btn_decrease->SetText(dn_arrow);
        btn_decrease->SetEnabled(false);
        REGISTER_CLIENT(EID_CLICK,    btn_decrease,  ModDlg, OnDecrease);
    }

    vid_btn = (Button*) FindControl(901);
    REGISTER_CLIENT(EID_CLICK, vid_btn, ModDlg, OnVideo);

    aud_btn = (Button*) FindControl(902);
    REGISTER_CLIENT(EID_CLICK, aud_btn, ModDlg, OnAudio);

    ctl_btn = (Button*) FindControl(903);
    REGISTER_CLIENT(EID_CLICK, ctl_btn, ModDlg, OnControls);

    opt_btn = (Button*) FindControl(904);
    REGISTER_CLIENT(EID_CLICK, opt_btn, ModDlg, OnOptions);

    mod_btn = (Button*) FindControl(905);
    if (mod_btn)
    REGISTER_CLIENT(EID_CLICK, mod_btn, ModDlg, OnMod);
}

// +--------------------------------------------------------------------+

void
ModDlg::Show()
{
    FormWindow::Show();
    UpdateLists();

    if (vid_btn)   vid_btn->SetButtonState(0);
    if (aud_btn)   aud_btn->SetButtonState(0);
    if (ctl_btn)   ctl_btn->SetButtonState(0);
    if (opt_btn)   opt_btn->SetButtonState(0);
    if (mod_btn)   mod_btn->SetButtonState(1);
}

void
ModDlg::UpdateLists()
{
    config = ModConfig::GetInstance();

    if (config && lst_disabled && lst_enabled) {
        lst_disabled->ClearItems();
        lst_enabled->ClearItems();

        ListIter<Text> iter_d = config->DisabledMods();
        while (++iter_d) {
            Text* t = iter_d.value();
            lst_disabled->AddItem(*t);
        }

        ListIter<Text> iter_e = config->EnabledMods();
        while (++iter_e) {
            Text* t = iter_e.value();
            lst_enabled->AddItem(*t);
        }
    }

    if (btn_disable)
    btn_disable->SetEnabled(false);

    if (btn_enable)
    btn_enable->SetEnabled(false);

    if (btn_increase)
    btn_increase->SetEnabled(false);

    if (btn_decrease)
    btn_decrease->SetEnabled(false);
}

// +--------------------------------------------------------------------+

void
ModDlg::ExecFrame()
{
    if (Keyboard::KeyDown(VK_RETURN)) {
        if (btn_accept && btn_accept->IsEnabled())
        OnAccept(0);
    }
}

// +--------------------------------------------------------------------+

void
ModDlg::OnSelectEnabled(AWEvent* event)
{
    static DWORD click_time = 0;

    if (lst_enabled) {
        if (btn_disable)
        btn_disable->SetEnabled(lst_enabled->GetSelCount() == 1);

        if (btn_increase)
        btn_increase->SetEnabled(lst_enabled->GetSelection() > 0);

        if (btn_decrease)
        btn_decrease->SetEnabled(lst_enabled->GetSelection() < lst_enabled->NumItems() - 1);

        // double-click:
        if (Game::RealTime() - click_time < 350) {
            if (lst_enabled->GetSelCount() == 1) {
                int         index        = lst_enabled->GetSelection();
                Text        mod_name     = lst_enabled->GetItemText(index);
                ModInfo*    mod_info     = config->GetModInfo(mod_name);
                ModInfoDlg* mod_info_dlg = manager->GetModInfoDlg();

                if (mod_info && mod_info_dlg) {
                    mod_info_dlg->SetModInfo(mod_info);
                    manager->ShowModInfoDlg();
                }
            }
        }
    }

    click_time = Game::RealTime();
}

void
ModDlg::OnSelectDisabled(AWEvent* event)
{
    if (btn_enable && lst_disabled) {
        btn_enable->SetEnabled(lst_disabled->GetSelCount() == 1);
    }
}

void
ModDlg::OnEnable(AWEvent* event)
{
    int  index    = lst_disabled->GetSelection();
    Text mod_name = lst_disabled->GetItemText(index);

    config->EnableMod(mod_name);
    changed = true;

    UpdateLists();

    ModInfo*    mod_info     = config->GetModInfo(mod_name);
    ModInfoDlg* mod_info_dlg = manager->GetModInfoDlg();

    if (mod_info && mod_info_dlg) {
        mod_info_dlg->SetModInfo(mod_info);
        manager->ShowModInfoDlg();
    }
}

void
ModDlg::OnDisable(AWEvent* event)
{
    int  index    = lst_enabled->GetSelection();
    Text mod_name = lst_enabled->GetItemText(index);

    config->DisableMod(mod_name);
    changed = true;

    UpdateLists();
}

void
ModDlg::OnIncrease(AWEvent* event)
{
    int index = lst_enabled->GetSelection();
    config->IncreaseModPriority(index--);

    UpdateLists();
    lst_enabled->SetSelected(index);
    btn_disable->SetEnabled(true);
    btn_increase->SetEnabled(index > 0);
    btn_decrease->SetEnabled(index < lst_enabled->NumItems()-1);
}

void
ModDlg::OnDecrease(AWEvent* event)
{
    int index = lst_enabled->GetSelection();
    config->DecreaseModPriority(index++);

    UpdateLists();
    lst_enabled->SetSelected(index);
    btn_disable->SetEnabled(true);
    btn_increase->SetEnabled(index > 0);
    btn_decrease->SetEnabled(index < lst_enabled->NumItems()-1);
}

// +--------------------------------------------------------------------+

void ModDlg::OnAudio(AWEvent* event)      { manager->ShowAudDlg();  }
void ModDlg::OnVideo(AWEvent* event)      { manager->ShowVidDlg();  }
void ModDlg::OnOptions(AWEvent* event)    { manager->ShowOptDlg();  }
void ModDlg::OnControls(AWEvent* event)   { manager->ShowCtlDlg();  }
void ModDlg::OnMod(AWEvent* event)        { manager->ShowModDlg();  }

// +--------------------------------------------------------------------+

void
ModDlg::Apply()
{
    if (changed) {
        config->Save();
        config->FindMods();
        config->Redeploy();
        changed = false;
    }
}

void
ModDlg::Cancel()
{
    if (changed) {
        config->Load();
        config->FindMods();
        config->Redeploy();
        changed = false;
    }
}

// +--------------------------------------------------------------------+

void
ModDlg::OnAccept(AWEvent* event)
{
    manager->ApplyOptions();
}

void
ModDlg::OnCancel(AWEvent* event)
{
    manager->CancelOptions();
}
