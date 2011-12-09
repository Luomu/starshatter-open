/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

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

