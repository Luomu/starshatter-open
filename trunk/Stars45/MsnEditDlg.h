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
    FILE:         MsnEditDlg.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Mission Editor Dialog Active Window class
*/

#ifndef MsnEditDlg_h
#define MsnEditDlg_h

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
class Campaign;
class Mission;
class MissionInfo;

// +--------------------------------------------------------------------+

class MsnEditDlg : public FormWindow
{
public:
    friend class MsnEditNavDlg;

    MsnEditDlg(Screen* s, FormDef& def, MenuScreen* mgr);
    virtual ~MsnEditDlg();

    virtual void      RegisterControls();
    virtual void      Show();
    virtual void      ExecFrame();
    virtual void      ScrapeForm();

    // Operations:
    virtual void      OnAccept(AWEvent* event);
    virtual void      OnCancel(AWEvent* event);
    virtual void      OnTabButton(AWEvent* event);
    virtual void      OnSystemSelect(AWEvent* event);
    virtual void      OnElemAdd(AWEvent* event);
    virtual void      OnElemEdit(AWEvent* event);
    virtual void      OnElemDel(AWEvent* event);
    virtual void      OnElemSelect(AWEvent* event);
    virtual void      OnElemInc(AWEvent* event);
    virtual void      OnElemDec(AWEvent* event);
    virtual void      OnEventAdd(AWEvent* event);
    virtual void      OnEventEdit(AWEvent* event);
    virtual void      OnEventDel(AWEvent* event);
    virtual void      OnEventSelect(AWEvent* event);
    virtual void      OnEventInc(AWEvent* event);
    virtual void      OnEventDec(AWEvent* event);

    virtual Mission*  GetMission() const      { return mission; }
    virtual void      SetMission(Mission* m);
    virtual void      SetMissionInfo(MissionInfo* m);

protected:
    virtual void      DrawPackages();
    virtual void      ShowTab(int tab);

    MenuScreen*       manager;

    Button*           btn_accept;
    Button*           btn_cancel;

    Button*           btn_elem_add;
    Button*           btn_elem_edit;
    Button*           btn_elem_del;
    Button*           btn_elem_inc;
    Button*           btn_elem_dec;

    Button*           btn_event_add;
    Button*           btn_event_edit;
    Button*           btn_event_del;
    Button*           btn_event_inc;
    Button*           btn_event_dec;

    Button*           btn_sit;
    Button*           btn_pkg;
    Button*           btn_map;

    EditBox*          txt_name;
    ComboBox*         cmb_type;
    ComboBox*         cmb_system;
    ComboBox*         cmb_region;

    EditBox*          txt_description;
    EditBox*          txt_situation;
    EditBox*          txt_objective;

    ListBox*          lst_elem;
    ListBox*          lst_event;

    Mission*          mission;
    MissionInfo*      mission_info;

    int               current_tab;
    bool              exit_latch;
};

#endif MsnEditDlg_h

