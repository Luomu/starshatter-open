/*  Project Starshatter 5.0
	Destroyer Studios LLC
	Copyright (C) 1997-2007. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         MFD.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Class for all Multi Function Displays
*/

#include "MemDebug.h"
#include "MFD.h"
#include "HUDView.h"
#include "Ship.h"
#include "NavSystem.h"
#include "Power.h"
#include "Shield.h"
#include "Sensor.h"
#include "Contact.h"
#include "ShipDesign.h"
#include "Shot.h"
#include "Weapon.h"
#include "WeaponGroup.h"
#include "Sim.h"
#include "StarSystem.h"
#include "Starshatter.h"
#include "Drive.h"
#include "QuantumDrive.h"
#include "Power.h"
#include "Instruction.h"

#include "NetGame.h"

#include "CameraView.h"
#include "Color.h"
#include "Font.h"
#include "FontMgr.h"
#include "Window.h"
#include "Video.h"
#include "Screen.h"
#include "DataLoader.h"
#include "Scene.h"
#include "Graphic.h"
#include "Sprite.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "Game.h"

static Bitmap sensor_fov;
static Bitmap sensor_fwd;
static Bitmap sensor_hsd;
static Bitmap sensor_3d;

static BYTE*  sensor_fov_shade;
static BYTE*  sensor_fwd_shade;
static BYTE*  sensor_hsd_shade;
static BYTE*  sensor_3d_shade;

static Color  hud_color = Color::Black;
static Color  txt_color = Color::Black;

// +--------------------------------------------------------------------+

MFD::MFD(Window* c, int n)
: window(c), rect(0,0,0,0), index(n), mode(MFD_MODE_OFF), sprite(0),
ship(0), hidden(true), camview(0), lines(0), mouse_latch(0), mouse_in(false),
cockpit_hud_texture(0)
{
	sprite = new(__FILE__,__LINE__) Sprite(&sensor_fov);

	sprite->SetBlendMode(2);
	sprite->SetFilter(0);
	sprite->Hide();

	Font* font = FontMgr::Find("HUD");

	for (int i = 0; i < TXT_LAST; i++) {
		mfd_text[i].font     = font;
		mfd_text[i].color    = Color::White;
		mfd_text[i].hidden   = true;
	}
}

MFD::~MFD()
{ 
	GRAPHIC_DESTROY(sprite);
}

// +--------------------------------------------------------------------+

void
MFD::Initialize()
{
	static int initialized = 0;
	if (initialized) return;

	HUDView::PrepareBitmap("sensor_fov.pcx",  sensor_fov,    sensor_fov_shade);
	HUDView::PrepareBitmap("sensor_fwd.pcx",  sensor_fwd,    sensor_fwd_shade);
	HUDView::PrepareBitmap("sensor_hsd.pcx",  sensor_hsd,    sensor_hsd_shade);
	HUDView::PrepareBitmap("sensor_3d.pcx",   sensor_3d,     sensor_3d_shade);

	sensor_fov.SetType(Bitmap::BMP_TRANSLUCENT);
	sensor_fwd.SetType(Bitmap::BMP_TRANSLUCENT);
	sensor_hsd.SetType(Bitmap::BMP_TRANSLUCENT);
	sensor_3d.SetType(Bitmap::BMP_TRANSLUCENT);

	initialized = 1;
}

void
MFD::Close()
{
	sensor_fov.ClearImage();
	sensor_fwd.ClearImage();
	sensor_hsd.ClearImage();
	sensor_3d.ClearImage();

	delete [] sensor_fov_shade;
	delete [] sensor_fwd_shade;
	delete [] sensor_hsd_shade;
	delete [] sensor_3d_shade;
}

// +--------------------------------------------------------------------+

void
MFD::UseCameraView(CameraView* v)
{
	if (v && !camview) {
		camview = v;
	}
}

void
MFD::SetColor(Color c)
{
	HUDView* hud = HUDView::GetInstance();

	if (hud) {
		hud_color = hud->GetHUDColor();
		txt_color = hud->GetTextColor();
	}
	else {
		hud_color = c;
		txt_color = c;
	}

	HUDView::ColorizeBitmap(sensor_fov, sensor_fov_shade, c);
	HUDView::ColorizeBitmap(sensor_fwd, sensor_fwd_shade, c);
	HUDView::ColorizeBitmap(sensor_hsd, sensor_hsd_shade, c);
	HUDView::ColorizeBitmap(sensor_3d,  sensor_3d_shade,  c);
}

void
MFD::SetText3DColor(Color c)
{
	for (int i = 0; i < TXT_LAST; i++)
	mfd_text[i].color = c;
}

// +--------------------------------------------------------------------+

void
MFD::Show()
{
	switch (mode) {
	case MFD_MODE_FOV:
	case MFD_MODE_HSD:
	case MFD_MODE_3D:
		if (sprite)
		sprite->Show();
		break;
	}

	hidden = false;
}

void
MFD::Hide()
{
	if (sprite)
	sprite->Hide();

	for (int i = 0; i < TXT_LAST; i++)
	HideMFDText(i);

	hidden = true;
}

// +--------------------------------------------------------------------+

void
MFD::SetRect(const Rect& r)
{
	rect        = r;

	if (sprite)
	sprite->MoveTo(Point(rect.x + sprite->Width()/2, 
	rect.y + sprite->Height()/2, 
	1));
}

// +--------------------------------------------------------------------+

void
MFD::SetMode(int m)
{
	if (m < MFD_MODE_OFF || m > MFD_MODE_3D)
	mode = MFD_MODE_OFF;
	else
	mode = m;

	sprite->Hide();

	for (int i = 0; i < TXT_LAST; i++)
	HideMFDText(i);

	switch (mode) {
	case MFD_MODE_GAME:
	case MFD_MODE_SHIP:
		lines = 0;
		break;

	case MFD_MODE_FOV:
		sprite->SetAnimation(&sensor_fov);
		sprite->Show();
		sprite->Reshape(sensor_fov.Width()-8, 16);
		break;
	case MFD_MODE_HSD:
		sprite->SetAnimation(&sensor_hsd);
		sprite->Show();
		sprite->Reshape(sensor_hsd.Width()-8, 16);
		break;
	case MFD_MODE_3D:
		sprite->SetAnimation(&sensor_3d);
		sprite->Show();
		sprite->Reshape(sensor_3d.Width()-8, 16);
		break;
	}
}

// +--------------------------------------------------------------------+

void
MFD::Draw()
{
	mouse_in = false;

	if (Mouse::LButton() == 0)
	mouse_latch = 0;

	if (rect.Contains(Mouse::X(), Mouse::Y()))
	mouse_in = true;

	// click to turn on MFD when off:
	if (mode < MFD_MODE_FOV && Mouse::LButton() && !mouse_latch) {
		mouse_latch = 1;
		if (mouse_in) {
			HUDView* hud = HUDView::GetInstance();
			if (hud)
			hud->CycleMFDMode(index);
		}
	}

	for (int i = 0; i < TXT_LAST; i++)
	HideMFDText(i);

	if (hidden || mode < MFD_MODE_FOV) {
		if (cockpit_hud_texture) {
			int x1 = index*128;
			int y1 = 256;
			int x2 = x1 + 128;
			int y2 = y1 + 128;

			cockpit_hud_texture->FillRect(x1, y1, x2, y2, Color::Black);
		}

		if (hidden)
		return;
	}

	if (sprite && !sprite->Hidden()) {
		if (cockpit_hud_texture) {
			int x1 = index*128;
			int y1 = 256;
			int w  = sprite->Width();
			int h  = sprite->Height();

			cockpit_hud_texture->BitBlt(x1, y1, *sprite->Frame(), 0,0,w,h);
		}
		else {
			int cx = rect.x + rect.w/2;
			int cy = rect.y + rect.h/2;
			int w2 = sprite->Width()/2;
			int h2 = sprite->Height()/2;

			window->DrawBitmap(cx-w2, cy-h2, cx+w2, cy+h2, sprite->Frame(), Video::BLEND_ALPHA);
		}
	}

	switch (mode) {
	default:
	case MFD_MODE_OFF:                        break;
	case MFD_MODE_GAME:     DrawGameMFD();    break;
	case MFD_MODE_SHIP:     DrawStatusMFD();  break;

		// sensor sub-modes:
	case MFD_MODE_FOV:      DrawSensorMFD();  break;
	case MFD_MODE_HSD:      DrawHSD();        break;
	case MFD_MODE_3D:       Draw3D();         break;
	}
}

// +--------------------------------------------------------------------+

void
MFD::DrawSensorLabels(const char* mfd_mode)
{
	Sensor*  sensor      = ship->GetSensor();
	char     mode_buf[8] = "       ";
	int      scan_r      = rect.w;
	int      scan_x      = rect.x;
	int      scan_y      = rect.y;

	switch (sensor->GetMode()) {
	case Sensor::PAS:    strcpy_s(mode_buf, Game::GetText("MFD.mode.passive").data()); break;
	case Sensor::STD:    strcpy_s(mode_buf, Game::GetText("MFD.mode.standard").data()); break;
	case Sensor::ACM:    strcpy_s(mode_buf, Game::GetText("MFD.mode.auto-combat").data()); break;
	case Sensor::GM:     strcpy_s(mode_buf, Game::GetText("MFD.mode.ground").data()); break;
	case Sensor::PST:    strcpy_s(mode_buf, Game::GetText("MFD.mode.passive").data()); break;
	case Sensor::CST:    strcpy_s(mode_buf, Game::GetText("MFD.mode.combined").data()); break;
	default:             break;
	}

	Rect mode_rect(scan_x+2, scan_y+2, 40, 12);
	DrawMFDText(0, mode_buf, mode_rect, DT_LEFT);

	char range_txt[12];
	double beam_range = sensor->GetBeamRange() + 1;
	if (beam_range >= 1e6)
	sprintf_s(range_txt, "-%dM+", (int) (beam_range / 1e6));
	else
	sprintf_s(range_txt, "-%3d+", (int) (beam_range / 1e3));

	Rect range_rect(scan_x+2, scan_y+scan_r-12, 40, 12);
	DrawMFDText(1, range_txt, range_rect, DT_LEFT);

	Rect disp_rect(scan_x+scan_r-41, scan_y+2, 40, 12);
	DrawMFDText(2, mfd_mode, disp_rect, DT_RIGHT);

	Rect probe_rect(scan_x+scan_r-41, scan_y+scan_r-12, 40, 12);

	if (ship->GetProbeLauncher()) {
		char probes[32];
		sprintf_s(probes, "%s %02d", Game::GetText("MFD.probe").data(), ship->GetProbeLauncher()->Ammo());
		DrawMFDText(3, probes, probe_rect, DT_RIGHT);
	}
	else {
		HideMFDText(3);
	}

	if (Mouse::LButton() && !mouse_latch) {
		mouse_latch = 1;

		if (mode_rect.Contains(Mouse::X(), Mouse::Y())) {
			if (sensor->GetMode() < Sensor::PST) {
				int sensor_mode = sensor->GetMode() + 1;
				if (sensor_mode > Sensor::GM)
				sensor_mode = Sensor::PAS;

				sensor->SetMode((Sensor::Mode) sensor_mode);
			}
		}

		else if (range_rect.Contains(Mouse::X(), Mouse::Y())) {
			if (Mouse::X() > range_rect.x+range_rect.w/2)
			sensor->IncreaseRange();
			else
			sensor->DecreaseRange();
		}

		else if (disp_rect.Contains(Mouse::X(), Mouse::Y())) {
			HUDView* hud = HUDView::GetInstance();
			if (hud)
			hud->CycleMFDMode(index);
		}

		else if (probe_rect.Contains(Mouse::X(), Mouse::Y())) {
			ship->LaunchProbe();
		}
	}
}

// +--------------------------------------------------------------------+

// AZIMUTH-ELEVATION ANGULAR SCANNER

void
MFD::DrawSensorMFD()
{
	int scan_r = rect.w;
	int scan_x = cockpit_hud_texture ? (index*128) : rect.x;
	int scan_y = cockpit_hud_texture ? 256         : rect.y;
	int r      = scan_r / 2;

	double xctr = (scan_r / 2.0) - 0.5;
	double yctr = (scan_r / 2.0) + 0.5;

	Sensor* sensor = ship->GetSensor();
	if (!sensor) {
		DrawMFDText(0, Game::GetText("MFD.inactive").data(), rect, DT_CENTER);
		return;
	}

	int w = sprite->Width();
	int h = sprite->Height();

	if (w < sprite->Frame()->Width())
	w += 2;

	if (h < sprite->Frame()->Height())
	h += 16;

	sprite->Reshape(w, h);
	sprite->Show();

	if (h < sprite->Frame()->Height())
	return;

	double sweep_scale = r / (PI/2);

	if (sensor->GetBeamLimit() > 90*DEGREES)
	sweep_scale = (double) r / (90*DEGREES);

	int az = (int) (sensor->GetBeamLimit() * sweep_scale);
	int el =       az;
	int xc = (int) (scan_x + xctr);
	int yc = (int) (scan_y + yctr);

	if (mode == MFD_MODE_FOV) {
		if (sensor->GetMode() < Sensor::GM) {
			if (cockpit_hud_texture)
			cockpit_hud_texture->DrawEllipse(xc-az, yc-el, xc+az, yc+el, hud_color);
			else
			window->DrawEllipse(xc-az, yc-el, xc+az, yc+el, hud_color);
		}
	}
	else {
		char az_txt[8];
		sprintf_s(az_txt, "%d", (int) (sensor->GetBeamLimit() / DEGREES));

		Rect az_rect(scan_x+2, scan_y+scan_r-12, 32, 12);
		DrawMFDText(1, az_txt, az_rect, DT_LEFT);

		az_rect.x = scan_x + (scan_r/2) - (az_rect.w/2);
		DrawMFDText(2, "0", az_rect, DT_CENTER);

		az_rect.x = scan_x + scan_r - az_rect.w - 2;
		DrawMFDText(3, az_txt, az_rect, DT_RIGHT);
	}

	// draw next nav point:
	Instruction* navpt = ship->GetNextNavPoint();
	if (navpt && navpt->Region() == ship->GetRegion()) {
		const Camera* cam = &ship->Cam();

		// translate:
		Point    pt  = navpt->Location().OtherHand() - ship->Location();

		// rotate:
		double   tx  = pt * cam->vrt();
		double   ty  = pt * cam->vup();
		double   tz  = pt * cam->vpn();

		if (tz > 1.0) {
			// convert to spherical coords:
			double   rng = pt.length();
			double   az  = asin(fabs(tx) / rng);
			double   el  = asin(fabs(ty) / rng);

			if (tx < 0) az = -az;
			if (ty < 0) el = -el;

			if (fabs(az) < 90*DEGREES) {
				az *= sweep_scale;
				el *= sweep_scale;

				int x = (int) (r + az);
				int y = (int) (r - el);

				// clip again:
				if (x > 0 && x < scan_r &&
						y > 0 && y < scan_r) {

					// draw:
					int xc   = scan_x + x;
					int yc   = scan_y + y;

					if (cockpit_hud_texture) {
						cockpit_hud_texture->DrawLine(xc-2, yc-2, xc+2, yc+2, Color::White);
						cockpit_hud_texture->DrawLine(xc-2, yc+2, xc+2, yc-2, Color::White);
					}
					else {
						window->DrawLine(xc-2, yc-2, xc+2, yc+2, Color::White);
						window->DrawLine(xc-2, yc+2, xc+2, yc-2, Color::White);
					}
				}
			}
		}
	}

	int num_contacts = ship->NumContacts();
	ListIter<Contact> iter = ship->ContactList();

	while (++iter) {
		Contact* contact = iter.value();
		Ship*    c_ship  = contact->GetShip();
		double   az, el, rng;
		bool     aft = false;

		if (c_ship == ship) continue;

		contact->GetBearing(ship, az, el, rng);

		// clip (is in-front):
		if (fabs(az) < 90*DEGREES) {
			az *= sweep_scale;
			el *= sweep_scale;
		}

		// rear anulus:
		else {
			double len = sqrt(az*az + el*el);

			if (len > 1e-6) {
				az = r * az/len;
				el = r * el/len;
			}
			else {
				az = -r;
				el = 0;
			}

			aft = true;
		}

		int x = (int) (r + az);
		int y = (int) (r - el);

		// clip again:
		if (x < 0 || x > scan_r) continue;
		if (y < 0 || y > scan_r) continue;

		// draw:
		Color mark = HUDView::MarkerColor(contact);

		if (aft)
		mark = mark * 0.75;

		int xc   = scan_x + x;
		int yc   = scan_y + y;
		int size = 1;

		if (c_ship && c_ship == ship->GetTarget())
		size = 2;

		if (cockpit_hud_texture)
		cockpit_hud_texture->FillRect(xc-size, yc-size, xc+size, yc+size, mark);
		else
		window->FillRect(xc-size, yc-size, xc+size, yc+size, mark);

		if (contact->Threat(ship)) {
			if (c_ship) {
				if (cockpit_hud_texture)
				cockpit_hud_texture->DrawEllipse(xc-4, yc-4, xc+3, yc+3, mark);
				else
				window->DrawEllipse(xc-4, yc-4, xc+3, yc+3, mark);
			}
			else {
				if (cockpit_hud_texture) {
					cockpit_hud_texture->DrawLine(xc, yc-5, xc+5, yc, mark);
					cockpit_hud_texture->DrawLine(xc+5, yc, xc, yc+5, mark);
					cockpit_hud_texture->DrawLine(xc, yc+5, xc-5, yc, mark);
					cockpit_hud_texture->DrawLine(xc-5, yc, xc, yc-5, mark);
				}
				else {
					window->DrawLine(xc, yc-5, xc+5, yc, mark);
					window->DrawLine(xc+5, yc, xc, yc+5, mark);
					window->DrawLine(xc, yc+5, xc-5, yc, mark);
					window->DrawLine(xc-5, yc, xc, yc-5, mark);
				}
			}
		}
	}

	DrawSensorLabels(Game::GetText("MFD.mode.field-of-view").data());
}

// +--------------------------------------------------------------------+

// HORIZONTAL SITUATION DISPLAY

void
MFD::DrawHSD()
{
	int scan_r = rect.w;
	int scan_x = cockpit_hud_texture ? (index*128) : rect.x;
	int scan_y = cockpit_hud_texture ? 256         : rect.y;
	int r      = scan_r / 2 - 4;

	double xctr = (scan_r / 2.0) - 0.5;
	double yctr = (scan_r / 2.0) + 0.5;

	int xc = (int) xctr + scan_x;
	int yc = (int) yctr + scan_y;

	Sensor* sensor = ship->GetSensor();
	if (!sensor) {
		DrawMFDText(0, Game::GetText("MFD.inactive").data(), rect, DT_CENTER);
		return;
	}

	int w = sprite->Width();
	int h = sprite->Height();

	if (w < sprite->Frame()->Width())
	w += 2;

	if (h < sprite->Frame()->Height())
	h += 16;

	sprite->Reshape(w, h);
	sprite->Show();

	if (h < sprite->Frame()->Height())
	return;

	if (sensor->GetMode() < Sensor::PST) {
		double s = sin(sensor->GetBeamLimit());
		double c = cos(sensor->GetBeamLimit());

		int x0 = (int) (0.1*r*s);
		int y0 = (int) (0.1*r*c);
		int x1 = (int) (1.0*r*s);
		int y1 = (int) (1.0*r*c);

		if (cockpit_hud_texture) {
			cockpit_hud_texture->DrawLine(xc-x0, yc-y0, xc-x1, yc-y1, hud_color);
			cockpit_hud_texture->DrawLine(xc+x0, yc-y0, xc+x1, yc-y1, hud_color);
		}
		else {
			window->DrawLine(xc-x0, yc-y0, xc-x1, yc-y1, hud_color);
			window->DrawLine(xc+x0, yc-y0, xc+x1, yc-y1, hud_color);
		}
	}

	double rscale = (double) r/(sensor->GetBeamRange());

	Camera hsd_cam = ship->Cam();
	Point  look    = ship->Location() + ship->Heading() * 1000;
	look.y  = ship->Location().y;

	hsd_cam.LookAt(look);

	// draw tick marks on range rings:
	for (int dir = 0; dir < 4; dir++) {
		Point tick;

		switch (dir) {
		case 0:  tick = Point(    0, 0,  1000); break;
		case 1:  tick = Point( 1000, 0,     0); break;
		case 2:  tick = Point(    0, 0, -1000); break;
		case 3:  tick = Point(-1000, 0,     0); break;
		}

		double tx = tick * hsd_cam.vrt();
		double tz = tick * hsd_cam.vpn();
		double az = asin(fabs(tx) / 1000);

		if (tx < 0) az = -az;

		if (tz < 0)
		if (az < 0) az = -PI - az;
		else        az =  PI - az;

		for (double range = 0.3; range < 1; range += 0.3) {
			int x0 = (int) (sin(az) * r * range);
			int y0 = (int) (cos(az) * r * range);
			int x1 = (int) (sin(az) * r * (range + 0.1));
			int y1 = (int) (cos(az) * r * (range + 0.1));

			if (cockpit_hud_texture) {
				cockpit_hud_texture->DrawLine(xc+x0, yc-y0, xc+x1, yc-y1, hud_color);
			}
			else {
				window->DrawLine(xc+x0, yc-y0, xc+x1, yc-y1, hud_color);
			}
		}
	}

	// draw next nav point:
	Instruction* navpt = ship->GetNextNavPoint();
	if (navpt && navpt->Region() == ship->GetRegion()) {
		const Camera* cam = &hsd_cam;

		// translate:
		Point    pt  = navpt->Location().OtherHand() - ship->Location();

		// rotate:
		double   tx  = pt * cam->vrt();
		double   ty  = pt * cam->vup();
		double   tz  = pt * cam->vpn();

		// convert to spherical coords:
		double   rng = pt.length();
		double   az  = asin(fabs(tx) / rng);

		if (rng > sensor->GetBeamRange())
		rng = sensor->GetBeamRange();

		if (tx < 0)
		az = -az;

		if (tz < 0)
		if (az < 0)
		az = -PI - az;
		else
		az =  PI - az;

		// draw:
		int x    = (int) (xc + sin(az) * rng * rscale);
		int y    = (int) (yc - cos(az) * rng * rscale);

		if (cockpit_hud_texture) {
			cockpit_hud_texture->DrawLine(x-2, y-2, x+2, y+2, Color::White);
			cockpit_hud_texture->DrawLine(x-2, y+2, x+2, y-2, Color::White);
		}
		else {
			window->DrawLine(x-2, y-2, x+2, y+2, Color::White);
			window->DrawLine(x-2, y+2, x+2, y-2, Color::White);
		}
	}

	// draw contact markers:
	double limit = sensor->GetBeamRange();
	ListIter<Contact> contact = ship->ContactList();

	while (++contact) {
		Ship* c_ship  = contact->GetShip();
		if (c_ship == ship) continue;

		// translate:
		Point targ_pt = contact->Location() - hsd_cam.Pos();

		// rotate:
		double tx = targ_pt * hsd_cam.vrt();
		double rg = contact->Range(ship, limit);
		double true_range = targ_pt.length();
		double az = asin(fabs(tx) / true_range);

		// clip:
		if (rg > limit || rg <= 0)
		continue;

		if (tx < 0)
		az = -az;

		if (!contact->InFront(ship))
		if (az < 0)
		az = -PI - az;
		else
		az =  PI - az;

		// draw:
		int x    = (int) (xc + sin(az) * rg * rscale);
		int y    = (int) (yc - cos(az) * rg * rscale);
		int size = 2;

		// clip again:
		if (x < scan_x || y < scan_y)
		continue;

		if (c_ship && c_ship == ship->GetTarget())
		size = 3;

		Color mark = HUDView::MarkerColor(contact.value());
		if (cockpit_hud_texture) {
			cockpit_hud_texture->FillRect(x-size, y-size, x+size, y+size, mark);
		}
		else {
			window->FillRect(x-size, y-size, x+size, y+size, mark);
		}

		if (contact->Threat(ship)) {
			if (c_ship) {
				if (cockpit_hud_texture) {
					cockpit_hud_texture->DrawEllipse(x-4, y-4, x+3, y+3, mark);
				}
				else {
					window->DrawEllipse(x-4, y-4, x+3, y+3, mark);
				}
			}
			else {
				if (cockpit_hud_texture) {
					cockpit_hud_texture->DrawLine(x, y-5, x+5, y, mark);
					cockpit_hud_texture->DrawLine(x+5, y, x, y+5, mark);
					cockpit_hud_texture->DrawLine(x, y+5, x-5, y, mark);
					cockpit_hud_texture->DrawLine(x-5, y, x, y-5, mark);
				}
				else {
					window->DrawLine(x, y-5, x+5, y, mark);
					window->DrawLine(x+5, y, x, y+5, mark);
					window->DrawLine(x, y+5, x-5, y, mark);
					window->DrawLine(x-5, y, x, y-5, mark);
				}
			}
		}
	}

	DrawSensorLabels(Game::GetText("MFD.mode.horizontal").data());
}

// +--------------------------------------------------------------------+

// ELITE-STYLE 3D RADAR

void
MFD::Draw3D()
{
	int scan_r = rect.w;
	int scan_x = cockpit_hud_texture ? (index*128) : rect.x;
	int scan_y = cockpit_hud_texture ? 256         : rect.y;
	int r      = scan_r / 2 - 4;

	double xctr = (scan_r / 2.0) - 0.5;
	double yctr = (scan_r / 2.0) + 0.5;

	int xc = (int) xctr + scan_x;
	int yc = (int) yctr + scan_y;

	Sensor* sensor = ship->GetSensor();
	if (!sensor) {
		DrawMFDText(0, Game::GetText("MFD.inactive").data(), rect, DT_CENTER);
		return;
	}

	int w = sprite->Width();
	int h = sprite->Height();

	if (w < sprite->Frame()->Width())
	w += 2;

	if (h < sprite->Frame()->Height())
	h += 16;

	sprite->Reshape(w, h);
	sprite->Show();

	if (h < sprite->Frame()->Height())
	return;

	double rscale = (double) r/(sensor->GetBeamRange());

	Camera hsd_cam = ship->Cam();

	if (ship->IsStarship()) {
		Point  look    = ship->Location() + ship->Heading() * 1000;
		look.y  = ship->Location().y;

		hsd_cam.LookAt(look);
	}


	// draw next nav point:
	Instruction* navpt = ship->GetNextNavPoint();
	if (navpt && navpt->Region() == ship->GetRegion()) {
		const Camera* cam = &hsd_cam;

		// translate:
		Point    pt  = navpt->Location().OtherHand() - ship->Location();

		// rotate:
		double   tx  = pt * cam->vrt();
		double   ty  = pt * cam->vup();
		double   tz  = pt * cam->vpn();

		// convert to cylindrical coords:
		double   rng = pt.length();
		double   az  = asin(fabs(tx) / rng);

		if (rng > sensor->GetBeamRange())
		rng = sensor->GetBeamRange();

		if (tx < 0)
		az = -az;

		if (tz < 0) {
			if (az < 0)
			az = -PI - az;
			else
			az =  PI - az;
		}

		// accentuate vertical:
		if (ty > 10)
		ty = log10(ty-9) * r/8;

		else if (ty < -10)
		ty = -log10(9-ty) * r/8;

		else
		ty = 0;

		// draw:
		int x  = (int) (sin(az) * rng * rscale);
		int y  = (int) (cos(az) * rng * rscale/2);
		int z  = (int) (ty);

		int x0 = xc+x;
		int y0 = yc-y-z;

		if (cockpit_hud_texture) {
			cockpit_hud_texture->DrawLine(x0-2, y0-2, x0+2, y0+2, Color::White);
			cockpit_hud_texture->DrawLine(x0-2, y0+2, x0+2, y0-2, Color::White);
		}
		else {
			window->DrawLine(x0-2, y0-2, x0+2, y0+2, Color::White);
			window->DrawLine(x0-2, y0+2, x0+2, y0-2, Color::White);
		}

		if (cockpit_hud_texture) {
			if (z > 0)
			cockpit_hud_texture->DrawLine(x0, y0+1, x0, y0+z, Color::White);
			else if (z < 0)
			cockpit_hud_texture->DrawLine(x0, y0+z, x0, y0-1, Color::White);
		}
		else {
			if (z > 0)
			window->DrawLine(x0, y0+1, x0, y0+z, Color::White);
			else if (z < 0)
			window->DrawLine(x0, y0+z, x0, y0-1, Color::White);
		}
	}


	// draw contact markers:
	double limit = sensor->GetBeamRange();
	ListIter<Contact> contact = ship->ContactList();

	while (++contact) {
		Ship* c_ship  = contact->GetShip();
		if (c_ship == ship) continue;

		// translate:
		Point targ_pt = contact->Location() - hsd_cam.Pos();

		// rotate:
		double tx = targ_pt * hsd_cam.vrt();
		double ty = targ_pt * hsd_cam.vup();
		double rg = contact->Range(ship, limit);
		double true_range = targ_pt.length();
		double az = asin(fabs(tx) / true_range);

		// clip:
		if (rg > limit || rg <= 0)
		continue;

		if (tx < 0)
		az = -az;

		if (!contact->InFront(ship))
		if (az < 0)
		az = -PI - az;
		else
		az =  PI - az;

		// accentuate vertical:
		ty *= 4;

		// draw:
		int x = (int) (sin(az) * rg * rscale);
		int y = (int) (cos(az) * rg * rscale/2);
		int z = (int) (ty           * rscale/2);
		int size = 1;

		int x0 = xc+x;
		int y0 = yc-y-z;

		if (c_ship && c_ship == ship->GetTarget())
		size = 2;

		Color mark = HUDView::MarkerColor(contact.value());

		if (cockpit_hud_texture) {
			cockpit_hud_texture->FillRect(x0-size, y0-size, x0+size, y0+size, mark);

			if (contact->Threat(ship)) {
				if (c_ship) {
					cockpit_hud_texture->DrawEllipse(x0-4, y0-4, x0+3, y0+3, mark);
				}
				else {
					cockpit_hud_texture->DrawLine(x0, y0-5, x0+5, y0, mark);
					cockpit_hud_texture->DrawLine(x0+5, y0, x0, y0+5, mark);
					cockpit_hud_texture->DrawLine(x0, y0+5, x0-5, y0, mark);
					cockpit_hud_texture->DrawLine(x0-5, y0, x0, y0-5, mark);
				}
			}

			if (z > 0)
			cockpit_hud_texture->FillRect(x0-1, y0+size, x0, y0+z,    mark);
			else if (z < 0)
			cockpit_hud_texture->FillRect(x0-1, y0+z,    x0, y0-size, mark);
		}
		else {
			window->FillRect(x0-size, y0-size, x0+size, y0+size, mark);

			if (contact->Threat(ship)) {
				if (c_ship) {
					window->DrawEllipse(x0-4, y0-4, x0+3, y0+3, mark);
				}
				else {
					window->DrawLine(x0, y0-5, x0+5, y0, mark);
					window->DrawLine(x0+5, y0, x0, y0+5, mark);
					window->DrawLine(x0, y0+5, x0-5, y0, mark);
					window->DrawLine(x0-5, y0, x0, y0-5, mark);
				}
			}

			if (z > 0)
			window->FillRect(x0-1, y0+size, x0, y0+z,    mark);
			else if (z < 0)
			window->FillRect(x0-1, y0+z,    x0, y0-size, mark);
		}
	}

	DrawSensorLabels(Game::GetText("MFD.mode.3D").data());
}

// +--------------------------------------------------------------------+

// GROUND MAP

void
MFD::DrawMap()
{
	DrawMFDText(0, Game::GetText("MFD.mode.ground").data(), Rect(rect.x, rect.y, rect.w, 12), DT_CENTER);
}

// +--------------------------------------------------------------------+

void
MFD::DrawGauge(int x, int y, int percent)
{
	if (cockpit_hud_texture) {
		x += this->index * 128 - this->rect.x;
		y += 256               - this->rect.y;
		cockpit_hud_texture->DrawRect(x, y, x+53, y+8, Color::DarkGray);
	}
	else {
		window->DrawRect(x, y, x+53, y+8, Color::DarkGray);
	}

	if (percent <   3) return;
	if (percent > 100) percent = 100;

	percent /= 2;

	if (cockpit_hud_texture)
	cockpit_hud_texture->FillRect(x+2, y+2, x+2+percent, y+7, Color::Gray);
	else
	window->FillRect(x+2, y+2, x+2+percent, y+7, Color::Gray);
}

void
MFD::DrawGameMFD()
{
	if (lines < 10) lines++;

	char txt[64];
	Rect txt_rect(rect.x, rect.y, rect.w, 12);

	int t = 0;

	if (!HUDView::IsArcade() && HUDView::ShowFPS()) {
		sprintf_s(txt, "FPS: %6.2f", Game::FrameRate());
		DrawMFDText(t++, txt, txt_rect, DT_LEFT); 
		txt_rect.y += 10;

		if (lines <= 1) return;

		Starshatter* game = Starshatter::GetInstance();
		sprintf_s(txt, "Polys: %d", game->GetPolyStats().npolys);
		DrawMFDText(t++, txt, txt_rect, DT_LEFT); 
		txt_rect.y += 10;
	}

	if (ship) {
		DrawMFDText(t++, ship->Name(), txt_rect, DT_LEFT);
		txt_rect.y += 10;
	}

	if (lines <= 2) return;

	int hours   = (Game::GameTime() / 3600000)     ;
	int minutes = (Game::GameTime() /   60000) % 60;
	int seconds = (Game::GameTime() /    1000) % 60;

	if (ship) {
		DWORD clock = ship->MissionClock();
		
		hours    = (clock / 3600000)     ;
		minutes  = (clock /   60000) % 60;
		seconds  = (clock /    1000) % 60;
	}

	if (Game::TimeCompression() > 1)
	sprintf_s(txt, "%02d:%02d:%02d x%d", hours, minutes, seconds, Game::TimeCompression());
	else
	sprintf_s(txt, "%02d:%02d:%02d", hours, minutes, seconds);

	DrawMFDText(t++, txt, txt_rect, DT_LEFT);
	txt_rect.y += 10;

	if (HUDView::IsArcade() || lines <= 3) return;

	DrawMFDText(t++, ship->GetRegion()->Name(), txt_rect, DT_LEFT);
	txt_rect.y += 10;

	if (lines <= 4) return;

	if (ship) {
		switch (ship->GetFlightPhase()) {
		case Ship::DOCKED:   DrawMFDText(t++, Game::GetText("MFD.phase.DOCKED").data(),   txt_rect, DT_LEFT); break;
		case Ship::ALERT:    DrawMFDText(t++, Game::GetText("MFD.phase.ALERT").data(),    txt_rect, DT_LEFT); break;
		case Ship::LOCKED:   DrawMFDText(t++, Game::GetText("MFD.phase.LOCKED").data(),   txt_rect, DT_LEFT); break;
		case Ship::LAUNCH:   DrawMFDText(t++, Game::GetText("MFD.phase.LAUNCH").data(),   txt_rect, DT_LEFT); break;
		case Ship::TAKEOFF:  DrawMFDText(t++, Game::GetText("MFD.phase.TAKEOFF").data(),  txt_rect, DT_LEFT); break;
		case Ship::ACTIVE:   DrawMFDText(t++, Game::GetText("MFD.phase.ACTIVE").data(),   txt_rect, DT_LEFT); break;
		case Ship::APPROACH: DrawMFDText(t++, Game::GetText("MFD.phase.APPROACH").data(), txt_rect, DT_LEFT); break;
		case Ship::RECOVERY: DrawMFDText(t++, Game::GetText("MFD.phase.RECOVERY").data(), txt_rect, DT_LEFT); break;
		case Ship::DOCKING:  DrawMFDText(t++, Game::GetText("MFD.phase.DOCKING").data(),  txt_rect, DT_LEFT); break;
		}
	}
}

void
MFD::DrawStatusMFD()
{
	if (lines < 10) lines++;

	Rect  status_rect(rect.x, rect.y, rect.w, 12);
	int   row = 0;
	char  txt[32];

    if (ship) {
        if (status_rect.y > 320 && !ship->IsStarship())
            status_rect.y += 32;

		Drive* drive = ship->GetDrive();
		if (drive) {
			DrawMFDText(row++, Game::GetText("MFD.status.THRUST").data(), status_rect, DT_LEFT);
			DrawGauge(status_rect.x+70, status_rect.y, (int) ship->Throttle());
			status_rect.y += 10;
		}

		if (lines <= 1) return;

		if (ship->Reactors().size() > 0) {
			PowerSource* reactor = ship->Reactors()[0];
			if (reactor) {
				DrawMFDText(row++, Game::GetText("MFD.status.FUEL").data(), status_rect, DT_LEFT);
				DrawGauge(status_rect.x+70, status_rect.y, reactor->Charge());
				status_rect.y += 10;
			}
		}

		if (lines <= 2) return;

		QuantumDrive* quantum_drive = ship->GetQuantumDrive();
		if (quantum_drive) {
			DrawMFDText(row++, Game::GetText("MFD.status.QUANTUM").data(), status_rect, DT_LEFT);
			DrawGauge(status_rect.x+70, status_rect.y, (int) quantum_drive->Charge());
			status_rect.y += 10;
		}

		if (lines <= 3) return;

		double hull = ship->Integrity() / ship->Design()->integrity * 100;
		int    hull_status = System::CRITICAL;

		if (hull > 66)
		hull_status = System::NOMINAL;
		else if (hull > 33)
		hull_status = System::DEGRADED;

		DrawMFDText(row++, Game::GetText("MFD.status.HULL").data(), status_rect, DT_LEFT);
		DrawGauge(status_rect.x+70, status_rect.y, (int) hull);
		status_rect.y += 10;

		if (lines <= 4) return;

		Shield* shield = ship->GetShield();
		if (shield) {
			DrawMFDText(row++, Game::GetText("MFD.status.SHIELD").data(), status_rect, DT_LEFT);
			DrawGauge(status_rect.x+70, status_rect.y, ship->ShieldStrength());
			status_rect.y += 10;
		}

		if (lines <= 5) return;

		Weapon* primary = ship->GetPrimary();
		if (primary) {
			DrawMFDText(row++, Game::GetText("MFD.status.GUNS").data(), status_rect, DT_LEFT);
			DrawGauge(status_rect.x+70, status_rect.y, primary->Charge());
			status_rect.y += 10;
		}

		if (lines <= 6) return;

		if (HUDView::IsArcade()) {
			for (int i = 0; i < ship->Weapons().size() && i < 4; i++) {
				WeaponGroup* w = ship->Weapons().at(i);

				if (w->IsMissile()) {
					char ammo[8];

					if (ship->GetSecondaryGroup() == w)
					sprintf_s(ammo, "%d *", w->Ammo());
					else
					sprintf_s(ammo, "%d", w->Ammo());

					DrawMFDText(row++, (const char*) w->GetDesign()->name, status_rect, DT_LEFT);
					status_rect.x += 70;
					DrawMFDText(row++, ammo, status_rect, DT_LEFT);
					status_rect.x -= 70;
					status_rect.y += 10;
				}
			}

			if (ship->GetDecoy()) {
				char ammo[8];
				sprintf_s(ammo, "%d", ship->GetDecoy()->Ammo());
				DrawMFDText(row++, Game::GetText("MFD.status.DECOY").data(), status_rect, DT_LEFT);
				status_rect.x += 70;
				DrawMFDText(row++, ammo, status_rect, DT_LEFT);
				status_rect.x -= 70;
				status_rect.y += 10;
			}

			if (NetGame::GetInstance()) {
				char lives[8];
				sprintf_s(lives, "%d", ship->RespawnCount() + 1);
				DrawMFDText(row++, Game::GetText("MFD.status.LIVES").data(), status_rect, DT_LEFT);
				status_rect.x += 70;
				DrawMFDText(row++, lives, status_rect, DT_LEFT);
				status_rect.x -= 70;
				status_rect.y += 10;
			}

			return;
		}

		Sensor* sensor = ship->GetSensor();
		if (sensor) {
			if (ship->GetFlightPhase() != Ship::ACTIVE) {
				DrawMFDText(row++, Game::GetText("MFD.status.SENSOR").data(), status_rect, DT_LEFT);
				status_rect.x += 70;
				DrawMFDText(row++, Game::GetText("MFD.status.OFFLINE").data(), status_rect, DT_LEFT);
				status_rect.x -= 70;
				status_rect.y += 10;
			}

			else {
				DrawMFDText(row++, Game::GetText("MFD.status.EMCON").data(), status_rect, DT_LEFT);
				status_rect.x += 70;

				sprintf_s(txt, "%s %d", Game::GetText("MFD.status.MODE").data(), ship->GetEMCON());

				if (!sensor->IsPowerOn() || sensor->GetEnergy() == 0) {
					if (!Game::Paused() && (Game::RealTime()/1000) & 2)
					strcpy_s(txt, Game::GetText("MFD.status.SENSOR-OFF").data());
				}

				DrawMFDText(row++, txt, status_rect, DT_LEFT);
				status_rect.x -= 70;
				status_rect.y += 10;
			}
		}

		if (lines <= 7) return;

		DrawMFDText(row++, Game::GetText("MFD.status.SYSTEMS").data(), status_rect, DT_LEFT);
		status_rect.x += 70;
		DrawMFDText(row++, ship->GetDirectorInfo(), status_rect, DT_LEFT);

		if (NetGame::GetInstance()) {
			char lives[8];
			sprintf_s(lives, "%d", ship->RespawnCount() + 1);
			status_rect.x -= 70;
			status_rect.y += 10;
			DrawMFDText(row++, Game::GetText("MFD.status.LIVES").data(), status_rect, DT_LEFT);
			status_rect.x += 70;
			DrawMFDText(row++, lives, status_rect, DT_LEFT);
		}
	}
}

// +--------------------------------------------------------------------+

void
MFD::SetStatusColor(int status)
{
	Color status_color;

	switch (status) {
	default:
	case System::NOMINAL:     status_color = txt_color;           break;
	case System::DEGRADED:    status_color = Color(255,255,  0);  break;
	case System::CRITICAL:    status_color = Color(255,  0,  0);  break;
	case System::DESTROYED:   status_color = Color(  0,  0,  0);  break;
	}
}

// +--------------------------------------------------------------------+

bool MFD::IsMouseLatched() const
{
	return mouse_in;
}

// +--------------------------------------------------------------------+

void MFD::DrawMFDText(int index, const char* txt, Rect& txt_rect, int align, int status)
{
	if (index >= MFD::TXT_LAST) {
		Print("MFD DrawMFDText() invalid mfd_text index %d '%s'\n", index, txt);
	}
	else {
		HUDText& mt = mfd_text[index];
		Color    mc = mt.color;

		switch (status) {
		default:
		case System::NOMINAL:     mc = txt_color;           break;
		case System::DEGRADED:    mc = Color(255,255,  0);  break;
		case System::CRITICAL:    mc = Color(255,  0,  0);  break;
		case System::DESTROYED:   mc = Color(  0,  0,  0);  break;
		}

		char txt_buf[256];
		int  n = strlen(txt);

		if (n > 250) n = 250;
		int i;

		for (i = 0; i < n; i++) {
			if (islower(txt[i]))
			txt_buf[i] = toupper(txt[i]);
			else
			txt_buf[i] = txt[i];
		}

		txt_buf[i] = 0;


		if (cockpit_hud_texture) {
			Rect hud_rect(txt_rect);

			hud_rect.x = txt_rect.x + this->index * 128 - this->rect.x;
			hud_rect.y = txt_rect.y + 256               - this->rect.y;

			mt.font->SetColor(mc);
			mt.font->DrawText(txt_buf, 0, hud_rect, align | DT_SINGLELINE, cockpit_hud_texture);
			mt.rect = rect;
			mt.hidden = false;
		}
		else {
			if (txt_rect.Contains(Mouse::X(), Mouse::Y()))
			mc = Color::White;

			mt.font->SetColor(mc);
			mt.font->DrawText(txt_buf, 0, txt_rect, align | DT_SINGLELINE);
			mt.rect = rect;
			mt.hidden = false;
		}

	}
}

void MFD::HideMFDText(int index)
{
	if (index >= MFD::TXT_LAST)
	Print("MFD HideMFDText() invalid mfd_text index %d\n", index);
	else
	mfd_text[index].hidden = true;
}




