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
    FILE:         MenuScreen.h
    AUTHOR:       John DiCamillo

*/

#ifndef MenuScreen_h
#define MenuScreen_h

#include "Types.h"
#include "Bitmap.h"
#include "Screen.h"
#include "BaseScreen.h"

// +--------------------------------------------------------------------+

class MenuDlg;
class AudDlg;
class VidDlg;
class OptDlg;
class CtlDlg;
class JoyDlg;
class KeyDlg;
class ExitDlg;
class ConfirmDlg;

class FirstTimeDlg;
class PlayerDlg;
class AwardShowDlg;

class MsnSelectDlg;
class CmpSelectDlg;
class ModDlg;
class ModInfoDlg;
class MsnEditDlg;
class MsnElemDlg;
class MsnEventDlg;

class NetClientDlg;
class NetAddrDlg;
class NetPassDlg;
class NetLobbyDlg;
class NetServerDlg;
class NetUnitDlg;

class LoadDlg;
class TacRefDlg;

class ActiveWindow;
class Bitmap;
class DataLoader;
class FadeView;
class Font;
class FormWindow;
class Screen;
class Video;
class VideoFactory;
class Window;

// +--------------------------------------------------------------------+

class MenuScreen : public BaseScreen
{
public:
    MenuScreen();
    virtual ~MenuScreen();

    virtual void   Setup(Screen* screen);
    virtual void   TearDown();
    virtual bool   CloseTopmost();

    virtual bool   IsShown()         const { return isShown;       }
    virtual void   Show();
    virtual void   Hide();

    virtual void   ExecFrame();

    virtual void   ShowMenuDlg();
    virtual void   ShowCmpSelectDlg();
    virtual void   ShowMsnSelectDlg();
    virtual void   ShowModDlg();
    virtual void   ShowModInfoDlg();
    virtual void   ShowMsnEditDlg();
    virtual void   ShowNetClientDlg();
    virtual void   ShowNetAddrDlg();
    virtual void   ShowNetPassDlg();
    virtual void   ShowNetLobbyDlg();
    virtual void   ShowNetServerDlg();
    virtual void   ShowNetUnitDlg();
    virtual void   ShowFirstTimeDlg();
    virtual void   ShowPlayerDlg();
    virtual void   ShowTacRefDlg();
    virtual void   ShowAwardDlg();
    virtual void   ShowAudDlg();
    virtual void   ShowVidDlg();
    virtual void   ShowOptDlg();
    virtual void   ShowCtlDlg();
    virtual void   ShowJoyDlg();
    virtual void   ShowKeyDlg();
    virtual void   ShowExitDlg();
    virtual void   ShowConfirmDlg();
    virtual void   HideConfirmDlg();
    virtual void   ShowLoadDlg();
    virtual void   HideLoadDlg();

    // base screen interface:
    virtual void            ShowMsnElemDlg();
    virtual void            HideMsnElemDlg();
    virtual MsnElemDlg*     GetMsnElemDlg()   { return msnElemDlg;    }

    virtual void            ShowMsnEventDlg();
    virtual void            HideMsnEventDlg();
    virtual MsnEventDlg*    GetMsnEventDlg()  { return msnEventDlg;   }

    virtual void            ShowNavDlg();
    virtual void            HideNavDlg();
    virtual bool            IsNavShown();
    virtual NavDlg*         GetNavDlg()       { return msnEditNavDlg; }

    virtual MsnSelectDlg*   GetMsnSelectDlg() const { return msnSelectDlg;  }
    virtual ModDlg*         GetModDlg()       const { return modDlg;        }
    virtual ModInfoDlg*     GetModInfoDlg()   const { return modInfoDlg;    }
    virtual MsnEditDlg*     GetMsnEditDlg()   const { return msnEditDlg;    }
    virtual NetClientDlg*   GetNetClientDlg() const { return netClientDlg;  }
    virtual NetAddrDlg*     GetNetAddrDlg()   const { return netAddrDlg;    }
    virtual NetPassDlg*     GetNetPassDlg()   const { return netPassDlg;    }
    virtual NetLobbyDlg*    GetNetLobbyDlg()  const { return netLobbyDlg;   }
    virtual NetServerDlg*   GetNetServerDlg() const { return netServerDlg;  }
    virtual NetUnitDlg*     GetNetUnitDlg()   const { return netUnitDlg;    }
    virtual LoadDlg*        GetLoadDlg()      const { return loadDlg;       }
    virtual TacRefDlg*      GetTacRefDlg()    const { return tacRefDlg;     }

    virtual AudDlg*         GetAudDlg()       const { return auddlg;        }
    virtual VidDlg*         GetVidDlg()       const { return viddlg;        }
    virtual OptDlg*         GetOptDlg()       const { return optdlg;        }
    virtual CtlDlg*         GetCtlDlg()       const { return ctldlg;        }
    virtual JoyDlg*         GetJoyDlg()       const { return joydlg;        }
    virtual KeyDlg*         GetKeyDlg()       const { return keydlg;        }
    virtual ExitDlg*        GetExitDlg()      const { return exitdlg;       }
    virtual FirstTimeDlg*   GetFirstTimeDlg() const { return firstdlg;      }
    virtual PlayerDlg*      GetPlayerDlg()    const { return playdlg;       }
    virtual AwardShowDlg*   GetAwardDlg()     const { return awarddlg;      }
    virtual ConfirmDlg*     GetConfirmDlg()   const { return confirmdlg;    }

    virtual void            ApplyOptions();
    virtual void            CancelOptions();

private:
    void           HideAll();

    Screen*        screen;
    MenuDlg*       menudlg;

    Window*        fadewin;
    FadeView*      fadeview;
    ExitDlg*       exitdlg;
    AudDlg*        auddlg;
    VidDlg*        viddlg;
    OptDlg*        optdlg;
    CtlDlg*        ctldlg;
    JoyDlg*        joydlg;
    KeyDlg*        keydlg;
    ConfirmDlg*    confirmdlg;
    PlayerDlg*     playdlg;
    AwardShowDlg*  awarddlg;
    ModDlg*        modDlg;
    ModInfoDlg*    modInfoDlg;
    MsnSelectDlg*  msnSelectDlg;
    MsnEditDlg*    msnEditDlg;
    MsnElemDlg*    msnElemDlg;
    MsnEventDlg*   msnEventDlg;
    NavDlg*        msnEditNavDlg;
    LoadDlg*       loadDlg;
    TacRefDlg*     tacRefDlg;

    CmpSelectDlg*  cmpSelectDlg;
    FirstTimeDlg*  firstdlg;
    NetClientDlg*  netClientDlg;
    NetAddrDlg*    netAddrDlg;
    NetPassDlg*    netPassDlg;
    NetLobbyDlg*   netLobbyDlg;
    NetServerDlg*  netServerDlg;
    NetUnitDlg*    netUnitDlg;

    FormWindow*    current_dlg;
    DataLoader*    loader;

    int            wc, hc;
    bool           isShown;
};

// +--------------------------------------------------------------------+

#endif MenuScreen_h

