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
    FILE:         DebriefDlg.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Mission Debriefing Dialog Active Window class
*/

#ifndef DebriefDlg_h
#define DebriefDlg_h

#include "Types.h"
#include "FormWindow.h"
#include "Bitmap.h"
#include "Button.h"
#include "ComboBox.h"
#include "ListBox.h"
#include "Font.h"
#include "Text.h"

// +--------------------------------------------------------------------+

class PlanScreen;
class Campaign;
class Mission;
class MissionInfo;
class Sim;
class Ship;

// +--------------------------------------------------------------------+

class DebriefDlg : public FormWindow
{
public:
    DebriefDlg(Screen* s, FormDef& def, PlanScreen* mgr);
    virtual ~DebriefDlg();

    virtual void      RegisterControls();
    virtual void      ExecFrame();
    virtual void      Show();

    // Operations:
    virtual void      OnClose(AWEvent* event);
    virtual void      OnUnit(AWEvent* event);

protected:
    virtual void      DrawUnits();

    PlanScreen*       manager;
    Button*           close_btn;

    ActiveWindow*     mission_name;
    ActiveWindow*     mission_system;
    ActiveWindow*     mission_sector;
    ActiveWindow*     mission_time_start;

    ActiveWindow*     objectives;
    ActiveWindow*     situation;
    ActiveWindow*     mission_score;

    ListBox*          unit_list;
    ListBox*          summary_list;
    ListBox*          event_list;

    Campaign*         campaign;
    Mission*          mission;
    MissionInfo*      info;
    int               unit_index;

    Sim*              sim;
    Ship*             ship;
};

#endif DebriefDlg_h

