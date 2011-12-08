/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         CmdMsgDlg.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
*/

#include "MemDebug.h"
#include "CmdMsgDlg.h"
#include "CmpnScreen.h"
#include "Starshatter.h"

#include "Game.h"
#include "ListBox.h"
#include "ComboBox.h"
#include "Button.h"
#include "Keyboard.h"
#include "FormatUtil.h"

// +--------------------------------------------------------------------+
// DECLARE MAPPING FUNCTIONS:

DEF_MAP_CLIENT(CmdMsgDlg, OnApply);

// +--------------------------------------------------------------------+

CmdMsgDlg::CmdMsgDlg(Screen* s, FormDef& def, CmpnScreen* mgr)
   : FormWindow(s, 0, 0, s->Width(), s->Height()), manager(mgr),
     exit_latch(false)
{
   Init(def);
}

CmdMsgDlg::~CmdMsgDlg()
{
}

void
CmdMsgDlg::RegisterControls()
{
   title   = FindControl(100);
   message = FindControl(101);

   apply   = (Button*) FindControl(1);
   REGISTER_CLIENT(EID_CLICK, apply, CmdMsgDlg, OnApply);
}

// +--------------------------------------------------------------------+

void
CmdMsgDlg::ExecFrame()
{
   if (Keyboard::KeyDown(VK_RETURN)) {
      OnApply(0);
   }

   if (Keyboard::KeyDown(VK_ESCAPE)) {
      if (!exit_latch)
         OnApply(0);

      exit_latch = true;
   }
   else {
      exit_latch = false;
   }
}

// +--------------------------------------------------------------------+

void
CmdMsgDlg::Show()
{
   FormWindow::Show();
   SetFocus();
}

// +--------------------------------------------------------------------+

void
CmdMsgDlg::OnApply(AWEvent* event)
{
   if (manager)
      manager->CloseTopmost();
}

// +--------------------------------------------------------------------+
