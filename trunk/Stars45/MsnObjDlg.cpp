/*  Project Starshatter 5.0
    Destroyer Studios LLC
    Copyright © 1997-2007. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         MsnObjDlg.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Mission Briefing Dialog Active Window class
*/

#include "MemDebug.h"
#include "MsnObjDlg.h"
#include "PlanScreen.h"
#include "Campaign.h"
#include "Mission.h"
#include "Ship.h"
#include "ShipDesign.h"
#include "ShipSolid.h"
#include "StarSystem.h"

#include "Game.h"
#include "Mouse.h"
#include "Button.h"
#include "ComboBox.h"
#include "ListBox.h"
#include "Slider.h"
#include "ParseUtil.h"
#include "FormatUtil.h"
#include "Light.h"
#include "Solid.h"
#include "Keyboard.h"

// +--------------------------------------------------------------------+
// DECLARE MAPPING FUNCTIONS:
DEF_MAP_CLIENT(MsnObjDlg, OnCommit);
DEF_MAP_CLIENT(MsnObjDlg, OnCancel);
DEF_MAP_CLIENT(MsnObjDlg, OnTabButton);
DEF_MAP_CLIENT(MsnObjDlg, OnSkin);

// +--------------------------------------------------------------------+

MsnObjDlg::MsnObjDlg(Screen* s, FormDef& def, PlanScreen* mgr)
   : FormWindow(s,  0,  0, s->Width(), s->Height()), MsnDlg(mgr),
     objectives(0), sitrep(0), beauty(0), camview(0), player_desc(0),
     ship(0)
{
   campaign = Campaign::GetCampaign();

   if (campaign)
      mission = campaign->GetMission();

   Init(def);
}

MsnObjDlg::~MsnObjDlg()
{
}

// +--------------------------------------------------------------------+

void
MsnObjDlg::RegisterControls()
{
   objectives  =             FindControl(400);
   sitrep      =             FindControl(401);
   beauty      =             FindControl(300);
   player_desc =             FindControl(301);
   cmb_skin    = (ComboBox*) FindControl(302);

   RegisterMsnControls(this);

   if (commit)
      REGISTER_CLIENT(EID_CLICK, commit, MsnObjDlg, OnCommit);

   if (cancel)
      REGISTER_CLIENT(EID_CLICK, cancel, MsnObjDlg, OnCancel);

   if (sit_button)
      REGISTER_CLIENT(EID_CLICK, sit_button, MsnObjDlg, OnTabButton);

   if (pkg_button)
      REGISTER_CLIENT(EID_CLICK, pkg_button, MsnObjDlg, OnTabButton);

   if (nav_button)
      REGISTER_CLIENT(EID_CLICK, nav_button, MsnObjDlg, OnTabButton);

   if (wep_button)
      REGISTER_CLIENT(EID_CLICK, wep_button, MsnObjDlg, OnTabButton);

   if (cmb_skin) {
      REGISTER_CLIENT(EID_SELECT, cmb_skin,  MsnObjDlg, OnSkin);
   }

   if (beauty) {
      scene.SetAmbient(Color(72,75,78));

      Point light_pos(3e6, 5e6, 4e6);

      Light* main_light = new Light(1.2f);
      main_light->MoveTo(light_pos);
      main_light->SetType(Light::LIGHT_DIRECTIONAL);
      main_light->SetColor(Color::White);
      main_light->SetShadow(true);

      scene.AddLight(main_light);

      Light* back_light = new Light(0.35f);
      back_light->MoveTo(light_pos * -1);
      back_light->SetType(Light::LIGHT_DIRECTIONAL);
      back_light->SetColor(Color::White);
      back_light->SetShadow(false);

      scene.AddLight(back_light);

      camview = new(__FILE__,__LINE__) CameraView(beauty, &cam, &scene);
      camview->SetProjectionType(Video::PROJECTION_PERSPECTIVE);
      camview->SetFieldOfView(2);

      beauty->AddView(camview);
   }
}

// +--------------------------------------------------------------------+

void
MsnObjDlg::Show()
{
   bool update_scene = !shown;

   FormWindow::Show();
   ShowMsnDlg();

   if (objectives) {
      if (mission) {
         if (mission->IsOK())
            objectives->SetText(mission->Objective());
         else
            objectives->SetText("");
      }
      else {
         objectives->SetText(Game::GetText("MsnDlg.no-mission"));
      }
   }

   if (sitrep) {
      if (mission) {
         if (mission->IsOK())
            sitrep->SetText(mission->Situation());
         else
            sitrep->SetText(Game::GetText("MsnDlg.found-errors") +
                            mission->ErrorMessage());
      }
      else {
         sitrep->SetText(Game::GetText("MsnDlg.no-mission"));
      }
   }

   if (cmb_skin) {
      cmb_skin->ClearItems();
      cmb_skin->Hide();
   }

   if (beauty) {
      if (mission && mission->IsOK()) {
         MissionElement*   elem   = mission->GetPlayer();

         if (elem) {
            const ShipDesign* design = elem->GetDesign();

            if (design && camview && update_scene) {
               double az   = -PI/6;
               double el   =  PI/8;
               double zoom = 1.8;

               scene.Graphics().clear();

               if (elem->IsStarship()) {
                  az   = -PI/8;
                  el   =  PI/12;
                  zoom = 1.7;
               }

               if (design->beauty_cam.z > 0) {
                  az   = design->beauty_cam.x;
                  el   = design->beauty_cam.y;
                  zoom = design->beauty_cam.z;
               }

               double r = design->radius;
               double x = zoom * r * sin(az) * cos(el);
               double y = zoom * r * cos(az) * cos(el);
               double z = zoom * r *           sin(el);

               cam.LookAt(Point(0,0,r/5), Point(x,z,y), Point(0,1,0));

               int n = design->lod_levels;

               if (n >= 1) {
                  Model* model = design->models[n-1].at(0);

                  if (model) {
                     ship = new(__FILE__,__LINE__) ShipSolid(0);
                     ship->UseModel(model);
                     ship->CreateShadows(1);
                     ship->SetSkin(elem->GetSkin());

                     Matrix o;
                     o.Pitch( 3 * DEGREES);
                     o.Roll( 13 * DEGREES);

                     ship->SetOrientation(o);

                     scene.Graphics().append(ship);
                  }
               }
            }

            if (cmb_skin && design && design->skins.size()) {
               cmb_skin->Show();
               cmb_skin->AddItem(Game::GetText("MsnDlg.default"));
               cmb_skin->SetSelection(0);
               ListIter<Skin> iter = ((ShipDesign*) design)->skins;

               while (++iter) {
                  Skin* s = iter.value();
                  cmb_skin->AddItem(s->Name());

                  if (elem && elem->GetSkin() && !strcmp(s->Name(), elem->GetSkin()->Name())) {
                     cmb_skin->SetSelection(cmb_skin->NumItems()-1);
                  }
               }
            }
         }
      }
   }

   if (player_desc) {
      player_desc->SetText("");

      if (mission && mission->IsOK()) {
         MissionElement* elem = mission->GetPlayer();

         if (elem) {
            const ShipDesign* design = elem->GetDesign();

            if (design) {
               char txt[256];

               if (design->type <= Ship::ATTACK)
                  sprintf(txt, "%s %s", design->abrv, design->display_name);
               else
                  sprintf(txt, "%s %s", design->abrv, elem->Name().data());

               player_desc->SetText(txt);
            }
         }
      }
   }
}

// +--------------------------------------------------------------------+

void
MsnObjDlg::ExecFrame()
{
   if (Keyboard::KeyDown(VK_RETURN)) {
      OnCommit(0);
   }
}

// +--------------------------------------------------------------------+

void
MsnObjDlg::OnSkin(AWEvent* event)
{
   Text skin_name = cmb_skin->GetSelectedItem();

   if (mission && mission->IsOK()) {
      MissionElement*   elem   = mission->GetPlayer();

      if (elem) {
         const ShipDesign* design = elem->GetDesign();

         if (design) {
            const Skin* skin = design->FindSkin(skin_name);

            elem->SetSkin(skin);

            if (ship)
               ship->SetSkin(skin);
         }
      }
   }
}

void
MsnObjDlg::OnCommit(AWEvent* event)
{
   MsnDlg::OnCommit(event);
}

void
MsnObjDlg::OnCancel(AWEvent* event)
{
   MsnDlg::OnCancel(event);
}

void
MsnObjDlg::OnTabButton(AWEvent* event)
{
   MsnDlg::OnTabButton(event);
}
