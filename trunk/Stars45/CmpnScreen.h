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
    FILE:         CmpnScreen.h
    AUTHOR:       John DiCamillo

*/

#ifndef CmpnScreen_h
#define CmpnScreen_h

#include "Types.h"
#include "Bitmap.h"
#include "Screen.h"
#include "BaseScreen.h"

// +--------------------------------------------------------------------+

class CmdForceDlg;
class CmdMissionsDlg;
class CmdOrdersDlg;
class CmdIntelDlg;
class CmdTheaterDlg;

class CmdMsgDlg;
class CmpFileDlg;
class CmpCompleteDlg;
class CmpSceneDlg;

class Campaign;
class Starshatter;

class Bitmap;
class DataLoader;
class Font;
class FormEx;
class Screen;
class Video;

// +--------------------------------------------------------------------+

class CmpnScreen
{
public:
    CmpnScreen();
    virtual ~CmpnScreen();

    virtual void         Setup(Screen* screen);
    virtual void         TearDown();
    virtual bool         CloseTopmost();

    virtual bool         IsShown()         const { return isShown; }
    virtual void         Show();
    virtual void         Hide();

    virtual void         ShowCmdDlg();

    virtual void         ShowCmdForceDlg();
    virtual void         HideCmdForceDlg();
    virtual bool         IsCmdForceShown();
    virtual CmdForceDlg* GetCmdForceDlg()        { return cmd_force_dlg; }

    virtual void         ShowCmdMissionsDlg();
    virtual void         HideCmdMissionsDlg();
    virtual bool         IsCmdMissionsShown();
    virtual CmdMissionsDlg* GetCmdMissionsDlg()  { return cmd_missions_dlg; }

    virtual void         ShowCmdOrdersDlg();
    virtual void         HideCmdOrdersDlg();
    virtual bool         IsCmdOrdersShown();
    virtual CmdOrdersDlg* GetCmdOrdersDlg()      { return cmd_orders_dlg; }

    virtual void         ShowCmdIntelDlg();
    virtual void         HideCmdIntelDlg();
    virtual bool         IsCmdIntelShown();
    virtual CmdIntelDlg* GetCmdIntelDlg()        { return cmd_intel_dlg; }

    virtual void         ShowCmdTheaterDlg();
    virtual void         HideCmdTheaterDlg();
    virtual bool         IsCmdTheaterShown();
    virtual CmdTheaterDlg* GetCmdTheaterDlg()    { return cmd_theater_dlg; }

    virtual void         ShowCmpFileDlg();
    virtual void         HideCmpFileDlg();
    virtual bool         IsCmpFileShown();
    virtual CmpFileDlg*  GetCmpFileDlg()         { return cmp_file_dlg; }

    virtual void         ShowCmdMsgDlg();
    virtual void         HideCmdMsgDlg();
    virtual bool         IsCmdMsgShown();
    virtual CmdMsgDlg*   GetCmdMsgDlg()          { return cmd_msg_dlg; }

    virtual void         ShowCmpCompleteDlg();
    virtual void         HideCmpCompleteDlg();
    virtual bool         IsCmpCompleteShown();
    virtual CmpCompleteDlg* GetCmpCompleteDlg()  { return cmp_end_dlg; }

    virtual void         ShowCmpSceneDlg();
    virtual void         HideCmpSceneDlg();
    virtual bool         IsCmpSceneShown();
    virtual CmpSceneDlg* GetCmpSceneDlg()        { return cmp_scene_dlg; }

    virtual void         HideAll();
    virtual void         ExecFrame();

    void                 SetFieldOfView(double fov);
    double               GetFieldOfView() const;

private:
    Screen*                 screen;

    CmdForceDlg*            cmd_force_dlg;
    CmdOrdersDlg*           cmd_orders_dlg;
    CmdMissionsDlg*         cmd_missions_dlg;
    CmdIntelDlg*            cmd_intel_dlg;
    CmdTheaterDlg*          cmd_theater_dlg;

    CmdMsgDlg*              cmd_msg_dlg;
    CmpFileDlg*             cmp_file_dlg;
    CmpCompleteDlg*         cmp_end_dlg;
    CmpSceneDlg*            cmp_scene_dlg;

    DataLoader*             loader;

    bool                    isShown;

    Campaign*               campaign;
    Starshatter*            stars;
    int                     completion_stage;
};

// +--------------------------------------------------------------------+

#endif CmpnScreen_h

