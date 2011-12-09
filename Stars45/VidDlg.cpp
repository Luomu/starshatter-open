/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         VidDlg.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Main Menu Dialog Active Window class
*/

#include "MemDebug.h"
#include "VidDlg.h"
#include "BaseScreen.h"
#include "Starshatter.h"
#include "Ship.h"
#include "Terrain.h"
#include "CameraDirector.h"

#include "DataLoader.h"
#include "Button.h"
#include "ListBox.h"
#include "Slider.h"
#include "Video.h"
#include "VideoSettings.h"
#include "Keyboard.h"
#include "MachineInfo.h"

// +--------------------------------------------------------------------+
// DECLARE MAPPING FUNCTIONS:

DEF_MAP_CLIENT(VidDlg, OnTexSize);
DEF_MAP_CLIENT(VidDlg, OnMode);
DEF_MAP_CLIENT(VidDlg, OnDetail);
DEF_MAP_CLIENT(VidDlg, OnTexture);
DEF_MAP_CLIENT(VidDlg, OnGamma);
DEF_MAP_CLIENT(VidDlg, OnApply);
DEF_MAP_CLIENT(VidDlg, OnCancel);
DEF_MAP_CLIENT(VidDlg, OnAudio);
DEF_MAP_CLIENT(VidDlg, OnVideo);
DEF_MAP_CLIENT(VidDlg, OnOptions);
DEF_MAP_CLIENT(VidDlg, OnControls);
DEF_MAP_CLIENT(VidDlg, OnMod);

// +--------------------------------------------------------------------+

VidDlg::VidDlg(Screen* s, FormDef& def, BaseScreen* mgr)
: FormWindow(s,  0,  0, s->Width(), s->Height()), manager(mgr),
selected_mode(0), selected_detail(0), orig_gamma(128),
selected_card(0), selected_tex_size(0), selected_render(0), selected_texture(0),
mode(0), tex_size(0), detail(0), texture(0), gamma(0), shadows(0), spec_maps(0), 
bump_maps(0), lens_flare(0), corona(0), nebula(0), dust(0),
apply(0), cancel(0), vid_btn(0), aud_btn(0), ctl_btn(0), opt_btn(0), mod_btn(0),
closed(true)
{
	stars = Starshatter::GetInstance();

	Init(def);
	orig_gamma = Game::GammaLevel();
}

VidDlg::~VidDlg()
{
}

// +--------------------------------------------------------------------+

void
VidDlg::RegisterControls()
{
	if (apply)
	return;

	mode    = (ComboBox*) FindControl(203);
	REGISTER_CLIENT(EID_SELECT, mode, VidDlg, OnMode);

	tex_size = (ComboBox*) FindControl(204);
	REGISTER_CLIENT(EID_SELECT, tex_size, VidDlg, OnTexSize);

	detail  = (ComboBox*) FindControl(205);
	REGISTER_CLIENT(EID_SELECT, detail, VidDlg, OnDetail);

	texture = (ComboBox*) FindControl(206);
	REGISTER_CLIENT(EID_SELECT, texture, VidDlg, OnTexture);

	gamma   = (Slider*) FindControl(215);

	if (gamma) {
		gamma->SetRangeMin(32);
		gamma->SetRangeMax(224);
		gamma->SetStepSize(16);

		REGISTER_CLIENT(EID_CLICK, gamma, VidDlg, OnGamma);
	}

	lens_flare  = (ComboBox*) FindControl(211);
	corona      = (ComboBox*) FindControl(212);
	nebula      = (ComboBox*) FindControl(213);
	dust        = (ComboBox*) FindControl(214);
	shadows     = (ComboBox*) FindControl(222);
	spec_maps   = (ComboBox*) FindControl(223);
	bump_maps   = (ComboBox*) FindControl(224);

	apply   = (Button*) FindControl(1);
	REGISTER_CLIENT(EID_CLICK, apply, VidDlg, OnApply);

	cancel  = (Button*) FindControl(2);
	REGISTER_CLIENT(EID_CLICK, cancel, VidDlg, OnCancel);

	vid_btn = (Button*) FindControl(901);
	REGISTER_CLIENT(EID_CLICK, vid_btn, VidDlg, OnVideo);

	aud_btn = (Button*) FindControl(902);
	REGISTER_CLIENT(EID_CLICK, aud_btn, VidDlg, OnAudio);

	ctl_btn = (Button*) FindControl(903);
	REGISTER_CLIENT(EID_CLICK, ctl_btn, VidDlg, OnControls);

	opt_btn = (Button*) FindControl(904);
	REGISTER_CLIENT(EID_CLICK, opt_btn, VidDlg, OnOptions);

	mod_btn = (Button*) FindControl(905);
	if (mod_btn)
	REGISTER_CLIENT(EID_CLICK, mod_btn, VidDlg, OnMod);
}

// +--------------------------------------------------------------------+

void
VidDlg::Show()
{
	FormWindow::Show();

	if (closed) {
		bool fullscreen = true;

		if (stars) {
			selected_render   = 9;
			selected_card     = 0;

			int n = stars->MaxTexSize();

			for (int i = 0; i < 7; i++) {
				if (n <= pow(2.0f, i+6)) {
					selected_tex_size = i;
					break;
				}
			}

			Video* video = Game::GetVideo();

			if (video) {
				if (shadows)
				shadows->SetSelection(video->IsShadowEnabled());

				if (spec_maps)
				spec_maps->SetSelection(video->IsSpecMapEnabled());

				if (bump_maps)
				bump_maps->SetSelection(video->IsBumpMapEnabled());

				fullscreen = video->IsFullScreen();
			}

			if (lens_flare)
			lens_flare->SetSelection(stars->LensFlare());

			if (corona)
			corona->SetSelection(stars->Corona());

			if (nebula)
			nebula->SetSelection(stars->Nebula());

			if (dust)
			dust->SetSelection(stars->Dust());
		}

		selected_detail  = Terrain::DetailLevel() - 2;
		selected_texture = true;

		if (mode) {
			BuildModeList();
			mode->SetSelection(selected_mode);
			mode->SetEnabled(fullscreen);
		}

		if (tex_size)
		tex_size->SetSelection(selected_tex_size);

		if (detail)
		detail->SetSelection(selected_detail);

		if (texture)
		texture->SetSelection(selected_texture);


		if (gamma) {
			orig_gamma = Game::GammaLevel();
			gamma->SetValue(orig_gamma);
		}
	}

	if (vid_btn)   vid_btn->SetButtonState(1);
	if (aud_btn)   aud_btn->SetButtonState(0);
	if (ctl_btn)   ctl_btn->SetButtonState(0);
	if (opt_btn)   opt_btn->SetButtonState(0);
	if (mod_btn)   mod_btn->SetButtonState(0);

	closed = false;
}

// +--------------------------------------------------------------------+

void
VidDlg::ExecFrame()
{
	if (Keyboard::KeyDown(VK_RETURN)) {
		OnApply(0);
	}
}

// +--------------------------------------------------------------------+

void
VidDlg::OnMode(AWEvent* event)
{
	selected_mode = mode->GetSelectedIndex();
}

void
VidDlg::OnTexSize(AWEvent* event)
{
	selected_tex_size = tex_size->GetSelectedIndex();
}

void
VidDlg::OnDetail(AWEvent* event)
{
	selected_detail = detail->GetSelectedIndex();
}

void
VidDlg::OnTexture(AWEvent* event)
{
	selected_texture = texture->GetSelectedIndex();
}

void
VidDlg::OnGamma(AWEvent* event)
{
	int g = gamma->GetValue();

	if (g >= 0 && g <= 255) {
		Game::SetGammaLevel(g);
	}
}

// +--------------------------------------------------------------------+

void VidDlg::OnAudio(AWEvent* event)      { manager->ShowAudDlg();  }
void VidDlg::OnVideo(AWEvent* event)      { manager->ShowVidDlg();  }
void VidDlg::OnOptions(AWEvent* event)    { manager->ShowOptDlg();  }
void VidDlg::OnControls(AWEvent* event)   { manager->ShowCtlDlg();  }
void VidDlg::OnMod(AWEvent* event)        { manager->ShowModDlg();  }

// +--------------------------------------------------------------------+

void
VidDlg::OnApply(AWEvent* event)
{
	manager->ApplyOptions();
}

void
VidDlg::OnCancel(AWEvent* event)
{
	manager->CancelOptions();
}

// +--------------------------------------------------------------------+

void
VidDlg::Apply()
{
	if (closed) return;

	int  w =   800;
	int  h =   600;
	int  d =    32;
	int  a =     1;
	int  g =   128;
	int  t =  2048;
	float bias = 0;

	const char* mode_desc = mode->GetSelectedItem();

	if (strstr(mode_desc, "800 x 600")) {
		w =  800;
		h =  600;
	}
	else if (strstr(mode_desc, "1024 x 768")) {
		w = 1024;
		h =  768;
	}
	else if (strstr(mode_desc, "1152 x 864")) {
		w = 1152;
		h =  864;
	}
	else if (strstr(mode_desc, "1280 x 800")) {
		w = 1280;
		h =  800;
	}
	else if (strstr(mode_desc, "1280 x 960")) {
		w = 1280;
		h =  960;
	}
	else if (strstr(mode_desc, "1280 x 1024")) {
		w = 1280;
		h = 1024;
	}
	else if (strstr(mode_desc, "1440 x 900")) {
		w = 1440;
		h = 900;
	}
	else if (strstr(mode_desc, "1600 x 900")) {
		w = 1600;
		h = 900;
	}
	else if (strstr(mode_desc, "1600 x 1200")) {
		w = 1600;
		h = 1200;
	}

	if (strstr(mode_desc, "x 16"))
	d = 16;
	else if (strstr(mode_desc, "x 32"))
	d = 32;

	if (selected_tex_size)
	t = (int) pow(2.0f, selected_tex_size + 6);

	bool video_change = false;

	Video* video = Game::GetVideo();
	if (video) {
		const VideoSettings* vs = video->GetVideoSettings();

		if (vs)
		bias = vs->depth_bias;

		if (video->IsFullScreen()) {
			if (video->Width()  != w)
			video_change = true;

			if (video->Height() != h)
			video_change = true;
			
			if (video->Depth()  != d)
			video_change = true;
		}
		else if (vs) {
			w = vs->fullscreen_mode.width;
			h = vs->fullscreen_mode.height;

			if (vs->fullscreen_mode.format == VideoMode::FMT_R5G6B5 ||
					vs->fullscreen_mode.format == VideoMode::FMT_R5G5B5)
			d = 16;
			else
			d = 32;
		}
		
		if (Game::MaxTexSize() != t)
		video_change = true;
	}

	FILE* f = 0;

	if (video_change)
	f = fopen("video2.cfg", "w");
	else
	f = fopen("video.cfg", "w");

	if (gamma) {
		g = gamma->GetValue();
	}

	if (f) {
		fprintf(f, "VIDEO\n\n");
		fprintf(f, "width:     %4d\n", w);
		fprintf(f, "height:    %4d\n", h);
		fprintf(f, "depth:     %4d\n", d);
		fprintf(f, "\n");
		fprintf(f, "max_tex:   %d\n",  (int) pow(2.0f, 6 + selected_tex_size));
		fprintf(f, "primary3D: %s\n",  (a>0)?"true":"false");
		fprintf(f, "gamma:     %4d\n", g);
		fprintf(f, "\n");
		fprintf(f, "terrain_detail_level:   %d\n", selected_detail + 2);
		fprintf(f, "terrain_texture_enable: %s\n", selected_texture ? "true" : "false");
		fprintf(f, "\n");
		fprintf(f, "shadows:   %s\n", shadows->GetSelectedIndex()    ? "true" : "false");
		fprintf(f, "spec_maps: %s\n", spec_maps->GetSelectedIndex()  ? "true" : "false");
		fprintf(f, "bump_maps: %s\n", bump_maps->GetSelectedIndex()  ? "true" : "false");
		fprintf(f, "bias:      %f\n", bias);
		fprintf(f, "\n");
		fprintf(f, "flare:     %s\n", lens_flare->GetSelectedIndex() ? "true" : "false");
		fprintf(f, "corona:    %s\n", corona->GetSelectedIndex()     ? "true" : "false");
		fprintf(f, "nebula:    %s\n", nebula->GetSelectedIndex()     ? "true" : "false");
		fprintf(f, "dust:      %d\n", dust->GetSelectedIndex());

		if (CameraDirector::GetRangeLimit() != 300e3)
		fprintf(f, "   cam_range_max: %f,\n", CameraDirector::GetRangeLimit());

		fclose(f);
	}

	Starshatter* stars = Starshatter::GetInstance();

	if (stars) {
		if (video_change)
		stars->RequestChangeVideo();
		else
		stars->LoadVideoConfig("video.cfg");
	}

	closed = true;
}

void
VidDlg::Cancel()
{
	Game::SetGammaLevel(orig_gamma);
	closed = true;
}

// +--------------------------------------------------------------------+

void
VidDlg::BuildModeList()
{
	char           mode_desc[32];
	Starshatter*   stars = Starshatter::GetInstance();

	mode->ClearItems();
	selected_mode = 0;

	if (Game::DisplayModeSupported( 800, 600, 16))  mode->AddItem("800 x 600 x 16");
	if (Game::DisplayModeSupported( 800, 600, 32))  mode->AddItem("800 x 600 x 32");

	if (Game::DisplayModeSupported(1024, 768, 16))  mode->AddItem("1024 x 768 x 16");
	if (Game::DisplayModeSupported(1024, 768, 32))  mode->AddItem("1024 x 768 x 32");

	if (Game::DisplayModeSupported(1152, 864, 16))  mode->AddItem("1152 x 864 x 16");
	if (Game::DisplayModeSupported(1152, 864, 32))  mode->AddItem("1152 x 864 x 32");

	if (Game::DisplayModeSupported(1280, 800, 16))  mode->AddItem("1280 x 800 x 16");
	if (Game::DisplayModeSupported(1280, 800, 32))  mode->AddItem("1280 x 800 x 32");

	if (Game::DisplayModeSupported(1280, 960, 16))  mode->AddItem("1280 x 960 x 16");
	if (Game::DisplayModeSupported(1280, 960, 32))  mode->AddItem("1280 x 960 x 32");

	if (Game::DisplayModeSupported(1280,1024, 16))  mode->AddItem("1280 x 1024 x 16");
	if (Game::DisplayModeSupported(1280,1024, 32))  mode->AddItem("1280 x 1024 x 32");

	if (Game::DisplayModeSupported(1440, 900, 16))  mode->AddItem("1440 x 900 x 16");
	if (Game::DisplayModeSupported(1440, 900, 32))  mode->AddItem("1440 x 900 x 32");

	if (Game::DisplayModeSupported(1600, 900, 16))  mode->AddItem("1600 x 900 x 16");
	if (Game::DisplayModeSupported(1600, 900, 32))  mode->AddItem("1600 x 900 x 32");

	if (Game::DisplayModeSupported(1600,1200, 16))  mode->AddItem("1600 x 1200 x 16");
	if (Game::DisplayModeSupported(1600,1200, 32))  mode->AddItem("1600 x 1200 x 32");

	if (Game::DisplayModeSupported(1680,1050, 16))  mode->AddItem("1680 x 1050 x 16");
	if (Game::DisplayModeSupported(1680,1050, 32))  mode->AddItem("1680 x 1050 x 32");

	Video* video = Game::GetVideo();

	if (stars && video) {
		switch (video->Width()) {
		case  800:  strcpy(mode_desc, "800 x 600 x ");     break;
		default:
		case 1024:  strcpy(mode_desc, "1024 x 768 x ");    break;
		case 1152:  strcpy(mode_desc, "1152 x 864 x ");    break;
		case 1280:
			if (video->Height() < 900)
			strcpy(mode_desc, "1280 x 800 x ");
			if (video->Height() < 1000)
			strcpy(mode_desc, "1280 x 960 x ");
			else
			strcpy(mode_desc, "1280 x 1024 x ");
			break;
		case 1440:  strcpy(mode_desc, "1440 x 900 x ");    break;
		case 1600:
			if (video->Height() < 1000)
			strcpy(mode_desc, "1600 x 900 x ");
			else
			strcpy(mode_desc, "1600 x 1200 x ");
			break;
		}

		switch (video->Depth()) {
		default:
		case 8:     strcat(mode_desc, "8");                break;
		case 16:    strcat(mode_desc, "16");               break;
		case 32:    strcat(mode_desc, "32");               break;
		}

		for (int i = 0; i < mode->GetCount(); i++) {
			if (!strcmp(mode->GetItem(i), mode_desc))
			selected_mode = i;
		}
	}
}
