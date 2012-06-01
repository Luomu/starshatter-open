/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

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

