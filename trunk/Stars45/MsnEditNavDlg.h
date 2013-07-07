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
    FILE:         MsnEditNavDlg.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Mission Briefing Dialog Active Window class
*/

#ifndef MsnEditNavDlg_h
#define MsnEditNavDlg_h

#include "Types.h"
#include "NavDlg.h"
#include "Bitmap.h"
#include "Button.h"
#include "ComboBox.h"
#include "EditBox.h"
#include "ListBox.h"
#include "Font.h"
#include "Text.h"

// +--------------------------------------------------------------------+

class MenuScreen;
class Campaign;
class Mission;
class MissionInfo;

// +--------------------------------------------------------------------+

class MsnEditNavDlg : public NavDlg
{
public:
    friend class MsnEditDlg;

    MsnEditNavDlg(Screen* s, FormDef& def, MenuScreen* mgr);
    virtual ~MsnEditNavDlg();

    virtual void      RegisterControls();
    virtual void      Show();
    virtual void      SetMissionInfo(MissionInfo* m);
    virtual void      ScrapeForm();

    // Operations:
    virtual void      OnCommit(AWEvent* event);
    virtual void      OnCancel(AWEvent* event);
    virtual void      OnTabButton(AWEvent* event);
    virtual void      OnSystemSelect(AWEvent* event);

protected:
    virtual void      ShowTab(int tab);

    MenuScreen*       menu_screen;

    Button*           btn_accept;
    Button*           btn_cancel;

    Button*           btn_sit;
    Button*           btn_pkg;
    Button*           btn_map;

    EditBox*          txt_name;
    ComboBox*         cmb_type;
    ComboBox*         cmb_system;
    ComboBox*         cmb_region;
    MissionInfo*      mission_info;

    bool              exit_latch;
};

#endif MsnEditNavDlg_h

