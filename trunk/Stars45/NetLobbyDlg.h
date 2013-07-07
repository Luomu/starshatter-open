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
    FILE:         NetLobbyDlg.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Main Menu Dialog Active Window class
*/

#ifndef NetLobbyDlg_h
#define NetLobbyDlg_h

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
class NetCampaignInfo;
class MissionInfo;
class NetChatEntry;
class NetUser;

// +--------------------------------------------------------------------+

class NetLobbyDlg : public FormWindow
{
public:
    NetLobbyDlg(Screen* s, FormDef& def, MenuScreen* mgr);
    virtual ~NetLobbyDlg();

    virtual void      RegisterControls();
    virtual void      Show();
    virtual void      ExecFrame();

    // Operations:
    virtual void      OnCampaignSelect(AWEvent* event);
    virtual void      OnMissionSelect(AWEvent* event);

    virtual void      OnApply(AWEvent* event);
    virtual void      OnCancel(AWEvent* event);

    virtual void      ExecLobbyFrame();

protected:
    virtual void      GetPlayers();
    virtual void      GetChat();
    virtual void      GetMissions();
    virtual void      GetSelectedMission();
    virtual void      SendChat(Text msg);
    virtual void      SelectMission();

    MenuScreen*       manager;

    ComboBox*         lst_campaigns;
    ListBox*          lst_missions;
    ActiveWindow*     txt_desc;
    ListBox*          lst_players;
    ListBox*          lst_chat;
    EditBox*          edt_chat;

    Button*           apply;
    Button*           cancel;

    NetLobby*         net_lobby;

    int               selected_campaign;
    int               selected_mission;
    int               last_chat;
    bool              host_mode;
};

// +--------------------------------------------------------------------+

#endif NetLobbyDlg_h

