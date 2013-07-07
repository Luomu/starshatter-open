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
    FILE:         MsnElemDlg.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Mission Editor Element Dialog Active Window class
*/

#ifndef MsnElemDlg_h
#define MsnElemDlg_h

#include "Types.h"
#include "FormWindow.h"
#include "Bitmap.h"
#include "Button.h"
#include "ComboBox.h"
#include "ListBox.h"
#include "Font.h"
#include "Text.h"

// +--------------------------------------------------------------------+

class MenuScreen;
class MsnEditDlg;
class Mission;
class MissionElement;

// +--------------------------------------------------------------------+

class MsnElemDlg : public FormWindow
{
public:
    MsnElemDlg(Screen* s, FormDef& def, MenuScreen* mgr);
    virtual ~MsnElemDlg();

    virtual void      RegisterControls();
    virtual void      Show();
    virtual void      ExecFrame();

    // Operations:
    virtual void      SetMission(Mission* elem);
    virtual void      SetMissionElement(MissionElement* elem);
    virtual void      OnAccept(AWEvent* event);
    virtual void      OnCancel(AWEvent* event);
    virtual void      OnClassSelect(AWEvent* event);
    virtual void      OnDesignSelect(AWEvent* event);
    virtual void      OnObjectiveSelect(AWEvent* event);
    virtual void      OnIFFChange(AWEvent* event);

    virtual void      UpdateTeamInfo();
    virtual bool      CanCommand(const MissionElement* commander,
    const MissionElement* subordinate) const;

protected:
    MenuScreen*       manager;

    EditBox*          edt_name;
    ComboBox*         cmb_class;
    ComboBox*         cmb_design;
    ComboBox*         cmb_skin;
    EditBox*          edt_size;
    EditBox*          edt_iff;
    ComboBox*         cmb_role;
    ComboBox*         cmb_region;
    EditBox*          edt_loc_x;
    EditBox*          edt_loc_y;
    EditBox*          edt_loc_z;
    ComboBox*         cmb_heading;
    EditBox*          edt_hold_time;

    Button*           btn_player;
    Button*           btn_playable;
    Button*           btn_alert;
    Button*           btn_command_ai;
    EditBox*          edt_respawns;
    ComboBox*         cmb_carrier;
    ComboBox*         cmb_squadron;
    ComboBox*         cmb_commander;
    ComboBox*         cmb_intel;
    ComboBox*         cmb_loadout;
    ComboBox*         cmb_objective;
    ComboBox*         cmb_target;

    Button*           btn_accept;
    Button*           btn_cancel;

    Mission*          mission;
    MissionElement*   elem;
    bool              exit_latch;
};

#endif MsnElemDlg_h

