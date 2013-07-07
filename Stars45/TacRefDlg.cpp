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
    FILE:         TacRefDlg.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Tactical Reference Dialog Active Window class
*/

#include "MemDebug.h"
#include "TacRefDlg.h"
#include "MenuScreen.h"
#include "Campaign.h"
#include "Mission.h"
#include "Ship.h"
#include "ShipDesign.h"
#include "WeaponDesign.h"
#include "WeaponGroup.h"

#include "Game.h"
#include "EventDispatch.h"
#include "Mouse.h"
#include "Button.h"
#include "ListBox.h"
#include "Slider.h"
#include "ParseUtil.h"
#include "FormatUtil.h"
#include "Light.h"
#include "Solid.h"

// +--------------------------------------------------------------------+
// DECLARE MAPPING FUNCTIONS:
DEF_MAP_CLIENT(TacRefDlg, OnClose);
DEF_MAP_CLIENT(TacRefDlg, OnSelect);
DEF_MAP_CLIENT(TacRefDlg, OnMode);
DEF_MAP_CLIENT(TacRefDlg, OnCamRButtonDown);
DEF_MAP_CLIENT(TacRefDlg, OnCamRButtonUp);
DEF_MAP_CLIENT(TacRefDlg, OnCamMove);
DEF_MAP_CLIENT(TacRefDlg, OnCamZoom);

// +--------------------------------------------------------------------+

TacRefDlg::TacRefDlg(Screen* s, FormDef& def, MenuScreen* mgr)
: FormWindow(s,  0,  0, s->Width(), s->Height()), manager(mgr),
beauty(0), camview(0), imgview(0), 
txt_caption(0), txt_stats(0), txt_description(0),
lst_designs(0), btn_close(0), btn_ships(0), btn_weaps(0),
mode(MODE_NONE), radius(100), mouse_x(0), mouse_y(0),
cam_zoom(2.5), cam_az(-PI/6), cam_el(PI/7), captured(false),
ship_index(0), weap_index(0)
{
    Init(def);
}

TacRefDlg::~TacRefDlg()
{
    if (beauty) {
        beauty->DelView(camview);
        beauty->DelView(imgview);
    }

    delete camview;
    delete imgview;
}

// +--------------------------------------------------------------------+

void
TacRefDlg::RegisterControls()
{
    btn_close         = (Button*)       FindControl(1);
    btn_ships         = (Button*)       FindControl(101);
    btn_weaps         = (Button*)       FindControl(102);
    lst_designs       = (ListBox*)      FindControl(200);
    txt_caption       =                 FindControl(301);
    beauty            =                 FindControl(401);
    txt_stats         = (RichTextBox*)  FindControl(402);
    txt_description   = (RichTextBox*)  FindControl(403);

    if (btn_close) {
        REGISTER_CLIENT(EID_CLICK,          btn_close,   TacRefDlg, OnClose);
    }

    if (btn_ships) {
        btn_ships->SetButtonState(mode == MODE_SHIPS);
        REGISTER_CLIENT(EID_CLICK,          btn_ships,   TacRefDlg, OnMode);
    }

    if (btn_weaps) {
        btn_weaps->SetButtonState(mode == MODE_WEAPONS);
        REGISTER_CLIENT(EID_CLICK,          btn_weaps,   TacRefDlg, OnMode);
    }

    if (lst_designs) {
        REGISTER_CLIENT(EID_SELECT,         lst_designs, TacRefDlg, OnSelect);
    }

    if (beauty) {
        REGISTER_CLIENT(EID_RBUTTON_DOWN,   beauty,     TacRefDlg, OnCamRButtonDown);
        REGISTER_CLIENT(EID_RBUTTON_UP,     beauty,     TacRefDlg, OnCamRButtonUp);
        REGISTER_CLIENT(EID_MOUSE_MOVE,     beauty,     TacRefDlg, OnCamMove);
        REGISTER_CLIENT(EID_MOUSE_WHEEL,    beauty,     TacRefDlg, OnCamZoom);

        scene.SetAmbient(Color(60,60,60));

        Point light_pos(3e6, 5e6, 4e6);

        Light* main_light = new Light(1.0f); //1.25f);
        main_light->MoveTo(light_pos);
        main_light->SetType(Light::LIGHT_DIRECTIONAL);
        main_light->SetColor(Color::White);
        main_light->SetShadow(true);

        scene.AddLight(main_light);

        Light* back_light = new Light(0.5f);
        back_light->MoveTo(light_pos * -1);
        back_light->SetType(Light::LIGHT_DIRECTIONAL);
        back_light->SetColor(Color::White);
        back_light->SetShadow(false);

        scene.AddLight(back_light);

        camview = new(__FILE__,__LINE__) CameraView(beauty, &cam, &scene);
        camview->SetProjectionType(Video::PROJECTION_PERSPECTIVE);
        camview->SetFieldOfView(2);

        beauty->AddView(camview);

        imgview = new(__FILE__,__LINE__) ImgView(beauty, 0);
        imgview->SetBlend(Video::BLEND_ALPHA);
    }
}

// +--------------------------------------------------------------------+

void
TacRefDlg::Show()
{
    update_scene = !shown;

    FormWindow::Show();

    if (update_scene) {
        AWEvent event(btn_ships, EID_CLICK);
        OnMode(&event);
    }
}

struct WepGroup {
    Text name;
    int  count;

    WepGroup() : count(0) { }
};
WepGroup* FindWepGroup(WepGroup* weapons, const char* name);


void
TacRefDlg::SelectShip(const ShipDesign* design)
{
    if (beauty && camview) {
        scene.Graphics().clear();

        if (design) {
            radius = design->radius;

            UpdateCamera();

            int level = design->lod_levels-1;
            int n     = design->models[level].size();

            for (int i = 0; i < n; i++) {
                Model* model = design->models[level].at(i);

                Solid* s = new(__FILE__,__LINE__) Solid;
                s->UseModel(model);
                s->CreateShadows(1);
                s->MoveTo(*design->offsets[level].at(i));

                scene.Graphics().append(s);
            }
        }
    }

    if (txt_caption) {
        txt_caption->SetText("");

        if (design) {
            char txt[256];
            sprintf_s(txt, "%s %s", design->abrv, design->DisplayName());
            txt_caption->SetText(txt);
        }
    }

    if (txt_stats) {
        txt_stats->SetText("");

        if (design) {
            Text desc;
            char txt[256];

            sprintf_s(txt, "%s\t\t\t%s\n", Game::GetText("tacref.type").data(), Ship::ClassName(design->type));
            desc += txt;

            sprintf_s(txt, "%s\t\t\t%s\n", Game::GetText("tacref.class").data(), design->DisplayName());
            desc += txt;
            desc += Game::GetText("tacref.length");
            desc += "\t\t";

            if (design->type < Ship::STATION)
            FormatNumber(txt, design->radius/2);
            else
            FormatNumber(txt, design->radius*2);

            strcat_s(txt, " m\n");
            desc += txt;
            desc += Game::GetText("tacref.mass");
            desc += "\t\t\t";

            FormatNumber(txt, design->mass);
            strcat_s(txt, " T\n");
            desc += txt;
            desc += Game::GetText("tacref.hull");
            desc += "\t\t\t";

            FormatNumber(txt, design->integrity);
            strcat_s(txt, "\n");
            desc += txt;

            if (design->weapons.size()) {
                desc += Game::GetText("tacref.weapons");

                WepGroup groups[8];
                for (int w = 0; w < design->weapons.size(); w++) {
                    Weapon*     gun   = design->weapons[w];
                    WepGroup*   group = FindWepGroup(groups, gun->Group());

                    if (group)
                    group->count++;
                }

                for (int g = 0; g < 8; g++) {
                    WepGroup* group = &groups[g];
                    if (group && group->count) {
                        sprintf_s(txt, "\t\t%s (%d)\n\t\t", group->name.data(), group->count);
                        desc += txt;

                        for (int w = 0; w < design->weapons.size(); w++) {
                            Weapon* gun = design->weapons[w];

                            if (group->name == gun->Group()) {
                                sprintf_s(txt, "\t\t\t%s\n\t\t", (const char*) gun->Design()->name);
                                desc += txt;
                            }
                        }
                    }
                }

                desc += "\n";
            }

            txt_stats->SetText(desc);
        }
    }

    if (txt_description) {
        if (design && design->description.length()) {
            txt_description->SetText(design->description);
        }
        else {
            txt_description->SetText(Game::GetText("tacref.mass"));
        }
    }
}

// +--------------------------------------------------------------------+

void
TacRefDlg::SelectWeapon(const WeaponDesign* design)
{
    if (beauty && imgview) {
        imgview->SetPicture(0);

        if (design)
        imgview->SetPicture(design->beauty_img);
    }

    if (txt_caption) {
        txt_caption->SetText("");

        if (design)
        txt_caption->SetText(design->name);
    }

    if (txt_stats) {
        txt_stats->SetText("");

        if (design) {
            Text desc;
            char txt[256];

            desc =  Game::GetText("tacref.name");
            desc += "\t";
            desc += design->name;
            desc += "\n";
            desc += Game::GetText("tacref.type");
            desc += "\t\t";

            if (design->damage < 1)
            desc += Game::GetText("tacref.wep.other");
            else if (design->beam)
            desc += Game::GetText("tacref.wep.beam");
            else if (design->primary)
            desc += Game::GetText("tacref.wep.bolt");
            else if (design->drone)
            desc += Game::GetText("tacref.wep.drone");
            else if (design->guided)
            desc += Game::GetText("tacref.wep.guided");
            else
            desc += Game::GetText("tacref.wep.missile");

            if (design->turret_model && design->damage >= 1) {
                desc += " ";
                desc += Game::GetText("tacref.wep.turret");
                desc += "\n";
            }
            else {
                desc += "\n";
            }

            desc += Game::GetText("tacref.targets");
            desc += "\t";

            if ((design->target_type & Ship::DROPSHIPS) != 0) {
                if ((design->target_type & Ship::STARSHIPS) != 0) {
                    if ((design->target_type & Ship::GROUND_UNITS) != 0) {
                        desc += Game::GetText("tacref.targets.fsg");
                    }
                    else {
                        desc += Game::GetText("tacref.targets.fs");
                    }
                }
                else {
                    if ((design->target_type & Ship::GROUND_UNITS) != 0) {
                        desc += Game::GetText("tacref.targets.fg");
                    }
                    else {
                        desc += Game::GetText("tacref.targets.f");
                    }
                }
            }

            else if ((design->target_type & Ship::STARSHIPS) != 0) {
                if ((design->target_type & Ship::GROUND_UNITS) != 0) {
                    desc += Game::GetText("tacref.targets.sg");
                }
                else {
                    desc += Game::GetText("tacref.targets.s");
                }
            }

            else if ((design->target_type & Ship::GROUND_UNITS) != 0) {
                desc += Game::GetText("tacref.targets.g");
            }

            desc += "\n";
            desc += Game::GetText("tacref.speed");
            desc += "\t";

            FormatNumber(txt, design->speed);
            desc += txt;
            desc += "m/s\n";
            desc += Game::GetText("tacref.range");
            desc += "\t";

            FormatNumber(txt, design->max_range);
            desc += txt;
            desc += "m\n";
            desc += Game::GetText("tacref.damage");
            desc += "\t";

            if (design->damage > 0) {
                FormatNumber(txt, design->damage * design->charge);
                desc += txt;
                if (design->beam)
                desc += "/s";
            }
            else {
                desc += Game::GetText("tacref.none");
            }

            desc += "\n";
            
            if (!design->primary && design->damage > 0) {
                desc += Game::GetText("tacref.kill-radius");
                desc += "\t";
                FormatNumber(txt, design->lethal_radius);
                desc += txt;
                desc += " m\n";
            }

            txt_stats->SetText(desc);
        }
    }

    if (txt_description) {
        if (design && design->description.length()) {
            txt_description->SetText(design->description);
        }
        else {
            txt_description->SetText(Game::GetText("tacref.no-info"));
        }
    }
}

// +--------------------------------------------------------------------+

void
TacRefDlg::ExecFrame()
{
}

// +--------------------------------------------------------------------+

bool
TacRefDlg::SetCaptureBeauty()
{
    EventDispatch* dispatch = EventDispatch::GetInstance();
    if (dispatch && beauty)
    return dispatch->CaptureMouse(beauty) ? true : false;

    return 0;
}

bool
TacRefDlg::ReleaseCaptureBeauty()
{
    EventDispatch* dispatch = EventDispatch::GetInstance();
    if (dispatch && beauty)
    return dispatch->ReleaseMouse(beauty) ? true : false;

    return 0;
}

// +--------------------------------------------------------------------+

void
TacRefDlg::UpdateAzimuth(double a)
{
    cam_az += a;

    if (cam_az > PI)
    cam_az = -2*PI + cam_az;

    else if (cam_az < -PI)
    cam_az = 2*PI + cam_az;
}

void
TacRefDlg::UpdateElevation(double e)
{
    cam_el += e;

    const double limit = (0.43 * PI);

    if (cam_el > limit)
    cam_el = limit;
    else if (cam_el < -limit)
    cam_el = -limit;
}

void
TacRefDlg::UpdateZoom(double delta)
{
    cam_zoom *= delta;

    if (cam_zoom < 1.2)
    cam_zoom = 1.2;

    else if (cam_zoom > 10)
    cam_zoom = 10;
}

void
TacRefDlg::UpdateCamera()
{
    double x = cam_zoom * radius * sin(cam_az) * cos(cam_el);
    double y = cam_zoom * radius * cos(cam_az) * cos(cam_el);
    double z = cam_zoom * radius *               sin(cam_el);

    cam.LookAt(Point(0,0,0), Point(x,z,y), Point(0,1,0));
}

// +--------------------------------------------------------------------+

void
TacRefDlg::OnSelect(AWEvent* event)
{
    if (lst_designs) {
        int   seln  = lst_designs->GetSelection();
        DWORD dsn   = lst_designs->GetItemData(seln);

        if (mode == MODE_SHIPS) {
            ship_index = seln;

            if (dsn) {
                SelectShip((ShipDesign*) dsn);
            }
        }

        else if (mode == MODE_WEAPONS) {
            weap_index = seln;

            if (dsn) {
                SelectWeapon((WeaponDesign*) dsn);
            }
        }
    }
}

// +--------------------------------------------------------------------+

void
TacRefDlg::OnCamRButtonDown(AWEvent* event)
{
    captured = SetCaptureBeauty();
    mouse_x  = event->x;
    mouse_y  = event->y;
}

void
TacRefDlg::OnCamRButtonUp(AWEvent* event)
{
    if (captured)
    ReleaseCaptureBeauty();

    captured = false;
    mouse_x  = 0;
    mouse_y  = 0;
}

void
TacRefDlg::OnCamMove(AWEvent* event)
{
    if (captured) {
        int mouse_dx = event->x - mouse_x;
        int mouse_dy = event->y - mouse_y;

        UpdateAzimuth(    mouse_dx * 0.3 * DEGREES);
        UpdateElevation(  mouse_dy * 0.3 * DEGREES);
        UpdateCamera();

        mouse_x = event->x;
        mouse_y = event->y;
    }
}

void
TacRefDlg::OnCamZoom(AWEvent* event)
{
    int w = Mouse::Wheel();

    if (w < 0) {
        while (w < 0) {
            UpdateZoom(1.25);
            w += 120;
        }
    }
    else {
        while (w > 0) {
            UpdateZoom(0.75f);
            w -= 120;
        }
    }

    UpdateCamera();
}

// +--------------------------------------------------------------------+

void
TacRefDlg::OnMode(AWEvent* event)
{
    if (event->window == btn_ships && mode != MODE_SHIPS) {
        mode = MODE_SHIPS;

        if (lst_designs) {
            lst_designs->ClearItems();

            List<Text> designs;

            for (int n = 0; n < 16; n++) {
                int type = 1 << n;
                ShipDesign::GetDesignList(type, designs);

                ListIter<Text> iter = designs;
                while (++iter) {
                    Text* val = iter.value();

                    const ShipDesign* dsn = ShipDesign::Get(*val);

                    if (dsn) {
                        char txt[256];
                        sprintf_s(txt, "%s %s", dsn->abrv, dsn->DisplayName());

                        lst_designs->AddItemWithData(txt, (DWORD) dsn);
                    }
                    else {
                        lst_designs->AddItemWithData(*val, 0);
                    }
                }
            }

            lst_designs->SetSelected(ship_index);
        }

        if (beauty) {
            beauty->AddView(camview);
            beauty->DelView(imgview);
        }

        DWORD dsn = lst_designs->GetItemData(ship_index);

        if (dsn) {
            SelectShip((ShipDesign*) dsn);
        }
    }

    else if (event->window == btn_weaps && mode != MODE_WEAPONS) {
        mode = MODE_WEAPONS;

        const WeaponDesign* design = 0;

        if (lst_designs) {
            lst_designs->ClearItems();
            List<Text> designs;

            WeaponDesign::GetDesignList(designs);

            ListIter<Text> iter = designs;
            while (++iter) {
                Text* val = iter.value();

                if (val->contains("Zolon") || val->contains("Inverted"))
                continue;

                const WeaponDesign* dsn = WeaponDesign::Find(*val);

                if (dsn && !dsn->secret) {
                    lst_designs->AddItemWithData(*val, (DWORD) dsn);

                    if (!design)
                    design = dsn;
                }
            }

            lst_designs->SetSelected(weap_index);
        }

        if (beauty) {
            beauty->DelView(camview);
            beauty->AddView(imgview);
        }

        DWORD dsn = lst_designs->GetItemData(weap_index);

        if (dsn) {
            SelectWeapon((WeaponDesign*) dsn);
        }
    }

    btn_ships->SetButtonState(mode == MODE_SHIPS);
    btn_weaps->SetButtonState(mode == MODE_WEAPONS);
}

// +--------------------------------------------------------------------+

void
TacRefDlg::OnClose(AWEvent* event)
{
    manager->ShowMenuDlg();
}

