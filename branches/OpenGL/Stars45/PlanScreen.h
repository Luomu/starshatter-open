/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

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

