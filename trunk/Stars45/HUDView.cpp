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
    FILE:         HUDView.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    View class for Heads Up Display
*/

#include "MemDebug.h"
#include "HUDView.h"
#include "HUDSounds.h"
#include "Ship.h"
#include "Element.h"
#include "Computer.h"
#include "Drive.h"
#include "Instruction.h"
#include "NavSystem.h"
#include "Power.h"
#include "Shield.h"
#include "Sensor.h"
#include "Contact.h"
#include "ShipDesign.h"
#include "Shot.h"
#include "Drone.h"
#include "Thruster.h"
#include "Weapon.h"
#include "WeaponGroup.h"
#include "FlightDeck.h"
#include "SteerAI.h"
#include "Sim.h"
#include "StarSystem.h"
#include "Starshatter.h"
#include "CameraDirector.h"
#include "MFD.h"
#include "RadioView.h"
#include "FormatUtil.h"
#include "Hoop.h"
#include "QuantumDrive.h"
#include "KeyMap.h"
#include "AudioConfig.h"
#include "Player.h"

#include "NetGame.h"
#include "NetPlayer.h"

#include "Color.h"
#include "CameraView.h"
#include "Screen.h"
#include "DataLoader.h"
#include "Scene.h"
#include "FontMgr.h"
#include "Graphic.h"
#include "Sprite.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "MouseController.h"
#include "Polygon.h"
#include "Sound.h"
#include "Game.h"
#include "Window.h"

static Bitmap hud_left_air;
static Bitmap hud_right_air;
static Bitmap hud_left_fighter;
static Bitmap hud_right_fighter;
static Bitmap hud_left_starship;
static Bitmap hud_right_starship;
static Bitmap instr_left;
static Bitmap instr_right;
static Bitmap warn_left;
static Bitmap warn_right;
static Bitmap lead;
static Bitmap cross;
static Bitmap cross1;
static Bitmap cross2;
static Bitmap cross3;
static Bitmap cross4;
static Bitmap fpm;
static Bitmap hpm;
static Bitmap pitch_ladder_pos;
static Bitmap pitch_ladder_neg;
static Bitmap chase_left;
static Bitmap chase_right;
static Bitmap chase_top;
static Bitmap chase_bottom;
static Bitmap icon_ship;
static Bitmap icon_target;

static BYTE*  hud_left_shade_air       = 0;
static BYTE*  hud_right_shade_air      = 0;
static BYTE*  hud_left_shade_fighter   = 0;
static BYTE*  hud_right_shade_fighter  = 0;
static BYTE*  hud_left_shade_starship  = 0;
static BYTE*  hud_right_shade_starship = 0;
static BYTE*  instr_left_shade         = 0;
static BYTE*  instr_right_shade        = 0;
static BYTE*  warn_left_shade          = 0;
static BYTE*  warn_right_shade         = 0;
static BYTE*  lead_shade               = 0;
static BYTE*  cross_shade              = 0;
static BYTE*  cross1_shade             = 0;
static BYTE*  cross2_shade             = 0;
static BYTE*  cross3_shade             = 0;
static BYTE*  cross4_shade             = 0;
static BYTE*  fpm_shade                = 0;
static BYTE*  hpm_shade                = 0;
static BYTE*  pitch_ladder_pos_shade   = 0;
static BYTE*  pitch_ladder_neg_shade   = 0;
static BYTE*  chase_left_shade         = 0;
static BYTE*  chase_right_shade        = 0;
static BYTE*  chase_top_shade          = 0;
static BYTE*  chase_bottom_shade       = 0;
static BYTE*  icon_ship_shade          = 0;
static BYTE*  icon_target_shade        = 0;

static Sprite*   hud_left_sprite     = 0;
static Sprite*   hud_right_sprite    = 0;
static Sprite*   fpm_sprite          = 0;
static Sprite*   hpm_sprite          = 0;
static Sprite*   lead_sprite         = 0;
static Sprite*   aim_sprite          = 0;
static Sprite*   tgt1_sprite         = 0;
static Sprite*   tgt2_sprite         = 0;
static Sprite*   tgt3_sprite         = 0;
static Sprite*   tgt4_sprite         = 0;
static Sprite*   chase_sprite        = 0;
static Sprite*   instr_left_sprite   = 0;
static Sprite*   instr_right_sprite  = 0;
static Sprite*   warn_left_sprite    = 0;
static Sprite*   warn_right_sprite   = 0;
static Sprite*   icon_ship_sprite    = 0;
static Sprite*   icon_target_sprite  = 0;

static Sound* missile_lock_sound;

const int NUM_HUD_COLORS = 4;

Color standard_hud_colors[NUM_HUD_COLORS] = {
    Color(130,190,140),  // green
    Color(130,200,220),  // cyan
    Color(250,170, 80),  // orange
    // Color(220,220,100),  // yellow
    Color( 16, 16, 16)   // dark gray
};

Color standard_txt_colors[NUM_HUD_COLORS] = {
    Color(150,200,170),  // green w/ green gray
    Color(220,220,180),  // cyan  w/ light yellow
    Color(220,220, 80),  // orange w/ yellow
    // Color(180,200,220),  // yellow w/ white
    Color( 32, 32, 32)   // dark gray
};

Color night_vision_colors[NUM_HUD_COLORS] = {
    Color( 20, 80, 20),  // green
    Color( 30, 80, 80),  // cyan
    Color( 80, 80, 20),  // yellow
    // Color(180,200,220),  // not used
    Color(  0,  0,  0)   // no night vision
};

static Font*  hud_font    = 0;
static Font*  big_font    = 0;

static bool   mouse_in    = false;
static int    mouse_latch = 0;
static int    mouse_index = -1;

static int ship_status = System::NOMINAL;
static int tgt_status  = System::NOMINAL;

// +--------------------------------------------------------------------+

static enum TXT {
    MAX_CONTACT = 50,

    TXT_CAUTION_TXT   = 0,
    TXT_LAST_CAUTION  = 23,
    TXT_CAM_ANGLE,
    TXT_CAM_MODE,
    TXT_PAUSED,
    TXT_GEAR_DOWN,

    TXT_HUD_MODE,
    TXT_PRIMARY_WEP,
    TXT_SECONDARY_WEP,
    TXT_DECOY,
    TXT_SHIELD,
    TXT_AUTO,
    TXT_SHOOT,
    TXT_NAV_INDEX,
    TXT_NAV_ACTION,
    TXT_NAV_FORMATION,
    TXT_NAV_SPEED,
    TXT_NAV_ETR,
    TXT_NAV_HOLD,

    TXT_SPEED,
    TXT_RANGE,
    TXT_CLOSING_SPEED,
    TXT_THREAT_WARN,
    TXT_COMPASS,
    TXT_HEADING,
    TXT_PITCH,
    TXT_ALTITUDE,
    TXT_GFORCE,
    TXT_MISSILE_T1,
    TXT_MISSILE_T2,
    TXT_ICON_SHIP_TYPE,
    TXT_ICON_TARGET_TYPE,
    TXT_TARGET_NAME,
    TXT_TARGET_DESIGN,
    TXT_TARGET_SHIELD,
    TXT_TARGET_HULL,
    TXT_TARGET_SUB,
    TXT_TARGET_ETA,

    TXT_MSG_1,
    TXT_MSG_2,
    TXT_MSG_3,
    TXT_MSG_4,
    TXT_MSG_5,
    TXT_MSG_6,

    TXT_NAV_PT,
    TXT_SELF,
    TXT_SELF_NAME,
    TXT_CONTACT_NAME,
    TXT_CONTACT_INFO  = TXT_CONTACT_NAME + MAX_CONTACT,
    TXT_LAST          = TXT_CONTACT_INFO + MAX_CONTACT,

    TXT_LAST_ACTIVE   = TXT_NAV_HOLD,
    TXT_INSTR_PAGE    = TXT_CAUTION_TXT  + 6,
};

static HUDText hud_text[TXT_LAST];

void 
HUDView::DrawHUDText(int index, const char* txt, Rect& rect, int align, int upcase, bool box)
{
    if (index < 0 || index >= TXT_LAST)
    return;

    HUDText& ht = hud_text[index];
    Color    hc = ht.color;

    char txt_buf[256];
    int  n = strlen(txt);

    if (n > 250) n = 250;

    int i;
    for (i = 0; i < n; i++) {
        if (upcase && islower(txt[i]))
        txt_buf[i] = toupper(txt[i]);
        else
        txt_buf[i] = txt[i];
    }

    txt_buf[i] = 0;

    if (box) {
        ht.font->DrawText(txt_buf, n, rect, DT_LEFT | DT_SINGLELINE | DT_CALCRECT);

        if ((align & DT_CENTER) != 0) {
            int cx = width/2;
            rect.x = cx - rect.w/2;
        }
    }

    if (!cockpit_hud_texture && rect.Contains(Mouse::X(), Mouse::Y())) {
        mouse_in = true;

        if (index <= TXT_LAST_ACTIVE)
        hc = Color::White;
        
        if (Mouse::LButton() && !mouse_latch) {
            mouse_latch = 2;
            mouse_index = index;
        }
    }

    if (cockpit_hud_texture       && 
            index >= TXT_HUD_MODE   && 
            index <= TXT_TARGET_ETA &&
            ht.font != big_font) {

        Sprite* s = hud_sprite[0];

        int cx = (int) s->Location().x;
        int cy = (int) s->Location().y;
        int w2 = s->Width()  / 2;
        int h2 = s->Height() / 2;

        Rect txt_rect(rect);
        txt_rect.x -= (cx-w2);
        txt_rect.y -= (cy-h2);

        if (index == TXT_ICON_SHIP_TYPE)
        txt_rect = Rect(0, 500, 128, 12);

        else if (index == TXT_ICON_TARGET_TYPE)
        txt_rect = Rect(128, 500, 128, 12);

        ht.font->SetColor(hc);
        ht.font->DrawText(txt_buf, n, txt_rect, align | DT_SINGLELINE, cockpit_hud_texture);
        ht.hidden = false;
    }
    else {
        ht.font->SetColor(hc);
        ht.font->DrawText(txt_buf, n, rect, align | DT_SINGLELINE);
        ht.rect = rect;
        ht.hidden = false;

        if (box) {
            rect.Inflate(3,2);
            rect.h--;
            window->DrawRect(rect, hud_color);
        }
    }
}

void
HUDView::HideHUDText(int index)
{
    if (index >= TXT_LAST)
    return;

    hud_text[index].hidden = true;
}

// +--------------------------------------------------------------------+

HUDView* HUDView::hud_view       = 0;
bool     HUDView::arcade         = false;
bool     HUDView::show_fps       = false;
int      HUDView::def_color_set  = 1;
int      HUDView::gunsight       = 1;

// +--------------------------------------------------------------------+

HUDView::HUDView(Window* c)
: View(c), projector(0), camview(0),
sim(0), ship(0), target(0), mode(HUD_MODE_TAC),
tactical(0), overlay(0), cockpit_hud_texture(0),
threat(0), active_region(0), transition(false), docking(false),
az_ring(0), az_pointer(0), el_ring(0), el_pointer(0), compass_scale(1),
show_warn(false), show_inst(false), inst_page(0)
{
    hud_view = this;

    sim = Sim::GetSim();

    if (sim)
    sim->ShowGrid(false);

    int i;

    width       = window->Width();
    height      = window->Height();
    xcenter     = (width  / 2.0) - 0.5;
    ycenter     = (height / 2.0) + 0.5;

    PrepareBitmap("HUDleftA.pcx",    hud_left_air,        hud_left_shade_air);
    PrepareBitmap("HUDrightA.pcx",   hud_right_air,       hud_right_shade_air);
    PrepareBitmap("HUDleft.pcx",     hud_left_fighter,    hud_left_shade_fighter);
    PrepareBitmap("HUDright.pcx",    hud_right_fighter,   hud_right_shade_fighter);
    PrepareBitmap("HUDleft1.pcx",    hud_left_starship,   hud_left_shade_starship);
    PrepareBitmap("HUDright1.pcx",   hud_right_starship,  hud_right_shade_starship);
    PrepareBitmap("INSTR_left.pcx",  instr_left,          instr_left_shade);
    PrepareBitmap("INSTR_right.pcx", instr_right,         instr_right_shade);
    PrepareBitmap("CAUTION_left.pcx",  warn_left,         warn_left_shade);
    PrepareBitmap("CAUTION_right.pcx", warn_right,        warn_right_shade);
    PrepareBitmap("hud_icon.pcx",    icon_ship,           icon_ship_shade);
    PrepareBitmap("hud_icon.pcx",    icon_target,         icon_target_shade);

    PrepareBitmap("lead.pcx",        lead,          lead_shade);
    PrepareBitmap("cross.pcx",       cross,         cross_shade);
    PrepareBitmap("cross1.pcx",      cross1,        cross1_shade);
    PrepareBitmap("cross2.pcx",      cross2,        cross2_shade);
    PrepareBitmap("cross3.pcx",      cross3,        cross3_shade);
    PrepareBitmap("cross4.pcx",      cross4,        cross4_shade);
    PrepareBitmap("fpm.pcx",         fpm,           fpm_shade);
    PrepareBitmap("hpm.pcx",         hpm,           hpm_shade);
    PrepareBitmap("chase_l.pcx",     chase_left,    chase_left_shade);
    PrepareBitmap("chase_r.pcx",     chase_right,   chase_right_shade);
    PrepareBitmap("chase_t.pcx",     chase_top,     chase_top_shade);
    PrepareBitmap("chase_b.pcx",     chase_bottom,  chase_bottom_shade);
    PrepareBitmap("ladder1.pcx",     pitch_ladder_pos,
    pitch_ladder_pos_shade);
    PrepareBitmap("ladder2.pcx",     pitch_ladder_neg,
    pitch_ladder_neg_shade);

    hud_left_air.SetType(Bitmap::BMP_TRANSLUCENT);
    hud_right_air.SetType(Bitmap::BMP_TRANSLUCENT);
    hud_left_fighter.SetType(Bitmap::BMP_TRANSLUCENT);
    hud_right_fighter.SetType(Bitmap::BMP_TRANSLUCENT);
    hud_left_starship.SetType(Bitmap::BMP_TRANSLUCENT);
    hud_right_starship.SetType(Bitmap::BMP_TRANSLUCENT);
    instr_left.SetType(Bitmap::BMP_TRANSLUCENT);
    instr_right.SetType(Bitmap::BMP_TRANSLUCENT);
    warn_left.SetType(Bitmap::BMP_TRANSLUCENT);
    warn_right.SetType(Bitmap::BMP_TRANSLUCENT);
    icon_ship.SetType(Bitmap::BMP_TRANSLUCENT);
    icon_target.SetType(Bitmap::BMP_TRANSLUCENT);
    fpm.SetType(Bitmap::BMP_TRANSLUCENT);
    hpm.SetType(Bitmap::BMP_TRANSLUCENT);
    lead.SetType(Bitmap::BMP_TRANSLUCENT);
    cross.SetType(Bitmap::BMP_TRANSLUCENT);
    cross1.SetType(Bitmap::BMP_TRANSLUCENT);
    cross2.SetType(Bitmap::BMP_TRANSLUCENT);
    cross3.SetType(Bitmap::BMP_TRANSLUCENT);
    cross4.SetType(Bitmap::BMP_TRANSLUCENT);
    chase_left.SetType(Bitmap::BMP_TRANSLUCENT);
    chase_right.SetType(Bitmap::BMP_TRANSLUCENT);
    chase_top.SetType(Bitmap::BMP_TRANSLUCENT);
    chase_bottom.SetType(Bitmap::BMP_TRANSLUCENT);
    pitch_ladder_pos.SetType(Bitmap::BMP_TRANSLUCENT);
    pitch_ladder_neg.SetType(Bitmap::BMP_TRANSLUCENT);

    hud_left_sprite    = new(__FILE__,__LINE__) Sprite(&hud_left_fighter);
    hud_right_sprite   = new(__FILE__,__LINE__) Sprite(&hud_right_fighter);
    instr_left_sprite  = new(__FILE__,__LINE__) Sprite(&instr_left);
    instr_right_sprite = new(__FILE__,__LINE__) Sprite(&instr_right);
    warn_left_sprite   = new(__FILE__,__LINE__) Sprite(&warn_left);
    warn_right_sprite  = new(__FILE__,__LINE__) Sprite(&warn_right);
    icon_ship_sprite   = new(__FILE__,__LINE__) Sprite(&icon_ship);
    icon_target_sprite = new(__FILE__,__LINE__) Sprite(&icon_target);
    fpm_sprite         = new(__FILE__,__LINE__) Sprite(&fpm);
    hpm_sprite         = new(__FILE__,__LINE__) Sprite(&hpm);
    lead_sprite        = new(__FILE__,__LINE__) Sprite(&lead);
    aim_sprite         = new(__FILE__,__LINE__) Sprite(&cross);
    tgt1_sprite        = new(__FILE__,__LINE__) Sprite(&cross1);
    tgt2_sprite        = new(__FILE__,__LINE__) Sprite(&cross2);
    tgt3_sprite        = new(__FILE__,__LINE__) Sprite(&cross3);
    tgt4_sprite        = new(__FILE__,__LINE__) Sprite(&cross4);
    chase_sprite       = new(__FILE__,__LINE__) Sprite(&chase_left);

    ZeroMemory(hud_sprite, sizeof(hud_sprite));

    hud_sprite[ 0] = hud_left_sprite;
    hud_sprite[ 1] = hud_right_sprite;
    hud_sprite[ 2] = instr_left_sprite;
    hud_sprite[ 3] = instr_right_sprite;
    hud_sprite[ 4] = warn_left_sprite;
    hud_sprite[ 5] = warn_right_sprite;
    hud_sprite[ 6] = icon_ship_sprite;
    hud_sprite[ 7] = icon_target_sprite;
    hud_sprite[ 8] = fpm_sprite;
    hud_sprite[ 9] = hpm_sprite;
    hud_sprite[10] = lead_sprite;
    hud_sprite[11] = aim_sprite;
    hud_sprite[12] = tgt1_sprite;
    hud_sprite[13] = tgt2_sprite;
    hud_sprite[14] = tgt3_sprite;
    hud_sprite[15] = tgt4_sprite;
    hud_sprite[16] = chase_sprite;

    double pitch_ladder_UV[8] = { 0.125,0.0625, 0.875,0.0625, 0.875,0, 0.125,0 };
    double UV[8];

    for (i = 0; i < 15; i++) {
        pitch_ladder[i] = new(__FILE__,__LINE__) Sprite(&pitch_ladder_pos);

        CopyMemory(UV, pitch_ladder_UV, sizeof(UV));
        UV[1] = UV[3] = (pitch_ladder_UV[1] * (i  ));
        UV[5] = UV[7] = (pitch_ladder_UV[1] * (i+1));

        pitch_ladder[i]->Reshape(192, 16);
        pitch_ladder[i]->SetTexCoords(UV);
        pitch_ladder[i]->SetBlendMode(2);
        pitch_ladder[i]->Hide();
    }

    // zero mark at i=15
    {
        pitch_ladder[i] = new(__FILE__,__LINE__) Sprite(&pitch_ladder_pos);

        UV[0] = UV[6] = 0;
        UV[2] = UV[4] = 1;
        UV[1] = UV[3] = (pitch_ladder_UV[1] * (i+1));
        UV[5] = UV[7] = (pitch_ladder_UV[1] * (i  ));

        pitch_ladder[i]->Reshape(256, 16);
        pitch_ladder[i]->SetTexCoords(UV);
        pitch_ladder[i]->SetBlendMode(2);
        pitch_ladder[i]->Hide();
    }

    for (i = 16; i < 31; i++) {
        pitch_ladder[i] = new(__FILE__,__LINE__) Sprite(&pitch_ladder_neg);

        CopyMemory(UV, pitch_ladder_UV, sizeof(UV));
        UV[1] = UV[3] = (pitch_ladder_UV[1] * (30 - i  ));
        UV[5] = UV[7] = (pitch_ladder_UV[1] * (30 - i+1));

        pitch_ladder[i]->Reshape(192, 16);
        pitch_ladder[i]->SetTexCoords(UV);
        pitch_ladder[i]->SetBlendMode(2);
        pitch_ladder[i]->Hide();
    }

    for (i = 0; i < 3; i++)
    mfd[i] = new(__FILE__,__LINE__) MFD(window, i);

    mfd[0]->SetRect(Rect(          8, height - 136, 128, 128));
    mfd[1]->SetRect(Rect(width - 136, height - 136, 128, 128));
    mfd[2]->SetRect(Rect(          8,            8, 128, 128));

    hud_left_sprite->MoveTo( Point(width/2-128, height/2, 1));
    hud_right_sprite->MoveTo(Point(width/2+128, height/2, 1));
    hud_left_sprite->SetBlendMode(2);
    hud_left_sprite->SetFilter(0);
    hud_right_sprite->SetBlendMode(2);
    hud_right_sprite->SetFilter(0);

    instr_left_sprite->MoveTo( Point(width/2-128, height-128, 1));
    instr_right_sprite->MoveTo(Point(width/2+128, height-128, 1));
    instr_left_sprite->SetBlendMode(2);
    instr_left_sprite->SetFilter(0);
    instr_right_sprite->SetBlendMode(2);
    instr_right_sprite->SetFilter(0);

    warn_left_sprite->MoveTo( Point(width/2-128, height-128, 1));
    warn_right_sprite->MoveTo(Point(width/2+128, height-128, 1));
    warn_left_sprite->SetBlendMode(2);
    warn_left_sprite->SetFilter(0);
    warn_right_sprite->SetBlendMode(2);
    warn_right_sprite->SetFilter(0);

    icon_ship_sprite->MoveTo(  Point(        184, height-72, 1));
    icon_target_sprite->MoveTo(Point(width - 184, height-72, 1));
    icon_ship_sprite->SetBlendMode(2);
    icon_ship_sprite->SetFilter(0);
    icon_target_sprite->SetBlendMode(2);
    icon_target_sprite->SetFilter(0);

    fpm_sprite->MoveTo(Point(width/2, height/2, 1));
    hpm_sprite->MoveTo(Point(width/2, height/2, 1));
    lead_sprite->MoveTo(Point(width/2, height/2, 1));
    aim_sprite->MoveTo(Point(width/2, height/2, 1));
    tgt1_sprite->MoveTo(Point(width/2, height/2, 1));
    tgt2_sprite->MoveTo(Point(width/2, height/2, 1));
    tgt3_sprite->MoveTo(Point(width/2, height/2, 1));
    tgt4_sprite->MoveTo(Point(width/2, height/2, 1));

    fpm_sprite->SetBlendMode(2);
    hpm_sprite->SetBlendMode(2);
    lead_sprite->SetBlendMode(2);
    aim_sprite->SetBlendMode(2);
    tgt1_sprite->SetBlendMode(2);
    tgt2_sprite->SetBlendMode(2);
    tgt3_sprite->SetBlendMode(2);
    tgt4_sprite->SetBlendMode(2);
    chase_sprite->SetBlendMode(2);

    fpm_sprite->SetFilter(0);
    hpm_sprite->SetFilter(0);
    lead_sprite->SetFilter(0);
    aim_sprite->SetFilter(0);
    tgt1_sprite->SetFilter(0);
    tgt2_sprite->SetFilter(0);
    tgt3_sprite->SetFilter(0);
    tgt4_sprite->SetFilter(0);
    chase_sprite->SetFilter(0);

    lead_sprite->Hide();
    aim_sprite->Hide();
    tgt1_sprite->Hide();
    tgt2_sprite->Hide();
    tgt3_sprite->Hide();
    tgt4_sprite->Hide();
    chase_sprite->Hide();

    aw = chase_left.Width()  / 2;
    ah = chase_left.Height() / 2;

    mfd[0]->SetMode(MFD::MFD_MODE_SHIP);
    mfd[1]->SetMode(MFD::MFD_MODE_FOV);
    mfd[2]->SetMode(MFD::MFD_MODE_GAME);

    hud_font = FontMgr::Find("HUD");
    big_font = FontMgr::Find("GUI");

    for (i = 0; i < TXT_LAST; i++) {
        hud_text[i].font = hud_font;
    }

    hud_text[TXT_THREAT_WARN].font   = big_font;
    hud_text[TXT_SHOOT].font         = big_font;
    hud_text[TXT_AUTO].font          = big_font;

    SetHUDColorSet(def_color_set);
    MFD::SetColor(standard_hud_colors[color]);

    DataLoader* loader = DataLoader::GetLoader();
    loader->SetDataPath("HUD/");

    az_ring    = new(__FILE__,__LINE__) Solid;
    az_pointer = new(__FILE__,__LINE__) Solid;
    el_ring    = new(__FILE__,__LINE__) Solid;
    el_pointer = new(__FILE__,__LINE__) Solid;

    az_ring->Load("CompassRing.mag",       compass_scale);
    az_pointer->Load("CompassPointer.mag", compass_scale);
    el_ring->Load("PitchRing.mag",         compass_scale);
    el_pointer->Load("CompassPointer.mag", compass_scale);

    loader->SetDataPath("Sounds/");
    loader->LoadSound("MissileLock.wav", missile_lock_sound, Sound::LOOP | Sound::LOCKED);

    loader->SetDataPath(0);

    for (i = 0; i < MAX_MSG; i++)
    msg_time[i] = 0;
}

HUDView::~HUDView()
{
    HideCompass();

    if (missile_lock_sound) {
        missile_lock_sound->Stop();
        missile_lock_sound->Release();
        missile_lock_sound = 0;
    }

    for (int i = 0; i < 3; i++) {
        delete mfd[i];
        mfd[i] = 0;
    }

    for (int i = 0; i < 32; i++) {
        GRAPHIC_DESTROY(hud_sprite[i]);
    }

    fpm.ClearImage();
    hpm.ClearImage();
    lead.ClearImage();
    cross.ClearImage();
    cross1.ClearImage();
    cross2.ClearImage();
    cross3.ClearImage();
    cross4.ClearImage();
    hud_left_air.ClearImage();
    hud_right_air.ClearImage();
    hud_left_fighter.ClearImage();
    hud_right_fighter.ClearImage();
    hud_left_starship.ClearImage();
    hud_right_starship.ClearImage();
    instr_left.ClearImage();
    instr_right.ClearImage();
    warn_left.ClearImage();
    warn_right.ClearImage();
    icon_ship.ClearImage();
    icon_target.ClearImage();
    chase_left.ClearImage();
    chase_right.ClearImage();
    chase_top.ClearImage();
    chase_bottom.ClearImage();
    pitch_ladder_pos.ClearImage();
    pitch_ladder_neg.ClearImage();

    delete [] fpm_shade;
    delete [] hpm_shade;
    delete [] lead_shade;
    delete [] cross_shade;
    delete [] cross1_shade;
    delete [] cross2_shade;
    delete [] cross3_shade;
    delete [] cross4_shade;
    delete [] hud_left_shade_air;
    delete [] hud_right_shade_air;
    delete [] hud_left_shade_fighter;
    delete [] hud_right_shade_fighter;
    delete [] hud_left_shade_starship;
    delete [] hud_right_shade_starship;
    delete [] instr_left_shade;
    delete [] instr_right_shade;
    delete [] warn_left_shade;
    delete [] warn_right_shade;
    delete [] icon_ship_shade;
    delete [] icon_target_shade;
    delete [] chase_left_shade;
    delete [] chase_right_shade;
    delete [] chase_top_shade;
    delete [] chase_bottom_shade;
    delete [] pitch_ladder_pos_shade;
    delete [] pitch_ladder_neg_shade;

    delete az_ring;
    delete az_pointer;
    delete el_ring;
    delete el_pointer;

    fpm_shade = 0;
    hpm_shade = 0;
    cross_shade = 0;
    cross1_shade = 0;
    cross2_shade = 0;
    cross3_shade = 0;
    cross4_shade = 0;
    hud_left_shade_air = 0;
    hud_right_shade_air = 0;
    hud_left_shade_fighter = 0;
    hud_right_shade_fighter = 0;
    hud_left_shade_starship = 0;
    hud_right_shade_starship = 0;
    instr_left_shade = 0;
    instr_right_shade = 0;
    warn_left_shade = 0;
    warn_right_shade = 0;
    icon_ship_shade = 0;
    icon_target_shade = 0;
    chase_left_shade = 0;
    chase_right_shade = 0;
    chase_top_shade = 0;
    chase_bottom_shade = 0;
    pitch_ladder_pos_shade = 0;
    pitch_ladder_neg_shade = 0;

    az_ring = 0;
    az_pointer = 0;
    el_ring = 0;
    el_pointer = 0;

    hud_view = 0;
}

void
HUDView::OnWindowMove()
{
    width       = window->Width();
    height      = window->Height();
    xcenter     = (width  / 2.0) - 0.5;
    ycenter     = (height / 2.0) + 0.5;

    mfd[0]->SetRect(Rect(          8, height - 136, 128, 128));
    mfd[1]->SetRect(Rect(width - 136, height - 136, 128, 128));
    mfd[2]->SetRect(Rect(          8,            8, 128, 128));

    hud_left_sprite->MoveTo( Point(width/2-128, height/2, 1));
    hud_right_sprite->MoveTo(Point(width/2+128, height/2, 1));

    instr_left_sprite->MoveTo( Point(width/2-128, height-128, 1));
    instr_right_sprite->MoveTo(Point(width/2+128, height-128, 1));
    warn_left_sprite->MoveTo( Point(width/2-128, height-128, 1));
    warn_right_sprite->MoveTo(Point(width/2+128, height-128, 1));
    icon_ship_sprite->MoveTo(  Point(        184, height-72, 1));
    icon_target_sprite->MoveTo(Point(width - 184, height-72, 1));

    for (int i = 0; i < TXT_LAST; i++) {
        hud_text[i].font  = hud_font;
        hud_text[i].color = standard_txt_colors[color];
    }

    if (big_font) {
        hud_text[TXT_THREAT_WARN].font   = big_font;
        hud_text[TXT_SHOOT].font         = big_font;
        hud_text[TXT_AUTO].font          = big_font;
    }

    MFD::SetColor(standard_hud_colors[color]);

    int   cx   = width/2;
    int   cy   = height/2;
}

// +--------------------------------------------------------------------+

void
HUDView::SetTacticalMode(int mode)
{
    if (tactical != mode) {
        tactical = mode;

        if (tactical) {
            hud_left_sprite->Hide();
            hud_right_sprite->Hide();

            for (int i = 0; i < 31; i++)
            pitch_ladder[i]->Hide();
        }
        else if (Game::MaxTexSize() > 128) {
            hud_left_sprite->Show();
            hud_right_sprite->Show();
        }
    }
}

void
HUDView::SetOverlayMode(int mode)
{
    if (overlay != mode) {
        overlay = mode;
    }
}

// +--------------------------------------------------------------------+

bool
HUDView::Update(SimObject* obj)
{
    if (obj == ship) {
        if (target)
        SetTarget(0);

        ship = 0;

        for (int i = 0; i < 3; i++)
        mfd[i]->SetShip(ship);
    }

    if (obj == target) {
        target = 0;
        PrepareBitmap("hud_icon.pcx", icon_target, icon_target_shade);
        ColorizeBitmap(icon_target, icon_target_shade, txt_color);
    }

    return SimObserver::Update(obj);
}

const char*
HUDView::GetObserverName() const
{
    return "HUDView";
}

// +--------------------------------------------------------------------+

void
HUDView::UseCameraView(CameraView* v)
{
    if (v && camview != v) {
        camview = v;

        for (int i = 0; i < 3; i++)
        mfd[i]->UseCameraView(camview);

        projector = camview->GetProjector();
    }
}

// +--------------------------------------------------------------------+

Color
HUDView::MarkerColor(Contact* contact)
{
    Color c(80,80,80);

    if (contact) {
        Sim*  sim  = Sim::GetSim();
        Ship* ship = sim->GetPlayerShip();

        int c_iff = contact->GetIFF(ship);

        c = Ship::IFFColor(c_iff) * contact->Age();

        if (contact->GetShot() && contact->Threat(ship)) {
            if ((Game::RealTime()/500) & 1)
            c = c * 2;
            else
            c = c * 0.5;
        }
    }
    
    return c;   
}

// +--------------------------------------------------------------------+

void
HUDView::DrawContactMarkers()
{
    threat = 0;

    for (int i = 0; i < MAX_CONTACT; i++) {
        HideHUDText(TXT_CONTACT_NAME+i);
        HideHUDText(TXT_CONTACT_INFO+i);
    }


    if (!ship)
    return;

    int               index   = 0;
    ListIter<Contact> contact = ship->ContactList();

    // draw own sensor contacts:
    while (++contact) {
        Contact* c = contact.value();

        // draw track ladder:
        if (c->TrackLength() > 0 && c->GetShip() != ship) {
            DrawTrack(c);
        }

        DrawContact(c, index++);
    }

    Color c = ship->MarkerColor();   

    // draw own ship track ladder:
    if (CameraDirector::GetCameraMode() == CameraDirector::MODE_ORBIT && ship->TrackLength() > 0) {
        int ctl = ship->TrackLength();

        Point t1 = ship->Location();
        Point t2 = ship->TrackPoint(0);

        if (t1 != t2)
        DrawTrackSegment(t1, t2, c);

        for (int i = 0; i < ctl-1; i++) {
            t1 = ship->TrackPoint(i);
            t2 = ship->TrackPoint(i+1);

            if (t1 != t2)
            DrawTrackSegment(t1, t2, c * ((double) (ctl-i)/ (double) ctl));
        }
    }

    // draw own ship marker:
    Point mark_pt = ship->Location();
    projector->Transform(mark_pt);

    // clip:
    if (CameraDirector::GetCameraMode() == CameraDirector::MODE_ORBIT && mark_pt.z > 1.0) {
        projector->Project(mark_pt);
        
        int x = (int) mark_pt.x;
        int y = (int) mark_pt.y;

        if (x > 4 && x < width-4 &&
                y > 4 && y < height-4) {

            DrawDiamond(x,y,5,c);

            if (tactical) {
                Rect self_rect(x+8, y-4, 200, 12);
                DrawHUDText(TXT_SELF, ship->Name(), self_rect, DT_LEFT, HUD_MIXED_CASE);

                if (NetGame::GetInstance()) {
                    Player* p = Player::GetCurrentPlayer();
                    if (p) {
                        Rect net_name_rect(x+8, y+6, 120, 12);
                        DrawHUDText(TXT_SELF_NAME, p->Name(), net_name_rect, DT_LEFT, HUD_MIXED_CASE);
                    }
                }
            }
        }
    }

    // draw life bars on targeted ship:
    if (target && target->Type() == SimObject::SIM_SHIP && target->Rep()) {
        Ship*       tgt_ship = (Ship*) target;
        if (tgt_ship == nullptr) {
            Print("   Null Pointer in HUDView::DrawContactMarkers(). Please investigate.");
            return;
        }
        Graphic*    g        = tgt_ship->Rep();
        Rect        r        = g->ScreenRect();

        Point mark_pt;

        if (tgt_ship)
        mark_pt = tgt_ship->Location();

        projector->Transform(mark_pt);

        // clip:
        if (mark_pt.z > 1.0) {
            projector->Project(mark_pt);

            int x = (int) mark_pt.x;
            int y = r.y;

            if (y >= 2000)
            y = (int) mark_pt.y;

            if (x > 4 && x < width-4 &&
                    y > 4 && y < height-4) {

                const int BAR_LENGTH = 40;

                // life bars:
                int sx = x - BAR_LENGTH/2;
                int sy = y - 8;

                double hull_strength = tgt_ship->Integrity() / tgt_ship->Design()->integrity;

                int hw = (int) (BAR_LENGTH * hull_strength);
                int sw = (int) (BAR_LENGTH * (tgt_ship->ShieldStrength() / 100.0));

                System::STATUS s = System::NOMINAL;

                if (hull_strength < 0.30)        s = System::CRITICAL;
                else if (hull_strength < 0.60)   s = System::DEGRADED;

                Color hc = GetStatusColor(s);
                Color sc = hud_color;

                window->FillRect(sx, sy,   sx+hw, sy+1, hc);
                window->FillRect(sx, sy+3, sx+sw, sy+4, sc);
            }
        }
    }
}

// +--------------------------------------------------------------------+

void
HUDView::DrawContact(Contact* contact, int index)
{
    if (index >= MAX_CONTACT) return;

    Color  c        = MarkerColor(contact);
    int    c_iff    = contact->GetIFF(ship);
    Ship*  c_ship   = contact->GetShip();
    Shot*  c_shot   = contact->GetShot();
    Point  mark_pt  = contact->Location();
    double distance = 0;

    if (!c_ship && !c_shot || c_ship == ship)
    return;

    if (c_ship && c_ship->GetFlightPhase() < Ship::ACTIVE)
    return;

    if (c_ship) {
        mark_pt = c_ship->Location();

        if (c_ship->IsGroundUnit())
        mark_pt += Point(0,150,0);
    }
    else {
        mark_pt = c_shot->Location();
    }

    projector->Transform(mark_pt);

    // clip:
    if (mark_pt.z > 1.0) {
        distance = mark_pt.length();

        projector->Project(mark_pt);

        int x = (int) mark_pt.x;
        int y = (int) mark_pt.y;

        if (x > 4 && x < width-4 &&
                y > 4 && y < height-4) {

            DrawDiamond(x,y,3,c);

            if (contact->Threat(ship)) {
                if (c_ship) {
                    window->DrawEllipse(x-6, y-6, x+6, y+6, c);
                }
                else {
                    DrawDiamond(x,y,7,c);
                }
            }

            bool name_crowded = false;

            if (x < width-8) {
                char code = *(Game::GetText("HUDView.symbol.fighter").data());

                if (c_ship) {
                    if (c_ship->Class() > Ship::LCA)
                    code = *(Game::GetText("HUDView.symbol.starship").data());
                }

                else if (c_shot) {
                    code = *(Game::GetText("HUDView.symbol.torpedo").data());
                }

                Sensor* sensor = ship->GetSensor();
                double  limit  = 75e3;

                if (sensor)
                limit = sensor->GetBeamRange();

                double  range  = contact->Range(ship, limit);

                char contact_buf[256];
                Rect contact_rect(x+8, y-4, 120, 12);

                if (range == 0) {
                    sprintf_s(contact_buf, "%c *", code);
                }
                else {
                    bool mega = false;

                    if (range > 999e3) {
                        range /= 1e6;
                        mega = true;
                    }
                    else if (range < 1e3)
                    range = 1;
                    else
                    range /= 1000;

                    if (arcade) {
                        if (c_ship)
                        strcpy_s(contact_buf, c_ship->Name());
                        else if (!mega)
                        sprintf_s(contact_buf, "%c %d", code, (int) range);
                        else
                        sprintf_s(contact_buf, "%c %.1f M", code, range);
                    }
                    else {
                        char  closing = '+';
                        Point delta_v;

                        if (c_ship)
                            delta_v = ship->Velocity() - c_ship->Velocity();
                        else if (c_shot)
                            delta_v = ship->Velocity() - c_shot->Velocity();

                        if (delta_v * ship->Velocity() < 0)    // losing ground
                        closing = '-';

                        if (!mega)
                        sprintf_s(contact_buf, "%c %d%c", code, (int) range, closing);
                        else
                        sprintf_s(contact_buf, "%c %.1f M", code, range);
                    }
                }

                if (!IsNameCrowded(x, y)) {
                    DrawHUDText(TXT_CONTACT_INFO+index, contact_buf, contact_rect, DT_LEFT, HUD_MIXED_CASE);

                    if (c_shot || (c_ship && (c_ship->IsDropship() || c_ship->IsStatic())))
                    name_crowded = distance > 50e3;
                }
                else {
                    name_crowded = true;
                }
            }

            bool name_drawn = false;
            if (NetGame::GetInstance() && c_ship) {
                NetPlayer* netp = NetGame::GetInstance()->FindPlayerByObjID(c_ship->GetObjID());
                if (netp && strcmp(netp->Name(), "Server A.I. Ship")) {
                    Rect contact_rect(x+8, y+6, 120, 12);
                    DrawHUDText(TXT_CONTACT_NAME+index, netp->Name(), contact_rect, DT_LEFT, HUD_MIXED_CASE);
                    name_drawn = true;
                }
            }

            if (!name_drawn && !name_crowded && c_ship && c_iff < 10 && !arcade) {
                Rect contact_rect(x+8, y+6, 120, 12);
                DrawHUDText(TXT_CONTACT_NAME+index, c_ship->Name(), contact_rect, DT_LEFT, HUD_MIXED_CASE);
            }
        }
    }

    if (contact->Threat(ship) && !ship->IsStarship()) {
        if (threat < 1 && c_ship && !c_ship->IsStarship())
        threat = 1;

        if (c_shot)
        threat = 2;
    }
}

// +--------------------------------------------------------------------+

void
HUDView::DrawTrackSegment(Point& t1, Point& t2, Color c)
{
    int   x1, y1, x2, y2;

    projector->Transform(t1);
    projector->Transform(t2);

    const double CLIP_Z = 0.1;

    if (t1.z < CLIP_Z && t2.z < CLIP_Z)
    return;

    if (t1.z < CLIP_Z && t2.z >= CLIP_Z) {
        double dx = t2.x - t1.x;
        double dy = t2.y - t1.y;
        double s  = (CLIP_Z - t1.z) / (t2.z - t1.z);

        t1.x += dx * s;
        t1.y += dy * s;
        t1.z  = CLIP_Z;
    }

    else if (t2.z < CLIP_Z && t1.z >= CLIP_Z) {
        double dx = t1.x - t2.x;
        double dy = t1.y - t2.y;
        double s  = (CLIP_Z - t2.z) / (t1.z - t2.z);

        t2.x += dx * s;
        t2.y += dy * s;
        t2.z  = CLIP_Z;
    }

    if (t1.z >= CLIP_Z && t2.z >= CLIP_Z) {
        projector->Project(t1, false);
        projector->Project(t2, false);

        x1 = (int) t1.x;
        y1 = (int) t1.y;
        x2 = (int) t2.x;
        y2 = (int) t2.y;

        if (window->ClipLine(x1,y1,x2,y2))
        window->DrawLine(x1,y1,x2,y2,c);
    }
}

void
HUDView::DrawTrack(Contact* contact)
{
    Ship* c_ship = contact->GetShip();

    if (c_ship && c_ship->GetFlightPhase() < Ship::ACTIVE)
    return;

    int   ctl   = contact->TrackLength();
    Color c     = MarkerColor(contact);   

    Point t1 = contact->Location();
    Point t2 = contact->TrackPoint(0);

    if (t1 != t2)
    DrawTrackSegment(t1, t2, c);

    for (int i = 0; i < ctl-1; i++) {
        t1 = contact->TrackPoint(i);
        t2 = contact->TrackPoint(i+1);

        if (t1 != t2)
        DrawTrackSegment(t1, t2, c * ((double) (ctl-i)/ (double) ctl));
    }
}

// +--------------------------------------------------------------------+

void
HUDView::DrawRect(SimObject* targ)
{
    Graphic*    g = targ->Rep();
    Rect        r = g->ScreenRect();
    Color       c;

    if (targ->Type() == SimObject::SIM_SHIP)
    c = ((Ship*) targ)->MarkerColor();
    else
    c = ((Shot*) targ)->MarkerColor();
    
    if (r.w > 0 && r.h > 0) {
        if (r.w < 8) {
            r.x -= (8-r.w)/2;
            r.w = 8;
        }
        
        if (r.h < 8) {
            r.y -= (8-r.h)/2;
            r.h = 8;
        }
    }

    else {
        Point mark_pt = targ->Location();
        projector->Transform(mark_pt);

        // clip:
        if (mark_pt.z < 1.0)
        return;
        
        projector->Project(mark_pt);
        
        int x = (int) mark_pt.x;
        int y = (int) mark_pt.y;
        
        if (x < 4 || x > width-4 || y < 4 || y > height-4)
        return;
        
        r.x = x-4;
        r.y = y-4;
        r.w = 8;
        r.h = 8;   
    }

    // horizontal
    window->DrawLine(r.x, r.y, r.x+8, r.y, c);
    window->DrawLine(r.x+r.w-8, r.y, r.x+r.w, r.y, c);
    window->DrawLine(r.x, r.y+r.h, r.x+8, r.y+r.h, c);
    window->DrawLine(r.x+r.w-8, r.y+r.h, r.x+r.w, r.y+r.h, c);
    // vertical
    window->DrawLine(r.x, r.y, r.x, r.y+8, c);
    window->DrawLine(r.x, r.y+r.h-8, r.x, r.y+r.h, c);
    window->DrawLine(r.x+r.w, r.y, r.x+r.w, r.y+8, c);
    window->DrawLine(r.x+r.w, r.y+r.h-8, r.x+r.w, r.y+r.h, c);
}

// +--------------------------------------------------------------------+

void
HUDView::DrawBars()
{
    fpm_sprite->Hide();
    hpm_sprite->Hide();
    lead_sprite->Hide();
    aim_sprite->Hide();
    tgt1_sprite->Hide();
    tgt2_sprite->Hide();
    tgt3_sprite->Hide();
    tgt4_sprite->Hide();
    chase_sprite->Hide();

    for (int i = 0; i < 31; i++)
    pitch_ladder[i]->Hide();

    const int   bar_width  = 256;
    const int   bar_height = 192;
    const int   box_width  = 120;

    int  cx       = width/2;
    int  cy       = height/2;
    int  l        = cx - bar_width/2;
    int  r        = cx + bar_width/2;
    int  t        = cy - bar_height/2;
    int  b        = cy + bar_height/2;
    int  align    = DT_LEFT;

    if (Game::Paused())
    DrawHUDText(TXT_PAUSED, Game::GetText("HUDView.PAUSED"), Rect(cx-128, cy-60, 256, 12), DT_CENTER);

    if (ship) {
        DrawContactMarkers();

        char   txt[256];
        double speed = ship->Velocity().length();

        if (ship->Velocity() * ship->Heading() < 0)
        speed = -speed;
        
        FormatNumber(txt, speed);

        if (tactical) {
            l = box_width + 16;
            r = width - box_width - 16;
        }

        Rect speed_rect(l-box_width-8, cy-5, box_width, 12);

        align = (tactical) ? DT_LEFT : DT_RIGHT;
        DrawHUDText(TXT_SPEED, txt, speed_rect, align);

        // upper left hud quadrant (airborne fighters)
        if (ship->IsAirborne()) {
            double alt_msl = ship->AltitudeMSL();
            double alt_agl = ship->AltitudeAGL();

            if (alt_agl <= 1000)
            sprintf_s(txt, "R %4d", (int) alt_agl);
            else
            FormatNumber(txt, alt_msl);

            speed_rect.y -= 20;

            if (arcade) {
                char arcade_txt[32];
                sprintf_s(arcade_txt, "%s %s", Game::GetText("HUDView.altitude").data(), txt);
                align = (tactical) ? DT_LEFT : DT_RIGHT;
                DrawHUDText(TXT_ALTITUDE, arcade_txt, speed_rect, align);
            }
            else {
                align = (tactical) ? DT_LEFT : DT_RIGHT;
                DrawHUDText(TXT_ALTITUDE, txt, speed_rect, align);
            }

            if (!arcade) {
                sprintf_s(txt, "%.1f G", ship->GForce());
                speed_rect.y -= 20;

                align = (tactical) ? DT_LEFT : DT_RIGHT;
                DrawHUDText(TXT_GFORCE, txt, speed_rect, align);

                speed_rect.y += 40;
            }
        }

        // upper left hud quadrant (starships)
        else if (ship->IsStarship() && ship->GetFLCSMode() == Ship::FLCS_HELM && !arcade) {
            sprintf_s(txt, "%s: %.1f", Game::GetText("HUDView.Pitch").data(), ship->GetHelmPitch()/DEGREES);
            speed_rect.y -= 50;

            align = (tactical) ? DT_LEFT : DT_RIGHT;
            DrawHUDText(TXT_PITCH, txt, speed_rect, align);

            speed_rect.y -= 10;
            int  heading_degrees = (int) (ship->GetHelmHeading()/DEGREES);
            if (heading_degrees < 0) heading_degrees += 360;
            sprintf_s(txt, "%s: %03d", Game::GetText("HUDView.Heading").data(), heading_degrees);
            DrawHUDText(TXT_HEADING, txt, speed_rect, align);

            speed_rect.y += 60;
        }

        // per user request, all ships should show compass heading
        if (!tactical && !arcade) {
            Rect heading_rect(l, t+5, bar_width, 12);
            int  heading_degrees = (int) (ship->CompassHeading()/DEGREES);
            if (heading_degrees < 0) heading_degrees += 360;
            sprintf_s(txt, "%d", heading_degrees);
            DrawHUDText(TXT_COMPASS, txt, heading_rect, DT_CENTER);
        }

        switch (mode) {
        case HUD_MODE_TAC: strcpy_s(txt, Game::GetText("HUDView.mode.tactical").data());   break;
        case HUD_MODE_NAV: strcpy_s(txt, Game::GetText("HUDView.mode.navigation").data()); break;
        case HUD_MODE_ILS: strcpy_s(txt, Game::GetText("HUDView.mode.landing").data());    break;
        }

        if (tactical) {
            speed_rect.y += 76;
            align = DT_LEFT;
        }
        else {
            speed_rect.y = cy+76;
            align = DT_RIGHT;
        }

        DrawHUDText(TXT_HUD_MODE, txt, speed_rect, align);

        // landing gear:
        if (ship->IsGearDown()) {
            const char* gear_down = Game::GetText("HUDView.gear-down");

            Rect gear_rect(l, b+20, box_width, 12);
            DrawHUDText(TXT_GEAR_DOWN, gear_down, gear_rect, DT_CENTER, HUD_UPPER_CASE, true);
        }

        // sensor/missile lock warnings and quantum drive countdown:
        QuantumDrive* quantum = ship->GetQuantumDrive();

        if (threat || (quantum && quantum->JumpTime() > 0)) {
            const char* threat_warn = Game::GetText("HUDView.threat-warn");
            bool        show_msg = true;

            if (quantum && quantum->JumpTime() > 0) {
                static char buf[64];
                sprintf_s(buf, "%s: %d", Game::GetText("HUDView.quantum-jump").data(), (int) quantum->JumpTime());
                threat_warn = buf;
            }

            else if (threat > 1) {
                threat_warn = Game::GetText("HUDView.missile-warn");
                show_msg = ((Game::RealTime()/500) & 1) != 0;
            }

            if (show_msg) {
                Rect lock_rect(l, t-25, box_width, 12);
                DrawHUDText(TXT_THREAT_WARN, threat_warn, lock_rect, DT_CENTER, HUD_MIXED_CASE, true);
            }
        }

        if (ship->CanTimeSkip()) {
            Rect auto_rect(l, t-40, box_width, 12);
            DrawHUDText(TXT_AUTO, Game::GetText("HUDView.AUTO"), auto_rect, DT_CENTER, HUD_MIXED_CASE, true);
        }

        if (mode == HUD_MODE_NAV) {
            Instruction* next = ship->GetNextNavPoint();
            
            if (next) {
                double distance = ship->RangeToNavPoint(next);
                FormatNumber(txt, distance);

                Rect range_rect(r-20, cy-5, box_width, 12);
                DrawHUDText(TXT_RANGE, txt, range_rect, DT_RIGHT);
                range_rect.Inflate(2,2);
            }
        }

        // lower left hud quadrant
        else if (mode == HUD_MODE_TAC) {
            speed_rect.x = l-box_width-8;
            speed_rect.y = cy-5 +20;
            speed_rect.w = box_width;
            align = (tactical) ? DT_LEFT : DT_RIGHT;

            if (!arcade && ship->GetPrimary() && !ship->IsNetObserver())
            DrawHUDText(TXT_PRIMARY_WEP, ship->GetPrimary()->Abbreviation(), speed_rect, align);

            WeaponGroup* missile = ship->GetSecondaryGroup();

            if (missile && missile->Ammo() > 0 && !ship->IsNetObserver()) {
                if (!arcade) {
                    speed_rect.y = cy-5 +30;
                    sprintf_s(txt, "%s %d", missile->Name(), missile->Ammo());
                    DrawHUDText(TXT_SECONDARY_WEP, txt, speed_rect, align);
                }

                // missile range indicator
                if (missile->GetSelected()->Locked()) {
                    Rect shoot_rect(l, b+5, box_width, 12);
                    DrawHUDText(TXT_SHOOT, Game::GetText("HUDView.SHOOT"), shoot_rect, DT_CENTER, HUD_MIXED_CASE, true);
                }
            }

            if (!arcade && !ship->IsNetObserver()) {
                if (ship->GetShield()) {
                    speed_rect.y = cy-5+40;
                    sprintf_s(txt, "%s - %03d +", Game::GetText("HUDView.SHIELD").data(), ship->ShieldStrength());
                    DrawHUDText(TXT_SHIELD, txt, speed_rect, align);
                }
                else if (ship->GetDecoy()) {
                    speed_rect.y = cy-5+40;
                    sprintf_s(txt, "%s %d", Game::GetText("HUDView.DECOY").data(), ship->GetDecoy()->Ammo());
                    DrawHUDText(TXT_DECOY, txt, speed_rect, align);
                }


                Rect eta_rect = speed_rect;
                eta_rect.y += 10;

                align = DT_RIGHT;

                if (tactical) {
                    eta_rect.x = 8;
                    align  = DT_LEFT;
                }

                for (int i = 0; i < 2; i++) {
                    int eta = ship->GetMissileEta(i);
                    if (eta > 0) {
                        int minutes = (eta/60) % 60;
                        int seconds = (eta   ) % 60;

                        char eta_buf[16];
                        sprintf_s(eta_buf, "T %d:%02d", minutes, seconds);
                        DrawHUDText(TXT_MISSILE_T1+i, eta_buf, eta_rect, align);
                        eta_rect.y += 10;
                    }
                }
            }

            NetGame* netgame = NetGame::GetInstance();
            if (netgame && !netgame->IsActive()) {
                Rect shoot_rect(l, b+5, box_width, 12);
                DrawHUDText(TXT_SHOOT, Game::GetText("HUDView.NET-GAME-OVER"), shoot_rect, DT_CENTER, HUD_MIXED_CASE, true);
            }

            else if (ship->IsNetObserver()) {
                Rect shoot_rect(l, b+5, box_width, 12);
                DrawHUDText(TXT_SHOOT, Game::GetText("HUDView.OBSERVER"), shoot_rect, DT_CENTER, HUD_MIXED_CASE, true);
            }

            DrawTarget();
        }

        else if (mode == HUD_MODE_ILS) {
            DrawTarget();
        }

        DrawNavInfo();
    }
}

// +--------------------------------------------------------------------+

void
HUDView::DrawFPM()
{
    fpm_sprite->Hide();

    if (ship->Velocity().length() > 50) {
        double xtarg = xcenter;
        double ytarg = ycenter;

        Point svel = ship->Velocity();
        svel.Normalize();

        Point tloc = ship->Location() + svel * 1e8;
        // Translate into camera relative:
        projector->Transform(tloc);

        int behind = tloc.z < 0;

        if (behind)
        return;

        // Project into screen coordinates:
        projector->Project(tloc);

        xtarg = tloc.x;
        ytarg = tloc.y;

        fpm_sprite->Show();
        fpm_sprite->MoveTo(Point(xtarg, ytarg, 1));
    }
}

// +--------------------------------------------------------------------+

void
HUDView::DrawPitchLadder()
{
    for (int i = 0; i < 31; i++)
    pitch_ladder[i]->Hide();

    if (ship->IsAirborne() && Game::MaxTexSize() > 128) {
        double xtarg = xcenter;
        double ytarg = ycenter;

        Point uvec = Point(0,1,0);
        Point svel = ship->Velocity();

        if (svel.length() == 0)
        svel = ship->Heading();

        if (svel.x == 0 && svel.z == 0)
        return;

        svel.y = 0;
        svel.Normalize();

        Point gloc = ship->Location();
        gloc.y = 0;

        const double baseline   = 1e9;
        const double clip_angle = 20*DEGREES;

        Point tloc = gloc + svel * baseline;

        // Translate into camera relative:
        projector->Transform(tloc);

        // Project into screen coordinates:
        projector->Project(tloc);

        xtarg = tloc.x;
        ytarg = tloc.y;

        // compute roll angle:
        double roll_angle  = 0;
        double pitch_angle = 0;

        Point heading = ship->Heading();
        heading.Normalize();

        if (heading.x != 0 || heading.z != 0) {
            Point gheading = heading;
            gheading.y = 0;
            gheading.Normalize();

            double dot   = gheading * heading;

            if (heading.y < 0) dot = -dot;

            pitch_angle = acos(dot);

            if (pitch_angle > PI/2)
            pitch_angle -= PI;

            double s0    = sin(pitch_angle);
            double c0    = cos(pitch_angle);
            double s1    = sin(pitch_angle + 10*DEGREES);
            double c1    = cos(pitch_angle + 10*DEGREES);

            tloc = gloc + (svel * baseline * c0) + (uvec * baseline * s0);
            projector->Transform(tloc);

            double x0    = tloc.x;
            double y0    = tloc.y;

            tloc = gloc + (svel * baseline * c1) + (uvec * baseline * s1);
            projector->Transform(tloc);

            double x1    = tloc.x;
            double y1    = tloc.y;

            double dx    = x1-x0;
            double dy    = y1-y0;

            roll_angle   = atan2(-dy,dx) + PI/2;
        }

        const double alias_limit = 0.1*DEGREES;

        if (fabs(roll_angle) <= alias_limit) {
            if (roll_angle > 0)
            roll_angle = alias_limit;
            else
            roll_angle = -alias_limit;
        }

        else if (fabs(roll_angle-PI) <= alias_limit) {
            roll_angle = PI - alias_limit;
        }

        if (fabs(pitch_angle) <= clip_angle) {
            pitch_ladder[15]->Show();
            pitch_ladder[15]->MoveTo(Point(xtarg, ytarg, 1));
            pitch_ladder[15]->SetAngle(roll_angle);
        }

        for (int i = 1; i <= 15; i++) {
            double angle = i * 5 * DEGREES;

            if (i > 12)
            angle = (60 + (i-12)*10) * DEGREES;

            double s     = sin(angle);
            double c     = cos(angle);

            if (fabs(pitch_angle - angle) <= clip_angle) {
                // positive angle:
                tloc = gloc + (svel * baseline * c) + (uvec * baseline * s);
                projector->Transform(tloc);

                if (tloc.z > 0) {
                    projector->Project(tloc);
                    pitch_ladder[15-i]->Show();
                    pitch_ladder[15-i]->MoveTo(Point(tloc.x, tloc.y, 1));
                    pitch_ladder[15-i]->SetAngle(roll_angle);
                }
            }

            if (fabs(pitch_angle + angle) <= clip_angle) {
                // negative angle:
                tloc = gloc + (svel * baseline * c) + (uvec * -baseline * s);
                projector->Transform(tloc);

                if (tloc.z > 0) {
                    projector->Project(tloc);
                    pitch_ladder[15+i]->Show();
                    pitch_ladder[15+i]->MoveTo(Point(tloc.x, tloc.y, 1));
                    pitch_ladder[15+i]->SetAngle(roll_angle);
                }
            }
        }
    }
}

// +--------------------------------------------------------------------+

void
HUDView::DrawHPM()
{
    hpm_sprite->Hide();

    if (!ship)
    return;

    double xtarg = xcenter;
    double ytarg = ycenter;

    double az = ship->GetHelmHeading() - PI;
    double el = ship->GetHelmPitch();

    Point hvec = Point(sin(az), sin(el), cos(az));
    hvec.Normalize();

    Point tloc = ship->Location() + hvec * 1e8;
    // Translate into camera relative:
    projector->Transform(tloc);

    int behind = tloc.z < 0;

    if (behind)
    return;

    // Project into screen coordinates:
    projector->Project(tloc);

    xtarg = tloc.x;
    ytarg = tloc.y;

    hpm_sprite->Show();
    hpm_sprite->MoveTo(Point(xtarg, ytarg, 1));
}

// +--------------------------------------------------------------------+

void
HUDView::HideCompass()
{
    az_ring->Hide();
    az_pointer->Hide();
    el_ring->Hide();
    el_pointer->Hide();

    Scene* scene = az_ring->GetScene();
    if (scene) {
        scene->DelGraphic(az_ring);
        scene->DelGraphic(az_pointer);
        scene->DelGraphic(el_ring);
        scene->DelGraphic(el_pointer);
    }
}

// +--------------------------------------------------------------------+

void
HUDView::DrawCompass()
{
    if (!ship || !ship->Rep())
    return;

    Solid* solid = (Solid*) ship->Rep();
    Point    loc   = solid->Location();

    az_ring->MoveTo(loc);
    az_pointer->MoveTo(loc);
    el_ring->MoveTo(loc);
    el_pointer->MoveTo(loc);

    double helm_heading = ship->GetHelmHeading();
    double helm_pitch   = ship->GetHelmPitch();
    double curr_heading = ship->CompassHeading();
    double curr_pitch   = ship->CompassPitch();

    bool show_az = fabs(helm_heading - curr_heading) > 5*DEGREES;
    bool show_el = fabs(helm_pitch   - curr_pitch)   > 5*DEGREES;

    Scene* scene = camview->GetScene();

    if (show_az || show_el) {
        scene->AddGraphic(az_ring);
        az_ring->Show();

        if (show_el || fabs(helm_pitch) > 5 * DEGREES) {
            scene->AddGraphic(el_ring);
            Matrix ring_orient;
            ring_orient.Yaw(helm_heading + PI);
            el_ring->SetOrientation(ring_orient);
            el_ring->Show();

            scene->AddGraphic(el_pointer);
            Matrix pointer_orient;
            pointer_orient.Yaw(helm_heading + PI);
            pointer_orient.Pitch(-helm_pitch);
            pointer_orient.Roll(PI/2);
            el_pointer->SetOrientation(pointer_orient);
            el_pointer->Show();
        }
        else {
            scene->AddGraphic(az_pointer);
            Matrix pointer_orient;
            pointer_orient.Yaw(helm_heading + PI);

            az_pointer->SetOrientation(pointer_orient);
            az_pointer->Show();
        }
    }
}

// +--------------------------------------------------------------------+

void
HUDView::DrawLCOS(SimObject* targ, double dist)
{
    lead_sprite->Hide();
    aim_sprite->Hide();
    chase_sprite->Hide();

    double xtarg = xcenter;
    double ytarg = ycenter;

    Weapon* prim = ship->GetPrimary();
    if (!prim) return;

    Point tloc = targ->Location();
    // Translate into camera relative:
    projector->Transform(tloc);

    int behind = tloc.z < 0;

    if (behind)
    tloc.z = -tloc.z;

    // Project into screen coordinates:
    projector->Project(tloc);

    // DRAW THE OFFSCREEN CHASE INDICATOR:
    if (behind     || 
            tloc.x <= 0 || tloc.x >= width-1 || 
            tloc.y <= 0 || tloc.y >= height-1) {

        // Left side:
        if (tloc.x <= 0 || (behind && tloc.x < width/2)) {
            if (tloc.y < ah) tloc.y = ah;
            else if (tloc.y >= height-ah) tloc.y = height-1-ah;

            chase_sprite->Show();
            chase_sprite->SetAnimation(&chase_left);
            chase_sprite->MoveTo(Point(aw, tloc.y, 1));
        }
        
        // Right side:
        else if (tloc.x >= width-1 || behind) {
            if (tloc.y < ah) tloc.y = ah;
            else if (tloc.y >= height-ah) tloc.y = height-1-ah;

            chase_sprite->Show();
            chase_sprite->SetAnimation(&chase_right);
            chase_sprite->MoveTo(Point(width-1-aw, tloc.y, 1));
        }
        else {
            if (tloc.x < aw) tloc.x = aw;
            else if (tloc.x >= width-aw) tloc.x = width-1-aw;

            // Top edge:
            if (tloc.y <= 0) {
                chase_sprite->Show();
                chase_sprite->SetAnimation(&chase_top);
                chase_sprite->MoveTo(Point(tloc.x, ah, 1));
            }
            
            // Bottom edge:
            else if (tloc.y >= height-1) {
                chase_sprite->Show();
                chase_sprite->SetAnimation(&chase_bottom);
                chase_sprite->MoveTo(Point(tloc.x, height-1-ah, 1));
            }
        }
    }

    // DRAW THE LCOS:
    else {
        if (!ship->IsStarship()) {
            Point aim_vec = ship->Heading();
            aim_vec.Normalize();

            // shot speed is relative to ship speed:
            Point shot_vel = ship->Velocity() + aim_vec * prim->Design()->speed;
            double shot_speed = shot_vel.length();

            // time for shot to reach target
            double time    = dist / shot_speed;

            // LCOS (Lead Computing Optical Sight)
            if (gunsight == 0) {
                // where the shot will be when it is the same distance
                // away from the ship as the target:
                Point impact   = ship->Location() + (shot_vel * time);

                // where the target will be when the shot reaches it:
                Point targ_vel = targ->Velocity();
                Point dest     = targ->Location() + (targ_vel * time);
                Point delta    = impact - dest;

                // draw the gun sight here in 3d world coordinates:
                Point sight    = targ->Location() + delta;

                // Project into screen coordinates:
                projector->Transform(sight);
                projector->Project(sight);

                xtarg = sight.x;
                ytarg = sight.y;

                aim_sprite->Show();
                aim_sprite->MoveTo(Point(xtarg, ytarg, 1));
            }

            // Wing Commander style lead indicator
            else {
                // where the target will be when the shot reaches it:
                Point targ_vel = targ->Velocity() - ship->Velocity();
                Point dest     = targ->Location() + (targ_vel * time);

                // Translate into camera relative:
                projector->Transform(dest);
                projector->Project(dest);

                xtarg = dest.x;
                ytarg = dest.y;

                lead_sprite->Show();
                lead_sprite->MoveTo(Point(xtarg, ytarg, 1));
            }
        }
    }
}

// +--------------------------------------------------------------------+

void
HUDView::DrawTarget()
{
    const int   bar_width  = 256;
    const int   bar_height = 192;
    const int   box_width  = 120;

    SimObject* old_target = target;

    if (mode == HUD_MODE_ILS) {
        Ship* controller = ship->GetController();
        if (controller && !target)
        target = controller;
    }

    if (target && target->Rep()) {
        Sensor*  sensor  = ship->GetSensor();
        Contact* contact = 0;

        if (sensor && target->Type() == SimObject::SIM_SHIP) {
            contact = sensor->FindContact((Ship*) target);
        }

        int    cx       = width/2;
        int    cy       = height/2;
        int    l        = cx - bar_width/2;
        int    r        = cx + bar_width/2;
        int    t        = cy - bar_height/2;
        int    b        = cy + bar_height/2;
        Point  delta    = target->Location() - ship->Location();
        double distance = delta.length();
        Point  delta_v  = ship->Velocity() - target->Velocity();
        double speed    = delta_v.length();
        char   txt[256];

        if (mode == HUD_MODE_ILS && ship->GetInbound() && ship->GetInbound()->GetDeck()) {
            delta    = ship->GetInbound()->GetDeck()->EndPoint() - ship->Location();
            distance = delta.length();
        }

        if (delta * ship->Velocity() > 0) {       // in front
            if (delta_v * ship->Velocity() < 0)    // losing ground
            speed = -speed;
        }
        else {                                    // behind
            if (delta_v * ship->Velocity() > 0)    // passing
            speed = -speed;
        }

        Rect range_rect(r-20, cy-5, box_width, 12);

        if (tactical)
        range_rect.x = width - range_rect.w - 8;

        if (contact) {
            Sensor* sensor = ship->GetSensor();
            double  limit  = 75e3;

            if (sensor)
            limit = sensor->GetBeamRange();

            distance = contact->Range(ship, limit);

            if (!contact->ActLock() && !contact->PasLock()) {
                strcpy_s(txt, Game::GetText("HUDView.No-Range").data());
                speed = 0;
            }
            else {
                FormatNumber(txt, distance);
            }
        }

        else {
            FormatNumber(txt, distance);
        }

        DrawHUDText(TXT_RANGE, txt, range_rect, DT_RIGHT);

        if (arcade) {
            target = old_target;
            return;
        }

        range_rect.y += 18;
        FormatNumber(txt, speed);
        DrawHUDText(TXT_CLOSING_SPEED, txt, range_rect, DT_RIGHT);

        // target info:
        if (!tactical) {
            range_rect.y = cy-76;
        }

        else {
            range_rect.x = width - 2*box_width - 8;
            range_rect.y = cy-76;
            range_rect.w = 2*box_width;
        }

        DrawHUDText(TXT_TARGET_NAME, target->Name(), range_rect, DT_RIGHT);

        if (target->Type() == SimObject::SIM_SHIP) {
            Ship* tgt_ship = (Ship*) target;

            range_rect.y += 10;
            DrawHUDText(TXT_TARGET_DESIGN, tgt_ship->Design()->display_name, range_rect, DT_RIGHT);

            if (mode != HUD_MODE_ILS) {
                if (tgt_ship->IsStarship()) {
                    range_rect.y += 10;
                    sprintf_s(txt, "%s %03d", Game::GetText("HUDView.symbol.shield").data(), (int) tgt_ship->ShieldStrength());
                    DrawHUDText(TXT_TARGET_SHIELD, txt, range_rect, DT_RIGHT);
                }

                range_rect.y += 10;
                sprintf_s(txt, "%s %03d", Game::GetText("HUDView.symbol.hull").data(), (int) (tgt_ship->Integrity() / tgt_ship->Design()->integrity * 100));
                DrawHUDText(TXT_TARGET_HULL, txt, range_rect, DT_RIGHT);

                System* sys = ship->GetSubTarget();
                if (sys) {
                    Color          stat  = hud_color;
                    static DWORD   blink = Game::RealTime();

                    int blink_delta = Game::RealTime() - blink;
                    sprintf_s(txt, "%s %03d", sys->Abbreviation(), (int) sys->Availability());

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

                    range_rect.y += 10;
                    DrawHUDText(TXT_TARGET_SUB, txt, range_rect, DT_RIGHT);
                }
            }
        }

        else if (target->Type() == SimObject::SIM_DRONE) {
            Drone* tgt_drone = (Drone*) target;

            range_rect.y += 10;
            DrawHUDText(TXT_TARGET_DESIGN, tgt_drone->DesignName(), range_rect, DT_RIGHT);

            range_rect.y += 10;
            int eta = tgt_drone->GetEta();

            if (eta > 0) {
                int minutes = (eta/60) % 60;
                int seconds = (eta   ) % 60;

                char eta_buf[16];
                sprintf_s(eta_buf, "T %d:%02d", minutes, seconds);
                DrawHUDText(TXT_TARGET_ETA, eta_buf, range_rect, DT_RIGHT);
            }
        }
    }

    target = old_target;
}

// +--------------------------------------------------------------------+

void
HUDView::DrawNavInfo()
{
    const int   bar_width  = 256;
    const int   bar_height = 192;
    const int   box_width  = 120;

    if (arcade) {
        if (ship->IsAutoNavEngaged()) {
            Rect info_rect(width/2-box_width, height/2+bar_height, box_width*2, 12);
            
            if (big_font) 
            hud_text[TXT_NAV_INDEX].font = big_font;

            DrawHUDText(TXT_NAV_INDEX, Game::GetText("HUDView.Auto-Nav"), info_rect, DT_CENTER);
        }

        return;
    }

    hud_text[TXT_NAV_INDEX].font = hud_font;

    Instruction* navpt = ship->GetNextNavPoint();

    if (navpt) {
        int    cx       = width/2;
        int    cy       = height/2;
        int    l        = cx - bar_width/2;
        int    r        = cx + bar_width/2;
        int    t        = cy - bar_height/2;
        int    b        = cy + bar_height/2;

        int    index    = ship->GetNavIndex(navpt);
        double distance = ship->RangeToNavPoint(navpt);
        double speed    = ship->Velocity().length();
        int    etr      = 0;
        char   txt[256];
        
        if (speed > 10)
        etr      = (int) (distance/speed);

        Rect info_rect(r-20, cy+32, box_width, 12);

        if (tactical)
        info_rect.x = width - info_rect.w - 8;

        if (ship->IsAutoNavEngaged())
        sprintf_s(txt, "%s %d", Game::GetText("HUDView.Auto-Nav").data(), index);
        else
        sprintf_s(txt, "%s %d", Game::GetText("HUDView.Nav").data(),      index);
        DrawHUDText(TXT_NAV_INDEX, txt, info_rect, DT_RIGHT);

        info_rect.y += 10;
        if (navpt->Action())
        DrawHUDText(TXT_NAV_ACTION, Instruction::ActionName(navpt->Action()), info_rect, DT_RIGHT);

        info_rect.y += 10;
        FormatNumber(txt, navpt->Speed());
        DrawHUDText(TXT_NAV_SPEED, txt, info_rect, DT_RIGHT);

        if (etr > 3600) {
            info_rect.y += 10;
            sprintf_s(txt, "%s XX:XX", Game::GetText("HUDView.time-enroute").data());
            DrawHUDText(TXT_NAV_ETR, txt, info_rect, DT_RIGHT);
        }
        else if (etr > 0) {
            info_rect.y += 10;

            int minutes = (etr/60) % 60;
            int seconds = (etr   ) % 60;
            sprintf_s(txt, "%s %2d:%02d", Game::GetText("HUDView.time-enroute").data(), minutes, seconds);
            DrawHUDText(TXT_NAV_ETR, txt, info_rect, DT_RIGHT);
        }

        if (navpt->HoldTime() > 0) {
            info_rect.y += 10;

            int hold    = (int) navpt->HoldTime();
            int minutes = (hold/60) % 60;
            int seconds = (hold   ) % 60;
            sprintf_s(txt, "%s %2d:%02d", Game::GetText("HUDView.HOLD").data(), minutes, seconds);
            DrawHUDText(TXT_NAV_HOLD, txt, info_rect, DT_RIGHT);
        }
    }
}

// +--------------------------------------------------------------------+

void
HUDView::DrawSight()
{
    if (target && target->Rep()) {
        Point  delta    = target->Location() - ship->Location();
        double distance = delta.length();

        // draw LCOS on target:
        if (!tactical)
        DrawLCOS(target, distance);
    }
}

// +--------------------------------------------------------------------+

void
HUDView::DrawDesignators()
{
    double      xtarg  = xcenter;
    double      ytarg  = ycenter;
    SimObject*  t1     = 0;
    SimObject*  t2     = 0;
    SimObject*  t3     = 0;
    Sprite*   sprite = 0;

    tgt1_sprite->Hide();
    tgt2_sprite->Hide();
    tgt3_sprite->Hide();
    tgt4_sprite->Hide();

    // fighters just show primary target:
    if (ship->IsDropship()) {
        SimObject* t = ship->GetTarget();
        System*    s = ship->GetSubTarget();

        if (t) {
            Point  tloc = t->Location();
            if (s) {
                tloc = s->MountLocation();
            }
            else if (t->Type() == SimObject::SIM_SHIP) {
                Ship* tgt_ship = (Ship*) t;

                if (tgt_ship->IsGroundUnit())
                tloc += Point(0,150,0);
            }

            projector->Transform(tloc);

            if (tloc.z > 0) {
                projector->Project(tloc);

                xtarg = tloc.x;
                ytarg = tloc.y;

                if (xtarg>0 && xtarg<width-1 && ytarg>0 && ytarg<height-1) {
                    double range = Point(t->Location() - ship->Location()).length();

                    // use out-of-range crosshair if out of range:
                    if (!ship->GetPrimaryDesign() || ship->GetPrimaryDesign()->max_range < range) {
                        tgt4_sprite->Show();
                        tgt4_sprite->MoveTo(Point(xtarg, ytarg, 1));
                    }

                    // else, use in-range primary crosshair:
                    else {
                        tgt1_sprite->Show();
                        tgt1_sprite->MoveTo(Point(xtarg, ytarg, 1));
                    }
                }
            }
        }
    }

    // starships show up to three targets:
    else {
        ListIter<WeaponGroup> w = ship->Weapons();
        while (!t3 && ++w) {
            SimObject* t = w->GetTarget();
            System*    s = w->GetSubTarget();

            if (w->Contains(ship->GetPrimary())) {
                if (t == 0) t = ship->GetTarget();
                t1     = t;
                sprite = tgt1_sprite;
            }

            else if (t && w->Contains(ship->GetSecondary())) {
                t2     = t;
                sprite = tgt2_sprite;

                if (t2 == t1)
                continue;   // don't overlap target designators
            }

            else if (t) {
                t3     = t;
                sprite = tgt3_sprite;

                if (t3 == t1 || t3 == t2)
                continue;   // don't overlap target designators
            }

            if (t) {
                Point  tloc = t->Location();

                if (s)
                tloc = s->MountLocation();

                projector->Transform(tloc);

                if (tloc.z > 0) {
                    projector->Project(tloc);

                    xtarg = tloc.x;
                    ytarg = tloc.y;

                    if (xtarg>0 && xtarg<width-1 && ytarg>0 && ytarg<height-1) {
                        double range = Point(t->Location() - ship->Location()).length();

                        // flip to out-of-range crosshair
                        if (sprite == tgt1_sprite) {
                            if (!ship->GetPrimaryDesign() || ship->GetPrimaryDesign()->max_range < range) {
                                sprite = tgt4_sprite;
                            }
                        }

                        sprite->Show();
                        sprite->MoveTo(Point(xtarg, ytarg, 1));
                    }
                }
            }
        }
    }
}

// +--------------------------------------------------------------------+

Color
HUDView::GetStatusColor(System::STATUS status)
{
    Color sc;

    switch (status) {
    default:
    case System::NOMINAL:     sc = Color( 32,192, 32);  break;
    case System::DEGRADED:    sc = Color(255,255,  0);  break;
    case System::CRITICAL:    sc = Color(255,  0,  0);  break;
    case System::DESTROYED:   sc = Color(  0,  0,  0);  break;
    }

    return sc;
}

void
HUDView::SetStatusColor(System::STATUS status)
{
    switch (status) {
    default:
    case System::NOMINAL:     status_color = txt_color;           break;
    case System::DEGRADED:    status_color = Color(255,255,  0);  break;
    case System::CRITICAL:    status_color = Color(255,  0,  0);  break;
    case System::DESTROYED:   status_color = Color(  0,  0,  0);  break;
    }
}

// +--------------------------------------------------------------------+

static int GetReactorStatus(Ship* ship)
{
    if (!ship || ship->Reactors().size() < 1)
    return -1;

    int   status = System::NOMINAL;
    bool  maint  = false;

    ListIter<PowerSource> iter = ship->Reactors();
    while (++iter) {
        PowerSource* s = iter.value();

        if (s->Status() < status)
        status = s->Status();
    }

    if (maint && status == System::NOMINAL)
    status = System::MAINT;

    return status;
}

static int GetDriveStatus(Ship* ship)
{
    if (!ship || ship->Drives().size() < 1)
    return -1;

    int   status = System::NOMINAL;
    bool  maint  = false;

    ListIter<Drive> iter = ship->Drives();
    while (++iter) {
        Drive* s = iter.value();

        if (s->Status() < status)
        status = s->Status();

        else if (s->Status() == System::MAINT)
        maint = true;
    }

    if (maint && status == System::NOMINAL)
    status = System::MAINT;

    return status;
}

static int GetQuantumStatus(Ship* ship)
{
    if (!ship || ship->GetQuantumDrive() == 0)
    return -1;

    QuantumDrive* s = ship->GetQuantumDrive();
    return s->Status();
}

static int GetThrusterStatus(Ship* ship)
{
    if (!ship || ship->GetThruster() == 0)
    return -1;

    Thruster* s = ship->GetThruster();
    return s->Status();
}

static int GetShieldStatus(Ship* ship)
{
    if (!ship)
    return -1;

    Shield* s = ship->GetShield();
    Weapon* d = ship->GetDecoy();

    if (!s && !d)
    return -1;

    int   status = System::NOMINAL;
    bool  maint  = false;

    if (s) {
        if (s->Status() < status)
        status = s->Status();

        else if (s->Status() == System::MAINT)
        maint = true;
    }

    if (d) {
        if (d->Status() < status)
        status = d->Status();

        else if (d->Status() == System::MAINT)
        maint = true;
    }

    if (maint && status == System::NOMINAL)
    status = System::MAINT;

    return status;
}

static int GetWeaponStatus(Ship* ship, int index)
{
    if (!ship || ship->Weapons().size() <= index)
    return -1;

    WeaponGroup* group = ship->Weapons().at(index);

    int   status = System::NOMINAL;
    bool  maint  = false;

    ListIter<Weapon> iter = group->GetWeapons();
    while (++iter) {
        Weapon* s = iter.value();

        if (s->Status() < status)
        status = s->Status();

        else if (s->Status() == System::MAINT)
        maint = true;
    }

    if (maint && status == System::NOMINAL)
    status = System::MAINT;

    return status;
}

static int GetSensorStatus(Ship* ship)
{
    if (!ship || ship->GetSensor() == 0)
    return -1;

    Sensor* s = ship->GetSensor();
    Weapon* p = ship->GetProbeLauncher();

    int   status = s->Status();
    bool  maint  = s->Status() == System::MAINT;

    if (p) {
        if (p->Status() < status)
        status = p->Status();

        else if (p->Status() == System::MAINT)
        maint = true;
    }

    if (maint && status == System::NOMINAL)
    status = System::MAINT;

    return status;
}

static int GetComputerStatus(Ship* ship)
{
    if (!ship || ship->Computers().size() < 1)
    return -1;

    int   status = System::NOMINAL;
    bool  maint  = false;

    ListIter<Computer> iter = ship->Computers();
    while (++iter) {
        Computer* s = iter.value();

        if (s->Status() < status)
        status = s->Status();

        else if (s->Status() == System::MAINT)
        maint = true;
    }

    if (ship->GetNavSystem()) {
        NavSystem* s = ship->GetNavSystem();

        if (s->Status() < status)
        status = s->Status();

        else if (s->Status() == System::MAINT)
        maint = true;
    }

    if (maint && status == System::NOMINAL)
    status = System::MAINT;

    return status;
}

static int GetFlightDeckStatus(Ship* ship)
{
    if (!ship || ship->FlightDecks().size() < 1)
    return -1;

    int   status = System::NOMINAL;
    bool  maint  = false;

    ListIter<FlightDeck> iter = ship->FlightDecks();
    while (++iter) {
        FlightDeck* s = iter.value();

        if (s->Status() < status)
        status = s->Status();

        else if (s->Status() == System::MAINT)
        maint = true;
    }

    if (maint && status == System::NOMINAL)
    status = System::MAINT;

    return status;
}

void
HUDView::DrawWarningPanel()
{
    int box_width  = 75;
    int box_height = 17;
    int row_height = 28;
    int box_left   = width/2 - box_width*2;

    if (cockpit_hud_texture) {
        box_left   = 275;
        box_height =  18;
        row_height =  18;
    }

    if (ship) {
        if (Game::MaxTexSize() > 128) {
            warn_left_sprite->Show();
            warn_right_sprite->Show();
        }

        int x = box_left;
        int y = cockpit_hud_texture ? 410 : height-97;
        int c = cockpit_hud_texture ?   3 : 4;
        
        static DWORD blink = Game::RealTime();

        for (int index = 0; index < 12; index++) {
            int  stat = -1;
            Text abrv = Game::GetText("HUDView.UNKNOWN");
            
            switch (index) {
            case 0:  stat = GetReactorStatus(ship);   abrv = Game::GetText("HUDView.REACTOR");    break;
            case 1:  stat = GetDriveStatus(ship);     abrv = Game::GetText("HUDView.DRIVE");   break;
            case 2:  stat = GetQuantumStatus(ship);   abrv = Game::GetText("HUDView.QUANTUM"); break;
            case 3:  stat = GetShieldStatus(ship);    abrv = Game::GetText("HUDView.SHIELD");
                if (ship->GetShield() == 0 && ship->GetDecoy())
                abrv = Game::GetText("HUDView.DECOY");
                break;

            case 4:
            case 5:
            case 6:
            case 7:  stat = GetWeaponStatus(ship, index-4);
                if (stat >= 0) {
                    WeaponGroup* g = ship->Weapons().at(index-4);
                    abrv = g->Name();
                }
                break;

            case 8:  stat = GetSensorStatus(ship);    abrv = Game::GetText("HUDView.SENSOR");  break;
            case 9:  stat = GetComputerStatus(ship);  abrv = Game::GetText("HUDView.COMPUTER");    break;
            case 10: stat = GetThrusterStatus(ship);  abrv = Game::GetText("HUDView.THRUSTER");  break;
            case 11: stat = GetFlightDeckStatus(ship);abrv = Game::GetText("HUDView.FLTDECK"); break;
            }

            Rect warn_rect(x, y, box_width, box_height);

            if (cockpit_hud_texture)
            cockpit_hud_texture->DrawRect(warn_rect, Color::DarkGray);

            if (stat >= 0) {
                SetStatusColor((System::STATUS) stat);
                Color tc = status_color;

                if (stat != System::NOMINAL) {
                    if (Game::RealTime() - blink < 250) {
                        tc = cockpit_hud_texture ? txt_color : Color(8,8,8);
                    }
                }

                if (cockpit_hud_texture) {
                    if (tc != txt_color) {
                        Rect r2 = warn_rect;
                        r2.Inset(1,1,1,1);
                        cockpit_hud_texture->FillRect(r2, tc);
                        tc = Color::Black;
                    }

                    warn_rect.y += 4;
                    
                    hud_font->SetColor(tc);
                    hud_font->DrawText(abrv, -1, 
                    warn_rect, 
                    DT_CENTER | DT_SINGLELINE, 
                    cockpit_hud_texture);
                    
                    warn_rect.y -= 4;
                }
                else {
                    DrawHUDText(TXT_CAUTION_TXT + index,
                    abrv, 
                    warn_rect,
                    DT_CENTER);

                    hud_text[TXT_CAUTION_TXT + index].color = tc;
                }
            }

            x += box_width;

            if (--c <= 0) {
                c = cockpit_hud_texture ? 3 : 4;
                x = box_left;
                y += row_height;
            }
        }

        if (Game::RealTime() - blink > 500)
        blink = Game::RealTime();

        // reset for next time
        SetStatusColor(System::NOMINAL);   
    }
}

// +--------------------------------------------------------------------+

void
HUDView::DrawInstructions()
{
    if (!ship) return;

    if (Game::MaxTexSize() > 128) {
        instr_left_sprite->Show();
        instr_right_sprite->Show();
    }

    int      ninst = 0;
    int      nobj  = 0;
    Element* elem  = ship->GetElement();

    if (elem) {
        ninst = elem->NumInstructions();
        nobj  = elem->NumObjectives();
    }

    Rect r = Rect(width/2 - 143, height - 105, 290, 17);

    if (ninst) {
        int npages = ninst/6 + (ninst%6 ? 1 : 0);

        if (inst_page >= npages)
        inst_page = npages-1;
        else if (inst_page < 0)
        inst_page = 0;

        int first = inst_page * 6;
        int last  = first + 6;
        if (last > ninst) last = ninst;

        int n = TXT_CAUTION_TXT;

        for (int i = first; i < last; i++) {
            hud_text[n].color = standard_txt_colors[color];
            DrawHUDText(n++, FormatInstruction(elem->GetInstruction(i)), r, DT_LEFT, HUD_MIXED_CASE);
            r.y += 14;
        }

        char page[32];
        sprintf_s(page, "%d / %d", inst_page+1, npages);
        r = Rect(width/2 + 40, height-16, 110, 16);
        DrawHUDText(TXT_INSTR_PAGE, page, r, DT_CENTER, HUD_MIXED_CASE);
    }

    else if (nobj) {
        int n = TXT_CAUTION_TXT;

        for (int i = 0; i < nobj; i++) {
            char desc[256];
            sprintf_s(desc, "* %s", elem->GetObjective(i)->GetShortDescription());
            hud_text[n].color = standard_txt_colors[color];
            DrawHUDText(n++, desc, r, DT_LEFT, HUD_MIXED_CASE);
            r.y += 14;
        }
    }

    else {
        hud_text[TXT_CAUTION_TXT].color = standard_txt_colors[color];
        DrawHUDText(TXT_CAUTION_TXT, Game::GetText("HUDView.No-Instructions"), r, DT_LEFT, HUD_MIXED_CASE);
    }
}

// +--------------------------------------------------------------------+

const char*
HUDView::FormatInstruction(Text instr)
{
    if (!instr.contains('$'))
    return (const char*) instr;

    static char result[256];

    const char* s = (const char*) instr;
    char*       d = result;

    KeyMap& keymap = Starshatter::GetInstance()->GetKeyMap();

    while (*s) {
        if (*s == '$') {
            s++;
            char action[32];
            char* a = action;
            while (*s && (isupper(*s) || isdigit(*s) || *s == '_')) *a++ = *s++;
            *a = 0;

            int         act = KeyMap::GetKeyAction(action);
            int         key = keymap.FindMapIndex(act);
            const char* s2  = keymap.DescribeKey(key);

            if (!s2) s2 = action;
            while (*s2) *d++ = *s2++;
        }
        else {
            *d++ = *s++;
        }
    }

    *d = 0;

    return result;
}

// +--------------------------------------------------------------------+

void
HUDView::CycleInstructions(int direction)
{
    if (direction > 0)
    inst_page++;
    else
    inst_page--;
}

// +--------------------------------------------------------------------+

void
HUDView::DrawMessages()
{
    int message_queue_empty = true;

    // age messages:
    for (int i = 0; i < MAX_MSG; i++) {
        if (msg_time[i] > 0) {
            msg_time[i] -= Game::GUITime();

            if (msg_time[i] <= 0) {
                msg_time[i] = 0;
                msg_text[i] = "";
            }

            message_queue_empty = false;
        }
    }

    if (!message_queue_empty) {
        // advance message pipeline:
        for (int i = 0; i < MAX_MSG; i++) {
            if (msg_time[0] == 0) {
                for (int j = 0; j < MAX_MSG-1; j++) {
                    msg_time[j] = msg_time[j+1];
                    msg_text[j] = msg_text[j+1];
                }

                msg_time[MAX_MSG-1] = 0;
                msg_text[MAX_MSG-1] = "";
            }
        }

        // draw messages:
        for (int i = 0; i < MAX_MSG; i++) {
            int index = TXT_MSG_1 + i;

            if (msg_time[i] > 0) {
                Rect msg_rect(10, 95 + i*10, width-20, 12);
                DrawHUDText(index, msg_text[i], msg_rect, DT_LEFT, HUD_MIXED_CASE);
                if (msg_time[i] > 1)
                hud_text[index].color = txt_color;
                else
                hud_text[index].color = txt_color.dim(0.5 + 0.5*msg_time[i]);
            }
        }
    }
}

// +--------------------------------------------------------------------+

void
HUDView::DrawNav()
{
    if (!sim)
    return;

    active_region = sim->GetActiveRegion();

    if (ship) {
        int nav_index = 1;
        Instruction* next = ship->GetNextNavPoint();

        if (mode == HUD_MODE_NAV) {
            if (next && next->Action() == Instruction::LAUNCH)
            DrawNavPoint(*next, 0, true);

            ListIter<Instruction> navpt = ship->GetFlightPlan();
            while (++navpt) {
                DrawNavPoint(*navpt.value(), nav_index++, (navpt.value() == next));
            }
        }
        else if (next) {
            DrawNavPoint(*next, 0, true);
        }
    }
}

void
HUDView::DrawILS()
{
    if (ship) {
        bool hoops_drawn = false;
        bool same_sector = false;

        InboundSlot* inbound = ship->GetInbound();
        if (inbound) {
            FlightDeck* fd = inbound->GetDeck();

            if (fd && fd->IsRecoveryDeck() && fd->GetCarrier()) {
                if (fd->GetCarrier()->GetRegion() == ship->GetRegion())
                same_sector = true;

                if (same_sector && mode == HUD_MODE_ILS && !transition && !docking) {
                    Point dst = fd->MountLocation();
                    projector->Transform(dst);

                    if (dst.z > 1.0) {
                        projector->Project(dst);

                        int x = (int) dst.x;
                        int y = (int) dst.y;

                        if (x > 4 && x < width-4 &&
                                y > 4 && y < height-4) {

                            window->DrawLine(x-6, y-6, x+6, y+6, hud_color);
                            window->DrawLine(x+6, y-6, x-6, y+6, hud_color);
                        }
                    }
                }

                // draw the hoops for this flight deck:
                Scene* scene = camview->GetScene();
                for (int h = 0; h < fd->NumHoops(); h++) {
                    Hoop*  hoop  = fd->GetHoops() + h;
                    if (hoop && scene) {
                        if (same_sector && mode == HUD_MODE_ILS && !transition && !docking) {
                            scene->AddGraphic(hoop);
                            hoop->Show();

                            hoops_drawn = true;
                        }
                        else {
                            hoop->Hide();
                            scene->DelGraphic(hoop);
                        }
                    }
                }
            }
        }

        if (!hoops_drawn) {
            ListIter<Ship> iter = ship->GetRegion()->Carriers();
            while (++iter) {
                Ship* carrier = iter.value();

                bool ours = (carrier->GetIFF() == ship->GetIFF()) ||
                (carrier->GetIFF() == 0);

                for (int i = 0; i < carrier->NumFlightDecks(); i++) {
                    FlightDeck* fd = carrier->GetFlightDeck(i);

                    if (fd && fd->IsRecoveryDeck()) {
                        if (mode == HUD_MODE_ILS && ours && !transition && !docking) {
                            Point dst = fd->MountLocation();
                            projector->Transform(dst);

                            if (dst.z > 1.0) {
                                projector->Project(dst);

                                int x = (int) dst.x;
                                int y = (int) dst.y;

                                if (x > 4 && x < width-4 &&
                                        y > 4 && y < height-4) {

                                    window->DrawLine(x-6, y-6, x+6, y+6, hud_color);
                                    window->DrawLine(x+6, y-6, x-6, y+6, hud_color);
                                }
                            }
                        }

                        // draw the hoops for this flight deck:
                        Scene* scene = camview->GetScene();
                        for (int h = 0; h < fd->NumHoops(); h++) {
                            Hoop*  hoop  = fd->GetHoops() + h;
                            if (hoop && scene) {
                                if (mode == HUD_MODE_ILS && ours && !transition && !docking) {
                                    hoop->Show();
                                    if (!hoop->GetScene())
                                    scene->AddGraphic(hoop);
                                }
                                else {
                                    hoop->Hide();
                                    if (hoop->GetScene())
                                    scene->DelGraphic(hoop);
                                }
                            }
                        }
                    }
                }
            }
        }
    }
}

void
HUDView::DrawObjective()
{
    if (ship && ship->GetDirector() && ship->GetDirector()->Type() >= SteerAI::SEEKER) {
        SteerAI* steer = (SteerAI*) ship->GetDirector();
        Point obj = steer->GetObjective();
        projector->Transform(obj);

        if (obj.z > 1.0) {
            projector->Project(obj);

            int x = (int) obj.x;
            int y = (int) obj.y;

            if (x > 4 && x < width-4 &&
                    y > 4 && y < height-4) {

                Color c = Color::Cyan;
                window->DrawRect(x-6, y-6, x+6, y+6, c);
                window->DrawLine(x-6, y-6, x+6, y+6, c);
                window->DrawLine(x+6, y-6, x-6, y+6, c);
            }
        }

        if (steer->GetOther()) {
            obj = steer->GetOther()->Location();
            projector->Transform(obj);

            if (obj.z > 1.0) {
                projector->Project(obj);

                int x = (int) obj.x;
                int y = (int) obj.y;

                if (x > 4 && x < width-4 &&
                        y > 4 && y < height-4) {

                    Color c = Color::Orange;
                    window->DrawRect(x-6, y-6, x+6, y+6, c);
                    window->DrawLine(x-6, y-6, x+6, y+6, c);
                    window->DrawLine(x+6, y-6, x-6, y+6, c);
                }
            }
        }
    }
    /***/
}

void
HUDView::DrawNavPoint(Instruction& navpt, int index, int next)
{
    if (index >= 15 || !navpt.Region()) return;

    // transform from starsystem to world coordinates:
    Point npt = navpt.Region()->Location() + navpt.Location();

    if (active_region)
    npt -= active_region->Location();

    npt = npt.OtherHand();

    // transform from world to camera:
    projector->Transform(npt);

    // clip:
    if (npt.z > 1.0) {

        // project:
        projector->Project(npt);

        int x = (int) npt.x;
        int y = (int) npt.y;

        // clip:
        if (x > 4 && x < width-4 &&
                y > 4 && y < height-4) {

            Color c = Color::White;
            if (navpt.Status() > Instruction::ACTIVE && navpt.HoldTime() <= 0)
            c = Color::DarkGray;

            // draw:
            if (next)
            window->DrawEllipse(x-6, y-6, x+5, y+5, c);

            window->DrawLine(x-6, y-6, x+6, y+6, c);
            window->DrawLine(x+6, y-6, x-6, y+6, c);

            if (index > 0) {
                char npt_buf[32];
                Rect npt_rect(x+10, y-4, 200, 12);

                if (navpt.Status() == Instruction::COMPLETE && navpt.HoldTime() > 0) {
                    char hold_time[32];
                    FormatTime(hold_time, navpt.HoldTime());
                    sprintf_s(npt_buf, "%d %s", index, hold_time);
                }
                else {
                    sprintf_s(npt_buf, "%d", index);
                }

                DrawHUDText(TXT_NAV_PT + index, npt_buf, npt_rect, DT_LEFT);
            }
        }
    }

    if (next && mode == HUD_MODE_NAV && navpt.Region() == ship->GetRegion()) {

        // Translate into camera relative:
        Point tloc = navpt.Location().OtherHand();
        projector->Transform(tloc);

        int behind = tloc.z < 0;

        if (behind)
        tloc.z = -tloc.z;

        // Project into screen coordinates:
        projector->Project(tloc);

        // DRAW THE OFFSCREEN CHASE INDICATOR:
        if (behind     || 
                tloc.x <= 0 || tloc.x >= width-1 || 
                tloc.y <= 0 || tloc.y >= height-1) {

            // Left side:
            if (tloc.x <= 0 || (behind && tloc.x < width/2)) {
                if (tloc.y < ah) tloc.y = ah;
                else if (tloc.y >= height-ah) tloc.y = height-1-ah;

                chase_sprite->Show();
                chase_sprite->SetAnimation(&chase_left);
                chase_sprite->MoveTo(Point(aw, tloc.y, 1));
            }

            // Right side:
            else if (tloc.x >= width-1 || behind) {
                if (tloc.y < ah) tloc.y = ah;
                else if (tloc.y >= height-ah) tloc.y = height-1-ah;

                chase_sprite->Show();
                chase_sprite->SetAnimation(&chase_right);
                chase_sprite->MoveTo(Point(width-1-aw, tloc.y, 1));
            }
            else {
                if (tloc.x < aw) tloc.x = aw;
                else if (tloc.x >= width-aw) tloc.x = width-1-aw;

                // Top edge:
                if (tloc.y <= 0) {
                    chase_sprite->Show();
                    chase_sprite->SetAnimation(&chase_top);
                    chase_sprite->MoveTo(Point(tloc.x, ah, 1));
                }
                
                // Bottom edge:
                else if (tloc.y >= height-1) {
                    chase_sprite->Show();
                    chase_sprite->SetAnimation(&chase_bottom);
                    chase_sprite->MoveTo(Point(tloc.x, height-1-ah, 1));
                }
            }
        }
    }
}

// +--------------------------------------------------------------------+

void
HUDView::SetShip(Ship* s)
{
    if (ship != s) {
        double new_scale = 1;

        ship_status = -1;
        ship        = s;

        if (ship) {
            if (ship->Life() == 0 || ship->IsDying() || ship->IsDead()) {
                ship = 0;
            }
            else {
                Observe(ship);
                new_scale = 1.1 * ship->Radius() / 64;

                if (ship->Design()->hud_icon.Width()) {
                    TransferBitmap(ship->Design()->hud_icon, icon_ship, icon_ship_shade);
                    ColorizeBitmap(icon_ship, icon_ship_shade, txt_color);
                }
            }
        }

        if (az_ring) {
            az_ring->Rescale(1/compass_scale);
            az_ring->Rescale(new_scale);
        }

        if (az_pointer) {
            az_pointer->Rescale(1/compass_scale);
            az_pointer->Rescale(new_scale);
        }

        if (el_ring) {
            el_ring->Rescale(1/compass_scale);
            el_ring->Rescale(new_scale);
        }

        if (el_pointer) {
            el_pointer->Rescale(1/compass_scale);
            el_pointer->Rescale(new_scale);
        }

        compass_scale = new_scale;
        inst_page = 0;

        if (ship && ship->GetElement() && ship->GetElement()->NumInstructions() > 0)
        if (!show_inst)
        CycleHUDInst();
    }

    else if (ship && ship->Design()->hud_icon.Width()) {
        bool update = false;
        System::STATUS s = System::NOMINAL;
        int integrity = (int) (ship->Integrity() / ship->Design()->integrity * 100);

        if (integrity < 30)        s = System::CRITICAL;
        else if (integrity < 60)   s = System::DEGRADED;

        if (s != ship_status) {
            ship_status = s;
            update      = true;
        }

        if (update) {
            SetStatusColor((System::STATUS) ship_status);
            ColorizeBitmap(icon_ship, icon_ship_shade, status_color);
        }
    }

    if (ship && ship->Cockpit()) {
        Solid* cockpit = (Solid*) ship->Cockpit();

        bool change = false;

        if (cockpit->Hidden()) {
            if (cockpit_hud_texture)
            change = true;

            cockpit_hud_texture = 0;
        }
        else {
            if (!cockpit_hud_texture)
            change = true;

            Model*    cockpit_model = cockpit->GetModel();
            Material* hud_material  = 0;

            if (cockpit_model) {
                hud_material = (Material*) cockpit_model->FindMaterial("HUD");
                if (hud_material) {
                    cockpit_hud_texture = hud_material->tex_emissive;
                }
            }
        }

        if (change) {
            SetHUDColorSet(color);
        }
    }
}

void
HUDView::SetTarget(SimObject* t)
{
    bool update = false;

    if (target != t) {
        tgt_status = -1;
        target = t;
        if (target) Observe(target);
        update = true;
    }

    if (target && target->Type() == SimObject::SIM_SHIP) {
        System::STATUS s = System::NOMINAL;
        Ship* tship = (Ship*) target;
        int integrity = (int) (tship->Integrity() / tship->Design()->integrity * 100);

        if (integrity < 30)        s = System::CRITICAL;
        else if (integrity < 60)   s = System::DEGRADED;

        if (s != tgt_status) {
            tgt_status = s;
            update     = true;
        }
    }

    if (update) {
        if (target && target->Type() == SimObject::SIM_SHIP) {
            Ship* tship = (Ship*) target;
            TransferBitmap(tship->Design()->hud_icon, icon_target, icon_target_shade);
        }
        else {
            PrepareBitmap("hud_icon.pcx", icon_target, icon_target_shade);
        }

        SetStatusColor((System::STATUS) tgt_status);
        ColorizeBitmap(icon_target, icon_target_shade, status_color);
    }
}

// +--------------------------------------------------------------------+

MFD*
HUDView::GetMFD(int n) const
{
    if (n >= 0 && n < 3)
    return mfd[n];

    return 0;
}

// +--------------------------------------------------------------------+

void
HUDView::Refresh()
{
    sim      = Sim::GetSim();
    mouse_in = false;

    if (!sim || !camview || !projector) {
        return;
    }

    if (Mouse::LButton() == 0) {
        mouse_latch = 0;
        mouse_index = -1;
    }

    int mouse_index_old = mouse_index;

    SetShip(sim->GetPlayerShip());

    if (mode == HUD_MODE_OFF) {
        if (cockpit_hud_texture) {
            cockpit_hud_texture->FillRect(0,0,512,256,Color::Black);
        }

        sim->ShowGrid(false);
        return;
    }

    if (cockpit_hud_texture && cockpit_hud_texture->Width() == 512) {
        Bitmap* hud_bmp = 0;

        if (hud_sprite[0]) {
            hud_bmp = hud_sprite[0]->Frame();
            int bmp_w = hud_bmp->Width();
            int bmp_h = hud_bmp->Height();

            cockpit_hud_texture->BitBlt(  0, 0, *hud_bmp, 0, 0, bmp_w, bmp_h);
        }

        if (hud_sprite[1]) {
            hud_bmp = hud_sprite[1]->Frame();
            int bmp_w = hud_bmp->Width();
            int bmp_h = hud_bmp->Height();

            cockpit_hud_texture->BitBlt(256, 0, *hud_bmp, 0, 0, bmp_w, bmp_h);
        }

        if (hud_sprite[6]) {
            if (hud_sprite[6]->Hidden()) {
                cockpit_hud_texture->FillRect(0,384,128,512,Color::Black);
            }
            else {
                hud_bmp = hud_sprite[6]->Frame();
                int bmp_w = hud_bmp->Width();
                int bmp_h = hud_bmp->Height();

                cockpit_hud_texture->BitBlt(0,384, *hud_bmp, 0, 0, bmp_w, bmp_h);
            }
        }

        if (hud_sprite[7]) {
            if (hud_sprite[7]->Hidden()) {
                cockpit_hud_texture->FillRect(128,384,256,512,Color::Black);
            }
            else {
                hud_bmp = hud_sprite[7]->Frame();
                int bmp_w = hud_bmp->Width();
                int bmp_h = hud_bmp->Height();

                cockpit_hud_texture->BitBlt(128,384, *hud_bmp, 0, 0, bmp_w, bmp_h);
            }
        }

        for (int i = 8; i < 32; i++) {
            if (hud_sprite[i] && !hud_sprite[i]->Hidden()) {
                Sprite* s = hud_sprite[i];

                int cx = (int) s->Location().x;
                int cy = (int) s->Location().y;
                int w2 = s->Width()  / 2;
                int h2 = s->Height() / 2;

                window->DrawBitmap(cx-w2, cy-h2, cx+w2, cy+h2, s->Frame(), Video::BLEND_ALPHA);
            }
        }
    }
    else {
        for (int i = 0; i < 32; i++) {
            if (hud_sprite[i] && !hud_sprite[i]->Hidden()) {
                Sprite* s = hud_sprite[i];

                int cx = (int) s->Location().x;
                int cy = (int) s->Location().y;
                int w2 = s->Width()  / 2;
                int h2 = s->Height() / 2;

                window->DrawBitmap(cx-w2, cy-h2, cx+w2, cy+h2, s->Frame(), Video::BLEND_ALPHA);
            }
        }

        Video* video = Video::GetInstance();

        for (int i = 0; i < 31; i++) {
            Sprite* s = pitch_ladder[i];

            if (s && !s->Hidden()) {
                s->Render2D(video);
            }
        }
    }

    //DrawStarSystem();
    DrawMessages();

    if (ship) {
        // no hud in transition:
        if (ship->InTransition()) {
            transition = true;
            HideAll();
            return;
        }

        else if (transition) {
            transition = false;
            RestoreHUD();
        }

        CameraDirector* cam_dir = CameraDirector::GetInstance();

        // everything is off during docking, except the final message:
        if (cam_dir && cam_dir->GetMode() == CameraDirector::MODE_DOCKING) {
            docking = true;
            HideAll();

            if (ship->GetFlightPhase() == Ship::DOCKING) {
                Rect dock_rect(width/2-100, height/6, 200, 20);

                if (ship->IsAirborne())
                DrawHUDText(TXT_AUTO, Game::GetText("HUDView.SUCCESSFUL-LANDING"), dock_rect, DT_CENTER);
                else
                DrawHUDText(TXT_AUTO, Game::GetText("HUDView.DOCKING-COMPLETE"), dock_rect, DT_CENTER);
            }
            return;
        }
        else if (docking) {
            docking = false;
            RestoreHUD();
        }

        // go to NAV mode during autopilot:
        if (ship->GetNavSystem() && ship->GetNavSystem()->AutoNavEngaged() && !arcade)
        mode = HUD_MODE_NAV;

        SetTarget(ship->GetTarget());

        // internal view of HUD reticule
        if (CameraDirector::GetCameraMode() <= CameraDirector::MODE_CHASE)
        SetTacticalMode(0);

        // external view
        else
        SetTacticalMode(!cockpit_hud_texture);

        sim->ShowGrid(tactical              && 
        !ship->IsAirborne()   &&
        CameraDirector::GetCameraMode() != CameraDirector::MODE_VIRTUAL);

        // draw HUD bars:
        DrawBars();

        if (missile_lock_sound) {
            if (threat > 1) {
                long max_vol       = AudioConfig::WrnVolume();
                long volume        = -1500;

                if (volume > max_vol)
                volume = max_vol;

                missile_lock_sound->SetVolume(volume);
                missile_lock_sound->Play();
            }
            else {
                missile_lock_sound->Stop();
            }
        }

        DrawNav();
        DrawILS();
        
        // FOR DEBUG PURPOSES ONLY:
        // DrawObjective();

        if (!overlay) {
            Rect fov_rect(0, 10, width, 10);
            int  fov_degrees = 180 - 2 * (int)(projector->XAngle()*180/PI);

            if (fov_degrees > 90)
            DrawHUDText(TXT_CAM_ANGLE, Game::GetText("HUDView.Wide-Angle"), fov_rect, DT_CENTER);

            fov_rect.y = 20;
            DrawHUDText(TXT_CAM_MODE, CameraDirector::GetModeName(), fov_rect, DT_CENTER);
        }

        DrawMFDs();

        instr_left_sprite->Hide();
        instr_right_sprite->Hide();
        warn_left_sprite->Hide();
        warn_right_sprite->Hide();

        if (cockpit_hud_texture)
        cockpit_hud_texture->FillRect(256,384,512,512,Color::Black);

        if (show_inst) {
            DrawInstructions();
        }

        else if (!arcade) {
            if (ship->MasterCaution() && !show_warn)
            ShowHUDWarn();

            if (show_warn)
            DrawWarningPanel();
        }

        if (width > 640 || (!show_inst && !show_warn)) {
            Rect icon_rect(120, height-24, 128, 16);

            if (ship)
            DrawHUDText(TXT_ICON_SHIP_TYPE, ship->DesignName(), icon_rect, DT_CENTER);

            icon_rect.x = width - 248;

            if (target && target->Type() == SimObject::SIM_SHIP) {
                Ship* tship = (Ship*) target;
                DrawHUDText(TXT_ICON_TARGET_TYPE, tship->DesignName(), icon_rect, DT_CENTER);
            }
        }
    }
    else {
        if (target) {
            SetTarget(0);
        }
    }

    // latch mouse down to prevent dragging into a control:
    if (Mouse::LButton() == 1)
    mouse_latch = 1;

    if (mouse_index > -1 && mouse_index_old != mouse_index)
    MouseFrame();
}

void
HUDView::DrawMFDs()
{
    for (int i = 0; i < 3; i++) {
        mfd[i]->Show();
        mfd[i]->SetShip(ship);
        mfd[i]->SetCockpitHUDTexture(cockpit_hud_texture);
        mfd[i]->Draw();
    }
}

// +--------------------------------------------------------------------+

void
HUDView::DrawStarSystem()
{
    if (sim && sim->GetStarSystem()) {
        StarSystem* sys = sim->GetStarSystem();

        ListIter<OrbitalBody> iter = sys->Bodies();
        while (++iter) {
            OrbitalBody* body = iter.value();
            DrawOrbitalBody(body);
        }
    }
}

void
HUDView::DrawOrbitalBody(OrbitalBody* body)
{
    if (body) {
        Point p = body->Rep()->Location();

        projector->Transform(p);

        if (p.z > 100) {
            float r = (float) body->Radius();
            r = projector->ProjectRadius(p, r);
            projector->Project(p, false);

            window->DrawEllipse((int) (p.x-r), 
            (int) (p.y-r), 
            (int) (p.x+r),
            (int) (p.y+r),
            Color::Cyan);
        }

        ListIter<OrbitalBody> iter = body->Satellites();
        while (++iter) {
            OrbitalBody* body = iter.value();
            DrawOrbitalBody(body);
        }
    }
}

// +--------------------------------------------------------------------+

void
HUDView::ExecFrame()
{
    // update the position of HUD elements that are
    // part of the 3D scene (like fpm and lcos sprites)
    HideCompass();

    if (ship && !transition && !docking && mode != HUD_MODE_OFF) {
        Player* p = Player::GetCurrentPlayer();
        gunsight = p->Gunsight();

        if (ship->IsStarship()) {
            if (tactical) {
                hud_left_sprite->Hide();
                hud_right_sprite->Hide();
            }

            else if (hud_left_sprite->Frame() != &hud_left_starship) {
                hud_left_sprite->SetAnimation(&hud_left_starship);
                hud_right_sprite->SetAnimation(&hud_right_starship);

                hud_left_sprite->MoveTo( Point(width/2-128, height/2, 1));
                hud_right_sprite->MoveTo(Point(width/2+128, height/2, 1));
            }
        }

        else if (!ship->IsStarship()) {
            if (ship->IsAirborne() && hud_left_sprite->Frame() != &hud_left_air) {
                hud_left_sprite->SetAnimation(&hud_left_air);
                hud_right_sprite->SetAnimation(&hud_right_air);
            }

            else if (!ship->IsAirborne() && hud_left_sprite->Frame() != &hud_left_fighter) {
                hud_left_sprite->SetAnimation(&hud_left_fighter);
                hud_right_sprite->SetAnimation(&hud_right_fighter);
            }
        }

        if (!tactical) {
            if (Game::MaxTexSize() > 128) {
                hud_left_sprite->Show();
                hud_right_sprite->Show();
            }

            if (!arcade)
            DrawFPM();

            if (ship->IsStarship() && ship->GetFLCSMode() == Ship::FLCS_HELM)
            DrawHPM();
            else if (!arcade)
            DrawPitchLadder();
        }

        else {
            if (ship->IsStarship() && ship->GetFLCSMode() == Ship::FLCS_HELM)
            DrawCompass();
        }

        if (mode == HUD_MODE_TAC) {
            DrawSight();
            DrawDesignators();
        }

        if (width > 640 || (!show_inst && !show_warn)) {
            icon_ship_sprite->Show();
            icon_target_sprite->Show();
        }
        else {
            icon_ship_sprite->Hide();
            icon_target_sprite->Hide();
        }
    }

    // if the hud is off or prohibited,
    // hide all of the sprites:

    else {
        hud_left_sprite->Hide();
        hud_right_sprite->Hide();
        instr_left_sprite->Hide();
        instr_right_sprite->Hide();
        warn_left_sprite->Hide();
        warn_right_sprite->Hide();
        icon_ship_sprite->Hide();
        icon_target_sprite->Hide();
        fpm_sprite->Hide();
        hpm_sprite->Hide();
        lead_sprite->Hide();
        aim_sprite->Hide();
        tgt1_sprite->Hide();
        tgt2_sprite->Hide();
        tgt3_sprite->Hide();
        tgt4_sprite->Hide();
        chase_sprite->Hide();

        for (int i = 0; i < 3; i++)
        mfd[i]->Hide();

        for (int i = 0; i < 31; i++)
        pitch_ladder[i]->Hide();

        DrawILS();
    }
}

// +--------------------------------------------------------------------+

void
HUDView::CycleMFDMode(int mfd_index)
{
    if (mfd_index < 0 || mfd_index > 2) return;

    int m = mfd[mfd_index]->GetMode();
    m++;

    if (mfd_index == 2) {
        if (m > MFD::MFD_MODE_SHIP)
        m = MFD::MFD_MODE_OFF;
    }
    else {
        if (m > MFD::MFD_MODE_3D)
        m = MFD::MFD_MODE_OFF;

        if (m == MFD::MFD_MODE_GAME)
        m++;

        if (mfd_index != 0 && m == MFD::MFD_MODE_SHIP)
        m++;
    }

    mfd[mfd_index]->SetMode(m);
    HUDSounds::PlaySound(HUDSounds::SND_MFD_MODE);
}

// +--------------------------------------------------------------------+

void
HUDView::ShowHUDWarn()
{
    if (!show_warn) {
        show_warn = true;

        if (ship && ship->HullStrength() <= 40) {
            // TOO OBNOXIOUS!!
            HUDSounds::PlaySound(HUDSounds::SND_RED_ALERT);
        }
    }
}

void
HUDView::ShowHUDInst()
{
    show_inst = true;
}

// +--------------------------------------------------------------------+

void
HUDView::HideHUDWarn()
{
    show_warn = false;

    if (ship) {
        ship->ClearCaution();
        HUDSounds::StopSound(HUDSounds::SND_RED_ALERT);
    }
}

void
HUDView::HideHUDInst()
{
    show_inst = false;
}

// +--------------------------------------------------------------------+

void
HUDView::CycleHUDWarn()
{
    HUDSounds::PlaySound(HUDSounds::SND_HUD_WIDGET);
    show_warn = !show_warn;

    if (ship && !show_warn) {
        ship->ClearCaution();
        HUDSounds::StopSound(HUDSounds::SND_RED_ALERT);
    }
}

void
HUDView::CycleHUDInst()
{
    show_inst = !show_inst;
    HUDSounds::PlaySound(HUDSounds::SND_HUD_WIDGET);
}

// +--------------------------------------------------------------------+

void
HUDView::SetHUDMode(int m)
{
    if (mode != m) {
        mode = m;

        if (mode > HUD_MODE_ILS || mode < HUD_MODE_OFF)
        mode = HUD_MODE_OFF;

        if (ship && !ship->IsDropship() && mode == HUD_MODE_ILS)
        mode = HUD_MODE_OFF;

        RestoreHUD();
    }
}

void
HUDView::CycleHUDMode()
{
    mode++;

    if (arcade && mode != HUD_MODE_TAC)
    mode = HUD_MODE_OFF;

    else if (mode > HUD_MODE_ILS || mode < HUD_MODE_OFF)
    mode = HUD_MODE_OFF;

    else if (!ship->IsDropship() && mode == HUD_MODE_ILS)
    mode = HUD_MODE_OFF;

    RestoreHUD();
    HUDSounds::PlaySound(HUDSounds::SND_HUD_MODE);
}

void
HUDView::RestoreHUD()
{
    if (mode == HUD_MODE_OFF) {
        HideAll();
    }
    else {
        for (int i = 0; i < 3; i++)
        mfd[i]->Show();

        if (width > 640 || (!show_inst && !show_warn)) {
            icon_ship_sprite->Show();
            icon_target_sprite->Show();
        }
        else {
            icon_ship_sprite->Hide();
            icon_target_sprite->Hide();
        }

        if (!tactical && Game::MaxTexSize() > 128) {
            hud_left_sprite->Show();
            hud_right_sprite->Show();
        }

        fpm_sprite->Show();

        if (ship && ship->IsStarship())
        hpm_sprite->Show();

        if (gunsight == 0)
        aim_sprite->Show();
        else
        lead_sprite->Show();
    }
}

void
HUDView::HideAll()
{
    for (int i = 0; i < 3; i++)
    mfd[i]->Hide();

    hud_left_sprite->Hide();
    hud_right_sprite->Hide();
    instr_left_sprite->Hide();
    instr_right_sprite->Hide();
    warn_left_sprite->Hide();
    warn_right_sprite->Hide();
    icon_ship_sprite->Hide();
    icon_target_sprite->Hide();
    fpm_sprite->Hide();
    hpm_sprite->Hide();
    lead_sprite->Hide();
    aim_sprite->Hide();
    tgt1_sprite->Hide();
    tgt2_sprite->Hide();
    tgt3_sprite->Hide();
    tgt4_sprite->Hide();
    chase_sprite->Hide();

    sim->ShowGrid(false);

    for (int i = 0; i < 31; i++)
    pitch_ladder[i]->Hide();

    if (missile_lock_sound)
    missile_lock_sound->Stop();

    HideCompass();
    DrawILS();
    Mouse::Show(false);
}

// +--------------------------------------------------------------------+

Color
HUDView::Ambient() const
{
    if (!sim || !ship || mode == HUD_MODE_OFF)
    return Color::Black;

    SimRegion* rgn = sim->GetActiveRegion();

    if (!rgn || !rgn->IsAirSpace())
    return Color::Black;

    Color c = sim->GetStarSystem()->Ambient();

    if (c.Red() > 32 || c.Green() > 32 || c.Blue() > 32)
    return Color::Black;

    // if we get this far, the night-vision aid is on
    return night_vision_colors[color];
}

Color
HUDView::CycleHUDColor()
{
    HUDSounds::PlaySound(HUDSounds::SND_HUD_MODE);
    SetHUDColorSet(color+1);
    return hud_color;
}

void
HUDView::SetHUDColorSet(int c)
{
    color     = c;
    if (color > NUM_HUD_COLORS-1) color = 0;
    hud_color = standard_hud_colors[color];
    txt_color = standard_txt_colors[color];

    ColorizeBitmap(fpm,           fpm_shade,           hud_color,           true);
    ColorizeBitmap(hpm,           hpm_shade,           hud_color,           true);
    ColorizeBitmap(lead,          lead_shade,          txt_color * 1.25,    true);
    ColorizeBitmap(cross,         cross_shade,         hud_color,           true);
    ColorizeBitmap(cross1,        cross1_shade,        hud_color,           true);
    ColorizeBitmap(cross2,        cross2_shade,        hud_color,           true);
    ColorizeBitmap(cross3,        cross3_shade,        hud_color,           true);
    ColorizeBitmap(cross4,        cross4_shade,        hud_color,           true);

    if (Game::MaxTexSize() > 128) {
        ColorizeBitmap(hud_left_air,        hud_left_shade_air,       hud_color);
        ColorizeBitmap(hud_right_air,       hud_right_shade_air,      hud_color);
        ColorizeBitmap(hud_left_fighter,    hud_left_shade_fighter,   hud_color);
        ColorizeBitmap(hud_right_fighter,   hud_right_shade_fighter,  hud_color);
        ColorizeBitmap(hud_left_starship,   hud_left_shade_starship,  hud_color);
        ColorizeBitmap(hud_right_starship,  hud_right_shade_starship, hud_color);

        ColorizeBitmap(instr_left,          instr_left_shade,         hud_color);
        ColorizeBitmap(instr_right,         instr_right_shade,        hud_color);
        ColorizeBitmap(warn_left,           warn_left_shade,          hud_color);
        ColorizeBitmap(warn_right,          warn_right_shade,         hud_color);

        ColorizeBitmap(pitch_ladder_pos,    pitch_ladder_pos_shade,   hud_color);
        ColorizeBitmap(pitch_ladder_neg,    pitch_ladder_neg_shade,   hud_color);
    }

    ColorizeBitmap(icon_ship,     icon_ship_shade,     txt_color);
    ColorizeBitmap(icon_target,   icon_target_shade,   txt_color);

    ColorizeBitmap(chase_left,    chase_left_shade,    hud_color,           true);
    ColorizeBitmap(chase_right,   chase_right_shade,   hud_color,           true);
    ColorizeBitmap(chase_top,     chase_top_shade,     hud_color,           true);
    ColorizeBitmap(chase_bottom,  chase_bottom_shade,  hud_color,           true);

    MFD::SetColor(hud_color);
    Hoop::SetColor(hud_color);

    for (int i = 0; i < 3; i++)
    mfd[i]->SetText3DColor(txt_color);

    Font* font = FontMgr::Find("HUD");
    if (font)
    font->SetColor(txt_color);

    for (int i = 0; i < TXT_LAST; i++)
    hud_text[i].color = txt_color;
}

// +--------------------------------------------------------------------+

void
HUDView::Message(const char* fmt, ...)
{
    if (fmt) {
        char msg[512];
        vsprintf(msg, fmt, (char *)(&fmt+1));

        char* newline = strchr(msg, '\n');
        if (newline)
        *newline = 0;

        Print("%s\n", msg);

        if (hud_view) {
            int index = -1;

            for (int i = 0; i < MAX_MSG; i++) {
                if (hud_view->msg_time[i] <= 0) {
                    index = i;
                    break;
                }
            }

            // no space; advance pipeline:
            if (index < 0) {
                for (int i = 0; i < MAX_MSG-1; i++) {
                    hud_view->msg_text[i] = hud_view->msg_text[i+1];
                    hud_view->msg_time[i] = hud_view->msg_time[i+1];
                }

                index = MAX_MSG-1;
            }

            hud_view->msg_text[index] = msg;
            hud_view->msg_time[index] = 10;
        }
    }
}

// +--------------------------------------------------------------------+

void 
HUDView::ClearMessages()
{
    if (hud_view) {
        for (int i = 0; i < MAX_MSG-1; i++) {
            hud_view->msg_text[i] = Text();
            hud_view->msg_time[i] = 0;
        }
    }
}

// +--------------------------------------------------------------------+

void 
HUDView::PrepareBitmap(const char* name, Bitmap& img, BYTE*& shades)
{
    delete [] shades;
    shades = 0;

    DataLoader* loader = DataLoader::GetLoader();

    loader->SetDataPath("HUD/");
    int loaded = loader->LoadBitmap(name, img, Bitmap::BMP_TRANSPARENT);
    loader->SetDataPath(0);

    if (!loaded)
    return;

    int w = img.Width();
    int h = img.Height();

    shades = new(__FILE__,__LINE__) BYTE[w*h];

    for (int y = 0; y < h; y++)
    for (int x = 0; x < w; x++)
    shades[y*w+x] = (BYTE) (img.GetColor(x,y).Red() * 0.66);
}

void 
HUDView::TransferBitmap(const Bitmap& src, Bitmap& img, BYTE*& shades)
{
    delete [] shades;
    shades = 0;

    if (src.Width() != img.Width() || src.Height() != img.Height())
    return;

    img.CopyBitmap(src);
    img.SetType(Bitmap::BMP_TRANSLUCENT);

    int w = img.Width();
    int h = img.Height();

    shades = new(__FILE__,__LINE__) BYTE[w*h];

    for (int y = 0; y < h; y++)
    for (int x = 0; x < w; x++)
    shades[y*w+x] = (BYTE) (img.GetColor(x,y).Red() * 0.5);
}

void 
HUDView::ColorizeBitmap(Bitmap& img, BYTE* shades, Color color, bool force_alpha)
{
    if (!shades) return;

    int max_tex_size = Game::MaxTexSize();

    if (max_tex_size < 128)
    Game::SetMaxTexSize(128);

    if (hud_view && hud_view->cockpit_hud_texture && !force_alpha) {
        img.FillColor(Color::Black);
        Color* dst = img.HiPixels();
        BYTE*  src = shades;

        for (int y = 0; y < img.Height(); y++) {
            for (int x = 0; x < img.Width(); x++) {
                if (*src)
                *dst = color.dim(*src/200.0);
                else
                *dst = Color::Black;

                dst++;
                src++;
            }
        }
        img.MakeTexture();
    }
    else {
        img.FillColor(color);
        img.CopyAlphaImage(img.Width(), img.Height(), shades);
        img.MakeTexture();
    }

    if (max_tex_size < 128)
    Game::SetMaxTexSize(max_tex_size);
}

// +--------------------------------------------------------------------+

void
HUDView::MouseFrame()
{
    MouseController* ctrl = MouseController::GetInstance();
    if (ctrl && ctrl->Active())
    return;

    if (mouse_index >= TXT_CAUTION_TXT && mouse_index <= TXT_LAST_CAUTION) {
        if (show_inst) {
            if (mouse_index == TXT_INSTR_PAGE) {
                if (Mouse::X() > width/2 + 125)
                CycleInstructions(1);
                else if (Mouse::X() < width/2 + 65)
                CycleInstructions(-1);
            }
            else
            show_inst = false;
        }
        else {
            CycleHUDWarn();
        }
        return;
    }

    Starshatter* stars = Starshatter::GetInstance();
    if (mouse_index == TXT_PAUSED)
    stars->Pause(!Game::Paused());

    if (mouse_index == TXT_GEAR_DOWN)
    ship->ToggleGear();

    if (mouse_index == TXT_HUD_MODE) {
        CycleHUDMode();

        if (mode == HUD_MODE_OFF)
        CycleHUDMode();
    }

    if (mouse_index == TXT_PRIMARY_WEP) {
        HUDSounds::PlaySound(HUDSounds::SND_WEP_MODE);
        ship->CyclePrimary();
    }

    if (mouse_index == TXT_SECONDARY_WEP) {
        HUDSounds::PlaySound(HUDSounds::SND_WEP_MODE);
        ship->CycleSecondary();
    }

    if (mouse_index == TXT_DECOY)
    ship->FireDecoy();

    if (mouse_index == TXT_SHIELD) {
        Shield* shield = ship->GetShield();

        if (shield) {
            double  level  = shield->GetPowerLevel();

            const Rect& r = hud_text[TXT_SHIELD].rect;
            if (Mouse::X() < r.x + r.w * 0.75)
            shield->SetPowerLevel(level - 10);
            else
            shield->SetPowerLevel(level + 10);

            HUDSounds::PlaySound(HUDSounds::SND_SHIELD_LEVEL);
        }
    }

    if (mouse_index == TXT_AUTO)
    ship->TimeSkip();

    if (mouse_index >= TXT_NAV_INDEX && mouse_index <= TXT_NAV_ETR) {
        ship->SetAutoNav(!ship->IsAutoNavEngaged());
        SetHUDMode(HUD_MODE_TAC);
    }
}

// +--------------------------------------------------------------------+

bool
HUDView::IsMouseLatched()
{
    bool result = mouse_in;

    if (!result) {
        HUDView* hud = HUDView::GetInstance();

        for (int i = 0; i < 3; i++)
        result = result || hud->mfd[i]->IsMouseLatched();
    }

    return result;
}

// +--------------------------------------------------------------------+

bool
HUDView::IsNameCrowded(int x, int y)
{
    for (int i = 0; i < MAX_CONTACT; i++) {
        HUDText& test = hud_text[TXT_CONTACT_NAME + i];

        if (!test.hidden) {
            Rect r = test.rect;

            int dx = r.x - x;
            int dy = r.y - y;
            int d  = dx*dx + dy*dy;

            if (d <= 400)
            return true;
        }

        test = hud_text[TXT_CONTACT_INFO + i];

        if (!test.hidden) {
            Rect r = test.rect;

            int dx = r.x - x;
            int dy = r.y - y;
            int d  = dx*dx + dy*dy;

            if (d <= 400)
            return true;
        }
    }

    return false;
}

void
HUDView::DrawDiamond(int x, int y, int r, Color c)
{
    POINT diamond[4];

    diamond[0].x = x;
    diamond[0].y = y-r;

    diamond[1].x = x+r;
    diamond[1].y = y;

    diamond[2].x = x;
    diamond[2].y = y+r;

    diamond[3].x = x-r;
    diamond[3].y = y;

    window->DrawPoly(4, diamond, c);
}
