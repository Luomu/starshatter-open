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
    FILE:         LoadDlg.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Loading progress dialog box
*/


#include "MemDebug.h"
#include "LoadDlg.h"
#include "Starshatter.h"

#include "Game.h"
#include "DataLoader.h"
#include "Video.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "ListBox.h"
#include "ComboBox.h"
#include "Slider.h"
#include "FormatUtil.h"

// +--------------------------------------------------------------------+

LoadDlg::LoadDlg(Screen* s, FormDef& def)
: FormWindow(s, 0, 0, s->Width(), s->Height()), 
progress(0), activity(0)
{
    Init(def);
}

LoadDlg::~LoadDlg()
{
}

void
LoadDlg::RegisterControls()
{
    title    = FindControl(100);
    activity = FindControl(101);
    progress = (Slider*) FindControl(102);
}

// +--------------------------------------------------------------------+

void
LoadDlg::ExecFrame()
{
    Starshatter* stars = Starshatter::GetInstance();

    if (stars) {
        if (title) {
            if (stars->GetGameMode() == Starshatter::CLOD_MODE ||
                    stars->GetGameMode() == Starshatter::CMPN_MODE)
            title->SetText(Game::GetText("LoadDlg.campaign"));

            else if (stars->GetGameMode() == Starshatter::MENU_MODE)
            title->SetText(Game::GetText("LoadDlg.tac-ref"));

            else
            title->SetText(Game::GetText("LoadDlg.mission"));
        }

        activity->SetText(stars->GetLoadActivity());
        progress->SetValue(stars->GetLoadProgress());
    }
}

// +--------------------------------------------------------------------+

