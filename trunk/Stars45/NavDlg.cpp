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
    FILE:         NavDlg.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
*/

#include "MemDebug.h"
#include "NavDlg.h"
#include "MapView.h"
#include "MsnElemDlg.h"
#include "BaseScreen.h"
#include "Element.h"
#include "Ship.h"
#include "ShipDesign.h"
#include "Sim.h"
#include "Galaxy.h"
#include "StarSystem.h"
#include "Instruction.h"
#include "NavSystem.h"
#include "FormatUtil.h"
#include "Campaign.h"
#include "Contact.h"
#include "Mission.h"

#include "Game.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "ListBox.h"

// +--------------------------------------------------------------------+
// DECLARE MAPPING FUNCTIONS:
DEF_MAP_CLIENT(NavDlg, OnView);
DEF_MAP_CLIENT(NavDlg, OnFilter);
DEF_MAP_CLIENT(NavDlg, OnSelect);
DEF_MAP_CLIENT(NavDlg, OnCommit);
DEF_MAP_CLIENT(NavDlg, OnCancel);
DEF_MAP_CLIENT(NavDlg, OnEngage);
DEF_MAP_CLIENT(NavDlg, OnMapDown);
DEF_MAP_CLIENT(NavDlg, OnMapMove);
DEF_MAP_CLIENT(NavDlg, OnMapClick);
DEF_MAP_CLIENT(NavDlg, OnClose);

// +--------------------------------------------------------------------+

static char*   filter_name[] = {
    "SYSTEM",   "PLANET", 
    "SECTOR",   "STATION",
    "STARSHIP", "FIGHTER"
};

static char*   commit_name = "Commit";
static char*   cancel_name = "Cancel";

static Color   commit_color(53,159,67);
static Color   cancel_color(160,8,8);


// Supported Selection Modes:

const int SELECT_NONE      = -1;
const int SELECT_SYSTEM    =  0;
const int SELECT_PLANET    =  1;
const int SELECT_REGION    =  2;
const int SELECT_STATION   =  3;
const int SELECT_STARSHIP  =  4;
const int SELECT_FIGHTER   =  5;

const int VIEW_GALAXY      =  0;
const int VIEW_SYSTEM      =  1;
const int VIEW_REGION      =  2;

// +--------------------------------------------------------------------+

NavDlg::NavDlg(Screen* s, FormDef& def, BaseScreen* mgr)
: FormWindow(s, 0, 0, s->Width(), s->Height()), manager(mgr),
loc_labels(0), dst_labels(0), loc_data(0),   dst_data(0),  
seln_list(0), info_list(0), seln_mode(SELECT_REGION), 
nav_edit_mode(NAV_EDIT_NONE), star_map(0), map_win(0),
star_system(0), ship(0), mission(0), editor(false)
{
    Init(def);
}

NavDlg::~NavDlg()
{ }

void
NavDlg::RegisterControls()
{
    int i;

    map_win = FindControl(100);

    if (map_win) {
        star_map = new(__FILE__,__LINE__) MapView(map_win);

        REGISTER_CLIENT(EID_LBUTTON_DOWN, map_win, NavDlg, OnMapDown);
        REGISTER_CLIENT(EID_MOUSE_MOVE,   map_win, NavDlg, OnMapMove);
        REGISTER_CLIENT(EID_MAP_CLICK,    map_win, NavDlg, OnMapClick);
    }

    for (i = 0; i < 3; i++) {
        view_btn[i] = (Button*) FindControl(101 + i);
        REGISTER_CLIENT(EID_CLICK, view_btn[i], NavDlg, OnView);
    }

    close_btn    = (Button*) FindControl(2);

    if (close_btn)
    REGISTER_CLIENT(EID_CLICK,    close_btn, NavDlg, OnClose);

    zoom_in_btn  = (Button*) FindControl(110);
    zoom_out_btn = (Button*) FindControl(111);

    for (i = 0; i < 6; i++) {
        filter_btn[i] = (Button*) FindControl(401 + i);
        REGISTER_CLIENT(EID_CLICK, filter_btn[i], NavDlg, OnFilter);
    }

    commit_btn = (Button*) FindControl(1);

    if (commit_btn) {
        REGISTER_CLIENT(EID_CLICK, commit_btn, NavDlg, OnEngage);
    }

    loc_labels = FindControl(601);
    dst_labels = FindControl(602);
    loc_data   = FindControl(701);
    dst_data   = FindControl(702);

    seln_list  = (ListBox*) FindControl(801);

    if (seln_list) {
        seln_list->SetSelectedStyle(ListBox::LIST_ITEM_STYLE_FILLED_BOX);
        REGISTER_CLIENT(EID_SELECT, seln_list, NavDlg, OnSelect);
    }

    info_list  = (ListBox*) FindControl(802);

    if (star_map) {
        star_map->SetViewMode(VIEW_SYSTEM);
        view_btn[1]->SetButtonState(1);

        star_map->SetSelectionMode(2);
    }

    UpdateSelection();
}

// +--------------------------------------------------------------------+

void
NavDlg::SetSystem(StarSystem* s)
{
    if (star_system == s)
    return;

    star_system = s;

    if (star_map) {
        Campaign* c    = Campaign::GetCampaign();
        Sim*      sim  = Sim::GetSim();

        if (sim && sim->GetSystemList().size()) {
            star_map->SetGalaxy(sim->GetSystemList());
        }
        else if (mission && mission->GetSystemList().size()) {
            star_map->SetGalaxy(mission->GetSystemList());
        }
        else if (c && c->GetSystemList().size()) {
            star_map->SetGalaxy(c->GetSystemList());
        }
        else {
            Galaxy* g = Galaxy::GetInstance();
            if (g)
            star_map->SetGalaxy(g->GetSystemList());
        }

        star_map->SetSystem(s);
    }

    // flush old object pointers:
    stars.clear();
    planets.clear();
    regions.clear();
    contacts.clear();

    if (star_system) {
        // insert objects from star system:
        ListIter<OrbitalBody> star = star_system->Bodies();
        while (++star) {
            switch (star->Type()) {
            case Orbital::STAR:     stars.append(star.value());
                break;
            case Orbital::PLANET:
            case Orbital::MOON:     planets.append(star.value());
                break;
            }

            ListIter<OrbitalBody> planet = star->Satellites();
            while (++planet) {
                planets.append(planet.value());
                
                ListIter<OrbitalBody> moon = planet->Satellites();
                while (++moon) {
                    planets.append(moon.value());
                }
            }
        }

        ListIter<OrbitalRegion> rgn = star_system->AllRegions();
        while (++rgn)
        regions.append(rgn.value());
    }

    // sort region list by distance from the star:
    regions.sort();
}

// +--------------------------------------------------------------------+

void
NavDlg::SetShip(Ship* s)
{
    if (ship == s)
    return;

    ship = s;

    if (ship)
    SetSystem(ship->GetRegion()->System());

    if (star_map) {
        Sim* sim = Sim::GetSim();

        if (sim && sim->GetSystemList().size())
        star_map->SetGalaxy(sim->GetSystemList());

        star_map->SetShip(ship);

        if (ship) {
            star_map->SetRegion(ship->GetRegion()->GetOrbitalRegion());
            UseViewMode(VIEW_REGION);
            star_map->SetSelectedShip(ship);
        }
    }

    for (int i = 0; i < 6; i++)
    filter_btn[i]->SetButtonState(0);

    filter_btn[SELECT_REGION]->SetButtonState(1);
    UseFilter(SELECT_STARSHIP);
    UpdateSelection();
}

// +--------------------------------------------------------------------+

void
NavDlg::SetMission(Mission* m)
{
    if (!m && mission == m)
    return;

    if (mission == m && star_system == m->GetStarSystem())
    return;

    mission = m;

    if (mission) {
        SetSystem(mission->GetStarSystem());
    }

    if (star_map) {
        Campaign* c    = Campaign::GetCampaign();
        Sim*      sim  = Sim::GetSim();

        star_map->SetMission(0);   // prevent building map menu twice

        if (sim && sim->GetSystemList().size()) {
            star_map->SetGalaxy(sim->GetSystemList());
        }
        else if (mission && mission->GetSystemList().size()) {
            star_map->SetGalaxy(mission->GetSystemList());
        }
        else if (c && c->GetSystemList().size()) {
            star_map->SetGalaxy(c->GetSystemList());
        }
        else {
            Galaxy* g = Galaxy::GetInstance();
            if (g)
            star_map->SetGalaxy(g->GetSystemList());
        }

        if (mission) {
            star_map->SetMission(mission);
            star_map->SetRegionByName(mission->GetRegion());

            if (star_map->GetViewMode() == VIEW_REGION) {
                ListIter<MissionElement> elem = mission->GetElements();
                while (++elem) {
                    MissionElement* e = elem.value();

                    if (e->Player())
                    star_map->SetSelectedElem(e);
                }
            }
        }
    }

    bool updated = false;

    if (mission) {
        Orbital* rgn = 0;
        rgn = mission->GetStarSystem()->FindOrbital(mission->GetRegion());

        if (rgn) {
            SelectRegion((OrbitalRegion*) rgn);
            updated = true;
        }
    }

    if (!updated)
    UpdateSelection();
}

// +--------------------------------------------------------------------+

void
NavDlg::SetEditorMode(bool e)
{
    editor = e;

    if (star_map)
    star_map->SetEditorMode(editor);
}

// +--------------------------------------------------------------------+

void
NavDlg::ExecFrame()
{
    Sim* sim = Sim::GetSim();

    if (loc_labels && ship) {
        char loc_buf[512];
        char x[16];
        char y[16];
        char z[16];
        char d[16];
        
        FormatNumber(x, -ship->Location().x);
        FormatNumber(y,  ship->Location().z);
        FormatNumber(z,  ship->Location().y);
        
        strcpy_s(loc_buf, Game::GetText("NavDlg.loc-labels").data());
        loc_labels->SetText(loc_buf);

        if (sim->GetActiveRegion()) {
            sprintf_s(loc_buf, "\n%s\n%s\n%s, %s, %s",
            (const char*)  star_system->Name(),
            (const char*)  sim->GetActiveRegion()->Name(),
            x, y, z);

        }
        else {
            sprintf_s(loc_buf, "\n%s\nPlanck Space?\n%s, %s, %s",
            (const char*)  star_system->Name(), x, y, z);
        }

        loc_data->SetText(loc_buf);
        
        if (ship) {
            NavSystem* navsys = ship->GetNavSystem();

            if (ship->GetNextNavPoint() == 0 || !navsys) {
                commit_btn->SetText(Game::GetText("NavDlg.commit"));
                commit_btn->SetBackColor(commit_color);
                commit_btn->SetEnabled(false);
            }
            else if (navsys) {
                commit_btn->SetEnabled(true);

                if (navsys->AutoNavEngaged()) {
                    commit_btn->SetText(Game::GetText("NavDlg.cancel"));
                    commit_btn->SetBackColor(cancel_color);
                }
                else {
                    commit_btn->SetText(Game::GetText("NavDlg.commit"));
                    commit_btn->SetBackColor(commit_color);
                }
            }
        }

        if (dst_labels) {
            Instruction* navpt = ship->GetNextNavPoint();
            
            if (navpt && navpt->Region()) {
                FormatNumber(x, navpt->Location().x);
                FormatNumber(y, navpt->Location().y);
                FormatNumber(z, navpt->Location().z);

                double distance = 0;
                Point npt = navpt->Region()->Location() + navpt->Location();
                if (sim->GetActiveRegion())
                npt -= sim->GetActiveRegion()->Location();

                npt = npt.OtherHand();
                
                // distance from self to navpt:
                distance = Point(npt - ship->Location()).length();
                FormatNumber(d, distance);
                
                strcpy_s(loc_buf, Game::GetText("NavDlg.dst-labels").data());
                dst_labels->SetText(loc_buf);

                sprintf_s(loc_buf, "\n%s\n%s\n%s, %s, %s\n%s",
                (const char*)  star_system->Name(),
                (const char*)  navpt->Region()->Name(),
                x, y, z, d);
                dst_data->SetText(loc_buf);
            }
            else {
                dst_labels->SetText(Game::GetText("NavDlg.destination"));
                dst_data->SetText(Game::GetText("NavDlg.not-avail"));
            }
        }
    }

    UpdateSelection();
    UpdateLists();

    if (Keyboard::KeyDown(VK_ADD) ||
            (zoom_in_btn && zoom_in_btn->GetButtonState() > 0)) {
        star_map->ZoomIn();
    }
    else if (Keyboard::KeyDown(VK_SUBTRACT) || 
            (zoom_out_btn && zoom_out_btn->GetButtonState() > 0)) {
        star_map->ZoomOut();
    }

    else if (star_map->TargetRect().Contains(Mouse::X(),Mouse::Y())) {

        if (Mouse::Wheel() > 0) {
            star_map->ZoomIn();
            star_map->ZoomIn();
            star_map->ZoomIn();
        }

        else if (Mouse::Wheel() < 0) {
            star_map->ZoomOut();
            star_map->ZoomOut();
            star_map->ZoomOut();
        }
    }

    if (nav_edit_mode == NAV_EDIT_NONE)
    Mouse::SetCursor(Mouse::ARROW);
    else
    Mouse::SetCursor(Mouse::CROSS);
}

// +--------------------------------------------------------------------+

void
NavDlg::OnView(AWEvent* event)
{
    int use_filter_mode = -1;

    view_btn[VIEW_GALAXY]->SetButtonState(0);
    view_btn[VIEW_SYSTEM]->SetButtonState(0);
    view_btn[VIEW_REGION]->SetButtonState(0);

    if (view_btn[0] == event->window) {
        star_map->SetViewMode(VIEW_GALAXY);
        view_btn[VIEW_GALAXY]->SetButtonState(1);
        use_filter_mode = SELECT_SYSTEM;
    }

    else if (view_btn[VIEW_SYSTEM] == event->window) {
        star_map->SetViewMode(VIEW_SYSTEM);
        view_btn[VIEW_SYSTEM]->SetButtonState(1);
        use_filter_mode = SELECT_REGION;
    }

    else if (view_btn[VIEW_REGION] == event->window) {
        star_map->SetViewMode(VIEW_REGION);
        view_btn[VIEW_REGION]->SetButtonState(1);
        use_filter_mode = SELECT_STARSHIP;
    }

    if (use_filter_mode >= 0) {
        for (int i = 0; i < 6; i++) {
            if (i == use_filter_mode)
            filter_btn[i]->SetButtonState(1);
            else
            filter_btn[i]->SetButtonState(0);
        }

        UseFilter(use_filter_mode);
    }
}

// +--------------------------------------------------------------------+

void
NavDlg::OnFilter(AWEvent* event)
{
    int filter_index = -1;
    for (int i = 0; i < 6; i++) {
        if (filter_btn[i] == event->window) {
            filter_index = i;
            filter_btn[i]->SetButtonState(1);
        }
        else {
            filter_btn[i]->SetButtonState(0);
        }
    }

    if (filter_index >= 0)
    UseFilter(filter_index);
}

void
NavDlg::UseFilter(int filter_index)
{
    seln_mode = filter_index;

    star_map->SetSelectionMode(seln_mode);
    UpdateSelection();
    UpdateLists();
}

// +--------------------------------------------------------------------+

void
NavDlg::UseViewMode(int mode)
{
    if (mode >= 0 && mode < 3) {
        int use_filter_mode = -1;

        view_btn[VIEW_GALAXY]->SetButtonState(0);
        view_btn[VIEW_SYSTEM]->SetButtonState(0);
        view_btn[VIEW_REGION]->SetButtonState(0);

        if (mode == 0) {
            star_map->SetViewMode(VIEW_GALAXY);
            view_btn[VIEW_GALAXY]->SetButtonState(1);
            use_filter_mode = SELECT_SYSTEM;
        }

        else if (mode == 1) {
            star_map->SetViewMode(VIEW_SYSTEM);
            view_btn[VIEW_SYSTEM]->SetButtonState(1);
            use_filter_mode = SELECT_REGION;
        }

        else if (mode == 2) {
            star_map->SetViewMode(VIEW_REGION);
            view_btn[VIEW_REGION]->SetButtonState(1);
            use_filter_mode = SELECT_STARSHIP;
        }

        if (use_filter_mode >= 0) {
            for (int i = 0; i < 6; i++) {
                filter_btn[i]->SetButtonState(i == use_filter_mode);
            }

            UseFilter(use_filter_mode);
        }
    }
}

void
NavDlg::SelectStar(Orbital* star)
{
    UseViewMode(0);

    if (stars.size()) {
        int sel = 0;

        ListIter<Orbital> iter = stars;
        while (++iter) {
            if (iter.value() == star) {
                int old_seln_mode = seln_mode;
                UseFilter(SELECT_SYSTEM);
                SelectObject(sel);
                UseFilter(old_seln_mode);
                return;
            }

            sel++;
        }
    }
}

void
NavDlg::SelectPlanet(Orbital* planet)
{
    UseViewMode(1);

    if (planets.size()) {
        int sel = 0;

        ListIter<Orbital> iter = planets;
        while (++iter) {
            if (iter.value() == planet) {
                int old_seln_mode = seln_mode;
                UseFilter(SELECT_PLANET);
                SelectObject(sel);
                UseFilter(old_seln_mode);
                return;
            }

            sel++;
        }
    }
}

void
NavDlg::SelectRegion(OrbitalRegion* rgn)
{
    UseViewMode(2);

    if (regions.size()) {
        int sel = 0;

        ListIter<OrbitalRegion> iter = regions;
        while (++iter) {
            if (iter.value() == rgn) {
                int old_seln_mode = seln_mode;
                UseFilter(SELECT_REGION);
                SelectObject(sel);
                UseFilter(old_seln_mode);
                return;
            }

            sel++;
        }
    }
}

// +--------------------------------------------------------------------+

void
NavDlg::OnSelect(AWEvent* event)
{
    int   index = -1;

    for (int i = 0; i < seln_list->NumItems(); i++)
    if (seln_list->IsSelected(i))
    index = i;

    if (index >= 0)
    SelectObject(index);
}

// +--------------------------------------------------------------------+

void
NavDlg::SelectObject(int index)
{
    Text selected  = seln_list->GetItemText(index);
    int  selection = seln_list->GetItemData(index);

    star_map->SetSelection(selection);
    SetSystem(star_map->GetSystem());
}

// +--------------------------------------------------------------------+

void
NavDlg::UpdateSelection()
{
    if (!info_list)
    return;

    if (!star_map)
    return;

    info_list->ClearItems();

    Text units_km     = Text(" ") + Game::GetText("NavDlg.units.kilometers");
    Text units_tonnes = Text(" ") + Game::GetText("NavDlg.units.tonnes");

    if (seln_mode <= SELECT_REGION) {
        Orbital* s = star_map->GetSelection();

        if (s) {
            char radius[32];
            char mass[32];
            char orbit[32];
            char period[32];
            char units[32];

            double p = s->Period();

            if (p < 60) {
                sprintf_s(units, " %s", Game::GetText("NavDlg.units.seconds").data());
            }
            else if (p < 3600) {
                p /= 60;
                sprintf_s(units, " %s", Game::GetText("NavDlg.units.minutes").data());
            }
            else if (p < 24 * 3600) {
                p /= 3600;
                sprintf_s(units, " %s", Game::GetText("NavDlg.units.hours").data());
            }
            else if (p < 365.25 * 24 * 3600) {
                p /= 24*3600;
                sprintf_s(units, " %s", Game::GetText("NavDlg.units.days").data());
            }
            else {
                p /= 365.25*24*3600;
                sprintf_s(units, " %s", Game::GetText("NavDlg.units.years").data());
            }

            FormatNumberExp(radius, s->Radius()/1000);
            FormatNumberExp(mass,   s->Mass()/1000);
            FormatNumberExp(orbit,  s->Orbit()/1000);
            FormatNumberExp(period, p);

            strcat_s(radius, units_km.data());
            strcat_s(mass,   units_tonnes.data());
            strcat_s(orbit,  units_km.data());
            strcat_s(period, units);

            if (seln_mode >= SELECT_SYSTEM) {
                info_list->AddItem(Game::GetText(Text("NavDlg.filter.") + filter_name[seln_mode]));
                info_list->AddItem(Game::GetText("NavDlg.radius"));
                if (s->Mass() > 0)
                info_list->AddItem(Game::GetText("NavDlg.mass"));
                info_list->AddItem(Game::GetText("NavDlg.orbit"));
                info_list->AddItem(Game::GetText("NavDlg.period"));

                int row = 0;
                info_list->SetItemText(row++, 1, s->Name());
                info_list->SetItemText(row++, 1, radius);
                if (s->Mass() > 0)
                info_list->SetItemText(row++, 1, mass);
                info_list->SetItemText(row++, 1, orbit);
                info_list->SetItemText(row++, 1, period);
            }
        }
    }

    else if (seln_mode == SELECT_STATION  || 
            seln_mode == SELECT_STARSHIP ||
            seln_mode == SELECT_FIGHTER) {

        Ship*           sel_ship = star_map->GetSelectedShip();
        MissionElement* sel_elem = star_map->GetSelectedElem();

        if (sel_ship) {
            Text order_desc = Game::GetText("NavDlg.none");
            char shield[16];
            char hull[16];
            char range[32];

            sprintf_s(shield, "%03d", sel_ship->ShieldStrength());
            sprintf_s(hull,   "%03d", sel_ship->HullStrength());
            sprintf_s(range,  "%s",  Game::GetText("NavDlg.not-avail").data());

            if (ship) {
                FormatNumberExp(range, Point(sel_ship->Location()-ship->Location()).length()/1000);
                strcat_s(range, units_km.data());
            }

            info_list->AddItem(Game::GetText("NavDlg.name"));
            info_list->AddItem(Game::GetText("NavDlg.class"));
            info_list->AddItem(Game::GetText("NavDlg.sector"));
            info_list->AddItem(Game::GetText("NavDlg.shield"));
            info_list->AddItem(Game::GetText("NavDlg.hull"));
            info_list->AddItem(Game::GetText("NavDlg.range"));
            info_list->AddItem(Game::GetText("NavDlg.orders"));

            int row = 0;
            info_list->SetItemText(row++, 1, sel_ship->Name());
            info_list->SetItemText(row++, 1, Text(sel_ship->Abbreviation()) + Text(" ") + Text(sel_ship->Design()->display_name));
            info_list->SetItemText(row++, 1, sel_ship->GetRegion()->Name());
            info_list->SetItemText(row++, 1, shield);
            info_list->SetItemText(row++, 1, hull);
            info_list->SetItemText(row++, 1, range);
            info_list->SetItemText(row++, 1, order_desc);
        }

        else if (sel_elem) {
            Text order_desc = Game::GetText("NavDlg.none");
            char range[32];

            MissionElement* self = mission->GetElements()[0];
            if (self)
            FormatNumberExp(range, Point(sel_elem->Location()-self->Location()).length()/1000);
            else
            strcpy_s(range, "0");

            strcat_s(range, units_km.data());

            info_list->AddItem(Game::GetText("NavDlg.name"));
            info_list->AddItem(Game::GetText("NavDlg.class"));
            info_list->AddItem(Game::GetText("NavDlg.sector"));
            info_list->AddItem(Game::GetText("NavDlg.range"));
            info_list->AddItem(Game::GetText("NavDlg.orders"));

            int row = 0;
            info_list->SetItemText(row++, 1, sel_elem->Name());

            if (sel_elem->GetDesign())
            info_list->SetItemText(row++, 1, sel_elem->Abbreviation() + Text(" ") + sel_elem->GetDesign()->name);
            else
            info_list->SetItemText(row++, 1, Game::GetText("NavDlg.unknown"));

            info_list->SetItemText(row++, 1, sel_elem->Region());
            info_list->SetItemText(row++, 1, range);
            info_list->SetItemText(row++, 1, order_desc);
        }
    }
}

// +--------------------------------------------------------------------+

void
NavDlg::UpdateLists()
{
    if (!seln_list)
    return;

    int seln_index = -1;
    int top_index  = -1;

    if (seln_list->IsSelecting())
    seln_index = seln_list->GetListIndex();

    top_index = seln_list->GetTopIndex();

    seln_list->ClearItems();

    switch (seln_mode) {
    case SELECT_SYSTEM: 
        {
            seln_list->SetColumnTitle(0, Game::GetText(Text("NavDlg.filter.") + filter_name[seln_mode]));
            int i = 0;
            ListIter<StarSystem> iter = star_map->GetGalaxy();
            while (++iter)
            seln_list->AddItemWithData(iter->Name(), i++);
        }
        break;

    case SELECT_PLANET:
        {
            seln_list->SetColumnTitle(0, Game::GetText(Text("NavDlg.filter.") + filter_name[seln_mode]));
            int i = 0;
            ListIter<Orbital> iter = planets;
            while (++iter) {
                if (iter->Type() == Orbital::MOON)
                seln_list->AddItemWithData(Text("- ") + Text(iter->Name()), i++);
                else
                seln_list->AddItemWithData(iter->Name(), i++);
            }
        }
        break;

    case SELECT_REGION:
        {
            seln_list->SetColumnTitle(0, Game::GetText(Text("NavDlg.filter.") + filter_name[seln_mode]));
            int i = 0;
            ListIter<OrbitalRegion> iter = regions;
            while (++iter) {
                seln_list->AddItemWithData(iter->Name(), i++);
            }
        }
        break;

    case SELECT_STATION:
    case SELECT_STARSHIP:
    case SELECT_FIGHTER:
        {
            seln_list->SetColumnTitle(0, Game::GetText(Text("NavDlg.filter.") + filter_name[seln_mode]));
            int i = 0;

            if (mission) {
                ListIter<MissionElement> elem = mission->GetElements();
                while (++elem) {
                    MissionElement* e = elem.value();
                    bool filter_ok =
                    (seln_mode == SELECT_STATION  && e->IsStatic())                      ||
                    (seln_mode == SELECT_STARSHIP && e->IsStarship() && !e->IsStatic())  ||
                    (seln_mode == SELECT_FIGHTER  && e->IsDropship() && !e->IsSquadron());

                    if (filter_ok) {
                        bool visible = editor                           ||
                        e->GetIFF() == 0                 ||
                        e->GetIFF() == mission->Team()   ||
                        e->IntelLevel() > Intel::KNOWN;

                        if (visible)
                        seln_list->AddItemWithData(e->Name(), e->Identity());
                    }
                }
            }

            else if (ship) {
                Sim* sim = Sim::GetSim();
                ListIter<SimRegion> r_iter = sim->GetRegions();
                while (++r_iter) {
                    SimRegion* rgn = r_iter.value();

                    ListIter<Ship> s_iter = rgn->Ships();
                    while (++s_iter) {
                        Ship* s = s_iter.value();
                        bool filter_ok =
                        (seln_mode == SELECT_STATION  && s->IsStatic())                      ||
                        (seln_mode == SELECT_STARSHIP && s->IsStarship() && !s->IsStatic())  ||
                        (seln_mode == SELECT_FIGHTER  && s->IsDropship());


                        if (filter_ok) {
                            bool visible = s->GetIFF() == 0 ||
                            s->GetIFF() == ship->GetIFF() ||
                            s->GetElement() && 
                            s->GetElement()->IntelLevel() > Intel::KNOWN;

                            if (visible)
                            seln_list->AddItemWithData(s->Name(), s->Identity());
                        }
                    }
                }
            }
        }
        break;

    default:
        break;
    }

    if (top_index >= 0)
    seln_list->ScrollTo(top_index);

    if (seln_index >= 0)
    seln_list->SetSelected(seln_index);

    else
    CoordinateSelection();
}

// +--------------------------------------------------------------------+

void
NavDlg::OnEngage(AWEvent* event)
{
    bool hide = false;

    if (ship) {
        NavSystem* navsys = ship->GetNavSystem();
        if (navsys) {
            if (navsys->AutoNavEngaged()) {
                navsys->DisengageAutoNav();
                commit_btn->SetText(Game::GetText("NavDlg.commit"));
                commit_btn->SetBackColor(commit_color);
            }
            else {
                navsys->EngageAutoNav();
                commit_btn->SetText(Game::GetText("NavDlg.cancel"));
                commit_btn->SetBackColor(cancel_color);
                hide = true;
            }

            Sim* sim = Sim::GetSim();
            if (sim)
            ship->SetControls(sim->GetControls());
        }
    }

    if (manager && hide)
    manager->ShowNavDlg();   // also hides
}

// +--------------------------------------------------------------------+

void
NavDlg::OnCommit(AWEvent* event)
{
    if (manager)
    manager->ShowNavDlg();   // also hides
}


// +--------------------------------------------------------------------+

void
NavDlg::OnCancel(AWEvent* event)
{
    if (manager)
    manager->ShowNavDlg();   // also hides
}

// +--------------------------------------------------------------------+

void
NavDlg::SetNavEditMode(int mode)
{
    if (nav_edit_mode != mode) {
        if (mode != NAV_EDIT_NONE) {
            int map_mode = star_map->GetSelectionMode();
            if (map_mode != -1)
            seln_mode = map_mode;

            star_map->SetSelectionMode(-1);
        }
        else {
            star_map->SetSelectionMode(seln_mode);
        }

        nav_edit_mode = mode;
    }
}

int
NavDlg::GetNavEditMode()
{
    return nav_edit_mode;
}

void
NavDlg::OnMapDown(AWEvent* event)
{
}

void
NavDlg::OnMapMove(AWEvent* event)
{
}

void
NavDlg::OnMapClick(AWEvent* event)
{
    static DWORD click_time = 0;

    SetSystem(star_map->GetSystem());
    CoordinateSelection();

    // double-click:
    if (Game::RealTime() - click_time < 350) {
        MissionElement* elem          = star_map->GetSelectedElem();
        MsnElemDlg*     msn_elem_dlg  = manager->GetMsnElemDlg();

        if (elem && msn_elem_dlg) {
            msn_elem_dlg->SetMission(mission);
            msn_elem_dlg->SetMissionElement(elem);
            manager->ShowMsnElemDlg();
        }
    }

    click_time = Game::RealTime();
}

void
NavDlg::CoordinateSelection()
{
    if (!seln_list || !star_map)
    return;

    switch (seln_mode) {
    default:
    case SELECT_SYSTEM:
    case SELECT_PLANET:
    case SELECT_REGION:
        {
            seln_list->ClearSelection();

            Orbital* selected = star_map->GetSelection();

            if (selected) {
                for (int i = 0; i < seln_list->NumItems(); i++) {
                    if (seln_list->GetItemText(i) == selected->Name() ||
                            seln_list->GetItemText(i) == Text("- ") + selected->Name()) {
                        seln_list->SetSelected(i, true);
                    }
                }
            }
        }
        break;

    case SELECT_STATION:
    case SELECT_STARSHIP:
    case SELECT_FIGHTER:
        {
            seln_list->ClearSelection();

            Ship*             selected = star_map->GetSelectedShip();
            MissionElement*   elem     = star_map->GetSelectedElem();

            if (selected) {
                for (int i = 0; i < seln_list->NumItems(); i++) {
                    if (seln_list->GetItemText(i) == selected->Name()) {
                        seln_list->SetSelected(i, true);
                    }
                }
            }

            else if (elem) {
                for (int i = 0; i < seln_list->NumItems(); i++) {
                    if (seln_list->GetItemText(i) == elem->Name()) {
                        seln_list->SetSelected(i, true);
                    }
                }
            }
        }
        break;
    }
}

// +--------------------------------------------------------------------+

void
NavDlg::OnClose(AWEvent* event)
{
    if (manager)
    manager->HideNavDlg();
}
