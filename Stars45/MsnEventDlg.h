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
    FILE:         MsnEventDlg.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Mission Editor Element Dialog Active Window class
*/

#ifndef MsnEventDlg_h
#define MsnEventDlg_h

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
class MissionEvent;

// +--------------------------------------------------------------------+

class MsnEventDlg : public FormWindow
{
public:
    MsnEventDlg(Screen* s, FormDef& def, MenuScreen* mgr);
    virtual ~MsnEventDlg();

    virtual void      RegisterControls();
    virtual void      Show();
    virtual void      ExecFrame();

    // Operations:
    virtual void      SetMission(Mission* msn);
    virtual void      SetMissionEvent(MissionEvent* event);
    virtual void      OnEventSelect(AWEvent* e);
    virtual void      OnAccept(AWEvent* e);
    virtual void      OnCancel(AWEvent* e);

protected:
    virtual void      FillShipList(ComboBox* cmb, const char* seln);
    virtual void      FillRgnList(ComboBox* cmb, const char* seln);

    MenuScreen*       manager;

    ActiveWindow*     lbl_id;
    EditBox*          edt_time;
    EditBox*          edt_delay;

    ComboBox*         cmb_event;
    ComboBox*         cmb_event_ship;
    ComboBox*         cmb_event_source;
    ComboBox*         cmb_event_target;
    EditBox*          edt_event_param;
    EditBox*          edt_event_chance;
    EditBox*          edt_event_sound;
    EditBox*          edt_event_message;

    ComboBox*         cmb_trigger;
    ComboBox*         cmb_trigger_ship;
    ComboBox*         cmb_trigger_target;
    EditBox*          edt_trigger_param;

    Button*           btn_accept;
    Button*           btn_cancel;

    Mission*          mission;
    MissionEvent*     event;
};

#endif MsnEventDlg_h

