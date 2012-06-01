/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2006. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         CmpSceneDlg.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
*/

#include "MemDebug.h"
#include "CmpSceneDlg.h"
#include "CmpnScreen.h"
#include "GameScreen.h"
#include "Campaign.h"
#include "CombatEvent.h"
#include "Starshatter.h"
#include "Sim.h"
#include "CameraDirector.h"
#include "Mission.h"
#include "MissionEvent.h"

#include "Game.h"
#include "DataLoader.h"
#include "Video.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "ImageBox.h"
#include "RichTextBox.h"
#include "Button.h"
#include "FormatUtil.h"

// +--------------------------------------------------------------------+
// DECLARE MAPPING FUNCTIONS:

// +--------------------------------------------------------------------+

CmpSceneDlg::CmpSceneDlg(Screen* s, FormDef& def, CmpnScreen* mgr)
: FormWindow(s, 0, 0, s->Width(), s->Height()), manager(mgr),
mov_scene(0), subtitles_box(0), cam_view(0), disp_view(0), old_disp_win(0),
flare1(0), flare2(0), flare3(0), flare4(0), subtitles_delay(0), subtitles_time(0)
{
	Init(def);

	DataLoader* loader = DataLoader::GetLoader();
	const char* oldpath = loader->GetDataPath();

	loader->SetDataPath(0);
	loader->LoadTexture("flare0+.pcx", flare1, Bitmap::BMP_TRANSLUCENT);
	loader->LoadTexture("flare2.pcx",  flare2, Bitmap::BMP_TRANSLUCENT);
	loader->LoadTexture("flare3.pcx",  flare3, Bitmap::BMP_TRANSLUCENT);
	loader->LoadTexture("flare4.pcx",  flare4, Bitmap::BMP_TRANSLUCENT);
	loader->SetDataPath(oldpath);
}

CmpSceneDlg::~CmpSceneDlg()
{
}

void
CmpSceneDlg::RegisterControls()
{
	mov_scene      =                 FindControl(101);
	subtitles_box  = (RichTextBox*)  FindControl(102);

	if (mov_scene) {
		CameraDirector* cam_dir = CameraDirector::GetInstance();
		cam_view = new(__FILE__,__LINE__) CameraView(mov_scene, cam_dir->GetCamera(), 0);
		if (cam_view)
		mov_scene->AddView(cam_view);

		disp_view = DisplayView::GetInstance();
	}
}

void
CmpSceneDlg::Show()
{
	FormWindow::Show();

	Starshatter* stars = Starshatter::GetInstance();

	if (stars->InCutscene()) {
		Sim* sim = Sim::GetSim();

		if (sim) {
			cam_view->UseCamera(CameraDirector::GetInstance()->GetCamera());
			cam_view->UseScene(sim->GetScene());
		}

		// initialize lens flare bitmaps:
		if (stars->LensFlare()) {
			cam_view->LensFlareElements(flare1, flare4, flare2, flare3);
			cam_view->LensFlare(true);
		}

		// if lens flare disabled, just create the corona:
		else if (stars->Corona()) {
			cam_view->LensFlareElements(flare1, 0, 0, 0);
			cam_view->LensFlare(true);
		}

		if (disp_view) {
			old_disp_win = disp_view->GetWindow();

			disp_view->SetWindow(mov_scene);
			mov_scene->AddView(disp_view);
		}

		if (subtitles_box) {
			subtitles_box->SetText(stars->GetSubtitles());
			subtitles_delay = 0;
			subtitles_time  = 0;
		}
	}
}

void
CmpSceneDlg::Hide()
{
	FormWindow::Hide();

	if (disp_view && mov_scene && old_disp_win) {
		mov_scene->DelView(disp_view);
		disp_view->SetWindow(old_disp_win);
	}
}

// +--------------------------------------------------------------------+

CameraView*
CmpSceneDlg::GetCameraView()
{
	return cam_view;
}

DisplayView*
CmpSceneDlg::GetDisplayView()
{
	return disp_view;
}

// +--------------------------------------------------------------------+

void
CmpSceneDlg::ExecFrame()
{
	Starshatter*   stars             = Starshatter::GetInstance();
	Mission*       cutscene_mission  = stars->GetCutsceneMission();

	if (cutscene_mission && disp_view) {
		disp_view->ExecFrame();

		if (subtitles_box && subtitles_box->GetLineCount() > 0) {
			if (subtitles_delay == 0) {
				int nlines = subtitles_box->GetLineCount();

				MissionEvent* begin_scene = cutscene_mission->FindEvent(MissionEvent::BEGIN_SCENE);
				MissionEvent* end_scene   = cutscene_mission->FindEvent(MissionEvent::END_SCENE);

				if (begin_scene && end_scene) {
					double total_time = end_scene->Time() - begin_scene->Time();
					subtitles_delay   = total_time / nlines;
					subtitles_time    = Game::RealTime() / 1000.0 + subtitles_delay;
				}
				else {
					subtitles_delay = -1;
				}
			}

			if (subtitles_delay > 0) {
				double seconds = Game::RealTime() / 1000.0;

				if (subtitles_time <= seconds) {
					subtitles_time = seconds + subtitles_delay;
					subtitles_box->Scroll(ScrollWindow::SCROLL_DOWN);
				}
			}
		}
	}
	else {
		manager->ShowCmdDlg();
	}
}

