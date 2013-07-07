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
    FILE:         PlayerDlg.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Main Menu Dialog Active Window class
*/

#ifndef PlayerDlg_h
#define PlayerDlg_h

#include "Types.h"
#include "FormWindow.h"
#include "Bitmap.h"
#include "Button.h"
#include "ComboBox.h"
#include "ListBox.h"
#include "Font.h"

// +--------------------------------------------------------------------+

class MenuScreen;
class Player;

// +--------------------------------------------------------------------+

class PlayerDlg : public FormWindow
{
public:
    PlayerDlg(Screen* s, FormDef& def, MenuScreen* mgr);
    virtual ~PlayerDlg();

    virtual void      RegisterControls();
    virtual void      Show();
    virtual void      ExecFrame();

    // Operations:
    virtual void      OnApply(AWEvent* event);
    virtual void      OnCancel(AWEvent* event);
    virtual void      OnSelectPlayer(AWEvent* event);
    virtual void      OnAdd(AWEvent* event);
    virtual void      OnDel(AWEvent* event);
    virtual void      OnDelConfirm(AWEvent* event);
    virtual void      OnRank(AWEvent* event);
    virtual void      OnMedal(AWEvent* event);

    virtual void      UpdatePlayer();
    virtual void      ShowPlayer();

protected:
    MenuScreen*       manager;

    ListBox*          lst_roster;
    Button*           btn_add;
    Button*           btn_del;

    EditBox*          txt_name;
    EditBox*          txt_password;
    EditBox*          txt_squadron;
    EditBox*          txt_signature;

    EditBox*          txt_chat[10];

    ActiveWindow*     lbl_createdate;
    ActiveWindow*     lbl_rank;
    ActiveWindow*     lbl_flighttime;
    ActiveWindow*     lbl_missions;
    ActiveWindow*     lbl_kills;
    ActiveWindow*     lbl_losses;
    ActiveWindow*     lbl_points;
    ImageBox*         img_rank;
    ImageBox*         img_medals[15];
    int               medals[15];

    Button*           apply;
    Button*           cancel;
};

#endif PlayerDlg_h

