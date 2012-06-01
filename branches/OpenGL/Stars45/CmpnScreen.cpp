/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2006. All Rights Reserved.

	SUBSYSTEM:    Stars
	FILE:         CmpnScreen.cpp
	AUTHOR:       John DiCamillo

*/

#include "MemDebug.h"
#include "CmpnScreen.h"

#include "CmdForceDlg.h"
#include "CmdMissionsDlg.h"
#include "CmdOrdersDlg.h"
#include "CmdTheaterDlg.h"
#include "CmdIntelDlg.h"
#include "CmpCompleteDlg.h"
#include "CmdMsgDlg.h"
#include "CmpFileDlg.h"
#include "CmpSceneDlg.h"
#include "Campaign.h"
#include "CombatEvent.h"
#include "Mission.h"
#include "Sim.h"
#include "Starshatter.h"
#include "StarSystem.h"
#include "Player.h"
#include "MusicDirector.h"

#include "Game.h"
#include "Video.h"
#include "Screen.h"
#include "Window.h"
#include "ActiveWindow.h"
#include "FormDef.h"
#include "Mouse.h"
#include "Color.h"
#include "Bitmap.h"
#include "Font.h"
#include "FontMgr.h"
#include "EventDispatch.h"
#include "DataLoader.h"
#include "Resource.h"

// +--------------------------------------------------------------------+

CmpnScreen::CmpnScreen()
: screen(0), 
cmd_force_dlg(0), cmd_missions_dlg(0), cmd_orders_dlg(0),
cmd_intel_dlg(0), cmd_theater_dlg(0), cmd_msg_dlg(0), cmp_file_dlg(0),
cmp_end_dlg(0), cmp_scene_dlg(0),
isShown(false), campaign(0), stars(0), completion_stage(0)
{
	loader   = DataLoader::GetLoader();
	stars    = Starshatter::GetInstance();
}

CmpnScreen::~CmpnScreen()
{
	TearDown();
}

// +--------------------------------------------------------------------+

void
CmpnScreen::Setup(Screen* s)
{
	if (!s)
	return;

	screen = s;

	loader->UseFileSystem(true);

	FormDef cmd_orders_def("CmdOrdersDlg", 0);
	cmd_orders_def.Load("CmdOrdersDlg");
	cmd_orders_dlg = new(__FILE__,__LINE__) CmdOrdersDlg(screen, cmd_orders_def, this);

	FormDef cmd_force_def("CmdForceDlg", 0);
	cmd_force_def.Load("CmdForceDlg");
	cmd_force_dlg = new(__FILE__,__LINE__) CmdForceDlg(screen, cmd_force_def, this);

	FormDef cmd_theater_def("CmdTheaterDlg", 0);
	cmd_theater_def.Load("CmdTheaterDlg");
	cmd_theater_dlg = new(__FILE__,__LINE__) CmdTheaterDlg(screen, cmd_theater_def, this);

	FormDef cmd_intel_def("CmdIntelDlg", 0);
	cmd_intel_def.Load("CmdIntelDlg");
	cmd_intel_dlg = new(__FILE__,__LINE__) CmdIntelDlg(screen, cmd_intel_def, this);

	FormDef cmd_missions_def("CmdMissionsDlg", 0);
	cmd_missions_def.Load("CmdMissionsDlg");
	cmd_missions_dlg = new(__FILE__,__LINE__) CmdMissionsDlg(screen, cmd_missions_def, this);

	FormDef file_def("FileDlg", 0);
	file_def.Load("FileDlg");
	cmp_file_dlg = new(__FILE__,__LINE__) CmpFileDlg(screen, file_def, this);

	FormDef msg_def("CmdMsgDlg", 0);
	msg_def.Load("CmdMsgDlg");
	cmd_msg_dlg = new(__FILE__,__LINE__) CmdMsgDlg(screen, msg_def, this);

	FormDef end_def("CmpCompleteDlg", 0);
	end_def.Load("CmpCompleteDlg");
	cmp_end_dlg = new(__FILE__,__LINE__) CmpCompleteDlg(screen, end_def, this);

	FormDef scene_def("CmpSceneDlg", 0);
	scene_def.Load("CmpSceneDlg");
	cmp_scene_dlg = new(__FILE__,__LINE__) CmpSceneDlg(screen, scene_def, this);

	loader->UseFileSystem(Starshatter::UseFileSystem());

	HideAll();
}

// +--------------------------------------------------------------------+

void
CmpnScreen::TearDown()
{
	if (screen) {
		screen->DelWindow(cmd_force_dlg);
		screen->DelWindow(cmd_missions_dlg);
		screen->DelWindow(cmd_orders_dlg);
		screen->DelWindow(cmd_intel_dlg);
		screen->DelWindow(cmd_theater_dlg);
		screen->DelWindow(cmd_msg_dlg);
		screen->DelWindow(cmp_file_dlg);
		screen->DelWindow(cmp_end_dlg);
		screen->DelWindow(cmp_scene_dlg);
	}

	delete cmd_force_dlg;
	delete cmd_missions_dlg;
	delete cmd_orders_dlg;
	delete cmd_intel_dlg;
	delete cmd_theater_dlg;
	delete cmd_msg_dlg;
	delete cmp_file_dlg;
	delete cmp_end_dlg;
	delete cmp_scene_dlg;

	cmd_force_dlg     = 0;
	cmd_missions_dlg  = 0;
	cmd_orders_dlg    = 0;
	cmd_intel_dlg     = 0;
	cmd_theater_dlg   = 0;
	cmd_msg_dlg       = 0;
	cmp_file_dlg      = 0;
	cmp_end_dlg       = 0;
	cmp_scene_dlg     = 0;
	screen            = 0;
}

// +--------------------------------------------------------------------+

void
CmpnScreen::SetFieldOfView(double fov)
{
	if (cmp_scene_dlg)
	cmp_scene_dlg->GetCameraView()->SetFieldOfView(fov);
}

// +--------------------------------------------------------------------+

double
CmpnScreen::GetFieldOfView() const
{
	if (cmp_scene_dlg)
	return cmp_scene_dlg->GetCameraView()->GetFieldOfView();

	return 2;
}

// +--------------------------------------------------------------------+

void
CmpnScreen::ExecFrame()
{
	Game::SetScreenColor(Color::Black);

	if (cmd_orders_dlg && cmd_orders_dlg->IsShown()) {
		cmd_orders_dlg->ExecFrame();
	}

	else if (cmd_force_dlg && cmd_force_dlg->IsShown()) {
		cmd_force_dlg->ExecFrame();
	}

	else if (cmd_theater_dlg && cmd_theater_dlg->IsShown()) {
		cmd_theater_dlg->ExecFrame();
	}

	else if (cmd_missions_dlg && cmd_missions_dlg->IsShown()) {
		cmd_missions_dlg->ExecFrame();
	}

	else if (cmd_intel_dlg && cmd_intel_dlg->IsShown()) {
		cmd_intel_dlg->ExecFrame();
	}

	if (cmp_file_dlg && cmp_file_dlg->IsShown()) {
		cmp_file_dlg->ExecFrame();
	}

	if (cmd_msg_dlg && cmd_msg_dlg->IsShown()) {
		cmd_msg_dlg->ExecFrame();
	}

	else if (cmp_end_dlg && cmp_end_dlg->IsShown()) {
		cmp_end_dlg->ExecFrame();
		completion_stage = 2;
	}

	else if (cmp_scene_dlg && cmp_scene_dlg->IsShown()) {
		cmp_scene_dlg->ExecFrame();

		if (completion_stage > 0)
		completion_stage = 2;
	}

	else if (campaign) {
		// if campaign is complete
		if (completion_stage == 0) {
			Player* player = Player::GetCurrentPlayer();
			char    msg_info[1024];

			if (!player)
			return;

			if (campaign->IsTraining()) {
				int all_missions = (1<<campaign->GetMissionList().size())-1;

				if (player->Trained() >= all_missions && player->Trained() < 255) {
					player->SetTrained(255);
					cmd_msg_dlg->Title()->SetText(Game::GetText("CmpnScreen.training"));
					sprintf_s(msg_info, Game::GetText("CmpnScreen.congrats"),
						Player::RankName(player->Rank()),
						player->Name().data());

					cmd_msg_dlg->Message()->SetText(msg_info);
					cmd_msg_dlg->Message()->SetTextAlign(DT_LEFT);

					ShowCmdMsgDlg();
					completion_stage = 1;
				}
			}

			else if (campaign->IsComplete() || campaign->IsFailed()) {
				bool           cutscene = false;
				CombatEvent*   event    = campaign->GetLastEvent();
				
				if (event && !event->Visited() && event->SceneFile() && *event->SceneFile()) {
					stars->ExecCutscene(event->SceneFile(), campaign->Path());

					if (stars->InCutscene()) {
						cutscene = true;
						ShowCmpSceneDlg();
					}
				}

				if (!cutscene) {
					ShowCmpCompleteDlg();
				}

				if (campaign->IsComplete())
				MusicDirector::SetMode(MusicDirector::VICTORY);
				else
				MusicDirector::SetMode(MusicDirector::DEFEAT);

				completion_stage = 1;
			}
		}

		// if message has been shown, restart
		else if (completion_stage > 1) {
			completion_stage = 0;

			if (campaign->IsTraining()) {
				List<Campaign>& list    = Campaign::GetAllCampaigns();
				Campaign*       c       = list[1];

				if (c) {
					c->Load();
					Campaign::SelectCampaign(c->Name());
					stars->SetGameMode(Starshatter::CLOD_MODE);
					return;
				}
			}

#ifdef STARSHATTER_DEMO_RELEASE
			if (!campaign->IsTraining()) {
				Mouse::Show(false);
				MusicDirector::SetMode(MusicDirector::MENU);
				stars->SetGameMode(Starshatter::MENU_MODE);
				return;
			}
#endif

			// continue on to the next available campaign:
			if (campaign->GetCampaignId() < Campaign::GetLastCampaignId()) {
				stars->StartOrResumeGame();
			}

			// if this was the last campaign, just go back to the menu:
			else {
				Mouse::Show(false);
				MusicDirector::SetMode(MusicDirector::MENU);
				stars->SetGameMode(Starshatter::MENU_MODE);
				return;
			}
		}
	}

	if (completion_stage < 1) {
		MusicDirector::SetMode(MusicDirector::MENU);
		Mouse::Show(!IsCmpSceneShown());
	}
}

// +--------------------------------------------------------------------+

bool
CmpnScreen::CloseTopmost()
{
	bool processed = false;

	if (IsCmdMsgShown()) {
		HideCmdMsgDlg();
		processed = true;
	}

	else if (IsCmpFileShown()) {
		HideCmpFileDlg();
		processed = true;
	}

	return processed;
}

void
CmpnScreen::Show()
{
	if (!isShown) {
		isShown = true;

		campaign = Campaign::GetCampaign();
		completion_stage = 0;

		bool           cutscene = false;
		CombatEvent*   event    = 0;
		
		if (campaign->IsActive() && !campaign->GetEvents().isEmpty()) {
			ListIter<CombatEvent> iter = campaign->GetEvents();
			while (++iter) {
				event = iter.value();

				if (event && !event->Visited() && event->SceneFile() && *event->SceneFile()) {
					stars->ExecCutscene(event->SceneFile(), campaign->Path());

					if (stars->InCutscene()) {
						cutscene = true;
						ShowCmpSceneDlg();
					}

					event->SetVisited(true);
					break;
				}
			}
		}

		if (!cutscene)
		ShowCmdDlg();
	}
}

void
CmpnScreen::Hide()
{
	if (isShown) {
		HideAll();
		isShown = false;
	}
}

void
CmpnScreen::HideAll()
{
	if (cmd_force_dlg)      cmd_force_dlg->Hide();
	if (cmd_missions_dlg)   cmd_missions_dlg->Hide();
	if (cmd_orders_dlg)     cmd_orders_dlg->Hide();
	if (cmd_intel_dlg)      cmd_intel_dlg->Hide();
	if (cmd_theater_dlg)    cmd_theater_dlg->Hide();
	if (cmd_msg_dlg)        cmd_msg_dlg->Hide();
	if (cmp_file_dlg)       cmp_file_dlg->Hide();
	if (cmp_end_dlg)        cmp_end_dlg->Hide();
	if (cmp_scene_dlg)      cmp_scene_dlg->Hide();
}

// +--------------------------------------------------------------------+

void
CmpnScreen::ShowCmdDlg()
{
	ShowCmdOrdersDlg();
}

// +--------------------------------------------------------------------+

void
CmpnScreen::ShowCmdForceDlg()
{
	HideAll();
	cmd_force_dlg->Show();
	Mouse::Show(true);
}

// +--------------------------------------------------------------------+

void
CmpnScreen::HideCmdForceDlg()
{
	if (IsCmdForceShown())
	cmd_force_dlg->Hide();
}

// +--------------------------------------------------------------------+

bool
CmpnScreen::IsCmdForceShown()
{
	return cmd_force_dlg && cmd_force_dlg->IsShown();
}

// +--------------------------------------------------------------------+

void
CmpnScreen::ShowCmdOrdersDlg()
{
	HideAll();
	cmd_orders_dlg->Show();
	Mouse::Show(true);
}

// +--------------------------------------------------------------------+

void
CmpnScreen::HideCmdOrdersDlg()
{
	if (IsCmdOrdersShown())
	cmd_orders_dlg->Hide();
}

// +--------------------------------------------------------------------+

bool
CmpnScreen::IsCmdOrdersShown()
{
	return cmd_orders_dlg && cmd_orders_dlg->IsShown();
}

// +--------------------------------------------------------------------+

void
CmpnScreen::ShowCmdMissionsDlg()
{
	HideAll();
	cmd_missions_dlg->Show();
	Mouse::Show(true);
}

// +--------------------------------------------------------------------+

void
CmpnScreen::HideCmdMissionsDlg()
{
	if (IsCmdMissionsShown())
	cmd_missions_dlg->Hide();
}

// +--------------------------------------------------------------------+

bool
CmpnScreen::IsCmdMissionsShown()
{
	return cmd_missions_dlg && cmd_missions_dlg->IsShown();
}

// +--------------------------------------------------------------------+

void
CmpnScreen::ShowCmdIntelDlg()
{
	HideAll();
	cmd_intel_dlg->Show();
	Mouse::Show(true);
}

// +--------------------------------------------------------------------+

void
CmpnScreen::HideCmdIntelDlg()
{
	if (IsCmdIntelShown())
	cmd_intel_dlg->Hide();
}

// +--------------------------------------------------------------------+

bool
CmpnScreen::IsCmdIntelShown()
{
	return cmd_intel_dlg && cmd_intel_dlg->IsShown();
}

// +--------------------------------------------------------------------+

void
CmpnScreen::ShowCmdTheaterDlg()
{
	HideAll();
	cmd_theater_dlg->Show();
	Mouse::Show(true);
}

// +--------------------------------------------------------------------+

void
CmpnScreen::HideCmdTheaterDlg()
{
	if (IsCmdTheaterShown())
	cmd_theater_dlg->Hide();
}

// +--------------------------------------------------------------------+

bool
CmpnScreen::IsCmdTheaterShown()
{
	return cmd_theater_dlg && cmd_theater_dlg->IsShown();
}

// +--------------------------------------------------------------------+

void
CmpnScreen::ShowCmpFileDlg()
{
	cmp_file_dlg->Show();
	Mouse::Show(true);
}

// +--------------------------------------------------------------------+

void
CmpnScreen::HideCmpFileDlg()
{
	if (IsCmpFileShown())
	cmp_file_dlg->Hide();
}

// +--------------------------------------------------------------------+

bool
CmpnScreen::IsCmpFileShown()
{
	return cmp_file_dlg && cmp_file_dlg->IsShown();
}

// +--------------------------------------------------------------------+

void
CmpnScreen::ShowCmdMsgDlg()
{
	cmd_msg_dlg->Show();
	Mouse::Show(true);
}

// +--------------------------------------------------------------------+

void
CmpnScreen::HideCmdMsgDlg()
{
	if (IsCmdMsgShown())
	cmd_msg_dlg->Hide();
}

// +--------------------------------------------------------------------+

bool
CmpnScreen::IsCmdMsgShown()
{
	return cmd_msg_dlg && cmd_msg_dlg->IsShown();
}

// +--------------------------------------------------------------------+

void
CmpnScreen::ShowCmpCompleteDlg()
{
	cmp_end_dlg->Show();
	Mouse::Show(true);
}

// +--------------------------------------------------------------------+

void
CmpnScreen::HideCmpCompleteDlg()
{
	if (IsCmpCompleteShown())
	cmp_end_dlg->Hide();
}

// +--------------------------------------------------------------------+

bool
CmpnScreen::IsCmpCompleteShown()
{
	return cmp_end_dlg && cmp_end_dlg->IsShown();
}

// +--------------------------------------------------------------------+

void
CmpnScreen::ShowCmpSceneDlg()
{
	cmp_scene_dlg->Show();
	Mouse::Show(false);
}

// +--------------------------------------------------------------------+

void
CmpnScreen::HideCmpSceneDlg()
{
	if (IsCmpSceneShown())
	cmp_scene_dlg->Hide();
}

// +--------------------------------------------------------------------+

bool
CmpnScreen::IsCmpSceneShown()
{
	return cmp_scene_dlg && cmp_scene_dlg->IsShown();
}
