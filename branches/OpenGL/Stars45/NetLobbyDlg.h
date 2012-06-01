/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

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

