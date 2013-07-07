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
    FILE:         CmdForceDlg.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Operational Command Dialog (Order of Battle Tab)
*/

#ifndef CmdForceDlg_h
#define CmdForceDlg_h

#include "Types.h"
#include "FormWindow.h"
#include "CmdDlg.h"
#include "Bitmap.h"
#include "Button.h"
#include "ComboBox.h"
#include "ListBox.h"
#include "Font.h"
#include "Text.h"

// +--------------------------------------------------------------------+

class CmdForceDlg : public FormWindow,
public CmdDlg
{
public:
    CmdForceDlg(Screen* s, FormDef& def, CmpnScreen* mgr);
    virtual ~CmdForceDlg();

    virtual void      RegisterControls();
    virtual void      Show();
    virtual void      ExecFrame();

    // Operations:
    virtual void      OnMode(AWEvent* event);
    virtual void      OnSave(AWEvent* event);
    virtual void      OnExit(AWEvent* event);
    virtual void      OnForces(AWEvent* event);
    virtual void      OnCombat(AWEvent* event);
    virtual void      OnTransfer(AWEvent* event);

protected:
    void              ShowCombatant(Combatant* c);
    void              AddCombatGroup(CombatGroup* grp, bool last_child=false);
    bool              CanTransfer(CombatGroup* grp);
    bool              IsVisible(Combatant* c);

    CmpnScreen*       manager;

    ComboBox*         cmb_forces;
    ListBox*          lst_combat;
    ListBox*          lst_desc;
    Button*           btn_transfer;

    Starshatter*      stars;
    Campaign*         campaign;
    CombatGroup*      current_group;
    CombatUnit*       current_unit;
};

#endif CmdForceDlg_h

