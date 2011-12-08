/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

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

