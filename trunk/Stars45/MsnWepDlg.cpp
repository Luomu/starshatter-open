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
    FILE:         MsnWepDlg.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Mission Briefing Dialog Active Window class
*/

#include "MemDebug.h"
#include "MsnWepDlg.h"
#include "PlanScreen.h"
#include "Starshatter.h"
#include "Campaign.h"
#include "Mission.h"
#include "Ship.h"
#include "ShipDesign.h"
#include "WeaponDesign.h"
#include "HardPoint.h"
#include "StarSystem.h"
#include "FormatUtil.h"

#include "Game.h"
#include "Mouse.h"
#include "Button.h"
#include "ListBox.h"
#include "Slider.h"
#include "ParseUtil.h"
#include "Keyboard.h"

// +--------------------------------------------------------------------+
// DECLARE MAPPING FUNCTIONS:
DEF_MAP_CLIENT(MsnWepDlg, OnCommit);
DEF_MAP_CLIENT(MsnWepDlg, OnCancel);
DEF_MAP_CLIENT(MsnWepDlg, OnTabButton);
DEF_MAP_CLIENT(MsnWepDlg, OnMount);
DEF_MAP_CLIENT(MsnWepDlg, OnLoadout);

// +--------------------------------------------------------------------+

MsnWepDlg::MsnWepDlg(Screen* s, FormDef& def, PlanScreen* mgr)
: FormWindow(s,  0,  0, s->Width(), s->Height()), MsnDlg(mgr),
elem(0), first_station(0), beauty(0), player_desc(0)
{
    campaign = Campaign::GetCampaign();

    if (campaign)
    mission = campaign->GetMission();

    ZeroMemory(designs, sizeof(designs));
    ZeroMemory(mounts,  sizeof(mounts));
    Init(def);
}

MsnWepDlg::~MsnWepDlg()
{
}

// +--------------------------------------------------------------------+

void
MsnWepDlg::RegisterControls()
{
    lbl_element    = FindControl(601);
    lbl_type       = FindControl(602);
    lbl_weight     = FindControl(603);
    loadout_list   = (ListBox*)  FindControl(604);
    beauty         = (ImageBox*) FindControl(300);
    player_desc    =             FindControl(301);

    if (loadout_list)
    REGISTER_CLIENT(EID_SELECT, loadout_list, MsnWepDlg, OnLoadout);

    for (int i = 0; i < 8; i++) {
        lbl_desc[i] = FindControl(500 + i*10);
        lbl_station[i] = FindControl(401 + i);

        for (int n = 0; n < 8; n++) {
            btn_load[i][n] = (Button*) FindControl(500 + i*10 + n + 1);

            if (btn_load[i][n]) {
                if (i == 0) {
                    if (n == 0)
                    btn_load[i][n]->GetPicture(led_off);
                    else if (n == 1)
                    btn_load[i][n]->GetPicture(led_on);
                }

                btn_load[i][n]->SetPicture(led_off);
                btn_load[i][n]->SetPictureLocation(4); // centered
                REGISTER_CLIENT(EID_CLICK, btn_load[i][n], MsnWepDlg, OnMount);
            }
        }
    }

    RegisterMsnControls(this);

    if (commit)
    REGISTER_CLIENT(EID_CLICK, commit, MsnWepDlg, OnCommit);

    if (cancel)
    REGISTER_CLIENT(EID_CLICK, cancel, MsnWepDlg, OnCancel);

    if (sit_button)
    REGISTER_CLIENT(EID_CLICK, sit_button, MsnWepDlg, OnTabButton);

    if (pkg_button)
    REGISTER_CLIENT(EID_CLICK, pkg_button, MsnWepDlg, OnTabButton);

    if (nav_button)
    REGISTER_CLIENT(EID_CLICK, nav_button, MsnWepDlg, OnTabButton);

    if (wep_button)
    REGISTER_CLIENT(EID_CLICK, wep_button, MsnWepDlg, OnTabButton);
}

// +--------------------------------------------------------------------+

void
MsnWepDlg::Show()
{
    FormWindow::Show();
    ShowMsnDlg();

    if (mission) {
        for (int i = 0; i < mission->GetElements().size(); i++) {
            MissionElement* e = mission->GetElements().at(i);
            if (e->Player()) {
                elem = e;
                break;
            }
        }
    }

    if (elem) {
        SetupControls();
    }
}

// +--------------------------------------------------------------------+

void
MsnWepDlg::SetupControls()
{
    ShipDesign* design = (ShipDesign*) elem->GetDesign();

    if (lbl_element)
    lbl_element->SetText(elem->Name());

    if (lbl_type)
    lbl_type->SetText(design->name);

    BuildLists();

    for (int i = 0; i < 8; i++) {
        if (!lbl_desc[i]) continue;

        if (designs[i]) {
            lbl_desc[i]->Show();
            lbl_desc[i]->SetText(designs[i]->group + " " + designs[i]->name);

            for (int n = 0; n < 8; n++) {
                if (mounts[i][n]) {
                    btn_load[i][n]->Show();
                    btn_load[i][n]->SetPicture((loads[n]==i) ? led_on : led_off);
                }
                else {
                    btn_load[i][n]->Hide();
                }
            }
        }
        else {
            lbl_desc[i]->Hide();

            for (int n = 0; n < 8; n++) {
                btn_load[i][n]->Hide();
            }
        }
    }

    double loaded_mass = 0;
    char   weight[32];

    if (loadout_list) {
        loadout_list->ClearItems();

        if (design) {
            ListIter<ShipLoad> sl = (List<ShipLoad>&) design->loadouts;
            while (++sl) {
                ShipLoad* load = sl.value();
                int item = loadout_list->AddItem(load->name) - 1;

                sprintf_s(weight, "%d kg", (int) ((design->mass + load->mass) * 1000));
                loadout_list->SetItemText(item, 1, weight);
                loadout_list->SetItemData(item, 1, (DWORD) (load->mass * 1000));

                if (elem->Loadouts().size() > 0 &&
                        elem->Loadouts().at(0)->GetName() == load->name) {
                    loadout_list->SetSelected(item, true);
                    loaded_mass = design->mass + load->mass;
                }
            }
        }
    }

    if (lbl_weight && design) {
        if (loaded_mass < 1)
        loaded_mass = design->mass;

        sprintf_s(weight, "%d kg", (int) (loaded_mass * 1000));
        lbl_weight->SetText(weight);
    }

    if (beauty && design) {
        beauty->SetPicture(design->beauty);
    }

    if (player_desc && design) {
        char txt[256];

        if (design->type <= Ship::ATTACK)
        sprintf_s(txt, "%s %s", design->abrv, design->display_name);
        else
        sprintf_s(txt, "%s %s", design->abrv, elem->Name().data());

        player_desc->SetText(txt);
    }
}

// +--------------------------------------------------------------------+

void
MsnWepDlg::BuildLists()
{
    ZeroMemory(designs, sizeof(designs));
    ZeroMemory(mounts,  sizeof(mounts));

    if (elem) {
        ShipDesign* d         = (ShipDesign*) elem->GetDesign();
        int         nstations = d->hard_points.size();

        first_station = (8 - nstations) / 2;

        int         index   = 0;
        int         station = first_station;

        for (int s = 0; s < 8; s++)
        if (lbl_station[s])
        lbl_station[s]->SetText("");

        ListIter<HardPoint> iter = d->hard_points;
        while (++iter) {
            HardPoint* hp = iter.value();

            if (lbl_station[station])
            lbl_station[station]->SetText(hp->GetAbbreviation());

            for (int n = 0; n < HardPoint::MAX_DESIGNS; n++) {
                WeaponDesign* wep_dsn = hp->GetWeaponDesign(n);

                if (wep_dsn) {
                    bool found = false;

                    for (int i = 0; i < 8 && !found; i++) {
                        if (designs[i] == wep_dsn) {
                            found               = true;
                            mounts[i][station]  = true;
                        }
                    }

                    if (!found) {
                        mounts[index][station] = true;
                        designs[index++]       = wep_dsn;
                    }
                }
            }

            station++;
        }

        if (elem->Loadouts().size()) {
            MissionLoad* msn_load = elem->Loadouts().at(0);

            for (int i = 0; i < 8; i++)
            loads[i] = -1;

            // map loadout:
            int* loadout = 0;
            if (msn_load->GetName().length()) {
                ListIter<ShipLoad> sl = ((ShipDesign*) elem->GetDesign())->loadouts;
                while (++sl) {
                    if (!_stricmp(sl->name, msn_load->GetName()))
                    loadout = sl->load;
                }
            }
            else {
                loadout = msn_load->GetStations();
            }

            for (int i = 0; i < nstations; i++) {
                loads[i + first_station] = loadout[i];
            }
        }
    }
}

// +--------------------------------------------------------------------+

void
MsnWepDlg::ExecFrame()
{
    if (Keyboard::KeyDown(VK_RETURN)) {
        OnCommit(0);
    }
}

// +--------------------------------------------------------------------+

int
MsnWepDlg::LoadToPointIndex(int n)
{
    int nn = n + first_station;

    if (!elem || nn < 0 || nn >= 8 || loads[nn] == -1)
    return -1;

    int           index      = -1;
    WeaponDesign* wep_design = designs[ loads[nn] ];
    ShipDesign*   design     = (ShipDesign*) elem->GetDesign();
    HardPoint*    hard_point = design->hard_points[n];

    for (int i = 0; i < 8 && index < 0; i++) {
        if (hard_point->GetWeaponDesign(i) == wep_design) {
            index = i;
        }
    }

    return index;
}

int
MsnWepDlg::PointIndexToLoad(int n, int index)
{
    int nn = n + first_station;

    if (!elem || nn < 0 || nn >= 8)
    return -1;

    int           result     = -1;
    ShipDesign*   design     = (ShipDesign*) elem->GetDesign();
    HardPoint*    hard_point = design->hard_points[n];
    WeaponDesign* wep_design = hard_point->GetWeaponDesign(index);

    for (int i = 0; i < 8 && result < 0; i++) {
        if (designs[i] == wep_design) {
            result = i;
        }
    }

    return result;
}

// +--------------------------------------------------------------------+

void
MsnWepDlg::OnMount(AWEvent* event)
{
    int station = -1;
    int item    = -1;

    for (int i = 0; i < 8 && item < 0; i++) {
        for (int n = 0; n < 8 && station < 0; n++) {
            if (btn_load[i][n] == event->window) {
                station = n;
                item    = i;
            }
        }
    }

    if (item >= 0 && station >= 0) {
        if (loads[station] == item)
        item = -1;

        loads[station] = item;

        for (int n = 0; n < 8; n++) {
            btn_load[n][station]->SetPicture(n == item ? led_on : led_off);
        }

        if (elem) {
            int nstations = elem->GetDesign()->hard_points.size();

            if (elem->Loadouts().size() < 1) {
                MissionLoad* l = new(__FILE__,__LINE__) MissionLoad;
                elem->Loadouts().append(l);

                for (int n = 0; n < nstations; n++)
                l->SetStation(n, LoadToPointIndex(n));
            }
            else {
                ListIter<MissionLoad> l = elem->Loadouts();
                while (++l) {
                    // if the player customizes the loadout,
                    // tell the sim loader not to use a named
                    // loadout from the ship design:
                    l->SetName("");

                    for (int n = 0; n < nstations; n++)
                    l->SetStation(n, LoadToPointIndex(n));
                }
            }
        }
    }

    if (loadout_list)
    loadout_list->ClearSelection();

    if (lbl_weight && elem) {
        ShipDesign* d         = (ShipDesign*) elem->GetDesign();
        int         nstations = d->hard_points.size();
        double      mass      = d->mass;

        for (int n = 0; n < nstations; n++) {
            int item = loads[n+first_station];
            mass += d->hard_points[n]->GetCarryMass(item);
        }

        char weight[32];
        sprintf_s(weight, "%d kg", (int) (mass * 1000));
        lbl_weight->SetText(weight);
    }
}

// +--------------------------------------------------------------------+

void
MsnWepDlg::OnLoadout(AWEvent* event)
{
    if (!elem) return;

    ShipDesign* design   = (ShipDesign*) elem->GetDesign();
    ShipLoad*   shipload = 0;

    if (loadout_list && design) {
        int   index    = loadout_list->GetListIndex();
        Text  loadname = loadout_list->GetItemText(index);

        ListIter<ShipLoad> sl = (List<ShipLoad>&) design->loadouts;
        while (++sl) {
            if (sl->name == loadname) {
                shipload = sl.value();
            }
        }

        if (!shipload) return;

        if (lbl_weight) {
            char weight[32];
            sprintf_s(weight, "%d kg", (int) ((design->mass + shipload->mass) * 1000));
            lbl_weight->SetText(weight);
        }

        if (elem->Loadouts().size() < 1) {
            MissionLoad* l = new(__FILE__,__LINE__) MissionLoad(-1, shipload->name);
            elem->Loadouts().append(l);
        }
        else {
            ListIter<MissionLoad> l = elem->Loadouts();
            while (++l) {
                // if the player chooses a std loadout,
                // tell the sim loader to use a named
                // loadout from the ship design:
                l->SetName(shipload->name);
            }
        }

        int  nstations = design->hard_points.size();
        int* loadout   = shipload->load;

        for (int i = 0; i < 8; i++)
        loads[i] = -1;

        for (int i = 0; i < nstations; i++)
        loads[i + first_station] = PointIndexToLoad(i, loadout[i]);

        for (int i = 0; i < 8; i++) {
            for (int n = 0; n < 8; n++) {
                btn_load[i][n]->SetPicture(i == loads[n] ? led_on: led_off);
            }
        }
    }
}

// +--------------------------------------------------------------------+

void
MsnWepDlg::OnCommit(AWEvent* event)
{
    MsnDlg::OnCommit(event);
}

void
MsnWepDlg::OnCancel(AWEvent* event)
{
    MsnDlg::OnCancel(event);
}

void
MsnWepDlg::OnTabButton(AWEvent* event)
{
    MsnDlg::OnTabButton(event);
}
