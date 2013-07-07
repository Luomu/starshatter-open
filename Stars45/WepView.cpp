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
    FILE:         WepView.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    View class for Tactical HUD Overlay
*/

#include "MemDebug.h"
#include "WepView.h"
#include "HUDView.h"
#include "HUDSounds.h"
#include "Ship.h"
#include "Computer.h"
#include "NavSystem.h"
#include "Drive.h"
#include "Power.h"
#include "Shield.h"
#include "Contact.h"
#include "ShipDesign.h"
#include "Shot.h"
#include "Drone.h"
#include "Weapon.h"
#include "Sim.h"
#include "StarSystem.h"
#include "WeaponGroup.h"

#include "CameraView.h"
#include "Color.h"
#include "Window.h"
#include "Video.h"
#include "Screen.h"
#include "DataLoader.h"
#include "Scene.h"
#include "FontMgr.h"
#include "Graphic.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "Game.h"
#include "FormatUtil.h"

static Bitmap tac_left;
static Bitmap tac_right;
static Bitmap tac_button;
static Bitmap tac_man;
static Bitmap tac_aut;
static Bitmap tac_def;

static BYTE*  tac_left_shade;
static BYTE*  tac_right_shade;
static BYTE*  tac_button_shade;
static BYTE*  tac_man_shade;
static BYTE*  tac_aut_shade;
static BYTE*  tac_def_shade;

static Color  hud_color = Color::Black;
static Color  txt_color = Color::Black;

static bool   mouse_in  = false;

static Font*  hud_font  = 0;
static Font*  big_font  = 0;

// +--------------------------------------------------------------------+

WepView* WepView::wep_view = 0;

// +--------------------------------------------------------------------+

WepView::WepView(Window* c)
: View(c), sim(0), ship(0), target(0), active_region(0),
transition(false), mode(0), mouse_down(0)
{
    wep_view = this;

    sim = Sim::GetSim();

    HUDView::PrepareBitmap("TAC_left.pcx",    tac_left,   tac_left_shade);
    HUDView::PrepareBitmap("TAC_right.pcx",   tac_right,  tac_right_shade);
    HUDView::PrepareBitmap("TAC_button.pcx",  tac_button, tac_button_shade);
    HUDView::PrepareBitmap("MAN.pcx",         tac_man,    tac_man_shade);
    HUDView::PrepareBitmap("AUTO.pcx",        tac_aut,    tac_aut_shade);
    HUDView::PrepareBitmap("DEF.pcx",         tac_def,    tac_def_shade);

    tac_left.SetType(Bitmap::BMP_TRANSLUCENT);
    tac_right.SetType(Bitmap::BMP_TRANSLUCENT);
    tac_man.SetType(Bitmap::BMP_TRANSLUCENT);
    tac_aut.SetType(Bitmap::BMP_TRANSLUCENT);
    tac_def.SetType(Bitmap::BMP_TRANSLUCENT);

    OnWindowMove();

    hud_font = FontMgr::Find("HUD");
    big_font = FontMgr::Find("GUI");

    hud = HUDView::GetInstance();
    if (hud)
    SetColor(hud->GetHUDColor());
}

WepView::~WepView()
{
    tac_left.ClearImage();
    tac_right.ClearImage();
    tac_button.ClearImage();
    tac_man.ClearImage();
    tac_aut.ClearImage();
    tac_def.ClearImage();

    delete [] tac_left_shade;
    delete [] tac_right_shade;
    delete [] tac_button_shade;
    delete [] tac_man_shade;
    delete [] tac_aut_shade;
    delete [] tac_def_shade;

    tac_left_shade    = 0;
    tac_right_shade   = 0;
    tac_button_shade  = 0;
    tac_man_shade     = 0;
    tac_aut_shade     = 0;
    tac_def_shade     = 0;

    wep_view          = 0;
}

void
WepView::OnWindowMove()
{
    width       = window->Width();
    height      = window->Height();
    xcenter     = (width  / 2.0) - 0.5;
    ycenter     = (height / 2.0) + 0.5;

    int btn_loc = width/2 - 147 - 45;
    int man_loc = width/2 - 177 - 16;
    int aut_loc = width/2 - 145 - 16;
    int def_loc = width/2 - 115 - 16;

    int index   = 0;

    for (int i = 0; i < MAX_WEP; i++) {
        btn_rect[index++] = Rect(btn_loc, 30, 90, 20);
        btn_rect[index++] = Rect(man_loc, 56, 32, 8);
        btn_rect[index++] = Rect(aut_loc, 56, 32, 8);
        btn_rect[index++] = Rect(def_loc, 56, 32, 8);

        btn_loc += 98;
        man_loc += 98;
        aut_loc += 98;
        def_loc += 98;
    }
}

// +--------------------------------------------------------------------+

bool
WepView::Update(SimObject* obj)
{
    if (obj == ship) {
        ship   = 0;
        target = 0;
    }
    else if (obj == target) {
        target = 0;
    }

    return SimObserver::Update(obj);
}

const char*
WepView::GetObserverName() const
{
    return "WepView";
}

// +--------------------------------------------------------------------+

void
WepView::Refresh()
{
    sim = Sim::GetSim();
    if (!sim || !hud || hud->GetHUDMode() == HUDView::HUD_MODE_OFF)
    return;

    if (ship != sim->GetPlayerShip()) {
        ship = sim->GetPlayerShip();

        if (ship) {
            if (ship->Life() == 0 || ship->IsDying() || ship->IsDead()) {
                ship = 0;
            }
            else {
                Observe(ship);
            }
        }
    }

    if (mode < 1)
    return;

    if (ship) {
        // no tactical overlay for fighters:
        if (ship->Design() && !ship->Design()->wep_screen) {
            mode = 0;
            return;
        }

        // no hud in transition:
        if (ship->InTransition()) {
            transition = true;
            return;
        }

        else if (transition) {
            transition = false;
            RestoreOverlay();
        }

        if (target != ship->GetTarget()) {
            target = ship->GetTarget();
            if (target) Observe(target);
        }

        DrawOverlay();
    }
    else {
        if (target) {
            target = 0;
        }
    }
}

// +--------------------------------------------------------------------+

void
WepView::ExecFrame()
{
    int hud_mode = 1;

    // update the position of HUD elements that are
    // part of the 3D scene (like fpm and lcos sprites)

    if (hud) {
        if (hud_color != hud->GetHUDColor()) {
            hud_color = hud->GetHUDColor();
            SetColor(hud_color);
        }

        if (hud->GetHUDMode() == HUDView::HUD_MODE_OFF)
        hud_mode = 0;
    }

    if (ship && !transition && mode > 0 && hud_mode > 0) {
        if (mode > 0) {
            DoMouseFrame();
        }
    }
}

// +--------------------------------------------------------------------+

void
WepView::SetOverlayMode(int m)
{
    if (mode != m) {
        mode = m;

        if (hud)
        hud->SetOverlayMode(mode);

        RestoreOverlay();
    }
}

void
WepView::CycleOverlayMode()
{
    SetOverlayMode(!mode);
}

void
WepView::RestoreOverlay()
{
    if (mode > 0) {
        HUDSounds::PlaySound(HUDSounds::SND_WEP_DISP);
    }

    else {
        HUDSounds::PlaySound(HUDSounds::SND_WEP_MODE);
    }
}

// +--------------------------------------------------------------------+

void
WepView::SetColor(Color c)
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

    HUDView::ColorizeBitmap(tac_left,   tac_left_shade,   hud_color);
    HUDView::ColorizeBitmap(tac_right,  tac_right_shade,  hud_color);
    HUDView::ColorizeBitmap(tac_button, tac_button_shade, hud_color);
    HUDView::ColorizeBitmap(tac_man,    tac_man_shade,    hud_color);
    HUDView::ColorizeBitmap(tac_aut,    tac_aut_shade,    hud_color);
    HUDView::ColorizeBitmap(tac_def,    tac_def_shade,    hud_color);
}

// +--------------------------------------------------------------------+

void
WepView::DrawOverlay()
{
    int cx = width/2;
    int cy = 0;
    int w  = tac_left.Width();
    int h  = tac_left.Height();

    window->DrawBitmap(cx-w, cy, cx,   cy+h, &tac_left,  Video::BLEND_ALPHA);
    window->DrawBitmap(cx,   cy, cx+w, cy+h, &tac_right, Video::BLEND_ALPHA);

    if (ship) {
        List<WeaponGroup>& weapons = ship->Weapons();
        for (int i = 0; i < MAX_WEP; i++) {
            if (weapons.size() > i) {
                // draw main fire button:
                Rect r = btn_rect[i*4];

                w  = tac_button.Width();
                h  = tac_button.Height();
                cx = r.x + r.w/2 - w/2;
                cy = r.y + r.h/2 - h/2;

                r.Deflate(5,5);

                big_font->SetColor(txt_color);
                window->SetFont(big_font);
                window->DrawBitmap(cx, cy, cx+w, cy+h, &tac_button, Video::BLEND_ALPHA);
                window->DrawText(weapons[i]->Name(), 0, r, DT_SINGLELINE | DT_CENTER);

                r.Inflate(5,5);

                // draw firing orders:
                int o = weapons[i]->GetFiringOrders();
                w     = tac_man.Width();
                h     = tac_man.Height();

                Color c0 = Color::Gray;
                Color c1 = Color::White;

                r = btn_rect[i*4 + 1];
                cx = r.x + r.w/2 - w/2;
                cy = r.y + r.h/2 - h/2;
                window->FadeBitmap(cx, cy, cx+w, cy+h, &tac_man, (o==0 ? c1 : c0), Video::BLEND_ALPHA);

                r = btn_rect[i*4 + 2];
                cx = r.x + r.w/2 - w/2;
                cy = r.y + r.h/2 - h/2;
                window->FadeBitmap(cx, cy, cx+w, cy+h, &tac_aut, (o==1 ? c1 : c0), Video::BLEND_ALPHA);

                r = btn_rect[i*4 + 3];
                cx = r.x + r.w/2 - w/2;
                cy = r.y + r.h/2 - h/2;
                window->FadeBitmap(cx, cy, cx+w, cy+h, &tac_def, (o==2 ? c1 : c0), Video::BLEND_ALPHA);
            }
        }

        Rect tgt_rect;
        tgt_rect.x = width/2 + 73;
        tgt_rect.y = 74;
        tgt_rect.w = 100;
        tgt_rect.h = 15;

        Text           subtxt;
        Color          stat  = hud_color;
        static DWORD   blink = Game::RealTime();

        if (ship->GetTarget()) {
            if (ship->GetSubTarget()) {
                int blink_delta = Game::RealTime() - blink;

                System* sys = ship->GetSubTarget();
                subtxt = sys->Abbreviation();
                switch (sys->Status()) {
                case System::DEGRADED:  stat = Color(255,255,  0);  break;
                case System::CRITICAL:    
                case System::DESTROYED: stat = Color(255,  0,  0);  break;
                case System::MAINT:
                    if (blink_delta < 250)
                    stat = Color(8,8,8);
                    break;
                }

                if (blink_delta > 500)
                blink = Game::RealTime();
            }

            else
            subtxt = ship->GetTarget()->Name();
        }
        else {
            subtxt = "NO TGT";
        }

        subtxt.toUpper();

        hud_font->SetColor(stat);
        window->SetFont(hud_font);
        window->DrawText(subtxt.data(), subtxt.length(), tgt_rect, DT_SINGLELINE | DT_CENTER);
    }
}

// +--------------------------------------------------------------------+

void
WepView::DoMouseFrame()
{
    static int mouse_down   = false;
    static int mouse_down_x = 0;
    static int mouse_down_y = 0;

    int x = Mouse::X();
    int y = Mouse::Y();

    // coarse-grained test: is mouse in overlay at all?
    if (x < width/2-256 || x > width/2+256 || y > 90) {
        mouse_in = false;
        return;
    }

    mouse_in = true;

    if (Mouse::LButton()) {
        if (!mouse_down) {
            mouse_down   = true;
            mouse_down_x = x;
            mouse_down_y = y;
        }

        // check weapons buttons:
        int max_wep = ship->Weapons().size();

        if (max_wep > MAX_WEP)
        max_wep = MAX_WEP;

        for (int i = 0; i < max_wep; i++) {
            int index = i * 4;

            if (CheckButton(index, mouse_down_x, mouse_down_y)) {
                ship->FireWeapon(i);
                return;
            }

            else if (CheckButton(index + 1, mouse_down_x, mouse_down_y)) {
                ship->Weapons()[i]->SetFiringOrders(Weapon::MANUAL);
                return;
            }

            else if (CheckButton(index + 2, mouse_down_x, mouse_down_y)) {
                ship->Weapons()[i]->SetFiringOrders(Weapon::AUTO);
                return;
            }

            else if (CheckButton(index + 3, mouse_down_x, mouse_down_y)) {
                ship->Weapons()[i]->SetFiringOrders(Weapon::POINT_DEFENSE);
                return;
            }
        }
    }

    else if (mouse_down) {
        mouse_down   = false;
        mouse_down_x = 0;
        mouse_down_y = 0;

        // check subtarget buttons:
        if (ship->GetTarget()) {
            Rect r(width/2+50, 70, 20, 20);
            if (r.Contains(x,y)) {
                CycleSubTarget(-1);
                return;
            }

            r.x = width/2 + 180;
            if (r.Contains(x,y)) {
                CycleSubTarget(1);
                return;
            }
        }
    }
}

bool
WepView::CheckButton(int index, int x, int y)
{
    if (index >= 0 && index < MAX_BTN) {
        return btn_rect[index].Contains(x,y)?true:false;
    }

    return false;
}

void
WepView::CycleSubTarget(int direction)
{
    if (ship->GetTarget() == 0 || ship->GetTarget()->Type() != SimObject::SIM_SHIP)
    return;

    ship->CycleSubTarget(direction);
}

bool
WepView::IsMouseLatched()
{
    return mouse_in;
}
