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
    FILE:         NetUnitDlg.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Multiplayer Unit Selection Dialog Active Window class
*/

#ifndef NetUnitDlg_h
#define NetUnitDlg_h

#include "Types.h"
#include "FormWindow.h"
#include "Bitmap.h"
#include "Button.h"
#include "ComboBox.h"
#include "ListBox.h"
#include "EditBox.h"
#include "Font.h"

// +--------------------------------------------------------------------+

class MenuScreen;
class NetClientConfig;
class NetLobby;
class NetChatEntry;
class NetUser;

// +--------------------------------------------------------------------+

class NetUnitDlg : public FormWindow
{
public:
    NetUnitDlg(Screen* s, FormDef& def, MenuScreen* mgr);
    virtual ~NetUnitDlg();

    virtual void      RegisterControls();
    virtual void      Show();
    virtual void      ExecFrame();

    // Operations:
    virtual void      OnSelect(AWEvent* event);
    virtual void      OnUnit(AWEvent* event);
    virtual void      OnMap(AWEvent* event);
    virtual void      OnUnMap(AWEvent* event);
    virtual void      OnBan(AWEvent* event);
    virtual void      OnBanConfirm(AWEvent* event);
    virtual void      OnApply(AWEvent* event);
    virtual void      OnCancel(AWEvent* event);

    virtual void      ExecLobbyFrame();

    virtual bool      GetHostMode()  const { return host_mode; }
    virtual void      SetHostMode(bool h)  { host_mode = h;    }

protected:
    virtual void      GetAvailable();
    virtual void      GetUnits();
    virtual void      GetChat();
    virtual void      SendChat(Text msg);
    virtual void      CheckUnitMapping();

    MenuScreen*       manager;

    ListBox*          lst_players;
    ListBox*          lst_units;
    ListBox*          lst_chat;
    EditBox*          edt_chat;

    Button*           btn_select;
    Button*           btn_map;
    Button*           btn_unmap;
    Button*           btn_ban;
    Button*           btn_apply;
    Button*           btn_cancel;

    NetLobby*         net_lobby;

    int               last_chat;
    int               unit_index;
    bool              host_mode;
};

// +--------------------------------------------------------------------+

#endif NetUnitDlg_h

