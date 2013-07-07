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
    FILE:         MenuDlg.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Main Menu Dialog Active Window class
*/

#ifndef MenuDlg_h
#define MenuDlg_h

#include "Types.h"
#include "FormWindow.h"
#include "Bitmap.h"
#include "Button.h"
#include "ComboBox.h"
#include "ListBox.h"
#include "Font.h"
#include "Text.h"

// +--------------------------------------------------------------------+

class  MenuScreen;
class  Campaign;
class  Starshatter;

// +--------------------------------------------------------------------+

class MenuDlg : public FormWindow
{
public:
    MenuDlg(Screen* s, FormDef& def, MenuScreen* mgr);
    virtual ~MenuDlg();

    virtual void      RegisterControls();
    virtual void      Show();
    virtual void      ExecFrame();

    // Operations:
    virtual void      OnStart(AWEvent* event);
    virtual void      OnCampaign(AWEvent* event);
    virtual void      OnMission(AWEvent* event);
    virtual void      OnPlayer(AWEvent* event);
    virtual void      OnMultiplayer(AWEvent* event);
    virtual void      OnMod(AWEvent* event);
    virtual void      OnTacReference(AWEvent* event);

    virtual void      OnVideo(AWEvent* event);
    virtual void      OnOptions(AWEvent* event);
    virtual void      OnControls(AWEvent* event);
    virtual void      OnQuit(AWEvent* event);

    virtual void      OnButtonEnter(AWEvent* event);
    virtual void      OnButtonExit(AWEvent* event);

protected:
    MenuScreen*       manager;

    Button*           btn_start;
    Button*           btn_campaign;
    Button*           btn_mission;
    Button*           btn_player;
    Button*           btn_multi;
    Button*           btn_mod;
    Button*           btn_tac;

    Button*           btn_video;
    Button*           btn_options;
    Button*           btn_controls;
    Button*           btn_quit;

    ActiveWindow*     version;
    ActiveWindow*     description;

    Starshatter*      stars;
    Campaign*         campaign;
};

#endif MenuDlg_h

