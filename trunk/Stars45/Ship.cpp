/*  Project Starshatter 5.0
    Destroyer Studios LLC
    Copyright © 1997-2007. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         Ship.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Starship class
*/

#include "MemDebug.h"
#include "Ship.h"
#include "ShipAI.h"
#include "ShipCtrl.h"
#include "ShipDesign.h"
#include "ShipKiller.h"
#include "Shot.h"
#include "Drone.h"
#include "SeekerAI.h"
#include "HardPoint.h"
#include "Weapon.h"
#include "WeaponGroup.h"
#include "Shield.h"
#include "ShieldRep.h"
#include "Computer.h"
#include "FlightComp.h"
#include "Drive.h"
#include "QuantumDrive.h"
#include "Farcaster.h"
#include "Thruster.h"
#include "Power.h"
#include "FlightDeck.h"
#include "LandingGear.h"
#include "Hangar.h."
#include "Sensor.h"
#include "Contact.h"
#include "CombatUnit.h"
#include "Element.h"
#include "Instruction.h"
#include "RadioMessage.h"
#include "RadioHandler.h"
#include "RadioTraffic.h"
#include "NavLight.h"
#include "NavSystem.h"
#include "NavAI.h"
#include "DropShipAI.h"
#include "Explosion.h"
#include "MissionEvent.h"
#include "ShipSolid.h"
#include "Sim.h"
#include "SimEvent.h"
#include "StarSystem.h"
#include "TerrainRegion.h"
#include "Terrain.h"
#include "System.h"
#include "Component.h"
#include "KeyMap.h"
#include "RadioView.h"
#include "AudioConfig.h"
#include "CameraDirector.h"
#include "HUDView.h"
#include "Random.h"
#include "RadioVox.h"

#include "NetGame.h"
#include "NetUtil.h"

#include "MotionController.h"
#include "Keyboard.h"
#include "Joystick.h"
#include "Bolt.h"
#include "Game.h"
#include "Solid.h"
#include "Shadow.h"
#include "Skin.h"
#include "Sprite.h"
#include "Light.h"
#include "Bitmap.h"
#include "Button.h"
#include "Sound.h"
#include "DataLoader.h"

#include "Parser.h"
#include "Reader.h"

// +----------------------------------------------------------------------+

static int     base_contact_id   = 0;
static double  range_min         = 0;
static double  range_max         = 250e3;

int      Ship::control_model        = 0; // standard
int      Ship::flight_model         = 0; // standard
int      Ship::landing_model        = 0; // standard
double   Ship::friendly_fire_level  = 1; // 100%

const int HIT_NOTHING   = 0;
const int HIT_HULL      = 1;
const int HIT_SHIELD    = 2;
const int HIT_BOTH      = 3;
const int HIT_TURRET    = 4;

// +----------------------------------------------------------------------+

Ship::Ship(const char* ship_name, const char* reg_num, ShipDesign* ship_dsn, int IFF, int cmd_ai, const int* load)
   : IFF_code(IFF), killer(0), throttle(0), augmenter(false), throttle_request(0),
     shield(0), shieldRep(0), main_drive(0), quantum_drive(0), farcaster(0),
     check_fire(false), probe(0), sensor_drone(0), primary(0), secondary(1),
     cmd_chain_index(0), target(0), subtarget(0), radio_orders(0), launch_point(0),
     g_force(0.0f), sensor(0), navsys(0), flcs(0), hangar(0), respawns(0), invulnerable(false),
     thruster(0), decoy(0), ai_mode(2), command_ai_level(cmd_ai), flcs_mode(FLCS_AUTO), loadout(0),
     emcon(3), old_emcon(3), master_caution(false), cockpit(0), gear(0), skin(0),
     auto_repair(true), last_repair_time(0), last_eval_time(0), last_beam_time(0), last_bolt_time(0),
     warp_fov(1), flight_phase(LAUNCH), launch_time(0), carrier(0), dock(0), ff_count(0),
     inbound(0), element(0), director_info("Init"), combat_unit(0), net_control(0), 
     track(0), ntrack(0), track_time(0), helm_heading(0.0f), helm_pitch(0.0f),
     altitude_agl(-1.0e6f), transition_time(0.0f), transition_type(TRANSITION_NONE),
     friendly_fire_time(0), ward(0), net_observer_mode(false), orig_elem_index(-1)
{
   sim = Sim::GetSim();

   strcpy(name,   ship_name);
   if (reg_num && *reg_num)
   strcpy(regnum, reg_num);
   else regnum[0] = 0;

   design = ship_dsn;
   
   if (!design) {
      char msg[256];
      sprintf(msg, "No ship design found for '%s'\n", ship_name);
      Game::Panic(msg);
   }

   obj_type    = SimObject::SIM_SHIP;

   radius      = design->radius;
   mass        = design->mass;
   integrity   = design->integrity;
   vlimit      = design->vlimit;

   agility     = design->agility;
   wep_mass    = 0.0f;
   wep_resist  = 0.0f;

   CL          = design->CL;
   CD          = design->CD;
   stall       = design->stall;
   
   chase_vec   = design->chase_vec;
   bridge_vec  = design->bridge_vec;
   
   acs         = design->acs;
   pcs         = design->acs;

   auto_repair = design->repair_auto;

   while (!base_contact_id)
      base_contact_id = rand() % 1000;

   contact_id  = base_contact_id++;
   int sys_id  = 0;

   for (int i = 0; i < design->reactors.size(); i++) {
      PowerSource* reactor = new(__FILE__,__LINE__) PowerSource(*design->reactors[i]);
      reactor->SetShip(this);
      reactor->SetID(sys_id++);
      reactors.append(reactor);
      systems.append(reactor);
   }

   for (int i = 0; i < design->drives.size(); i++) {
      Drive* drive = new(__FILE__,__LINE__) Drive(*design->drives[i]);
      drive->SetShip(this);
      drive->SetID(sys_id++);

      int src_index = drive->GetSourceIndex();
      if (src_index >= 0 && src_index < reactors.size())
         reactors[src_index]->AddClient(drive);

      drives.append(drive);
      systems.append(drive);
   }

   if (design->quantum_drive) {
      quantum_drive = new(__FILE__,__LINE__) QuantumDrive(*design->quantum_drive);
      quantum_drive->SetShip(this);
      quantum_drive->SetID(sys_id++);

      int src_index = quantum_drive->GetSourceIndex();
      if (src_index >= 0 && src_index < reactors.size())
         reactors[src_index]->AddClient(quantum_drive);

      quantum_drive->SetShip(this);
      systems.append(quantum_drive);
   }

   if (design->farcaster) {
      farcaster = new(__FILE__,__LINE__) Farcaster(*design->farcaster);
      farcaster->SetShip(this);
      farcaster->SetID(sys_id++);

      int src_index = farcaster->GetSourceIndex();
      if (src_index >= 0 && src_index < reactors.size())
         reactors[src_index]->AddClient(farcaster);

      farcaster->SetShip(this);
      systems.append(farcaster);
   }

   if (design->thruster) {
      thruster = new(__FILE__,__LINE__) Thruster(*design->thruster);
      thruster->SetShip(this);
      thruster->SetID(sys_id++);

      int src_index = thruster->GetSourceIndex();
      if (src_index >= 0 && src_index < reactors.size())
         reactors[src_index]->AddClient(thruster);

      thruster->SetShip(this);
      systems.append(thruster);
   }

   if (design->shield) {
      shield = new(__FILE__,__LINE__) Shield(*design->shield);
      shield->SetShip(this);
      shield->SetID(sys_id++);

      int src_index = shield->GetSourceIndex();
      if (src_index >= 0 && src_index < reactors.size())
         reactors[src_index]->AddClient(shield);

      if (design->shield_model) {
         shieldRep = new(__FILE__,__LINE__) ShieldRep;
         shieldRep->UseModel(design->shield_model);
      }
      
      systems.append(shield);
   }

   for (int i = 0; i < design->flight_decks.size(); i++) {
      FlightDeck* deck = new(__FILE__,__LINE__) FlightDeck(*design->flight_decks[i]);
      deck->SetShip(this);
      deck->SetCarrier(this);
      deck->SetID(sys_id++);
      deck->SetIndex(i);

      int src_index = deck->GetSourceIndex();
      if (src_index >= 0 && src_index < reactors.size())
         reactors[src_index]->AddClient(deck);

      flight_decks.append(deck);
      systems.append(deck);
   }

   if (design->flight_decks.size() > 0) {
      if (!hangar) {
         hangar = new(__FILE__,__LINE__) Hangar;
         hangar->SetShip(this);
      }
   }

   if (design->squadrons.size() > 0) {
      if (!hangar) {
         hangar = new(__FILE__,__LINE__) Hangar;
         hangar->SetShip(this);
      }

      for (int i = 0; i < design->squadrons.size(); i++) {
         ShipSquadron* s = design->squadrons[i];
         hangar->CreateSquadron(s->name, 0, s->design, s->count, GetIFF(), 0, 0, s->avail);
      }
   }

   if (design->gear) {
      gear = new(__FILE__,__LINE__) LandingGear(*design->gear);
      gear->SetShip(this);
      gear->SetID(sys_id++);

      int src_index = gear->GetSourceIndex();
      if (src_index >= 0 && src_index < reactors.size())
         reactors[src_index]->AddClient(gear);

      systems.append(gear);
   }

   if (design->sensor) {
      sensor = new(__FILE__,__LINE__) Sensor(*design->sensor);
      sensor->SetShip(this);
      sensor->SetID(sys_id++);

      int src_index = sensor->GetSourceIndex();
      if (src_index >= 0 && src_index < reactors.size())
         reactors[src_index]->AddClient(sensor);

      if (IsStarship() || IsStatic() || !strncmp(design->name, "Camera", 6))
         sensor->SetMode(Sensor::CST);

      systems.append(sensor);
   }

   int wep_index = 1;

   for (int i = 0; i < design->weapons.size(); i++) {
      Weapon* gun = new(__FILE__,__LINE__) Weapon(*design->weapons[i]);
      gun->SetID(sys_id++);
      gun->SetOwner(this);
      gun->SetIndex(wep_index++);

      int src_index = gun->GetSourceIndex();
      if (src_index >= 0 && src_index < reactors.size())
         reactors[src_index]->AddClient(gun);

      WeaponGroup* group = FindWeaponGroup(gun->Group());
      group->AddWeapon(gun);
      group->SetAbbreviation(gun->Abbreviation());

      systems.append(gun);

      if (IsDropship() && gun->GetTurret())
         gun->SetFiringOrders(Weapon::POINT_DEFENSE);
      else
         gun->SetFiringOrders(Weapon::MANUAL);
   }

   int loadout_size = design->hard_points.size();

   if (load && loadout_size > 0) {
      loadout = new(__FILE__,__LINE__) int[loadout_size];

      for (int i = 0; i < loadout_size; i++) {
         int mounted_weapon = loadout[i] = load[i];

         if (mounted_weapon < 0)
            continue;

         Weapon* missile = design->hard_points[i]->CreateWeapon(mounted_weapon);

         if (missile) {
            missile->SetID(sys_id++);
            missile->SetOwner(this);
            missile->SetIndex(wep_index++);

            WeaponGroup* group = FindWeaponGroup(missile->Group());
            group->AddWeapon(missile);
            group->SetAbbreviation(missile->Abbreviation());

            systems.append(missile);
         }
      }
   }

   if (weapons.size() > 1) {
      primary   = -1;
      secondary = -1;

      for (int i = 0; i < weapons.size(); i++) {
         WeaponGroup* group = weapons[i];
         if (group->IsPrimary() && primary < 0) {
            primary = i;

            // turrets on fighters are set to point defense by default,
            // this forces the primary turret back to manual control
            group->SetFiringOrders(Weapon::MANUAL);
         }

         else if (group->IsMissile() && secondary < 0) {
            secondary = i;
         }
      }

      if (primary   < 0)   primary   = 0;
      if (secondary < 0)   secondary = 1;

      if (weapons.size() > 4) {
         ::Print("WARNING: Ship '%s' type '%s' has %d wep groups (max=4)\n",
            Name(), DesignName(), weapons.size());
      }
   }

   if (design->decoy) {
      decoy = new(__FILE__,__LINE__) Weapon(*design->decoy);
      decoy->SetOwner(this);
      decoy->SetID(sys_id++);
      decoy->SetIndex(wep_index++);

      int src_index = decoy->GetSourceIndex();
      if (src_index >= 0 && src_index < reactors.size())
         reactors[src_index]->AddClient(decoy);

      systems.append(decoy);
   }

   for (int i = 0; i < design->navlights.size(); i++) {
      NavLight* navlight = new(__FILE__,__LINE__) NavLight(*design->navlights[i]);
      navlight->SetShip(this);
      navlight->SetID(sys_id++);
      navlight->SetOffset(((DWORD) this) << 2);
      navlights.append(navlight);
      systems.append(navlight);
   }

   if (design->navsys) {
      navsys = new(__FILE__,__LINE__) NavSystem(*design->navsys);
      navsys->SetShip(this);
      navsys->SetID(sys_id++);

      int src_index = navsys->GetSourceIndex();
      if (src_index >= 0 && src_index < reactors.size())
         reactors[src_index]->AddClient(navsys);

      systems.append(navsys);
   }

   if (design->probe) {
      probe = new(__FILE__,__LINE__) Weapon(*design->probe);
      probe->SetOwner(this);
      probe->SetID(sys_id++);
      probe->SetIndex(wep_index++);

      int src_index = probe->GetSourceIndex();
      if (src_index >= 0 && src_index < reactors.size())
         reactors[src_index]->AddClient(probe);

      systems.append(probe);
   }

   for (int i = 0; i < design->computers.size(); i++) {
      Computer* comp = 0;

      if (design->computers[i]->Subtype() == Computer::FLIGHT) {
         flcs = new(__FILE__,__LINE__) FlightComp(*design->computers[i]);

         flcs->SetShip(this);
         flcs->SetMode(flcs_mode);
         flcs->SetVelocityLimit(vlimit);

         if (thruster)
            flcs->SetTransLimit(thruster->TransXLimit(),
                                thruster->TransYLimit(),
                                thruster->TransZLimit());
         else
            flcs->SetTransLimit(design->trans_x,
                                design->trans_y,
                                design->trans_z);

         comp = flcs;
      }
      else {
         comp = new(__FILE__,__LINE__) Computer(*design->computers[i]);
      }

      comp->SetShip(this);
      comp->SetID(sys_id++);
      int src_index = comp->GetSourceIndex();
      if (src_index >= 0 && src_index < reactors.size())
         reactors[src_index]->AddClient(comp);

      computers.append(comp);
      systems.append(comp);
   }

   radio_orders = new(__FILE__,__LINE__) Instruction("", Point(0,0,0));

   // Load Detail Set:
   for (int i = 0; i < DetailSet::MAX_DETAIL; i++) {
      if (design->models[i].size() > 0) {
         Solid* solid = new(__FILE__,__LINE__) ShipSolid(this);
         solid->UseModel(design->models[i].at(0));
         solid->CreateShadows(1);

         Point* offset = 0;
         Point* spin   = 0;

         if (design->offsets[i].size() > 0)
            offset = new(__FILE__,__LINE__) Point(*design->offsets[i].at(0));

         if (design->spin_rates.size() > 0)
            spin = new(__FILE__,__LINE__) Point(*design->spin_rates.at(0));

         detail_level = detail.DefineLevel(design->feature_size[i], solid, offset, spin);
      }

      if (design->models[i].size() > 1) {
         for (int n = 1; n < design->models[i].size(); n++) {
            Solid* solid = new(__FILE__,__LINE__) ShipSolid(this); //Solid;
            solid->UseModel(design->models[i].at(n));
            solid->CreateShadows(1);

            Point* offset = 0;
            Point* spin   = 0;

            if (design->offsets[i].size() > n)
               offset = new(__FILE__,__LINE__) Point(*design->offsets[i].at(n));

            if (design->spin_rates.size() > n)
               spin = new(__FILE__,__LINE__) Point(*design->spin_rates.at(n));

            detail.AddToLevel(detail_level, solid, offset, spin);
         }
      }
   }

   // start with lowest available detail:
   detail_level = 0; // this is highest -> detail.NumLevels()-1);
   rep = detail.GetRep(detail_level);

   if (design->cockpit_model) {
      cockpit = new(__FILE__,__LINE__) Solid;
      cockpit->UseModel(design->cockpit_model);
      cockpit->SetForeground(true);
   }

   if (design->main_drive >= 0 && design->main_drive < drives.size())
      main_drive = drives[design->main_drive];

   // only use light from drives:
   light = 0;

   // setup starship helm stuff:
   if (IsStarship()) {
      flcs_mode = FLCS_HELM;
   }

   // initialize the AI:
   dir = 0;
   SetControls(0);

   for (int i = 0; i < 4; i++) {
      missile_id[i]  = 0;
      missile_eta[i] = 0;
      trigger[i]     = false;
   }
}

// +--------------------------------------------------------------------+

Ship::~Ship()
{
   // the loadout can not be cleared during Destroy, because it
   // is needed after Destroy to create the re-spawned ship

   delete [] loadout;
   loadout = 0;

   Destroy();
}

// +--------------------------------------------------------------------+

void
Ship::Destroy()
{
   // destroy fighters on deck:
   ListIter<FlightDeck> deck = flight_decks;
   while (++deck) {
      for (int i = 0; i < deck->NumSlots(); i++) {
         Ship* s = deck->GetShip(i);

         if (s && !s->IsDying() && !s->IsDead()) {
            if (sim && sim->IsActive()) {
               s->DeathSpiral();
            }
            else {
               s->transition_type = TRANSITION_DEAD;
               s->Destroy();
            }
         }
      }
   }

   if (element) {
      // mission ending for this ship, evaluate objectives one last time:
      for (int i = 0; i < element->NumObjectives(); i++) {
         Instruction* obj = element->GetObjective(i);

         if (obj->Status() <= Instruction::ACTIVE) {
            obj->Evaluate(this);
         }
      }

      combat_unit = element->GetCombatUnit();
      SetElement(0);
   }

   delete [] track;
   track = 0;

   delete shield;
   shield = 0;
   delete sensor;
   sensor = 0;
   delete navsys;
   navsys = 0;
   delete thruster;
   thruster = 0;
   delete farcaster;
   farcaster = 0;
   delete quantum_drive;
   quantum_drive = 0;
   delete decoy;
   decoy = 0;
   delete probe;
   probe = 0;
   delete gear;
   gear = 0;

   main_drive  = 0;
   flcs        = 0;

   // repair queue does not own the systems under repair:
   repair_queue.clear();

   navlights.destroy();
   flight_decks.destroy();
   computers.destroy();
   weapons.destroy();
   drives.destroy();
   reactors.destroy();

   // this is now a list of dangling pointers:
   systems.clear();

   delete hangar;
   hangar = 0;

   // this also destroys the rep:
   detail.Destroy();
   rep = 0;

   GRAPHIC_DESTROY(cockpit);
   GRAPHIC_DESTROY(shieldRep);
   LIGHT_DESTROY(light);

   delete launch_point;
   launch_point = 0;

   delete radio_orders;
   radio_orders = 0;

   delete dir;
   dir = 0;
   
   delete killer;
   killer = 0;

   // inbound slot is deleted by flight deck:
   inbound = 0;

   life = 0;
   Notify();
}

// +--------------------------------------------------------------------+

void
Ship::Initialize()
{
   ShipDesign::Initialize();
   Thruster::Initialize();
}

// +--------------------------------------------------------------------+

void
Ship::Close()
{
   ShipDesign::Close();
   Thruster::Close();
}

void
Ship::SetupAgility()
{
   const float ROLL_SPEED  =  (float)(PI *  0.1500);
   const float PITCH_SPEED =  (float)(PI *  0.0250);
   const float YAW_SPEED   =  (float)(PI *  0.0250);

   drag   = design->drag;
   dr_drg = design->roll_drag;
   dp_drg = design->pitch_drag;
   dy_drg = design->yaw_drag;

   if (IsDying()) {
      drag   =  0.0f;
      dr_drg *= 0.25f;
      dp_drg *= 0.25f;
      dy_drg *= 0.25f;
   }

   if (flight_model > 0) {
      drag   = design->arcade_drag;
      thrust *= 10.0f;
   }

   float yaw_air_factor = 1.0f;

   if (IsAirborne()) {
      bool grounded = AltitudeAGL() < Radius()/2;

      if (flight_model > 0) {
         drag *= 2.0f;

         if (gear && gear->GetState() != LandingGear::GEAR_UP)
            drag *= 2.0f;

         if (grounded)
            drag *= 3.0f;
      }

      else {
         if (Class() != LCA)
            yaw_air_factor = 0.3f;

         double rho        = GetDensity();
         double speed      = Velocity().length();

         agility = design->air_factor * rho * speed - wep_resist;

         if (grounded && agility < 0)
            agility = 0;

         else if (!grounded && agility < 0.5 * design->agility)
            agility = 0.5 * design->agility;

         else if (agility > 2 * design->agility)
            agility = 2 * design->agility;

         // undercarriage aerodynamic drag
         if (gear && gear->GetState() != LandingGear::GEAR_UP)
            drag *= 5.0f;

         // wheel rolling friction
         if (grounded)
            drag *= 10.0f;

         // dead engine drag ;-)
         if (thrust < 10)
            drag *= 5.0f;
      }
   }

   else {
      agility = design->agility - wep_resist;

      if (agility < 0.5 * design->agility)
         agility = 0.5 * design->agility;

      if (flight_model == 0)
         drag = 0.0f;
   }

   float rr = (float) (design->roll_rate  * PI / 180);
   float pr = (float) (design->pitch_rate * PI / 180);
   float yr = (float) (design->yaw_rate   * PI / 180);

   if (rr == 0) rr = (float) agility * ROLL_SPEED;
   if (pr == 0) pr = (float) agility * PITCH_SPEED;
   if (yr == 0) yr = (float) agility * YAW_SPEED * yaw_air_factor;

   SetAngularRates(rr, pr, yr);
}

// +--------------------------------------------------------------------+

void
Ship::SetRegion(SimRegion* rgn)
{
   SimObject::SetRegion(rgn);

   const double GRAV = 6.673e-11;

   if (IsGroundUnit()) {
      // glue buildings to the terrain:
      Point    loc     = Location();
      Terrain* terrain = region->GetTerrain();

      if (terrain) {
         loc.y = terrain->Height(loc.x, loc.z);
         MoveTo(loc);
      }
   }

   else if (IsAirborne()) {
      Orbital* primary = GetRegion()->GetOrbitalRegion()->Primary();

      double m0 = primary->Mass();
      double r  = primary->Radius();

      SetGravity((float) (GRAV * m0 / (r*r)));
      SetBaseDensity(1.0f);
   }

   else {
      SetGravity(0.0f);
      SetBaseDensity(0.0f);

      if (IsStarship())
         flcs_mode = FLCS_HELM;
      else
         flcs_mode = FLCS_AUTO;
   }
}

// +--------------------------------------------------------------------+

int
Ship::GetTextureList(List<Bitmap>& textures)
{
   textures.clear();

   for (int d = 0; d < detail.NumLevels(); d++) {
      for (int i = 0; i < detail.NumModels(d); i++) {
         Graphic* g = detail.GetRep(d, i);

         if (g->IsSolid()) {
            Solid* solid = (Solid*) g;
            Model* model = solid->GetModel();

            if (model) {
               for (int n = 0; n < model->NumMaterials(); n++) {
                  //textures.append(model->textures[n]);
               }
            }
         }
      }
   }

   return textures.size();
}

// +--------------------------------------------------------------------+

void
Ship::Activate(Scene& scene)
{
   int i = 0;
   SimObject::Activate(scene);

   for (i = 0; i < detail.NumModels(detail_level); i++) {
      Graphic* g = detail.GetRep(detail_level, i);
      scene.AddGraphic(g);
   }

   for (i = 0; i < flight_decks.size(); i++)
      scene.AddLight(flight_decks[i]->GetLight());

   if (shieldRep)
      scene.AddGraphic(shieldRep);

   if (cockpit) {
      scene.AddForeground(cockpit);
      cockpit->Hide();
   }

   Drive* drive = GetDrive();
   if (drive) {
      for (i = 0; i < drive->NumEngines(); i++) {
         Graphic* flare = drive->GetFlare(i);
         if (flare) {
            scene.AddGraphic(flare);
         }

         Graphic* trail = drive->GetTrail(i);
         if (trail) {
            scene.AddGraphic(trail);
         }
      }
   }

   Thruster* thruster = GetThruster();
   if (thruster) {
      for (i = 0; i < thruster->NumThrusters(); i++) {
         Graphic* flare = thruster->Flare(i);
         if (flare) {
            scene.AddGraphic(flare);
         }

         Graphic* trail = thruster->Trail(i);
         if (trail) {
            scene.AddGraphic(trail);
         }
      }
   }

   for (int n = 0; n < navlights.size(); n++) {
      NavLight* navlight = navlights[n];
      for (i = 0; i < navlight->NumBeacons(); i++) {
         Graphic* beacon = navlight->Beacon(i);
         if (beacon)
            scene.AddGraphic(beacon);
      }
   }

   ListIter<WeaponGroup> g = weapons;
   while (++g) {
      ListIter<Weapon> w = g->GetWeapons();
      while (++w) {
         Solid* turret = w->GetTurret();
         if (turret) {
            scene.AddGraphic(turret);

            Solid* turret_base = w->GetTurretBase();
            if (turret_base)
               scene.AddGraphic(turret_base);
         }
         if (w->IsMissile()) {
            for (i = 0; i < w->Ammo(); i++) {
               Solid* store = w->GetVisibleStore(i);
               if (store)
                  scene.AddGraphic(store);
            }
         }
      }
   }

   if (gear && gear->GetState() != LandingGear::GEAR_UP) {
      for (int i = 0; i < gear->NumGear(); i++) {
         scene.AddGraphic(gear->GetGear(i));
      }
   }
}

void
Ship::Deactivate(Scene& scene)
{
   int i = 0;
   SimObject::Deactivate(scene);

   for (i = 0; i < detail.NumModels(detail_level); i++) {
      Graphic* g = detail.GetRep(detail_level, i);
      scene.DelGraphic(g);
   }

   for (i = 0; i < flight_decks.size(); i++)
      scene.DelLight(flight_decks[i]->GetLight());

   if (shieldRep)
      scene.DelGraphic(shieldRep);

   if (cockpit)
      scene.DelForeground(cockpit);

   Drive* drive = GetDrive();
   if (drive) {
      for (i = 0; i < drive->NumEngines(); i++) {
         Graphic* flare = drive->GetFlare(i);
         if (flare) {
            scene.DelGraphic(flare);
         }

         Graphic* trail = drive->GetTrail(i);
         if (trail) {
            scene.DelGraphic(trail);
         }
      }
   }

   Thruster* thruster = GetThruster();
   if (thruster) {
      for (i = 0; i < thruster->NumThrusters(); i++) {
         Graphic* flare = thruster->Flare(i);
         if (flare) {
            scene.DelGraphic(flare);
         }

         Graphic* trail = thruster->Trail(i);
         if (trail) {
            scene.DelGraphic(trail);
         }
      }
   }

   for (int n = 0; n < navlights.size(); n++) {
      NavLight* navlight = navlights[n];
      for (i = 0; i < navlight->NumBeacons(); i++) {
         Graphic* beacon = navlight->Beacon(i);
         if (beacon)
            scene.DelGraphic(beacon);
      }
   }
   
   ListIter<WeaponGroup> g = weapons;
   while (++g) {
      ListIter<Weapon> w = g->GetWeapons();
      while (++w) {
         Solid* turret = w->GetTurret();
         if (turret) {
            scene.DelGraphic(turret);

            Solid* turret_base = w->GetTurretBase();
            if (turret_base)
               scene.DelGraphic(turret_base);
         }
         if (w->IsMissile()) {
            for (i = 0; i < w->Ammo(); i++) {
               Solid* store = w->GetVisibleStore(i);
               if (store)
                  scene.DelGraphic(store);
            }
         }
      }
   }

   if (gear) {
      for (int i = 0; i < gear->NumGear(); i++) {
         scene.DelGraphic(gear->GetGear(i));
      }
   }
}

// +--------------------------------------------------------------------+

void
Ship::MatchOrientation(const Ship& s)
{
   Point pos = cam.Pos();
   cam.Clone(s.cam);
   cam.MoveTo(pos);

   if (rep)
      rep->SetOrientation(cam.Orientation());

   if (cockpit)
      cockpit->SetOrientation(cam.Orientation());
}

// +--------------------------------------------------------------------+

void
Ship::ClearTrack()
{
   const int DEFAULT_TRACK_LENGTH =  20; // 10 seconds

   if (!track) {
      track = new(__FILE__,__LINE__) Point[DEFAULT_TRACK_LENGTH];
   }

   track[0] = Location();
   ntrack   = 1;
   track_time = Game::GameTime();
}

void
Ship::UpdateTrack()
{
   const int DEFAULT_TRACK_UPDATE = 500; // milliseconds
   const int DEFAULT_TRACK_LENGTH =  20; // 10 seconds

   DWORD time = Game::GameTime();

   if (!track) {
      track = new(__FILE__,__LINE__) Point[DEFAULT_TRACK_LENGTH];
      track[0] = Location();
      ntrack   = 1;
      track_time = time;
   }

   else if (time - track_time > DEFAULT_TRACK_UPDATE) {
      if (Location() != track[0]) {
         for (int i = DEFAULT_TRACK_LENGTH-2; i >= 0; i--)
            track[i+1] = track[i];

         track[0] = Location();
         if (ntrack < DEFAULT_TRACK_LENGTH) ntrack++;
      }

      track_time = time;
   }
}

Point
Ship::TrackPoint(int i) const
{
   if (track && i < ntrack)
      return track[i];

   return Point();
}

// +--------------------------------------------------------------------+

const char*
Ship::Abbreviation() const
{
   return design->abrv;
}

const char*
Ship::DesignName() const
{
   return design->DisplayName();
}

const char*
Ship::DesignFileName() const
{
   return design->filename;
}

const char*
Ship::ClassName() const
{
   return ShipDesign::ClassName(design->type);
}

const char*
Ship::ClassName(int c)
{
   return ShipDesign::ClassName(c);
}

int
Ship::ClassForName(const char* name)
{
   return ShipDesign::ClassForName(name);
}

Ship::CLASSIFICATION
Ship::Class() const
{
   return (CLASSIFICATION) design->type;
}

bool
Ship::IsGroundUnit() const
{
   return (design->type & GROUND_UNITS) ? true : false;
}

bool
Ship::IsStarship() const
{
   return (design->type & STARSHIPS) ? true : false;
}

bool
Ship::IsDropship() const
{
   return (design->type & DROPSHIPS) ? true : false;
}

bool
Ship::IsStatic() const
{
   return design->type >= STATION;
}

bool
Ship::IsRogue() const
{
   return ff_count >= 50;
}

// +--------------------------------------------------------------------+

bool
Ship::IsHostileTo(const SimObject* o) const
{
   if (o) {
      if (IsRogue())
         return true;

      if (o->Type() == SIM_SHIP) {
         Ship* s = (Ship*) o;

         if (s->IsRogue())
            return true;

         if (GetIFF() == 0) {
            if (s->GetIFF() > 1)
               return true;
         }
         else {
            if (s->GetIFF() > 0 && s->GetIFF() != GetIFF())
               return true;
         }
      }

      else if (o->Type() == SIM_SHOT || o->Type() == SIM_DRONE) {
         Shot* s = (Shot*) o;

         if (GetIFF() == 0) {
            if (s->GetIFF() > 1)
               return true;
         }
         else {
            if (s->GetIFF() > 0 && s->GetIFF() != GetIFF())
               return true;
         }
      }
   }

   return false;
}

// +--------------------------------------------------------------------+

double
Ship::RepairSpeed() const
{
   return design->repair_speed;
}

int
Ship::RepairTeams() const
{
   return design->repair_teams;
}

// +--------------------------------------------------------------------+

int
Ship::NumContacts() const
{
   // cast-away const:
   return ((Ship*)this)->ContactList().size();
}   

List<Contact>&
Ship::ContactList()
{
   if (region)
      return region->TrackList(GetIFF());

   static List<Contact> empty_contact_list;
   return empty_contact_list;
}

Contact*
Ship::FindContact(SimObject* s) const
{
   if (!s) return 0;

   ListIter<Contact> c_iter = ((Ship*) this)->ContactList();
   while (++c_iter) {
      Contact* c = c_iter.value();

      if (c->GetShip() == s)
         return c;

      if (c->GetShot() == s)
         return c;
   }

   return 0;
}

// +--------------------------------------------------------------------+

Ship*
Ship::GetController() const
{
   Ship*  controller = 0;

   if (carrier) {
      // are we in same region as carrier?
      if (carrier->GetRegion() == GetRegion()) {
         return carrier;
      }

      // if not, figure out who our control unit is:
      else {
         double distance   = 10e6;

         ListIter<Ship> iter = GetRegion()->Carriers();
         while (++iter) {
            Ship* test = iter.value();
            if (test->GetIFF() == GetIFF()) {
               double d = Point(Location() - test->Location()).length();
               if (d < distance) {
                  controller  = test;
                  distance    = d;
               }
            }
         }
      }
   }

   if (!controller) {
      if (element && element->GetCommander())
         controller = element->GetCommander()->GetShip(1);
   }

   return controller;
}

int
Ship::NumInbound() const
{
   int result = 0;

   for (int i = 0; i < flight_decks.size(); i++) {
      result += flight_decks[i]->GetRecoveryQueue().size();
   }

   return result;
}

int
Ship::NumFlightDecks() const
{
   return flight_decks.size();
}

FlightDeck*
Ship::GetFlightDeck(int i) const
{
   if (i >= 0 && i < flight_decks.size())
      return flight_decks[i];

   return 0;
}

// +--------------------------------------------------------------------+

void
Ship::SetFlightPhase(OP_MODE phase)
{
   if (phase == ACTIVE && !launch_time) {
      launch_time  = Game::GameTime() + 1;
      dock         = 0;

      if (element)
         element->SetLaunchTime(launch_time);
   }

   flight_phase = phase;

   if (flight_phase == ACTIVE)
      dock = 0;
}

void
Ship::SetCarrier(Ship* c, FlightDeck* d)
{
   carrier   = c;
   dock      = d;

   if (carrier)
      Observe(carrier);
}

void
Ship::SetInbound(InboundSlot* s)
{
   inbound = s;

   if (inbound && flight_phase == ACTIVE) {
      flight_phase = APPROACH;

      SetCarrier((Ship*) inbound->GetDeck()->GetCarrier(), inbound->GetDeck());

      HUDView* hud = HUDView::GetInstance();

      if (hud && hud->GetShip() == this)
         hud->SetHUDMode(HUDView::HUD_MODE_ILS);
   }
}

void
Ship::Stow()
{
   if (carrier && carrier->GetHangar())
      carrier->GetHangar()->Stow(this);
}

bool
Ship::IsGearDown()
{
   if (gear && gear->GetState() == LandingGear::GEAR_DOWN)
      return true;

   return false;
}

void
Ship::LowerGear()
{
   if (gear && gear->GetState() != LandingGear::GEAR_DOWN) {
      gear->SetState(LandingGear::GEAR_LOWER);
      Scene* scene = 0;

      if (rep)
         scene = rep->GetScene();

      if (scene) {
         for (int i = 0; i < gear->NumGear(); i++) {
            Solid* g = gear->GetGear(i);
            if (g) {
               if (detail_level == 0)
                  scene->DelGraphic(g);
               else
                  scene->AddGraphic(g);
            }
         }
      }
   }
}

void
Ship::RaiseGear()
{
   if (gear && gear->GetState() != LandingGear::GEAR_UP)
      gear->SetState(LandingGear::GEAR_RAISE);
}

void
Ship::ToggleGear()
{
   if (gear) {
      if (gear->GetState() == LandingGear::GEAR_UP ||
          gear->GetState() == LandingGear::GEAR_RAISE) {
         LowerGear();
      }
      else {
         RaiseGear();
      }
   }
}

void
Ship::ToggleNavlights()
{
   bool enable = false;

   for (int i = 0; i < navlights.size(); i++) {
      if (i == 0)
         enable = !navlights[0]->IsEnabled();

      if (enable)
         navlights[i]->Enable();
      else
         navlights[i]->Disable();
   }
}

// +--------------------------------------------------------------------+

int
Ship::CollidesWith(Physical& o)
{
   // bounding spheres test:
   Point delta_loc = Location() - o.Location();
   if (delta_loc.length() > radius + o.Radius())
      return 0;

   if (!o.Rep())
      return 1;

   for (int i = 0; i < detail.NumModels(detail_level); i++) {
      Graphic* g = detail.GetRep(detail_level, i);

      if (o.Type() == SimObject::SIM_SHIP) {
         Ship* o_ship = (Ship*) &o;
         int   o_det  = o_ship->detail_level;

         for (int j = 0; j < o_ship->detail.NumModels(o_det); j++) {
            Graphic* o_g = o_ship->detail.GetRep(o_det, j);

            if (g->CollidesWith(*o_g))
               return 1;
         }
      }
      else {
         // representation collision test (will do bounding spheres first):
         if (g->CollidesWith(*o.Rep()))
            return 1;
      }
   }

   return 0;
}

// +--------------------------------------------------------------------+

static DWORD ff_warn_time = 0;

int
Ship::HitBy(Shot* shot, Point& impact)
{
   if (shot->Owner() == this || IsNetObserver())
      return HIT_NOTHING;

   if (shot->IsFlak())
      return HIT_NOTHING;

   if (InTransition())
      return HIT_NOTHING;

   Point    shot_loc = shot->Location();
   Point    delta    = shot_loc - Location();
   double   dlen     = delta.length();

   Point    hull_impact;
   int      hit_type = HIT_NOTHING;
   double   dscale   = 1;
   float    scale    = design->explosion_scale;
   Weapon*  wep      = 0;

   if (!shot->IsMissile() && !shot->IsBeam()) {
      if (dlen > Radius() * 2)
         return HIT_NOTHING;
   }

   if (scale <= 0)
      scale = design->scale;

   if (shot->Owner()) {
      const ShipDesign* owner_design = shot->Owner()->Design();
      if (owner_design && owner_design->scale < scale)
         scale = (float) owner_design->scale;
   }


   // MISSILE PROCESSING ------------------------------------------------

   if (shot->IsMissile() && rep) {
      if (dlen < rep->Radius()) {
         hull_impact = impact = shot_loc;

         hit_type = CheckShotIntersection(shot, impact, hull_impact, &wep);

         if (hit_type) {
            if (shot->Damage() > 0) {
               DWORD flash = Explosion::HULL_FLASH;
               
               if ((hit_type & HIT_SHIELD) != 0) 
                  flash = Explosion::SHIELD_FLASH;

               sim->CreateExplosion(impact, Velocity(), flash,                   0.3f * scale, scale, region);
               sim->CreateExplosion(impact, Point(),    Explosion::SHOT_BLAST,   2.0f,         scale, region);
            }
         }
      }

      if (hit_type == HIT_NOTHING && shot->IsArmed()) {
         SeekerAI* seeker = (SeekerAI*) shot->GetDirector();

         // if the missile overshot us, take damage proportional to distance
         double damage_radius = shot->Design()->lethal_radius;
         if (dlen < (damage_radius + Radius())) {
            if (seeker && seeker->Overshot()) {
               dscale = 1.0 - (dlen / (damage_radius + Radius()));
            
               if (dscale > 1)
                  dscale = 1;

               if (ShieldStrength() > 5) {
                  hull_impact = impact = shot_loc;

                  if (shot->Damage() > 0) {
                     if (shieldRep)
                        shieldRep->Hit(impact, shot, shot->Damage()*dscale);
                     sim->CreateExplosion(impact, Velocity(), Explosion::SHIELD_FLASH, 0.20f * scale, scale, region);
                     sim->CreateExplosion(impact, Point(),    Explosion::SHOT_BLAST,   20.0f * scale, scale, region);
                  }

                  hit_type = HIT_BOTH;
               }
               else {
                  hull_impact = impact = shot_loc;

                  if (shot->Damage() > 0) {
                     sim->CreateExplosion(impact, Velocity(), Explosion::HULL_FLASH,   0.30f * scale, scale, region);
                     sim->CreateExplosion(impact, Point(),    Explosion::SHOT_BLAST,   20.0f * scale, scale, region);
                  }

                  hit_type = HIT_HULL;
               }
            }
         }
      }
   }

   // ENERGY WEP PROCESSING ---------------------------------------------

   else {
      hit_type = CheckShotIntersection(shot, impact, hull_impact, &wep);

      // impact:
      if (hit_type) {

         if (hit_type & HIT_SHIELD) {
            if (shieldRep)
               shieldRep->Hit(impact, shot, shot->Damage());
            sim->CreateExplosion(impact, Velocity(), Explosion::SHIELD_FLASH, 0.20f * scale, scale, region);
         }

         else {
            if (shot->IsBeam())
               sim->CreateExplosion(impact, Velocity(), Explosion::BEAM_FLASH, 0.30f * scale, scale, region);
            else
               sim->CreateExplosion(impact, Velocity(), Explosion::HULL_FLASH, 0.30f * scale, scale, region);

            if (IsStarship()) {
               Point burst_vel = hull_impact - Location();
               burst_vel.Normalize();
               burst_vel *= Radius() * 0.5;
               burst_vel += Velocity();

               sim->CreateExplosion(hull_impact, burst_vel, Explosion::HULL_BURST, 0.50f * scale, scale, region, this);
            }
         }
      }
   }
 
   // DAMAGE RESOLUTION -------------------------------------------------

   if (hit_type != HIT_NOTHING && shot->IsArmed()) {
      double effective_damage = shot->Damage() * dscale;

      // FRIENDLY FIRE --------------------------------------------------

      if (shot->Owner()) {
         Ship* s = (Ship*) shot->Owner();

         if (!IsRogue() && s->GetIFF() == GetIFF() &&
               s->GetDirector() && s->GetDirector()->Type() < 1000) {
            bool was_rogue = s->IsRogue();

            // only count beam hits once
            if (shot->Damage() && !shot->HitTarget() && GetFriendlyFireLevel() > 0) {
               int penalty = 1;

               if (shot->IsBeam())           penalty = 5;
               else if (shot->IsDrone())     penalty = 7;

               if (s->GetTarget() == this)   penalty *= 3;

               s->IncFriendlyFire(penalty);
            }

            effective_damage *= GetFriendlyFireLevel();

            if (Class() > DRONE && s->Class() > DRONE) {
               if (s->IsRogue() && !was_rogue) {
                  RadioMessage* warn = new(__FILE__,__LINE__) RadioMessage(s, this, RadioMessage::DECLARE_ROGUE);
                  RadioTraffic::Transmit(warn);
               }
               else if (!s->IsRogue() && (Game::GameTime() - ff_warn_time) > 5000) {
                  ff_warn_time = Game::GameTime();

                  RadioMessage* warn = 0;
                  if (s->GetTarget() == this)
                     warn = new(__FILE__,__LINE__) RadioMessage(s, this, RadioMessage::WARN_TARGETED);
                  else
                     warn = new(__FILE__,__LINE__) RadioMessage(s, this, RadioMessage::WARN_ACCIDENT);

                  RadioTraffic::Transmit(warn);
               }
            }
         }
      }

      if (effective_damage > 0) {
         if (!shot->IsBeam() && shot->Design()->damage_type == WeaponDesign::DMG_NORMAL)
            ApplyTorque(shot->Velocity() * (float) effective_damage * 1e-6f);

         if (!NetGame::IsNetGameClient()) {
            InflictDamage(effective_damage, shot, hit_type, hull_impact);
         }
      }
   }
   
   return hit_type;
}

static bool CheckRaySphereIntersection(Point loc, double radius, Point Q, Point w, double len)
{
   Point  d0 = loc - Q;
   Point  d1 = d0.cross(w);
   double dlen = d1.length();          // distance of point from line

   if (dlen > radius)                  // clean miss
      return false;                    // (no impact)

   // possible collision course...
   // find the point on the ray that is closest
   // to the sphere's location:
   Point closest = Q + w * (d0 * w);

   // find the leading edge, and it's distance from the location:
   Point  leading_edge  = Q + w*len;
   Point  leading_delta = leading_edge - loc;
   double leading_dist  = leading_delta.length();

   // if the leading edge is not within the sphere,
   if (leading_dist > radius) {
      // check to see if the closest point is between the
      // ray's endpoints:
      Point delta1 = closest      - Q;
      Point delta2 = leading_edge - Q; // this is w*len

      // if the closest point is not between the leading edge
      // and the origin, this ray does not intersect:
      if (delta1 * delta2 < 0 || delta1.length() > len) {
         return false;
      }
   }

   return true;
}

int
Ship::CheckShotIntersection(Shot* shot, Point& ipt, Point& hpt, Weapon** wep)
{
   int      hit_type = HIT_NOTHING;
   Point    shot_loc = shot->Location();
   Point    shot_org = shot->Origin();
   Point    shot_vpn = shot_loc - shot_org;
   double   shot_len = shot_vpn.Normalize();
   double   blow_len = shot_len;
   bool     hit_hull = false;
   bool     easy     = false;

   if (shot_len <= 0)
      return hit_type;

   if (shot_len < 1000)
      shot_len = 1000;

   Point    hull_impact;
   Point    shield_impact;
   Point    turret_impact;
   Point    closest;
   double   d0 = 1e9;
   double   d1 = 1e9;
   double   ds = 1e9;

   if (dir && dir->Type() == SteerAI::FIGHTER) {
      ShipAI* shipAI = (ShipAI*) dir;
      easy = shipAI->GetAILevel() < 2;
   }

   if (shieldRep && ShieldStrength() > 5) {
      if (shieldRep->CheckRayIntersection(shot_org, shot_vpn, shot_len, shield_impact)) {
         hit_type = HIT_SHIELD;
         closest  = shield_impact;
         d0 = Point(closest - shot_org).length();
         ds = d0;

         ipt = shield_impact;
      }
   }

   if (shieldRep && hit_type == HIT_SHIELD && !shot->IsBeam())
      blow_len = shieldRep->Radius() * 2;

   for (int i = 0; i < detail.NumModels(detail_level) && !hit_hull; i++) {
      Solid* s = (Solid*) detail.GetRep(detail_level, i);
      if (s) {
         if (easy) {
            hit_hull = CheckRaySphereIntersection(s->Location(), s->Radius(), shot_org, shot_vpn, shot_len);
         }
         else {
            hit_hull = s->CheckRayIntersection(shot_org, shot_vpn, blow_len, hull_impact)?true:false;
         }
      }
   }

   if (hit_hull) {
      if (ShieldStrength() > 5 && !shieldRep)
         hit_type = HIT_SHIELD;

      hit_type = hit_type | HIT_HULL;
      hpt      = hull_impact;

      d1 = Point(hull_impact - shot_org).length();

      if (d1 < d0) {
         closest = hull_impact;
         d0 = d1;
      }
   }

   if (IsStarship() || IsStatic()) {
      ListIter<WeaponGroup> g_iter = Weapons();
      while (++g_iter) {
         WeaponGroup* g = g_iter.value();

         if (g->GetDesign() && g->GetDesign()->turret_model) {
            double tsize = g->GetDesign()->turret_model->Radius();

            ListIter<Weapon> w_iter = g->GetWeapons();
            while (++w_iter) {
               Weapon* w = w_iter.value();

               Point tloc = w->GetTurret()->Location();

               if (CheckRaySphereIntersection(tloc, tsize, shot_org, shot_vpn, shot_len)) {
                  Point  delta = tloc - shot_org;
                  d1  = delta.length();

                  if (d1 < d0) {
                     if (wep) *wep = w;
                     hit_type = hit_type | HIT_TURRET;
                     turret_impact = tloc;

                     d0 = d1;

                     closest     = turret_impact;
                     hull_impact = turret_impact;
                     hpt         = turret_impact;

                     if (d1 < ds)
                     ipt         = turret_impact;
                  }
               }
            }
         }
      }
   }

   // trim beam shots to closest impact point:
   if (hit_type && shot->IsBeam()) {
      shot->SetBeamPoints(shot_org, closest);
   }

   return hit_type;
}

// +--------------------------------------------------------------------+

void
Ship::InflictNetDamage(double damage, Shot* shot)
{
   if (damage > 0 && !IsNetObserver()) {
      Physical::InflictDamage(damage, 0);

      // shake by percentage of maximum damage
      double newshake = 50 * damage/design->integrity;
      const double MAX_SHAKE = 7;

      if (shake < MAX_SHAKE)  shake += (float) newshake;
      if (shake > MAX_SHAKE)  shake  = (float) MAX_SHAKE;
   }
}

void
Ship::InflictNetSystemDamage(System* system, double damage, BYTE dmg_type)
{
   if (system && damage > 0 && !IsNetObserver()) {
      bool    dmg_normal   = dmg_type == WeaponDesign::DMG_NORMAL;
      bool    dmg_power    = dmg_type == WeaponDesign::DMG_POWER;
      bool    dmg_emp      = dmg_type == WeaponDesign::DMG_EMP;

      double sys_damage  = damage;
      double avail       = system->Availability();

      if (dmg_normal || system->IsPowerCritical() && dmg_emp) {
         system->ApplyDamage(sys_damage);
         master_caution = true;

         if (system->GetExplosionType() && (avail - system->Availability()) >= 50) {
            float scale = design->explosion_scale;
            if (scale <= 0)
               scale = design->scale;

            sim->CreateExplosion(system->MountLocation(), Velocity() * 0.7f, system->GetExplosionType(), 0.2f * scale, scale, region, this, system);
         }
      }
   }
}

void
Ship::SetNetSystemStatus(System* system, int status, int power, int reactor, double avail)
{
   if (system && !IsNetObserver()) {
      if (system->GetPowerLevel() != power)
         system->SetPowerLevel(power);

      if (system->GetSourceIndex() != reactor) {
         System* s = GetSystem(reactor);

         if (s && s->Type() == System::POWER_SOURCE) {
            PowerSource* reac = (PowerSource*) s;
            reac->AddClient(system);
         }
      }

      if (system->Status() != status) {
         if (status == System::MAINT) {
            ListIter<Component> comp = system->GetComponents();
            while (++comp) {
               Component* c = comp.value();

               if (c->Status() < Component::NOMINAL && c->Availability() < 75) {
                  if (c->SpareCount()           &&
                      c->ReplaceTime() <= 300   &&
                        (c->Availability() < 50 || 
                         c->ReplaceTime()  < c->RepairTime())) {

                     c->Replace();
                  }

                  else if (c->Availability() >= 50 || c->NumJerried() < 5) {
                     c->Repair();
                  }
               }
            }

            RepairSystem(system);
         }
      }

      if (system->Availability() < avail) {
         system->SetNetAvail(avail);
      }
      else {
         system->SetNetAvail(-1);
      }
   }
}

// +----------------------------------------------------------------------+

bool IsWeaponBlockedFriendly(Weapon* w, const SimObject* test)
{
   if (w->GetTarget()) {
      Point  tgt = w->GetTarget()->Location();
      Point  obj = test->Location();
      Point  wep = w->MountLocation();

      Point  dir = tgt - wep;
      double d   = dir.Normalize();
      Point  rho = obj - wep;
      double r   = rho.Normalize();

      // if target is much closer than obstacle,
      // don't worry about friendly fire...
      if (d < 1.5 * r)
         return false;

      Point  dst = dir * r + wep;
      double err = (obj - dst).length();

      if (err < test->Radius() * 1.5)
         return true;
   }

   return false;
}

void
Ship::CheckFriendlyFire()
{
   // if no weapons, there is no worry about friendly fire...
   if (weapons.size() < 1)
      return;

   // only check once each second
   if (Game::GameTime() - friendly_fire_time < 1000)
      return;

   List<Weapon>   w_list;
   int            i, j;

   // clear the FF blocked flag on all weapons
   for (i = 0; i < weapons.size(); i++) {
      WeaponGroup* g = weapons[i];

      for (j = 0; j < g->NumWeapons(); j++) {
         Weapon* w  = g->GetWeapon(j);
         w_list.append(w);
         w->SetBlockedFriendly(false);
      }
   }

   // for each friendly ship within some kind of weapons range,
   ListIter<Contact> c_iter = ContactList();
   while (++c_iter) {
      Contact* c     = c_iter.value();
      Ship*    cship = c->GetShip();
      Shot*    cshot = c->GetShot();

      if (cship && cship != this && (cship->GetIFF() == 0 || cship->GetIFF() == GetIFF())) {
         double range = (cship->Location() - Location()).length();

         if (range > 100e3)
            continue;

         // check each unblocked weapon to see if it is blocked by that ship
         ListIter<Weapon> iter = w_list;
         while (++iter) {
            Weapon* w = iter.value();

            if (!w->IsBlockedFriendly())
               w->SetBlockedFriendly(IsWeaponBlockedFriendly(w, cship));
         }
      }

      else if (cshot && cshot->GetIFF() == GetIFF()) {
         double range = (cshot->Location() - Location()).length();

         if (range > 30e3)
            continue;

         // check each unblocked weapon to see if it is blocked by that shot
         ListIter<Weapon> iter = w_list;
         while (++iter) {
            Weapon* w = iter.value();

            if (!w->IsBlockedFriendly())
               w->SetBlockedFriendly(IsWeaponBlockedFriendly(w, cshot));
         }
      }
   }

   friendly_fire_time = Game::GameTime() + (DWORD) Random(0, 500);
}

// +----------------------------------------------------------------------+

Ship*
Ship::GetLeader() const
{
   if (element)
      return element->GetShip(1);

   return (Ship*) this;
}

int
Ship::GetElementIndex() const
{
   if (element)
      return element->FindIndex(this);

   return 0;
}

int
Ship::GetOrigElementIndex() const
{
   return orig_elem_index;
}

void
Ship::SetElement(Element* e)
{
   element = e;

   if (element) {
      combat_unit = element->GetCombatUnit();

      if (combat_unit) {
         integrity = (float) (design->integrity - combat_unit->GetSustainedDamage());
      }

      orig_elem_index = element->FindIndex(this);
   }
}

void
Ship::SetLaunchPoint(Instruction* pt)
{
   if (pt && !launch_point)
      launch_point = pt;
}

void
Ship::AddNavPoint(Instruction* pt, Instruction* after)
{
   if (GetElementIndex() == 1)
      element->AddNavPoint(pt, after);
}

void
Ship::DelNavPoint(Instruction* pt)
{
   if (GetElementIndex() == 1)
      element->DelNavPoint(pt);
}

void
Ship::ClearFlightPlan()
{
   if (GetElementIndex() == 1)
      element->ClearFlightPlan();
}

// +----------------------------------------------------------------------+

bool
Ship::IsAutoNavEngaged()
{
   if (navsys && navsys->AutoNavEngaged())
      return true;

   return false;
}

void
Ship::SetAutoNav(bool engage)
{
   if (navsys) {
      if (navsys->AutoNavEngaged()) {
         if (!engage)
            navsys->DisengageAutoNav();
      }
      else {
         if (engage)
            navsys->EngageAutoNav();
      }

      if (sim)
         SetControls(sim->GetControls());
   }
}

void
Ship::CommandMode()
{
   if (!dir || dir->Type() != ShipCtrl::DIR_TYPE) {
      const char* msg = "Captain on the bridge";
      RadioVox*   vox = new(__FILE__,__LINE__) RadioVox(0, "1", msg);
      vox->AddPhrase(msg);

      if (vox && !vox->Start()) {
         RadioView::Message( RadioTraffic::TranslateVox(msg) );
         delete vox;
      }

      SetControls(sim->GetControls());
   }

   else {
      const char* msg = "Exec, you have the conn";
      RadioVox*   vox = new(__FILE__,__LINE__) RadioVox(0, "1", msg);
      vox->AddPhrase(msg);

      if (vox && !vox->Start()) {
         RadioView::Message( RadioTraffic::TranslateVox(msg) );
         delete vox;
      }

      SetControls(0);
   }
}

// +----------------------------------------------------------------------+

Instruction*
Ship::GetNextNavPoint()
{
   if (launch_point && launch_point->Status() <= Instruction::ACTIVE)
      return launch_point;

   if (element)
      return element->GetNextNavPoint();

   return 0;
}

int
Ship::GetNavIndex(const Instruction* n)
{
   if (element)
      return element->GetNavIndex(n);

   return 0;
}

double
Ship::RangeToNavPoint(const Instruction* n)
{
   double distance = 0;

   if (n && n->Region()) {
      Point npt = n->Region()->Location() + n->Location();
      npt -= GetRegion()->Location();
      npt = npt.OtherHand(); // convert from map to sim coords

      distance = Point(npt - Location()).length();
   }

   return distance;
}

void
Ship::SetNavptStatus(Instruction* navpt, int status)
{
   if (navpt && navpt->Status() != status) {
      if (status == Instruction::COMPLETE) {
         if (navpt->Action() == Instruction::ASSAULT)
            ::Print("Completed Assault\n");

         else if (navpt->Action() == Instruction::STRIKE)
            ::Print("Completed Strike\n");
      }

      navpt->SetStatus(status);
      
      if (status == Instruction::COMPLETE)
         sim->ProcessEventTrigger(MissionEvent::TRIGGER_NAVPT, 0, Name(), GetNavIndex(navpt));

      if (element) {
         int index = element->GetNavIndex(navpt);

         if (index >= 0)
            NetUtil::SendNavData(false, element, index-1, navpt);
      }
   }
}

List<Instruction>&
Ship::GetFlightPlan()
{
   if (element)
      return element->GetFlightPlan();

   static List<Instruction> dummy_flight_plan;
   return dummy_flight_plan;
}

int
Ship::FlightPlanLength()
{
   if (element)
      return element->FlightPlanLength();

   return 0;
}

// +--------------------------------------------------------------------+

void
Ship::SetWard(Ship* s)
{
   if (ward == s)
      return;

   ward = s;

   if (ward)
      Observe(ward);
}

// +--------------------------------------------------------------------+

void
Ship::SetTarget(SimObject* targ, System* sub, bool from_net)
{
   if (targ && targ->Type() == SimObject::SIM_SHIP) {
      Ship* targ_ship = (Ship*) targ;

      if (targ_ship && targ_ship->IsNetObserver())
         return;
   }

   if (target != targ) {
      // DON'T IGNORE TARGET, BECAUSE IT MAY BE IN THREAT LIST
      target = targ;
      if (target) Observe(target);

      if (sim && target)
         sim->ProcessEventTrigger(MissionEvent::TRIGGER_TARGET, 0, target->Name());
   }
   
   subtarget = sub;

   ListIter<WeaponGroup> weapon = weapons;
   while (++weapon) {
      if (weapon->GetFiringOrders() != Weapon::POINT_DEFENSE) {
         weapon->SetTarget(target, subtarget);

         if (sub || !IsStarship())
            weapon->SetSweep(Weapon::SWEEP_NONE);
         else
            weapon->SetSweep(Weapon::SWEEP_TIGHT);
      }
   }

   if (!from_net && NetGame::GetInstance())
      NetUtil::SendObjTarget(this);

   // track engagement:
   if (target && target->Type() == SimObject::SIM_SHIP) {
      Element* elem     = GetElement();
      Element* tgt_elem = ((Ship*) target)->GetElement();

      if (elem)
         elem->SetAssignment(tgt_elem);
   }
}

void
Ship::DropTarget()
{
   target       = 0;
   subtarget    = 0;

   SetTarget(target, subtarget);
}

// +--------------------------------------------------------------------+

void
Ship::CycleSubTarget(int dir)
{
   if (!target || target->Type() != SimObject::SIM_SHIP)
      return;

   Ship* tgt = (Ship*) target;

   if (tgt->IsDropship())
      return;

   System*  subtgt   = 0;

   ListIter<System> sys = tgt->Systems();

   if (dir > 0) {
      int latch = (subtarget == 0);
      while (++sys) {
         if (sys->Type() == System::COMPUTER || // computers are not targetable
             sys->Type() == System::SENSOR)     // sensors   are not targetable
            continue;

         if (sys.value() == subtarget) {
            latch = 1;
         }

         else if (latch) {
            subtgt = sys.value();
            break;
         }
      }
   }

   else {
      System* prev = 0;

      while (++sys) {
         if (sys->Type() == System::COMPUTER || // computers are not targetable
             sys->Type() == System::SENSOR)     // sensors   are not targetable
            continue;

         if (sys.value() == subtarget) {
            subtgt = prev;
            break;
         }

         prev = sys.value();
      }

      if (!subtarget)
         subtgt = prev;
   }

   SetTarget(tgt, subtgt);
}

// +--------------------------------------------------------------------+

void
Ship::ExecFrame(double seconds)
{
   ZeroMemory(trigger, sizeof(trigger));
   altitude_agl = -1.0e6f;

   if (flight_phase < LAUNCH) {
      DockFrame(seconds);
      return;
   }

   if (flight_phase == LAUNCH  || 
      (flight_phase == TAKEOFF && AltitudeAGL() > Radius())) {
      SetFlightPhase(ACTIVE);
   }

   if (transition_time > 0) {
      transition_time -= (float) seconds;

      if (transition_time <= 0) {
         CompleteTransition();
         return;
      }

      if (rep && IsDying() && killer) {
         killer->ExecFrame(seconds);
      }
   }

   // observers do not run out of power:
   if (IsNetObserver()) {
      for (int i = 0; i < reactors.size(); i++)
         reactors[i]->SetFuelRange(1e6);
   }

   if (IsStatic()) {
      StatFrame(seconds);
      return;
   }

   CheckFriendlyFire();
   ExecNavFrame(seconds);
   ExecEvalFrame(seconds);

   if (IsAirborne()) {
      // are we trying to make orbit?
      if (Location().y >= TERRAIN_ALTITUDE_LIMIT)
         MakeOrbit();
   }

   if (!InTransition()) {
      ExecSensors(seconds);
      ExecThrottle(seconds);
   }

   else if (IsDropping() || IsAttaining() || IsSkipping()) {
      throttle = 100;
   }

   if (target && target->Life() == 0) {
      DropTarget();
   }

   ExecPhysics(seconds);

   if (!InTransition()) {
      UpdateTrack();
   }

   // are we docking?
   if (IsDropship()) {
      ListIter<Ship> iter = GetRegion()->Carriers();

      while (++iter) {
         Ship* carrier_target = iter.value();

         double range = (Location() - carrier_target->Location()).length();
         if (range > carrier_target->Radius() * 1.5)
            continue;

         if (carrier_target->GetIFF() == GetIFF() || carrier_target->GetIFF() == 0) {
            for (int i = 0; i < carrier_target->NumFlightDecks(); i++) {
               if (carrier_target->GetFlightDeck(i)->Recover(this))
                  break;
            }
         }
      }
   }

   ExecSystems(seconds);
   ExecMaintFrame(seconds);

   if (flight_decks.size() > 0) {
      Camera*  global_cam        = CameraDirector::GetInstance()->GetCamera();
      Point    global_cam_loc    = global_cam->Pos();
      bool     disable_shadows   = false;

      for (int i = 0; i < flight_decks.size(); i++) {
         if (flight_decks[i]->ContainsPoint(global_cam_loc))
            disable_shadows = true;
      }

      EnableShadows(!disable_shadows);
   }

   if (!_finite(Location().x)) {
      DropTarget();
   }

   if (!IsStatic() && !IsGroundUnit() && GetFlightModel() < 2)
      CalcFlightPath();
}

// +--------------------------------------------------------------------+

void
Ship::LaunchProbe()
{
   if (net_observer_mode)
      return;

   if (sensor_drone) {
      sensor_drone = 0;
   }

   if (probe) {
      sensor_drone = (Drone*) probe->Fire();

      if (sensor_drone)
         Observe(sensor_drone);

      else if (sim->GetPlayerShip() == this)
         Button::PlaySound(Button::SND_REJECT);
   }
}

void
Ship::SetProbe(Drone* d)
{
   if (sensor_drone != d) {
      sensor_drone = d;

      if (sensor_drone)
         Observe(sensor_drone);
   }
}

void
Ship::ExecSensors(double seconds)
{
   // how visible are we?
   DoEMCON();
 
   // what can we see?
   if (sensor)
      sensor->ExecFrame(seconds);

   // can we still see our target?   
   if (target) {
      int target_found = 0;
      ListIter<Contact> c_iter = ContactList();
      while (++c_iter) {
         Contact* c = c_iter.value();

         if (target == c->GetShip() || target == c->GetShot()) {
            target_found = 1;

            bool vis = c->Visible(this) || c->Threat(this);

            if (!vis && !c->PasLock() && !c->ActLock())
               DropTarget();
         }
      }

      if (!target_found)
         DropTarget();
   }   
}

// +--------------------------------------------------------------------+

void
Ship::ExecNavFrame(double seconds)
{
   bool auto_pilot = false;

   // update director info string:
   SetFLCSMode(flcs_mode);

   if (navsys) {
      navsys->ExecFrame(seconds);
      
      if (navsys->AutoNavEngaged()) {
         if (dir && dir->Type() == NavAI::DIR_TYPE) {
            NavAI* navai =  (NavAI*) dir;
   
            if (navai->Complete()) {
               navsys->DisengageAutoNav();
               SetControls(sim->GetControls());
            }
            else {
               auto_pilot = true;
            }
         }
      }
   }
   
   // even if we are not on auto pilot,
   // have we completed the next navpoint?

   Instruction* navpt = GetNextNavPoint();
   if (navpt && !auto_pilot) {
      if (navpt->Region() == GetRegion()) {
         double distance = 0;
      
         Point npt = navpt->Location();

         if (navpt->Region())
            npt += navpt->Region()->Location();
      
         Sim* sim = Sim::GetSim();
         if (sim->GetActiveRegion())
            npt -= sim->GetActiveRegion()->Location();

         npt = npt.OtherHand();
      
         // distance from self to navpt:
         distance = Point(npt - Location()).length();
         
         if (distance < 10 * Radius())
            SetNavptStatus(navpt, Instruction::COMPLETE);
      }
   }
}

// +--------------------------------------------------------------------+

void
Ship::ExecEvalFrame(double seconds)
{
   // is it already too late?
   if (life == 0 || integrity < 1) return;

   const  DWORD EVAL_FREQUENCY  = 1000;   // once every second
   static DWORD last_eval_frame = 0;      // one ship per game frame

   if (element && element->NumObjectives() > 0            &&
       Game::GameTime() - last_eval_time > EVAL_FREQUENCY &&
       last_eval_frame != Game::Frame()) {

      last_eval_time  = Game::GameTime();
      last_eval_frame = Game::Frame();

      for (int i = 0; i < element->NumObjectives(); i++) {
         Instruction* obj = element->GetObjective(i);

         if (obj->Status() <= Instruction::ACTIVE) {
            obj->Evaluate(this);
         }
      }
   }
}

// +--------------------------------------------------------------------+

void
Ship::ExecPhysics(double seconds)
{
   if (net_control) {
      net_control->ExecFrame(seconds);
      Thrust(seconds);  // drive flare
   }
   else {
      thrust = (float) Thrust(seconds);
      SetupAgility();

      if (seconds > 0) {
         g_force = 0.0f;
      }

      if (IsAirborne()) {
         Point v1 = velocity;
         AeroFrame(seconds);
         Point v2 = velocity;
         Point dv = v2 - v1 + Point(0, g_accel*seconds, 0);

         if (seconds > 0) {
            g_force = (float) (dv * cam.vup() / seconds) / 9.8f;
         }
      }

      else if (IsDying() || flight_model < 2) { // standard and relaxed modes
         Physical::ExecFrame(seconds);
      }

      else {                                    // arcade mode
         Physical::ArcadeFrame(seconds);
      }
   }
}

// +--------------------------------------------------------------------+

void
Ship::ExecThrottle(double seconds)
{
   double spool = 75 * seconds;

   if (throttle < throttle_request)
      if (throttle_request-throttle < spool)
         throttle = throttle_request;
      else
         throttle += spool;

   else if (throttle > throttle_request)
      if (throttle - throttle_request < spool)
         throttle = throttle_request;
      else
         throttle -= spool;
}

// +--------------------------------------------------------------------+

void
Ship::ExecSystems(double seconds)
{
   if (!rep)
      return;

   int i;

   ListIter<System> iter = systems;
   while (++iter) {
      System* sys = iter.value();

      sys->Orient(this);

      // sensors have already been executed,
      // they can not be run twice in a frame!
      if (sys->Type() != System::SENSOR)
         sys->ExecFrame(seconds);
   }

   // hangars and weapon groups are not systems
   // they must be executed separately from above
   if (hangar)
      hangar->ExecFrame(seconds);

   wep_mass   = 0.0f;
   wep_resist = 0.0f;

   bool winchester_cycle = false;

   for (i = 0; i < weapons.size(); i++) {
      WeaponGroup* w_group = weapons[i];
      w_group->ExecFrame(seconds);

      if (w_group->GetTrigger() && w_group->GetFiringOrders() == Weapon::MANUAL) {

         Weapon* gun = w_group->GetSelected();

         SimObject* gun_tgt = gun->GetTarget();

         // if no target has been designated for this
         // weapon, let it guide on the contact closest
         // to its boresight.  this must be done before
         // firing the weapon.

         if (sensor && gun->Guided() && !gun->Design()->beam && !gun_tgt) {
            gun->SetTarget(sensor->AcquirePassiveTargetForMissile(), 0);
         }

         gun->Fire();

         w_group->SetTrigger(false);
         w_group->CycleWeapon();
         w_group->CheckAmmo();

         // was that the last shot from this missile group?
         if (w_group->IsMissile() && w_group->Ammo() < 1) {

            // is this the current secondary weapon group?
            if (weapons[secondary] == w_group) {
               winchester_cycle = true;
            }
         }
      }
   
      wep_mass   += w_group->Mass();
      wep_resist += w_group->Resistance();
   }

   // if we just fired the last shot in the current secondary
   // weapon group, auto cycle to another secondary weapon:
   if (winchester_cycle) {
      int old_secondary = secondary;

      CycleSecondary();

      // do not winchester-cycle to an A2G missile type, 
      // or a missile that is also out of ammo,
      // keep going!

      while (secondary != old_secondary) {
         Weapon* missile = GetSecondary();
         if (missile && missile->CanTarget(Ship::GROUND_UNITS))
            CycleSecondary();

         else if (weapons[secondary]->Ammo() < 1)
            CycleSecondary();

         else
            break;
      }
   }

   mass    = (float) design->mass    + wep_mass;

   if (IsDropship())
   agility = (float) design->agility - wep_resist;

   if (shieldRep) {
      Solid* solid = (Solid*) rep;
      shieldRep->MoveTo(solid->Location());
      shieldRep->SetOrientation(solid->Orientation());

      bool bubble = false;
      if (shield)
         bubble = shield->ShieldBubble();

      if (shieldRep->ActiveHits()) {
         shieldRep->Energize(seconds, bubble);
         shieldRep->Show();
      }
      else {
         shieldRep->Hide();
      }
   }

   if (cockpit) {
      Solid* solid = (Solid*) rep;

      Point cpos   = cam.Pos()                + 
                     cam.vrt() * bridge_vec.x +
                     cam.vpn() * bridge_vec.y + 
                     cam.vup() * bridge_vec.z;

      cockpit->MoveTo(cpos);
      cockpit->SetOrientation(solid->Orientation());
   }
}

// +--------------------------------------------------------------------+

void
Ship::AeroFrame(double seconds)
{
   float g_save = g_accel;

   if (Class() == LCA) {
      lat_thrust = true;
      SetGravity(0.0f);
   }

   if (AltitudeAGL() < Radius()) {
      SetGravity(0.0f);

      // on the ground/runway?
      double bottom = 1e9;
      double tlevel = Location().y - AltitudeAGL();

      // taking off or landing?
      if (flight_phase < ACTIVE || flight_phase > APPROACH) {
         if (dock)
            tlevel = dock->MountLocation().y;
      }

      if (tlevel < 0)
         tlevel = 0;

      if (gear)
         bottom = gear->GetTouchDown()-1;
      else
         bottom = Location().y-6;

      if (bottom < tlevel)
         TranslateBy(Point(0, bottom-tlevel, 0));
   }

   // MODEL 2: ARCADE
   if (flight_model >= 2) {
      Physical::ArcadeFrame(seconds);
   }

   // MODEL 1: RELAXED
   else if (flight_model == 1) {
      Physical::ExecFrame(seconds);
   }

   // MODEL 0: STANDARD
   else {
      // apply drag-torque (i.e. turn ship into
      // velocity vector to minimize drag):

      Point  vnrm = velocity;
      double v    = vnrm.Normalize();
      double pitch_deflection = vnrm * cam.vup();
      double yaw_deflection   = vnrm * cam.vrt();

      if (lat_thrust && v < 250) {
      }

      else {
         if (v < 250) {
            double factor = 1.2 + (250 - v) / 100;

            ApplyPitch(pitch_deflection * -factor);
            ApplyYaw(yaw_deflection * factor);

            dp += (float) (dp_acc * seconds);
            dy += (float) (dy_acc * seconds);
         }

         else {
            if (fabs(pitch_deflection) > stall) {
               ApplyPitch(pitch_deflection * -1.2);
               dp += (float) (dp_acc * seconds);
            }

            ApplyYaw(yaw_deflection * 2);
            dy += (float) (dy_acc * seconds);
         }
      }

      // compute rest of physics:
      Physical::AeroFrame(seconds);
   }

   SetGravity(g_save);
}

// +--------------------------------------------------------------------+

void
Ship::LinearFrame(double seconds)
{
   Physical::LinearFrame(seconds);

   if (!IsAirborne() || Class() != LCA)
      return;

   // damp lateral movement in atmosphere:

   // side-to-side
   if (!trans_x) {
      Point transvec = cam.vrt();
      transvec *= (transvec * velocity) * seconds * 0.5;
      velocity -= transvec;
   }

   // fore-and-aft
   if (!trans_y && fabs(thrust) < 1.0f) {
      Point transvec = cam.vpn();
      transvec *= (transvec * velocity) * seconds * 0.25;
      velocity -= transvec;
   }

   // up-and-down
   if (!trans_z) {
      Point transvec = cam.vup();
      transvec *= (transvec * velocity) * seconds * 0.5;
      velocity -= transvec;
   }
}

// +--------------------------------------------------------------------+

void
Ship::DockFrame(double seconds)
{
   SelectDetail(seconds);

   if (sim->GetPlayerShip() == this) {
      // Make sure the thruster sound is diabled
      // when the player is on the runway or catapult
      if (thruster) {
         thruster->ExecTrans(0,0,0);
      }
   }

   if (rep) {
      // Update the graphic rep and light sources:
      // (This is usually done by the physics class,
      // but when the ship is in dock, we skip the
      // standard physics processing):
      rep->MoveTo(cam.Pos());
      rep->SetOrientation(cam.Orientation());
   
      if (light)
         light->MoveTo(cam.Pos());

      ListIter<System> iter = systems;
      while (++iter)
         iter->Orient(this);

      double spool = 75 * seconds;

      if (flight_phase == DOCKING) {
         throttle_request = 0;
         throttle         = 0;
      }

      else if (throttle < throttle_request)
         if (throttle_request-throttle < spool)
            throttle = throttle_request;
         else
            throttle += spool;

      else if (throttle > throttle_request)
         if (throttle - throttle_request < spool)
            throttle = throttle_request;
         else
            throttle -= spool;

      // make sure there is power to run the drive:
      for (int i = 0; i < reactors.size(); i++)
         reactors[i]->ExecFrame(seconds);

      // count up weapon ammo for status mfd:
      for (int i = 0; i < weapons.size(); i++)
         weapons[i]->ExecFrame(seconds);

      // show drive flare while on catapult:
      if (main_drive) {
         main_drive->SetThrottle(throttle);

         if (throttle > 0)
            main_drive->Thrust(seconds);  // show drive flare
      }
   }

   if (cockpit && !cockpit->Hidden()) {
      Solid* solid = (Solid*) rep;

      Point cpos   = cam.Pos()                + 
                     cam.vrt() * bridge_vec.x +
                     cam.vpn() * bridge_vec.y + 
                     cam.vup() * bridge_vec.z;

      cockpit->MoveTo(cpos);
      cockpit->SetOrientation(solid->Orientation());
   }
}

// +--------------------------------------------------------------------+

void
Ship::StatFrame(double seconds)
{
   if (flight_phase != ACTIVE) {
      flight_phase = ACTIVE;
      launch_time  = Game::GameTime() + 1;

      if (element)
         element->SetLaunchTime(launch_time);
   }

   if (IsGroundUnit()) {
      // glue buildings to the terrain:
      Point    loc     = Location();
      Terrain* terrain = region->GetTerrain();

      if (terrain) {
         loc.y = terrain->Height(loc.x, loc.z);
         MoveTo(loc);
      }
   }

   if (rep) {
      rep->MoveTo(cam.Pos());
      rep->SetOrientation(cam.Orientation());
   }

   if (light) {
      light->MoveTo(cam.Pos());
   }

   ExecSensors(seconds);

   if (target && target->Life() == 0) {
      DropTarget();
   }

   if (dir) dir->ExecFrame(seconds);

   SelectDetail(seconds);

   int i = 0;

   if (rep) {
      ListIter<System> iter = systems;
      while (++iter)
         iter->Orient(this);

      for (i = 0; i < reactors.size(); i++)
         reactors[i]->ExecFrame(seconds);

      for (i = 0; i < navlights.size(); i++)
         navlights[i]->ExecFrame(seconds);

      for (i = 0; i < weapons.size(); i++)
         weapons[i]->ExecFrame(seconds);

      if (farcaster) {
         farcaster->ExecFrame(seconds);

         if (navlights.size() == 2) {
            if (farcaster->Charge() > 99) {
               navlights[0]->Enable();
               navlights[1]->Disable();
            }
            else {
               navlights[0]->Disable();
               navlights[1]->Enable();
            }
         }
      }

      if (shield)
         shield->ExecFrame(seconds);

      if (hangar)
         hangar->ExecFrame(seconds);

      if (flight_decks.size() > 0) {
         Camera*  global_cam        = CameraDirector::GetInstance()->GetCamera();
         Point    global_cam_loc    = global_cam->Pos();
         bool     disable_shadows   = false;

         for (i = 0; i < flight_decks.size(); i++) {
            flight_decks[i]->ExecFrame(seconds);

            if (flight_decks[i]->ContainsPoint(global_cam_loc))
               disable_shadows = true;
         }

         EnableShadows(!disable_shadows);
      }
   }

   if (shieldRep) {
      Solid* solid = (Solid*) rep;
      shieldRep->MoveTo(solid->Location());
      shieldRep->SetOrientation(solid->Orientation());

      bool bubble = false;
      if (shield)
         bubble = shield->ShieldBubble();

      if (shieldRep->ActiveHits()) {
         shieldRep->Energize(seconds, bubble);
         shieldRep->Show();
      }
      else {
         shieldRep->Hide();
      }
   }

   if (!_finite(Location().x)) {
      DropTarget();
   }
}

// +--------------------------------------------------------------------+

Graphic*
Ship::Cockpit() const
{
   return cockpit;
}

void
Ship::ShowCockpit()
{
   if (cockpit) {
      cockpit->Show();

      ListIter<WeaponGroup> g = weapons;
      while (++g) {
         ListIter<Weapon> w = g->GetWeapons();
         while (++w) {
            Solid* turret = w->GetTurret();
            if (turret) {
               turret->Show();

               Solid* turret_base = w->GetTurretBase();
               if (turret_base)
                  turret_base->Show();
            }

            if (w->IsMissile()) {
               for (int i = 0; i < w->Ammo(); i++) {
                  Solid* store = w->GetVisibleStore(i);
                  if (store) {
                     store->Show();
                  }
               }
            }
         }
      }
   }
}

void
Ship::HideCockpit()
{
   if (cockpit)
      cockpit->Hide();
}

// +--------------------------------------------------------------------+

void
Ship::SelectDetail(double seconds)
{
   detail.ExecFrame(seconds);
   detail.SetLocation(GetRegion(), Location());
   
   int new_level = detail.GetDetailLevel();

   if (detail_level != new_level) {
      Scene* scene = 0;

      // remove current rep from scene (if necessary):
      for (int i = 0; i < detail.NumModels(detail_level); i++) {
         Graphic* g = detail.GetRep(detail_level, i);
         if (g) {
            scene = g->GetScene();
            if (scene)
               scene->DelGraphic(g);
         }
      }
         
      // switch to new rep:
      detail_level = new_level;
      rep = detail.GetRep(detail_level);

      // add new rep to scene (if necessary):
      if (scene) {
         for (int i = 0; i < detail.NumModels(detail_level); i++) {
            Graphic*   g = detail.GetRep(detail_level, i);
            Point      s = detail.GetSpin(detail_level, i);
            Matrix     m = cam.Orientation();

            m.Pitch(s.x);
            m.Yaw(s.z);
            m.Roll(s.y);

            scene->AddGraphic(g);
            g->MoveTo(cam.Pos() + detail.GetOffset(detail_level, i));
            g->SetOrientation(m);
         }

         // show/hide external stores and landing gear...
         if (detail.NumLevels() > 0) {
            if (gear && (gear->GetState() != LandingGear::GEAR_UP)) {
               for (int i = 0; i < gear->NumGear(); i++) {
                  Solid* g = gear->GetGear(i);

                  if (g) {
                     if (detail_level == 0)
                        scene->DelGraphic(g);
                     else
                        scene->AddGraphic(g);
                  }
               }
            }

            ListIter<WeaponGroup> g = weapons;
            while (++g) {
               ListIter<Weapon> w = g->GetWeapons();
               while (++w) {
                  Solid* turret = w->GetTurret();
                  if (turret) {
                     if (detail_level == 0)
                        scene->DelGraphic(turret);
                     else
                        scene->AddGraphic(turret);

                     Solid* turret_base = w->GetTurretBase();
                     if (turret_base) {
                        if (detail_level == 0)
                           scene->DelGraphic(turret_base);
                        else
                           scene->AddGraphic(turret_base);
                     }
                  }
                  if (w->IsMissile()) {
                     for (int i = 0; i < w->Ammo(); i++) {
                        Solid* store = w->GetVisibleStore(i);
                        if (store) {
                           if (detail_level == 0)
                              scene->DelGraphic(store);
                           else
                              scene->AddGraphic(store);
                        }
                     }
                  }
               }
            }
         }
      }
   }

   else {
      int nmodels = detail.NumModels(detail_level);

      if (nmodels > 1) {
         for (int i = 0; i < nmodels; i++) {
            Graphic*   g = detail.GetRep(detail_level, i);
            Point      s = detail.GetSpin(detail_level, i);
            Matrix     m = cam.Orientation();

            m.Pitch(s.x);
            m.Yaw(s.z);
            m.Roll(s.y);

            g->MoveTo(cam.Pos() + detail.GetOffset(detail_level, i));
            g->SetOrientation(m);
         }
      }
   }
}

// +--------------------------------------------------------------------+

void
Ship::ShowRep()
{
   for (int i = 0; i < detail.NumModels(detail_level); i++) {
      Graphic* g = detail.GetRep(detail_level, i);
      g->Show();
   }

   if (gear && (gear->GetState() != LandingGear::GEAR_UP)) {
      for (int i = 0; i < gear->NumGear(); i++) {
         Solid* g = gear->GetGear(i);
         if (g) g->Show();
      }
   }

   ListIter<WeaponGroup> g = weapons;
   while (++g) {
      ListIter<Weapon> w = g->GetWeapons();
      while (++w) {
         Solid* turret = w->GetTurret();
         if (turret) {
            turret->Show();

            Solid* turret_base = w->GetTurretBase();
            if (turret_base)
               turret_base->Show();
         }

         if (w->IsMissile()) {
            for (int i = 0; i < w->Ammo(); i++) {
               Solid* store = w->GetVisibleStore(i);
               if (store) {
                  store->Show();
               }
            }
         }
      }
   }
}

void
Ship::HideRep()
{
   for (int i = 0; i < detail.NumModels(detail_level); i++) {
      Graphic* g = detail.GetRep(detail_level, i);
      g->Hide();
   }

   if (gear && (gear->GetState() != LandingGear::GEAR_UP)) {
      for (int i = 0; i < gear->NumGear(); i++) {
         Solid* g = gear->GetGear(i);
         if (g) g->Hide();
      }
   }

   ListIter<WeaponGroup> g = weapons;
   while (++g) {
      ListIter<Weapon> w = g->GetWeapons();
      while (++w) {
         Solid* turret = w->GetTurret();
         if (turret) {
            turret->Hide();

            Solid* turret_base = w->GetTurretBase();
            if (turret_base)
               turret_base->Hide();
         }

         if (w->IsMissile()) {
            for (int i = 0; i < w->Ammo(); i++) {
               Solid* store = w->GetVisibleStore(i);
               if (store) {
                  store->Hide();
               }
            }
         }
      }
   }
}

void
Ship::EnableShadows(bool enable)
{
   for (int i = 0; i < detail.NumModels(detail_level); i++) {
      Graphic* g = detail.GetRep(detail_level, i);
      
      if (g->IsSolid()) {
         Solid* s = (Solid*) g;

         ListIter<Shadow> iter = s->GetShadows();
         while (++iter) {
            Shadow* shadow = iter.value();
            shadow->SetEnabled(enable);
         }
      }
   }
}

// +--------------------------------------------------------------------+

bool
Ship::Update(SimObject* obj)
{
   if (obj == ward)
      ward = 0;

   if (obj == target) {
      target       = 0;
      subtarget    = 0;
   }

   if (obj == carrier) {
      carrier      = 0;
      dock         = 0;
      inbound      = 0;
   }

   if (obj->Type() == SimObject::SIM_SHOT ||
       obj->Type() == SimObject::SIM_DRONE) {
      Shot* s = (Shot*) obj;

      if (sensor_drone == s)
         sensor_drone = 0;

      if (decoy_list.contains(s))
         decoy_list.remove(s);

      if (threat_list.contains(s))
         threat_list.remove(s);
   }

   return SimObserver::Update(obj);
}

// +--------------------------------------------------------------------+

int
Ship::GetFuelLevel() const
{
   if (reactors.size() > 0) {
      PowerSource* reactor = reactors[0];
      if (reactor)
         return reactor->Charge();
   }

   return 0;
}

void
Ship::SetThrottle(double percent)
{
   throttle_request = percent;
   
   if (throttle_request < 0)          throttle_request = 0;
   else if (throttle_request > 100)   throttle_request = 100;

   if (throttle_request < 50)
      augmenter = false;
}

void
Ship::SetAugmenter(bool enable)
{
   if (throttle <= 50)
      enable = false;

   if (main_drive && main_drive->MaxAugmenter() <= 0)
      enable = false;

   augmenter = enable;
}

// +--------------------------------------------------------------------+

void
Ship::SetTransition(double trans_time, int trans_type, const Point& trans_loc)
{
   transition_time = (float) trans_time;
   transition_type = trans_type;
   transition_loc  = trans_loc;
}

void
Ship::DropOrbit()
{
   if (IsDropship() && transition_type == TRANSITION_NONE && !IsAirborne()) {
      SimRegion* dst_rgn = sim->FindNearestTerrainRegion(this);

      if (dst_rgn &&
          dst_rgn->GetOrbitalRegion()->Primary() ==
          GetRegion()->GetOrbitalRegion()->Primary()) {

         transition_time = 10.0f;
         transition_type = TRANSITION_DROP_ORBIT;
         transition_loc  = Location() + Heading() * (-2*Radius());

         RadioTraffic::SendQuickMessage(this, RadioMessage::BREAK_ORBIT);
         SetControls(0);
      }
   }
}

void
Ship::MakeOrbit()
{
   if (IsDropship() && transition_type == TRANSITION_NONE && IsAirborne()) {
      transition_time = 5.0f;
      transition_type = TRANSITION_MAKE_ORBIT;
      transition_loc  = Location() + Heading() * (-2*Radius());

      RadioTraffic::SendQuickMessage(this, RadioMessage::MAKE_ORBIT);
      SetControls(0);
   }
}

// +--------------------------------------------------------------------+

bool
Ship::IsInCombat()
{
   if (IsRogue())
      return true;

   bool combat = false;

   ListIter<Contact> c_iter = ContactList();
   while (++c_iter) {
      Contact* c     = c_iter.value();
      Ship*    cship = c->GetShip();
      int      ciff  = c->GetIFF(this);
      Point    delta = c->Location() - Location();
      double   dist  = delta.length();

      if (c->Threat(this) && !cship) {
         if (IsStarship())
            combat = dist < 120e3;
         else
            combat = dist <  60e3;
      }

      else if (cship && ciff > 0 && ciff != GetIFF()) {
         if (IsStarship() && cship->IsStarship())
            combat = dist < 120e3;
         else
            combat = dist <  60e3;
      }
   }

   return combat;
}

// +--------------------------------------------------------------------+

bool
Ship::CanTimeSkip()
{
   bool         go    = false;
   Instruction* navpt = GetNextNavPoint();

   if (MissionClock() < 10000 || NetGame::IsNetGame())
      return go;

   if (navpt) {
      go = true;

      if (navpt->Region() != GetRegion())
         go = false;

      else if (Point(navpt->Location().OtherHand() - Location()).length() < 30e3)
         go = false;
   }

   if (go)
      go = !IsInCombat();

   return go;
}

void
Ship::TimeSkip()
{
   if (CanTimeSkip()) {
      // go back to regular time before performing the skip:
      Game::SetTimeCompression(1);

      transition_time = 7.5f;
      transition_type = TRANSITION_TIME_SKIP;
      transition_loc  = Location() + Heading() * (Velocity().length() * 4);
         // 2500; //(8*Radius());
      
      if (rand() < 16000)
         transition_loc += BeamLine() * (2.5*Radius());
      else
         transition_loc += BeamLine() * (-2 *Radius());

      if (rand() < 8000)
         transition_loc += LiftLine() * (-1*Radius());
      else
         transition_loc += LiftLine() * (1.8*Radius());

      SetControls(0);
   }

   else if (sim->GetPlayerShip() == this) {
      SetAutoNav(true);
   }
}

// +--------------------------------------------------------------------+

void
Ship::DropCam(double time, double range)
{
   transition_type = TRANSITION_DROP_CAM;

   if (time > 0)
      transition_time = (float) time;
   else
      transition_time = 10.0f;

   Point    offset          = Heading() * (Velocity().length() * 5);
   double   lateral_offset  = 2 * Radius();
   double   vertical_offset = Radius();

   if (vertical_offset > 300)
      vertical_offset = 300;

   if (rand() < 16000)
      lateral_offset *= -1;

   if (rand() <  8000)
      vertical_offset *= -1;

   offset += BeamLine() * lateral_offset;
   offset += LiftLine() * vertical_offset;

   if (range > 0)
      offset *= range;

   transition_loc  = Location() + offset;
}

// +--------------------------------------------------------------------+

void
Ship::DeathSpiral()
{
   if (!killer)
      killer = new(__FILE__,__LINE__) ShipKiller(this);

   ListIter<System> iter = systems;
   while (++iter)
      iter->PowerOff();

   // transfer arcade velocity to newtonian velocity:
   if (flight_model >= 2) {
      velocity += arcade_velocity;
   }

   if (GetIFF() < 100 && !IsGroundUnit()) {
      RadioTraffic::SendQuickMessage(this, RadioMessage::DISTRESS);
   }

   transition_type = TRANSITION_DEATH_SPIRAL;

   killer->BeginDeathSpiral();

   transition_time = killer->TransitionTime();
   transition_loc  = killer->TransitionLoc();
}

// +--------------------------------------------------------------------+

void
Ship::CompleteTransition()
{
   int old_type = transition_type;
   transition_time = 0.0f;
   transition_type = TRANSITION_NONE;

   switch (old_type) {
   case TRANSITION_NONE:
   case TRANSITION_DROP_CAM:
   default:
      return;

   case TRANSITION_DROP_ORBIT: {
         SetControls(0);
         SimRegion*  dst_rgn   = sim->FindNearestTerrainRegion(this);
         Point       dst_loc   = Location().OtherHand() * 0.20;
                     dst_loc.x += 6000 * GetElementIndex();
                     dst_loc.z = TERRAIN_ALTITUDE_LIMIT * 0.95;
                     dst_loc   += RandomDirection() * 2e3;

         sim->RequestHyperJump(this, dst_rgn, dst_loc, TRANSITION_DROP_ORBIT);

         ShipStats* stats = ShipStats::Find(Name());
         stats->AddEvent(SimEvent::BREAK_ORBIT, dst_rgn->Name());
      }
      break;

   case TRANSITION_MAKE_ORBIT: {
         SetControls(0);
         SimRegion*  dst_rgn = sim->FindNearestSpaceRegion(this);
         double      dist    = 200.0e3 + 10.0e3 * GetElementIndex();
         Point       esc_vec = dst_rgn->GetOrbitalRegion()->Location() -
                               dst_rgn->GetOrbitalRegion()->Primary()->Location();
 
         esc_vec.z = -100 * GetElementIndex();
         esc_vec.Normalize();
         esc_vec *= -dist;
         esc_vec += RandomDirection() * 2e3;

         sim->RequestHyperJump(this, dst_rgn, esc_vec, TRANSITION_MAKE_ORBIT);

         ShipStats* stats = ShipStats::Find(Name());
         stats->AddEvent(SimEvent::MAKE_ORBIT, dst_rgn->Name());
      }
      break;

   case TRANSITION_TIME_SKIP: {
         Instruction* navpt = GetNextNavPoint();

         if (navpt) {
            Point    delta = navpt->Location().OtherHand() - Location();
            Point    unit  = delta; unit.Normalize();
            Point    trans = delta + unit * -20e3;
            double   dist  = trans.length();
            double   speed = navpt->Speed();

            if (speed < 50) speed = 500;

            double   etr   = dist / speed;

            sim->ResolveTimeSkip(etr);
         }
      }
      break;

   case TRANSITION_DEATH_SPIRAL:
         SetControls(0);
         transition_type = TRANSITION_DEAD;
      break;
   }

}

bool
Ship::IsAirborne() const
{
   if (region)
      return region->Type() == SimRegion::AIR_SPACE;

   return false;
}

double
Ship::CompassHeading() const
{
   Point heading = Heading();
   double compass_heading = atan2(fabs(heading.x), heading.z);

   if (heading.x < 0)
      compass_heading *= -1;

   double result = compass_heading + PI;

   if (result >= 2*PI)
      result -= 2*PI;

   return result;
}

double
Ship::CompassPitch() const
{
   Point heading = Heading();
   return asin(heading.y);
}

double
Ship::AltitudeMSL() const
{
   return Location().y;
}

double
Ship::AltitudeAGL() const
{
   if (altitude_agl < -1000) {
      Ship*    pThis   = (Ship*) this; // cast-away const
      Point    loc     = Location();

      Terrain* terrain = region->GetTerrain();

      if (terrain)
         pThis->altitude_agl = (float) (loc.y - terrain->Height(loc.x, loc.z));

      else
         pThis->altitude_agl = (float) loc.y;

      if (!_finite(altitude_agl)) {
         pThis->altitude_agl = 0.0f;
      }
   }

   return altitude_agl;
}

double
Ship::GForce() const
{
   return g_force;
}

// +--------------------------------------------------------------------+

WeaponGroup*
Ship::FindWeaponGroup(const char* name)
{
   WeaponGroup* group = 0;

   ListIter<WeaponGroup> iter = weapons;
   while (!group && ++iter)
      if (!stricmp(iter->Name(), name))
         group = iter.value();

   if (!group) {
      group = new(__FILE__,__LINE__) WeaponGroup(name);
      weapons.append(group);
   }

   return group;
}

void
Ship::SelectWeapon(int n, int w)
{
   if (n < weapons.size())
      weapons[n]->SelectWeapon(w);
}

// +--------------------------------------------------------------------+

void
Ship::CyclePrimary()
{
   if (weapons.isEmpty())
      return;

   if (IsDropship() && primary < weapons.size()) {
      WeaponGroup* p = weapons[primary];
      Weapon*      w = p->GetSelected();

      if (w && w->GetTurret()) {
         p->SetFiringOrders(Weapon::POINT_DEFENSE);
      }
   }

   int n = primary + 1;
   while (n != primary) {
      if (n >= weapons.size())
         n = 0;

      if (weapons[n]->IsPrimary()) {
         weapons[n]->SetFiringOrders(Weapon::MANUAL);
         break;
      }

      n++;
   }

   primary = n;
}

// +--------------------------------------------------------------------+

void
Ship::CycleSecondary()
{
   if (weapons.isEmpty())
      return;

   int n = secondary + 1;
   while (n != secondary) {
      if (n >= weapons.size())
         n = 0;

      if (weapons[n]->IsMissile())
         break;

      n++;
   }

   secondary = n;

   // automatically switch sensors to appropriate mode:
   if (IsAirborne()) {
      Weapon* missile = GetSecondary();
      if (missile && missile->CanTarget(Ship::GROUND_UNITS))
         SetSensorMode(Sensor::GM);
      else if (sensor && sensor->GetMode() == Sensor::GM)
         SetSensorMode(Sensor::STD);
   }
}

int
Ship::GetMissileEta(int index) const
{
   if (index >= 0 && index < 4)
      return missile_eta[index];

   return 0;
}

void
Ship::SetMissileEta(int id, int eta)
{
   int index = -1;

   // are we tracking this missile's eta?
   for (int i = 0; i < 4; i++)
      if (id == missile_id[i])
         index = i;

   // if not, can we find an open slot to track it in?
   if (index < 0) {
      for (int i = 0; i < 4 && index < 0; i++) {
         if (missile_eta[i] == 0) {
            index = i;
            missile_id[i] = id;
         }
      }
   }

   // track the eta:
   if (index >= 0 && index < 4) {
      if (eta > 3599)
         eta = 3599;

      missile_eta[index] = (BYTE) eta;
   }
}

// +--------------------------------------------------------------------+

void
Ship::DoEMCON()
{
   ListIter<System> iter = systems;
   while (++iter) {
      System* s = iter.value();
      s->DoEMCON(emcon);
   }

   old_emcon = emcon;
}

// +--------------------------------------------------------------------+

double
Ship::Thrust(double seconds) const
{
   double total_thrust = 0;
   
   if (main_drive) {
      // velocity limiter:
      Point  H             = Heading();
      Point  V             = Velocity();
      double vmag          = V.Normalize();
      double eff_throttle  = throttle;
      double thrust_factor = 1;
      double vfwd          = H * V;
      bool   aug_on        = main_drive->IsAugmenterOn();

      if (vmag > vlimit && vfwd > 0) {
         double vmax = vlimit;
         if (aug_on)
            vmax *= 1.5;

         vfwd = 0.5 * vfwd + 0.5;

         // reduce drive efficiency at high fwd speed:
         thrust_factor = (vfwd * pow(vmax,3) / pow(vmag,3)) + (1-vfwd);
      }

      if (flcs)
         eff_throttle = flcs->Throttle();

      // square-law throttle curve to increase sensitivity
      // at lower throttle settings:
      if (flight_model > 1) {
         eff_throttle /= 100;
         eff_throttle *= eff_throttle;
         eff_throttle *= 100;
      }

      main_drive->SetThrottle(eff_throttle, augmenter);
      total_thrust += thrust_factor * main_drive->Thrust(seconds);

      if (aug_on && shake < 1.5)
         ((Ship*) this)->shake = 1.5f;
   }

   return total_thrust;
}

// +--------------------------------------------------------------------+

void
Ship::CycleFLCSMode()
{
   switch (flcs_mode) {
   case FLCS_MANUAL: SetFLCSMode(FLCS_HELM);    break;
   case FLCS_AUTO:   SetFLCSMode(FLCS_MANUAL);  break;
   case FLCS_HELM:   SetFLCSMode(FLCS_AUTO);    break;

   default:
      if (IsStarship())
         flcs_mode = (BYTE) FLCS_HELM;
      else
         flcs_mode = (BYTE) FLCS_AUTO;
      break;
   }

   // reset helm heading to compass heading when switching
   // back to helm mode from manual mode:

   if (flcs_mode == FLCS_HELM) {
      if (IsStarship()) {
         SetHelmHeading(CompassHeading());
         SetHelmPitch(CompassPitch());
      }
      else {
         flcs_mode = (BYTE) FLCS_AUTO;
      }
   }
}

void
Ship::SetFLCSMode(int mode)
{
   flcs_mode = (BYTE) mode;
   
   if (IsAirborne())
      flcs_mode = (BYTE) FLCS_MANUAL;

   if (dir && dir->Type() < SteerAI::SEEKER) {
      switch (flcs_mode) {
      case FLCS_MANUAL:    director_info = Game::GetText("flcs.manual"); break;
      case FLCS_AUTO:      director_info = Game::GetText("flcs.auto");   break;
      case FLCS_HELM:      director_info = Game::GetText("flcs.helm");   break;
      default:             director_info = Game::GetText("flcs.fault");  break;
      }

      if (!flcs || !flcs->IsPowerOn())
         director_info = Game::GetText("flcs.offline");

      else if (IsAirborne())
         director_info = Game::GetText("flcs.atmospheric");
   }

   if (flcs)
      flcs->SetMode(mode);
}

int
Ship::GetFLCSMode() const
{
   return (int) flcs_mode;
}

void
Ship::SetTransX(double t)
{
   float limit = design->trans_x;

   if (thruster)
      limit = (float) thruster->TransXLimit();

   trans_x = (float) t;

   if (trans_x) {
      if (trans_x > limit)
         trans_x = limit;
      else if (trans_x < -limit)
         trans_x = -limit;

      // reduce thruster efficiency at high fwd speed:
      double vfwd = cam.vrt() * Velocity();
      double vmag = fabs(vfwd);
      if (vmag > vlimit) {
         if (trans_x > 0 && vfwd > 0 || trans_x < 0 && vfwd < 0)
            trans_x *= (float) (pow(vlimit,4) / pow(vmag,4));
      }
   }
}

void
Ship::SetTransY(double t)
{
   float limit = design->trans_y;

   if (thruster)
      limit = (float) thruster->TransYLimit();

   trans_y = (float) t;

   if (trans_y) {
      double vmag = Velocity().length();

      if (trans_y > limit)
         trans_y = limit;
      else if (trans_y < -limit)
         trans_y = -limit;

      // reduce thruster efficiency at high fwd speed:
      if (vmag > vlimit) {
         double vfwd = cam.vpn() * Velocity();

         if (trans_y > 0 && vfwd > 0 || trans_y < 0 && vfwd < 0)
            trans_y *= (float) (pow(vlimit,4) / pow(vmag,4));
      }
   }
}

void
Ship::SetTransZ(double t)
{
   float limit = design->trans_z;

   if (thruster)
      limit = (float) thruster->TransZLimit();

   trans_z = (float) t;

   if (trans_z) {

      if (trans_z > limit)
         trans_z = limit;
      else if (trans_z < -limit)
         trans_z = -limit;

      // reduce thruster efficiency at high fwd speed:
      double vfwd = cam.vup() * Velocity();
      double vmag = fabs(vfwd);
      if (vmag > vlimit) {
         if (trans_z > 0 && vfwd > 0 || trans_z < 0 && vfwd < 0)
            trans_z *= (float) (pow(vlimit,4) / pow(vmag,4));
      }
   }
}

void
Ship::ExecFLCSFrame()
{
   if (flcs)
      flcs->ExecSubFrame();
}

// +--------------------------------------------------------------------+

void
Ship::SetHelmHeading(double h)
{
   while (h < 0)
      h += 2*PI;

   while (h >= 2*PI)
      h -= 2*PI;

   helm_heading = (float) h;
}

void
Ship::SetHelmPitch(double p)
{
   const double PITCH_LIMIT = 80 * DEGREES;

   if (p < -PITCH_LIMIT)
      p = -PITCH_LIMIT;

   else if (p > PITCH_LIMIT)
      p =  PITCH_LIMIT;

   helm_pitch = (float) p;
}

void
Ship::ApplyHelmYaw(double y)
{
   // rotate compass into helm-relative orientation:
   double compass = CompassHeading() - helm_heading;
   double turn    = y * PI/4;

   if (compass > PI)
      compass -= 2*PI;
   else if (compass < -PI)
      compass += 2*PI;

   // if requested turn is more than 170, reject it:
   if (fabs(compass + turn) > 170*DEGREES)
      return;

   SetHelmHeading(helm_heading + turn);
}

void
Ship::ApplyHelmPitch(double p)
{
   SetHelmPitch(helm_pitch - p * PI/4);
}

void
Ship::ApplyPitch(double p)
{
   if (flight_model == 0) { // standard flight model
      if (IsAirborne())
         p *= 0.5;

      // command for pitch up is negative
      if (p < 0) {
         if (alpha > PI/6) {
            p *= 0.05;
         }
         else if (g_force > 12.0) {
            double limit = 0.5 - (g_force - 12.0)/10.0;

            if (limit < 0)
               p = 0;
            else
               p *= limit;
         }
      }

      // command for pitch down is positive
      else if (p > 0) {
         if (alpha < -PI/8) {
            p *= 0.05;
         }
         else if (g_force < -3) {
            p *= 0.1;
         }
      }
   }

   Physical::ApplyPitch(p);
}

// +--------------------------------------------------------------------+

bool
Ship::FireWeapon(int n)
{
   bool fired = false;

   if (n >= 0 && !CheckFire()) {
      if (n < 4)
         trigger[n] = true;

      if (n < weapons.size()) {
         weapons[n]->SetTrigger(true);
         fired = weapons[n]->GetTrigger();
      }
   }

   if (!fired && sim->GetPlayerShip() == this)
      Button::PlaySound(Button::SND_REJECT);

   return fired;
}

bool
Ship::FireDecoy()
{
   Shot* drone = 0;

   if (decoy && !CheckFire()) {
      drone = decoy->Fire();

      if (drone) {
         Observe(drone);
         decoy_list.append(drone);
      }
   }

   if (sim->GetPlayerShip() == this) {
      if (NetGame::IsNetGame()) {
         if (decoy && decoy->Ammo() < 1)
            Button::PlaySound(Button::SND_REJECT);
      }

      else if (!drone) {
         Button::PlaySound(Button::SND_REJECT);
      }
   }

   return drone != 0;
}

void
Ship::AddActiveDecoy(Drone* drone)
{
   if (drone) {
      Observe(drone);
      decoy_list.append(drone);
   }
}

Weapon*
Ship::GetPrimary() const
{
   if (weapons.size() > primary)
      return weapons[primary]->GetSelected();
   return 0;
}

Weapon*
Ship::GetSecondary() const
{
   if (weapons.size() > secondary)
      return weapons[secondary]->GetSelected();
   return 0;
}

Weapon*
Ship::GetWeaponByIndex(int n)
{
   for (int i = 0; i < weapons.size(); i++) {
      WeaponGroup* g = weapons[i];

      List<Weapon>& wlist = g->GetWeapons();
      for (int j = 0; j < wlist.size(); j++) {
         Weapon* w = wlist[j];

         if (w->GetIndex() == n) {
            return w;
         }
      }
   }

   return 0;
}

WeaponGroup*
Ship::GetPrimaryGroup() const
{
   if (weapons.size() > primary)
      return weapons[primary];
   return 0;
}

WeaponGroup*
Ship::GetSecondaryGroup() const
{
   if (weapons.size() > secondary)
      return weapons[secondary];
   return 0;
}

WeaponDesign*
Ship::GetPrimaryDesign() const
{
   if (weapons.size() > primary)
      return (WeaponDesign*) weapons[primary]->GetSelected()->Design();
   return 0;   
}

WeaponDesign*
Ship::GetSecondaryDesign() const
{
   if (weapons.size() > secondary)
      return (WeaponDesign*) weapons[secondary]->GetSelected()->Design();
   return 0;   
}

Weapon*
Ship::GetDecoy() const
{
   return decoy;
}

List<Shot>& 
Ship::GetActiveDecoys()
{
   return decoy_list;
}

List<Shot>& 
Ship::GetThreatList()
{
   return threat_list;
}

void
Ship::AddThreat(Shot* s)
{
   if (!threat_list.contains(s)) {
      Observe(s);
      threat_list.append(s);
   }
}

void
Ship::DropThreat(Shot* s)
{
   if (threat_list.contains(s)) {
      threat_list.remove(s);
   }
}

bool
Ship::GetTrigger(int i) const
{
   if (i >= 0) {
      if (i < 4)
         return trigger[i];

      else if (i < weapons.size())
         return weapons[i]->GetTrigger();
   }

   return false;
}

void
Ship::SetTrigger(int i)
{
   if (i >= 0 && !CheckFire()) {
      if (i < 4)
         trigger[i] = true;

      if (i < weapons.size())
         weapons[i]->SetTrigger();
   }
}

// +--------------------------------------------------------------------+

void
Ship::SetSensorMode(int mode)
{
   if (sensor)
      sensor->SetMode((Sensor::Mode) mode);
}

int
Ship::GetSensorMode() const
{
   if (sensor)
      return (int) sensor->GetMode();

   return 0;
}

// +--------------------------------------------------------------------+

bool
Ship::IsTracking(SimObject* tgt)
{
   if (tgt && sensor)
      return sensor->IsTracking(tgt);

   return false;
}

// +--------------------------------------------------------------------+

void
Ship::LockTarget(int type, bool closest, bool hostile)
{
   if (sensor)
      SetTarget(sensor->LockTarget(type, closest, hostile));
}

// +--------------------------------------------------------------------+

void
Ship::LockTarget(SimObject* candidate)
{
   if (sensor)
      SetTarget(sensor->LockTarget(candidate));
   else
      SetTarget(candidate);
}

// +--------------------------------------------------------------------+

double
Ship::InflictDamage(double damage, Shot* shot, int hit_type, Point impact)
{
   double damage_applied = 0;

   if (Game::Paused() || IsNetObserver() || IsInvulnerable())
      return damage_applied;

   if (Integrity() == 0) // already dead?
      return damage_applied;

   const double   MAX_SHAKE   = 7;
   double         hull_damage = damage;
   bool           hit_shield  = (hit_type & HIT_SHIELD) != 0;
   bool           hit_hull    = (hit_type & HIT_HULL)   != 0;
   bool           hit_turret  = (hit_type & HIT_TURRET) != 0;

   if (impact == Point(0,0,0))
      impact = Location();

   if (hit_shield && ShieldStrength() > 0) {
      hull_damage = shield->DeflectDamage(shot, damage);

      if (shot) {
         if (shot->IsBeam()) {
            if (design->beam_hit_sound_resource) {
               if (Game::RealTime() - last_beam_time > 400) {
                  Sound* s = design->beam_hit_sound_resource->Duplicate();
                  s->SetLocation(impact);
                  s->SetVolume(AudioConfig::EfxVolume());
                  s->Play();

                  last_beam_time = Game::RealTime();
               }
            }
         }

         else {
            if (design->bolt_hit_sound_resource) {
               if (Game::RealTime() - last_bolt_time > 400) {
                  Sound* s = design->bolt_hit_sound_resource->Duplicate();
                  s->SetLocation(impact);
                  s->SetVolume(AudioConfig::EfxVolume());
                  s->Play();

                  last_bolt_time = Game::RealTime();
               }
            }
         }
      }
   }

   if (hit_hull) {
      hull_damage = InflictSystemDamage(hull_damage, shot, impact);

      int damage_type = WeaponDesign::DMG_NORMAL;

      if (shot && shot->Design())
         damage_type = shot->Design()->damage_type;

      if (damage_type == WeaponDesign::DMG_NORMAL) {
         damage_applied = hull_damage;
         Physical::InflictDamage(damage_applied, 0);
         NetUtil::SendObjDamage(this, damage_applied, shot);
      }
   }

   else if (hit_turret) {
      hull_damage = InflictSystemDamage(hull_damage, shot, impact) * 0.3;

      int damage_type = WeaponDesign::DMG_NORMAL;

      if (shot && shot->Design())
         damage_type = shot->Design()->damage_type;

      if (damage_type == WeaponDesign::DMG_NORMAL) {
         damage_applied = hull_damage;
         Physical::InflictDamage(damage_applied, 0);
         NetUtil::SendObjDamage(this, damage_applied, shot);
      }
   }

   // shake by percentage of maximum damage
   double newshake = 50 * damage/design->integrity;

   if (shake < MAX_SHAKE)  shake += (float) newshake;
   if (shake > MAX_SHAKE)  shake  = (float) MAX_SHAKE;

   // start fires as needed:
   if ((IsStarship() || IsGroundUnit() || RandomChance(1,3)) && hit_hull && damage_applied > 0) {
      int old_integrity = (int) ((integrity + damage_applied)/design->integrity * 10);
      int new_integrity = (int) ((integrity                 )/design->integrity * 10);

      if (new_integrity < 5 && new_integrity < old_integrity) {
         // need accurate hull impact for starships,
         if (rep) {
            Point  detonation = impact*2 - Location();
            Point  direction  = Location() - detonation;
            double distance   = direction.Normalize() * 3;
            rep->CheckRayIntersection(detonation, direction, distance, impact);

            // pull fire back into hull a bit:
            direction = Location() - impact;
            impact += direction * 0.2;

            float scale = (float) design->scale;

            if (IsDropship())
               sim->CreateExplosion(impact, Velocity(), Explosion::SMOKE_TRAIL, 0.01f * scale, 0.5f * scale, region, this);
            else
               sim->CreateExplosion(impact, Velocity(), Explosion::HULL_FIRE,   0.10f * scale,        scale, region, this);
         }
      }
   }

   return damage_applied;
}

double
Ship::InflictSystemDamage(double damage, Shot* shot, Point impact)
{
   if (IsNetObserver())
      return 0;

   // find the system that is closest to the impact point:
   System* system       = 0;
   double  distance     = 1e6;
   double  blast_radius = 0;
   int     dmg_type     = 0;
   
   if (shot)
      dmg_type = shot->Design()->damage_type;

   bool    dmg_normal   = dmg_type == WeaponDesign::DMG_NORMAL;
   bool    dmg_power    = dmg_type == WeaponDesign::DMG_POWER;
   bool    dmg_emp      = dmg_type == WeaponDesign::DMG_EMP;
   double  to_level     = 0;

   if (dmg_power) {
      to_level = 1 - damage / 1e4;

      if (to_level < 0)
         to_level = 0;
   }

   // damage caused by weapons applies to closest system:
   if (shot) {
      if (shot->IsMissile())
         blast_radius   = 300;

      ListIter<System> iter = systems;
      while (++iter) {
         System* candidate = iter.value();
         double  sysrad    = candidate->Radius();

         if (dmg_power)
            candidate->DrainPower(to_level);

         if (sysrad > 0 || dmg_emp && candidate->IsPowerCritical()) {
            double test_distance = (impact - candidate->MountLocation()).length();

            if ((test_distance-blast_radius) < sysrad || dmg_emp && candidate->IsPowerCritical()) {
               if (test_distance < distance) {
                  system   = candidate;
                  distance = test_distance;
               }
            }
         }
      }  

      // if a system was in range of the blast, assess the damage:
      if (system) {
         double hull_damage = damage * system->HullProtection();
         double sys_damage  = damage - hull_damage;
         double avail       = system->Availability();

         if (dmg_normal || system->IsPowerCritical() && dmg_emp) {
            system->ApplyDamage(sys_damage);
            NetUtil::SendSysDamage(this, system, sys_damage);

            master_caution = true;

            if (dmg_normal) {
               if (sys_damage < 100)
                  damage -= sys_damage;
               else
                  damage -= 100;
            }

            if (system->GetExplosionType() && (avail - system->Availability()) >= 50) {
               float scale = design->explosion_scale;
               if (scale <= 0)
                  scale = design->scale;

               sim->CreateExplosion(system->MountLocation(), Velocity() * 0.7f, system->GetExplosionType(), 0.2f * scale, scale, region, this, system);
            }
         }
      }
   }

   // damage caused by collision applies to all systems:
   else {
      // ignore incidental bumps:
      if (damage < 100)
         return damage;

      ListIter<System> iter = systems;
      while (++iter) {
         System* sys = iter.value();

         if (rand() > 24000) {
            double base_damage = 33.0 + rand()/1000.0;
            double sys_damage  = base_damage * (1.0 - sys->HullProtection());
            sys->ApplyDamage(sys_damage);
            NetUtil::SendSysDamage(this, system, sys_damage);
            damage -= sys_damage;

            master_caution = true;
         }
      }

      // just in case this ship has lots of systems...
      if (damage < 0)
         damage = 0;
   }

   // return damage remaining
   return damage;
}

// +--------------------------------------------------------------------+

int
Ship::ShieldStrength() const
{
   if (!shield) return 0;

   return (int) shield->ShieldLevel();
}

int
Ship::HullStrength() const
{
   if (design)
      return (int) (Integrity() / design->integrity * 100);

   return 10;
}

// +--------------------------------------------------------------------+

System*
Ship::GetSystem(int sys_id)
{
   System* s = 0;

   if (sys_id >= 0) {
      if (sys_id < systems.size()) {
         s = systems[sys_id];
         if (s->GetID() == sys_id)
            return s;
      }

      ListIter<System> iter = systems;
      while (++iter) {
         s = iter.value();

         if (s->GetID() == sys_id)
            return s;
      }
   }

   return 0;
}

// +--------------------------------------------------------------------+

void
Ship::RepairSystem(System* sys)
{
   if (!repair_queue.contains(sys)) {
      repair_queue.append(sys);
      sys->Repair();
   }
}

// +--------------------------------------------------------------------+

void
Ship::IncreaseRepairPriority(int task_index)
{
   if (task_index > 0 && task_index < repair_queue.size()) {
      System* task1 = repair_queue.at(task_index-1);
      System* task2 = repair_queue.at(task_index);

      repair_queue.at(task_index-1) = task2;
      repair_queue.at(task_index)   = task1;
   }
}

void
Ship::DecreaseRepairPriority(int task_index)
{
   if (task_index >= 0 && task_index < repair_queue.size()-1) {
      System* task1 = repair_queue.at(task_index);
      System* task2 = repair_queue.at(task_index+1);

      repair_queue.at(task_index)   = task2;
      repair_queue.at(task_index+1) = task1;
   }
}

// +--------------------------------------------------------------------+

void
Ship::ExecMaintFrame(double seconds)
{
   // is it already too late?
   if (life == 0 || integrity < 1) return;

   const  DWORD REPAIR_FREQUENCY  = 5000;  // once every five seconds
   static DWORD last_repair_frame = 0;     // one ship per game frame

   if (auto_repair && 
       Game::GameTime() - last_repair_time > REPAIR_FREQUENCY &&
       last_repair_frame != Game::Frame()) {

      last_repair_time  = Game::GameTime();
      last_repair_frame = Game::Frame();

      ListIter<System> iter = systems;
      while (++iter) {
         System* sys = iter.value();

         if (sys->Status() != System::NOMINAL) {
            bool started_repairs = false;

            // emergency power routing:
            if (sys->Type() == System::POWER_SOURCE && sys->Availability() < 33) {
               PowerSource* src = (PowerSource*) sys;
               PowerSource* dst = 0;

               for (int i = 0; i < reactors.size(); i++) {
                  PowerSource* pwr = reactors[i];

                  if (pwr != src && pwr->Availability() > src->Availability()) {
                     if (!dst || 
                         (pwr->Availability() > dst->Availability() &&
                          pwr->Charge()       > dst->Charge()))
                        dst = pwr;
                  }
               }

               if (dst) {
                  while (src->Clients().size() > 0) {
                     System* s = src->Clients().at(0);
                     src->RemoveClient(s);
                     dst->AddClient(s);
                  }
               }
            }

            ListIter<Component> comp = sys->GetComponents();
            while (++comp) {
               Component* c = comp.value();

               if (c->Status() < Component::NOMINAL && c->Availability() < 75) {
                  if (c->SpareCount()           &&
                      c->ReplaceTime() <= 300   &&
                        (c->Availability() < 50 || 
                         c->ReplaceTime()  < c->RepairTime())) {

                     c->Replace();
                     started_repairs = true;
                  }

                  else if (c->Availability() >= 50 || c->NumJerried() < 5) {
                     c->Repair();
                     started_repairs = true;
                  }
               }
            }

            if (started_repairs)
               RepairSystem(sys);
         }
      }
   }

   if (repair_queue.size() > 0 && RepairTeams() > 0) {
      int team = 0;
      ListIter<System> iter = repair_queue;
      while (++iter && team < RepairTeams()) {
         System* sys = iter.value();

         sys->ExecMaintFrame(seconds * RepairSpeed());
         team++;

         if (sys->Status() != System::MAINT) {
            iter.removeItem();

            // emergency power routing (restore):
            if (sys->Type() == System::POWER_SOURCE && 
                sys->Status() == System::NOMINAL) {
               PowerSource* src = (PowerSource*) sys;
               int          isrc = reactors.index(src);

               for (int i = 0; i < reactors.size(); i++) {
                  PowerSource* pwr = reactors[i];

                  if (pwr != src) {
                     List<System> xfer;

                     for (int j = 0; j < pwr->Clients().size(); i++) {
                        System* s = pwr->Clients().at(j);

                        if (s->GetSourceIndex() == isrc) {
                           xfer.append(s);
                        }
                     }

                     for (int j = 0; j < xfer.size(); i++) {
                        System* s = xfer.at(j);
                        pwr->RemoveClient(s);
                        src->AddClient(s);
                     }
                  }
               }
            }
         }
      }
   }
}

// +--------------------------------------------------------------------+

void
Ship::SetNetworkControl(Director* net)
{
   net_control = net;

   delete dir;
   dir = 0;

   if (!net_control && GetIFF() < 100) {
      if (IsStatic())
         dir = 0;
      else if (IsStarship())
         dir = SteerAI::Create(this, SteerAI::STARSHIP);
      else
         dir = SteerAI::Create(this, SteerAI::FIGHTER);
   }
}

void
Ship::SetControls(MotionController* m)
{
   if (IsDropping() || IsAttaining()) {
      if (dir && dir->Type() != DropShipAI::DIR_TYPE) {
         delete dir;
         dir = new(__FILE__,__LINE__) DropShipAI(this);
      }

      return;
   }

   else if (IsSkipping()) {
      if (navsys && sim->GetPlayerShip() == this)
         navsys->EngageAutoNav();
   }

   else if (IsDying() || IsDead()) {
      if (dir) {
         delete dir;
         dir = 0;
      }

      if (navsys && navsys->AutoNavEngaged()) {
         navsys->DisengageAutoNav();
      }

      return;
   }

   else if (life == 0) {
      if (dir || navsys) {
         ::Print("Warning: dying ship '%' still has not been destroyed!\n", name);
         delete dir;
         dir = 0;

         if (navsys && navsys->AutoNavEngaged())
            navsys->DisengageAutoNav();
      }

      return;
   }

   if (navsys && navsys->AutoNavEngaged()) {
      NavAI* nav = 0;

      if (dir) {
         if (dir->Type() != NavAI::DIR_TYPE) {
            delete dir;
            dir = 0;
         }
         else {
            nav = (NavAI*) dir;
         }
      }

      if (!nav) {
         nav = new(__FILE__,__LINE__) NavAI(this);
         dir = nav;
         return;
      }
      
      else if (!nav->Complete()) {
         return;
      }
   }

   if (dir) {
      delete dir;
      dir = 0;
   }

   if (m) {
      Keyboard::FlushKeys();
      m->Acquire();
      dir = new(__FILE__,__LINE__) ShipCtrl(this, m);
      director_info = Game::GetText("flcs.auto");
   }
   else if (GetIFF() < 100) {
      if (IsStatic())
         dir = SteerAI::Create(this, SteerAI::GROUND);

      else if (IsStarship() && !IsAirborne())
         dir = SteerAI::Create(this, SteerAI::STARSHIP);

      else
         dir = SteerAI::Create(this, SteerAI::FIGHTER);
   }
}

// +--------------------------------------------------------------------+

Color
Ship::IFFColor(int iff)
{
   Color c;

   switch (iff) {
   case 0:  // NEUTRAL, NON-COMBAT
      c = Color(192,192,192);
      break;

   case 1:  // TERELLIAN ALLIANCE
      c = Color(70,70,220);
      break;
   
   case 2:  // MARAKAN HEGEMONY
      c = Color(220,20,20);
      break;

   case 3:  // BROTHERHOOD OF IRON
      c = Color(200,180,20);
      break;

   case 4:  // ZOLON EMPIRE
      c = Color(20,200,20);
      break;

   case 5:
      c = Color(128, 0, 128);
      break;

   case 6:
      c = Color(40,192,192);
      break;

   default:
      c = Color(128,128,128);
      break;
   }

   return c;   
}

Color
Ship::MarkerColor() const
{
   return IFFColor(IFF_code);
}

// +--------------------------------------------------------------------+

void
Ship::SetIFF(int iff)
{
   IFF_code = iff;

   if (hangar)
      hangar->SetAllIFF(iff);

   DropTarget();

   if (dir && dir->Type() >= 1000) {
      SteerAI* ai = (SteerAI*) dir;
      ai->DropTarget();
   }
}

// +--------------------------------------------------------------------+

void
Ship::SetRogue(bool r)
{
   bool rogue = IsRogue();
   
   ff_count = r ? 1000 : 0;

   if (!rogue && IsRogue()) {
      Print("Ship '%s' has been made rogue\n", Name());
   }
   else if (rogue && !IsRogue()) {
      Print("Ship '%s' is no longer rogue\n", Name());
   }
}

void
Ship::SetFriendlyFire(int f)
{
   bool rogue = IsRogue();

   ff_count = f;

   if (!rogue && IsRogue()) {
      Print("Ship '%s' has been made rogue with ff_count = %d\n", Name(), ff_count);
   }
   else if (rogue && !IsRogue()) {
      Print("Ship '%s' is no longer rogue\n", Name());
   }
}

void
Ship::IncFriendlyFire(int f)
{
   if (f > 0) {
      bool rogue = IsRogue();

      ff_count += f;

      if (!rogue && IsRogue()) {
         Print("Ship '%s' has been made rogue with ff_count = %d\n", Name(), ff_count);
      }
   }
}

// +--------------------------------------------------------------------+

void
Ship::SetEMCON(int e, bool from_net)
{
   if (e < 1)        emcon = 1;
   else if (e > 3)   emcon = 3;
   else              emcon = (BYTE) e;

   if (emcon != old_emcon && !from_net && NetGame::GetInstance())
      NetUtil::SendObjEmcon(this);
}

double
Ship::PCS() const
{
   double e_factor = design->e_factor[emcon-1];

   if (IsAirborne() && !IsGroundUnit()) {
      if (AltitudeAGL() < 40)
         return 0;

      if (AltitudeAGL() < 200) {
         double clutter = AltitudeAGL() / 200;
         return clutter * e_factor;
      }
   }

   return e_factor * pcs;
}

double
Ship::ACS() const
{
   if (IsAirborne() && !IsGroundUnit()) {
      if (AltitudeAGL() < 40)
         return 0;

      if (AltitudeAGL() < 200) {
         double clutter = AltitudeAGL() / 200;
         return clutter * acs;
      }
   }

   return acs;
}

DWORD
Ship::MissionClock() const
{
   if (launch_time > 0)
      return Game::GameTime() + 1 - launch_time;

   return 0;
}

// +----------------------------------------------------------------------+

Instruction*
Ship::GetRadioOrders() const
{
   return radio_orders;
}

void
Ship::ClearRadioOrders()
{
   if (radio_orders) {
      radio_orders->SetAction(0);
      radio_orders->ClearTarget();
      radio_orders->SetLocation(Point());
   }
}

void
Ship::HandleRadioMessage(RadioMessage* msg)
{
   if (!msg) return;

   static RadioHandler rh;

   if (rh.ProcessMessage(msg, this))
      rh.AcknowledgeMessage(msg, this);
}

// +----------------------------------------------------------------------+

int
Ship::Value() const
{
   return Value(design->type);
}

// +----------------------------------------------------------------------+

int
Ship::Value(int type)
{
   int value = 0;

   switch (type) {
   case DRONE:       value =   10; break;
   case FIGHTER:     value =   20; break;
   case ATTACK:      value =   40; break;
   case LCA:         value =   50; break;

   case COURIER:     value =  100; break;
   case CARGO:       value =  100; break;
   case CORVETTE:    value =  100; break;
   case FREIGHTER:   value =  250; break;
   case FRIGATE:     value =  200; break;
   case DESTROYER:   value =  500; break;
   case CRUISER:     value =  800; break;
   case BATTLESHIP:  value = 1000; break;
   case CARRIER:     value = 1500; break;
   case DREADNAUGHT: value = 1500; break;

   case STATION:     value = 2500; break;
   case FARCASTER:   value = 5000; break;

   case MINE:        value =   20; break;
   case COMSAT:      value =  200; break;
   case DEFSAT:      value =  300; break;
   case SWACS:       value =  500; break;

   case BUILDING:    value =  100; break;
   case FACTORY:     value =  250; break;
   case SAM:         value =  100; break;
   case EWR:         value =  200; break;
   case C3I:         value =  500; break;
   case STARBASE:    value = 2000; break;

   default:          value =  100; break;
   }

   return value;
}

// +----------------------------------------------------------------------+

double
Ship::AIValue() const
{
   int    i     = 0;
   double value = 0;

   for (i = 0; i < reactors.size(); i++) {
      const PowerSource* r = reactors[i];
      value += r->Value();
   }

   for (i = 0; i < drives.size(); i++) {
      const Drive* d = drives[i];
      value += d->Value();
   }

   for (i = 0; i < weapons.size(); i++) {
      const WeaponGroup* w = weapons[i];
      value += w->Value();
   }

   return value;
}
