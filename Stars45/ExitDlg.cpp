/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         ExitDlg.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
*/

#include "MemDebug.h"
#include "ExitDlg.h"
#include "MenuScreen.h"
#include "MusicDirector.h"
#include "Starshatter.h"
#include "FormatUtil.h"

#include "Game.h"
#include "Keyboard.h"
#include "Button.h"
#include "RichTextBox.h"
#include "DataLoader.h"

// +--------------------------------------------------------------------+
// DECLARE MAPPING FUNCTIONS:

DEF_MAP_CLIENT(ExitDlg, OnApply);
DEF_MAP_CLIENT(ExitDlg, OnCancel);

// +--------------------------------------------------------------------+

ExitDlg::ExitDlg(Screen* s, FormDef& def, MenuScreen* mgr)
   : FormWindow(s,  0,  0, s->Width(), s->Height()), 
     manager(mgr), exit_latch(false),
     credits(0), apply(0), cancel(0),
     def_rect(def.GetRect())
{
   Init(def);
}

ExitDlg::~ExitDlg()
{
}

void
ExitDlg::RegisterControls()
{
   if (apply)
      return;

   credits = (RichTextBox*) FindControl(201);

   apply   = (Button*) FindControl(1);
   REGISTER_CLIENT(EID_CLICK, apply, ExitDlg, OnApply);

   cancel  = (Button*) FindControl(2);
   REGISTER_CLIENT(EID_CLICK, cancel, ExitDlg, OnCancel);
}

// +--------------------------------------------------------------------+

void
ExitDlg::ExecFrame()
{
   if (credits && credits->GetLineCount() > 0) {
      credits->SmoothScroll(ScrollWindow::SCROLL_DOWN, Game::GUITime());

      if (credits->GetTopIndex() >= credits->GetLineCount()-1) {
         credits->ScrollTo(0);
      }
   }

   if (Keyboard::KeyDown(VK_RETURN)) {
      OnApply(0);
   }

   if (Keyboard::KeyDown(VK_ESCAPE)) {
      if (!exit_latch)
         OnCancel(0);
   }
   else {
      exit_latch = false;
   }
}

// +--------------------------------------------------------------------+

void
ExitDlg::Show()
{
   if (!IsShown()) {
      Rect r = def_rect;

      if (r.w > screen->Width()) {
         int extra = r.w - screen->Width();
         r.w -= extra;
      }

      if (r.h > screen->Height()) {
         int extra = r.h - screen->Height();
         r.h -= extra;
      }

      r.x = (screen->Width()  - r.w) / 2;
      r.y = (screen->Height() - r.h) / 2;

      MoveTo(r);

      exit_latch = true;
      Button::PlaySound(Button::SND_CONFIRM);
      MusicDirector::SetMode(MusicDirector::CREDITS);

      DataLoader* loader = DataLoader::GetLoader();
      BYTE*       block    = 0;

      loader->SetDataPath(0);
      loader->LoadBuffer("credits.txt", block, true);

      if (block && credits) {
         credits->SetText((const char*) block);
      }

      loader->ReleaseBuffer(block);
   }

   FormWindow::Show();
}

// +--------------------------------------------------------------------+

void
ExitDlg::OnApply(AWEvent* event)
{
   Starshatter* stars = Starshatter::GetInstance();

   if (stars) {
      ::Print("Exit Confirmed.\n");
      stars->Exit();
   }
}

void
ExitDlg::OnCancel(AWEvent* event)
{
   manager->ShowMenuDlg();
   MusicDirector::SetMode(MusicDirector::MENU);
}

// +--------------------------------------------------------------------+
