/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         CmdTitleDlg.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Operational Command Dialog (Intel/Newsfeed Tab)
*/

#include "MemDebug.h"
#include "CmdTitleDlg.h"
#include "CmpnScreen.h"
#include "Starshatter.h"
#include "Campaign.h"
#include "Combatant.h"
#include "CombatEvent.h"
#include "CombatGroup.h"
#include "CombatUnit.h"
#include "ShipDesign.h"

#include "Game.h"
#include "DataLoader.h"
#include "Button.h"
#include "ComboBox.h"
#include "ListBox.h"
#include "Slider.h"
#include "Video.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "ParseUtil.h"
#include "FormatUtil.h"

// +--------------------------------------------------------------------+
// DECLARE MAPPING FUNCTIONS:

// +--------------------------------------------------------------------+

CmdTitleDlg::CmdTitleDlg(Screen* s, FormDef& def, CmpnScreen* mgr)
   : FormWindow(s, 0, 0, s->Width(), s->Height()), manager(mgr),
     stars(0), campaign(0), showTime(0)
{
   stars    = Starshatter::GetInstance();
   campaign = Campaign::GetCampaign();

   Init(def);
}

CmdTitleDlg::~CmdTitleDlg()
{
}

// +--------------------------------------------------------------------+

void
CmdTitleDlg::RegisterControls()
{
   img_title = (ImageBox*)  FindControl(200);
}

// +--------------------------------------------------------------------+

void
CmdTitleDlg::Show()
{
   FormWindow::Show();
}

// +--------------------------------------------------------------------+

void
CmdTitleDlg::ExecFrame()
{
}
