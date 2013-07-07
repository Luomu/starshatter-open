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
    FILE:         CmdDlg.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Operational Command Dialog Active Window class
*/

#ifndef CmdDlg_h
#define CmdDlg_h

#include "Types.h"
#include "FormWindow.h"
#include "Bitmap.h"
#include "Button.h"
#include "ComboBox.h"
#include "ListBox.h"
#include "Font.h"
#include "Text.h"

// +--------------------------------------------------------------------+

class CmpnScreen;
class Campaign;
class Combatant;
class CombatGroup;
class CombatUnit;
class Starshatter;

// +--------------------------------------------------------------------+

class CmdDlg
{
public:
    enum MODE { 
        MODE_ORDERS,
        MODE_THEATER,
        MODE_FORCES,
        MODE_INTEL,
        MODE_MISSIONS,
        NUM_MODES
    };

    CmdDlg(CmpnScreen* mgr);
    virtual ~CmdDlg();

    virtual void      RegisterCmdControls(FormWindow* win);
    virtual void      ShowCmdDlg();

    virtual void      ExecFrame();

    // Operations:
    virtual void      OnMode(AWEvent* event);
    virtual void      OnSave(AWEvent* event);
    virtual void      OnExit(AWEvent* event);

protected:
    virtual void      ShowMode();

    CmpnScreen*       cmpn_screen;

    ActiveWindow*     txt_group;
    ActiveWindow*     txt_score;
    ActiveWindow*     txt_name;
    ActiveWindow*     txt_time;
    Button*           btn_mode[NUM_MODES];
    Button*           btn_save;
    Button*           btn_exit;

    Starshatter*      stars;
    Campaign*         campaign;

    int               mode;
};

#endif CmdDlg_h

