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
    FILE:         NetClientDlg.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Main Menu Dialog Active Window class
*/

#ifndef NetClientDlg_h
#define NetClientDlg_h

#include "Types.h"
#include "NetClientConfig.h"
#include "NetLobby.h"

#include "FormWindow.h"
#include "Bitmap.h"
#include "Button.h"
#include "ComboBox.h"
#include "ListBox.h"
#include "Font.h"

// +--------------------------------------------------------------------+

class MenuScreen;

// +--------------------------------------------------------------------+

class NetClientDlg : public FormWindow
{
public:
    NetClientDlg(Screen* s, FormDef& def, MenuScreen* mgr);
    virtual ~NetClientDlg();

    virtual void      RegisterControls();
    virtual void      Show();
    virtual void      ExecFrame();

    // Operations:
    virtual void      OnSelect(AWEvent* event);
    virtual void      OnAdd(AWEvent* event);
    virtual void      OnDel(AWEvent* event);
    virtual void      OnServer(AWEvent* event);
    virtual void      OnHost(AWEvent* event);
    virtual void      OnJoin(AWEvent* event);
    virtual void      OnCancel(AWEvent* event);

    virtual void      ShowServers();
    virtual void      UpdateServers();
    virtual void      PingServer(int n);
    virtual bool      PingComplete();
    virtual void      StopNetProc();

protected:
    MenuScreen*       manager;
    NetClientConfig*  config;

    Button*           btn_add;
    Button*           btn_del;
    ListBox*          lst_servers;
    ActiveWindow*     lbl_info;
    int               server_index;
    int               ping_index;
    HANDLE            hnet;
    NetServerInfo     ping_info;

    Button*           btn_server;
    Button*           btn_host;
    Button*           btn_join;
    Button*           btn_cancel;
};

#endif NetClientDlg_h

