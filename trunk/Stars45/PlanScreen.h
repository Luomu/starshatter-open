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
    FILE:         PlanScreen.h
    AUTHOR:       John DiCamillo

*/

#ifndef PlanScreen_h
#define PlanScreen_h

#include "Types.h"
#include "Bitmap.h"
#include "Screen.h"
#include "BaseScreen.h"

// +--------------------------------------------------------------------+

class MsnObjDlg;
class MsnPkgDlg;
class MsnWepDlg;
class MsnNavDlg;
class DebriefDlg;
class AwardDlg;

class Bitmap;
class DataLoader;
class Font;
class Screen;
class Video;
class VideoFactory;

// +--------------------------------------------------------------------+

class PlanScreen : public BaseScreen
{
public:
    PlanScreen();
    virtual ~PlanScreen();

    virtual void        Setup(Screen* screen);
    virtual void        TearDown();
    virtual bool        CloseTopmost();

    virtual bool        IsShown()         const { return isShown; }
    virtual void        Show();
    virtual void        Hide();

    virtual void        ShowMsnDlg();
    virtual void        HideMsnDlg();
    virtual bool        IsMsnShown();

    virtual void        ShowMsnObjDlg();
    virtual void        HideMsnObjDlg();
    virtual bool        IsMsnObjShown();
    virtual MsnObjDlg*  GetMsnObjDlg()          { return objdlg; }

    virtual void        ShowMsnPkgDlg();
    virtual void        HideMsnPkgDlg();
    virtual bool        IsMsnPkgShown();
    virtual MsnPkgDlg*  GetMsnPkgDlg()          { return pkgdlg; }

    virtual void        ShowMsnWepDlg();
    virtual void        HideMsnWepDlg();
    virtual bool        IsMsnWepShown();
    virtual MsnWepDlg*  GetMsnWepDlg()          { return wepdlg; }

    virtual void        ShowNavDlg();
    virtual void        HideNavDlg();
    virtual bool        IsNavShown();
    virtual NavDlg*     GetNavDlg()             { return (NavDlg*) navdlg; }

    virtual void        ShowDebriefDlg();
    virtual void        HideDebriefDlg();
    virtual bool        IsDebriefShown();
    virtual DebriefDlg* GetDebriefDlg()         { return debrief_dlg; }

    virtual void        ShowAwardDlg();
    virtual void        HideAwardDlg();
    virtual bool        IsAwardShown();
    virtual AwardDlg*   GetAwardDlg()           { return award_dlg; }

    virtual void        ExecFrame();
    virtual void        HideAll();


private:
    Screen*                 screen;

    MsnObjDlg*              objdlg;
    MsnPkgDlg*              pkgdlg;
    MsnWepDlg*              wepdlg;
    MsnNavDlg*              navdlg;
    DebriefDlg*             debrief_dlg;
    AwardDlg*               award_dlg;

    DataLoader*             loader;

    int                     wc, hc;
    bool                    isShown;
};

// +--------------------------------------------------------------------+

#endif PlanScreen_h

