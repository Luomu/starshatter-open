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
    FILE:         MsnDlg.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Mission Briefing Dialog Active Window class
*/

#ifndef MsnDlg_h
#define MsnDlg_h

#include "Types.h"
#include "Bitmap.h"
#include "Button.h"
#include "Font.h"
#include "Text.h"

// +--------------------------------------------------------------------+

class FormWindow;
class PlanScreen;
class Campaign;
class Mission;
class MissionInfo;

// +--------------------------------------------------------------------+

class MsnDlg
{
public:
    MsnDlg(PlanScreen* mgr);
    virtual ~MsnDlg();

    void              RegisterMsnControls(FormWindow* win);
    void              ShowMsnDlg();

    // Operations:
    virtual void      OnCommit(AWEvent* event);
    virtual void      OnCancel(AWEvent* event);
    virtual void      OnTabButton(AWEvent* event);

protected:
    virtual int       CalcTimeOnTarget();

    PlanScreen*       plan_screen;
    Button*           commit;
    Button*           cancel;
    Button*           sit_button;
    Button*           pkg_button;
    Button*           nav_button;
    Button*           wep_button;

    ActiveWindow*     mission_name;
    ActiveWindow*     mission_system;
    ActiveWindow*     mission_sector;
    ActiveWindow*     mission_time_start;
    ActiveWindow*     mission_time_target;
    ActiveWindow*     mission_time_target_label;

    Campaign*         campaign;
    Mission*          mission;
    MissionInfo*      info;
    int               pkg_index;
};

#endif MsnDlg_h

