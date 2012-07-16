/*  Project Starshatter 5.0
	Destroyer Studios LLC
	Copyright (C) 1997-2007. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         Sim.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Simulation Universe and Region classes
*/

#include "MemDebug.h"
#include "Sim.h"
#include "SimEvent.h"
#include "SimObject.h"
#include "Starshatter.h"
#include "StarSystem.h"
#include "Contact.h"
#include "Ship.h"
#include "ShipDesign.h"
#include "Element.h"
#include "Instruction.h"
#include "RadioTraffic.h"
#include "Shot.h"
#include "Drone.h"
#include "Explosion.h"
#include "Debris.h"
#include "Asteroid.h"
#include "Drive.h"
#include "QuantumDrive.h"
#include "Sensor.h"
#include "NavLight.h"
#include "Shield.h"
#include "Weapon.h"
#include "WeaponGroup.h"
#include "Hangar.h"
#include "FlightDeck.h"
#include "Sky.h"
#include "Grid.h"
#include "MFD.h"
#include "AudioConfig.h"
#include "Mission.h"
#include "MissionEvent.h"
#include "CameraDirector.h"
#include "MusicDirector.h"
#include "Combatant.h"
#include "CombatGroup.h"
#include "CombatUnit.h"
#include "HUDView.h"
#include "SeekerAI.h"
#include "ShipAI.h"
#include "Power.h"
#include "Callsign.h"
#include "GameScreen.h"
#include "Terrain.h"
#include "TerrainPatch.h"

#include "NetGame.h"
#include "NetClientConfig.h"
#include "NetServerConfig.h"
#include "NetPlayer.h"
#include "NetUtil.h"
#include "NetData.h"

#include "Game.h"
#include "Sound.h"
#include "Bolt.h"
#include "Solid.h"
#include "Sprite.h"
#include "Light.h"
#include "Bitmap.h"
#include "DataLoader.h"
#include "ParseUtil.h"
#include "MouseController.h"
#include "Player.h"
#include "Random.h"
#include "Video.h"

const char* FormatGameTime();

// +--------------------------------------------------------------------+

class SimHyper
{
public:
	SimHyper(Ship* o, SimRegion* r, const Point& l, int t, bool h, Ship* fc1, Ship* fc2)
	: ship(o), rgn(r), loc(l), type(t), hyperdrive(h), fc_src(fc1), fc_dst(fc2)  { }

	Ship*       ship;
	SimRegion*  rgn;
	Point       loc;
	int         type;
	bool        hyperdrive;
	Ship*       fc_src;
	Ship*       fc_dst;
};

// +--------------------------------------------------------------------+

class SimSplash
{
public:
	SimSplash(SimRegion* r, const Point& l, double d, double n)
	: rgn(r), loc(l), damage(d), range(n),
	owner_name("Collateral Damage"), missile(false) { }

	Text        owner_name;
	Point       loc;
	double      damage;
	double      range;
	SimRegion*  rgn;
	bool        missile;
};

// +--------------------------------------------------------------------+

static bool first_frame = true;
Sim*        Sim::sim    = 0;

Sim::Sim(MotionController* c)
: ctrl(c), test_mode(false), grid_shown(false), dust(0),
star_system(0), active_region(0), mission(0), netgame(0),
start_time(0)
{
	Drive::Initialize();
	Explosion::Initialize();
	FlightDeck::Initialize();
	NavLight::Initialize();
	Shot::Initialize();
	MFD::Initialize();
	Asteroid::Initialize();

	if (!sim)
	sim = this;

	cam_dir = CameraDirector::GetInstance();
}

Sim::~Sim()
{
	UnloadMission();

	Shot::Close();
	FlightDeck::Close();
	NavLight::Close();
	Token::close();
	Asteroid::Close();

	if (sim == this)
	sim = 0;
}

// +--------------------------------------------------------------------+

void
Sim::CommitMission()
{
	for (int i = 0; i < regions.size(); i++)
	regions[i]->CommitMission();

	if (ShipStats::NumStats() > 0) {
		Print("\n\nFINAL SCORE '%s'\n", (const char*) mission->Name());
		Print("Name              Kill1  Kill2  Died   Colls  Points  Cmd Pts\n");
		Print("----------------  -----  -----  -----  -----  ------  ------\n");

		int tk1 = 0;
		int tk2 = 0;
		int td  = 0;
		int tc  = 0;

		for (int i = 0; i < ShipStats::NumStats(); i++) {
			ShipStats* s = ShipStats::GetStats(i);
			s->Summarize();

			Print("%-16s  %5d  %5d  %5d  %5d  %6d  %6d\n",
			s->GetName(),
			s->GetGunKills(),
			s->GetMissileKills(),
			s->GetDeaths(),
			s->GetColls(),
			s->GetPoints(),
			s->GetCommandPoints());

			tk1 += s->GetGunKills();
			tk2 += s->GetMissileKills();
			td  += s->GetDeaths();
			tc  += s->GetColls();

			CombatGroup* group = s->GetCombatGroup();

			if (group) {
				Combatant* c = group->GetCombatant();

				if (c)
				c->AddScore(s->GetPoints());

				if (s->GetElementIndex() == 1)
				group->SetSorties(group->Sorties() + 1);

				group->SetKills(group->Kills() + s->GetGunKills() + s->GetMissileKills());
				group->SetPoints(group->Points() + s->GetPoints());
			}

			if (s->IsPlayer()) {
				Player* p = Player::GetCurrentPlayer();
				p->ProcessStats(s, start_time);

				if (mission && mission->Type() == Mission::TRAINING && 
						s->GetDeaths() == 0 && s->GetColls() == 0)
				p->SetTrained(mission->Identity());

				Player::Save(); // save training state right now before we forget!
			}
		}

		Print("--------------------------------------------\n");
		Print("TOTAL             %5d  %5d  %5d  %5d\n\n", tk1, tk2, td, tc);

		ShipStats::Initialize();
	}
}

// +--------------------------------------------------------------------+

void
Sim::UnloadMission()
{
	if (netgame) {
		delete netgame;
		netgame = 0;
	}

	HUDView* hud = HUDView::GetInstance();
	if (hud)
	hud->HideAll();

	ShipStats::Initialize();

	events.destroy();
	mission_elements.destroy();
	elements.destroy();
	finished.destroy();

	if (active_region)
	active_region->Deactivate();

	if (star_system)
	star_system->Deactivate();

	if (mission) {
		mission->SetActive(false);
		mission->SetComplete(true);
	}

	regions.destroy();
	scene.Collect();

	GRAPHIC_DESTROY(dust);

	star_system   = 0;
	active_region = 0;
	mission       = 0;

	// reclaim memory used by radio traffic:
	RadioTraffic::DiscardMessages();

	// release texture memory for 2D screens:
	Starshatter* stars = Starshatter::GetInstance();
	if (stars)
	stars->InvalidateTextureCache();

	cam_dir = CameraDirector::GetInstance();
	if (cam_dir)
	cam_dir->SetShip(0);

	AudioConfig::SetTraining(false);
}

bool
Sim::IsActive() const
{
	return mission && mission->IsActive();
}

bool
Sim::IsComplete() const
{
	return mission && mission->IsComplete();
}

// +--------------------------------------------------------------------+

void
Sim::LoadMission(Mission* m, bool preload_textures)
{
	cam_dir = CameraDirector::GetInstance();

	if (!mission) {
		mission = m;
		mission->SetActive(true);

		if (preload_textures) {
			Video*         video = Game::GetVideo();
			List<Model>    all_models;
			//List<Bitmap>   all_textures;

			ListIter<MissionElement> elem_iter = mission->GetElements();
			while (++elem_iter) {
				MissionElement*   elem   = elem_iter.value();
				const ShipDesign* design = elem->GetDesign();

				if (design) {
					for (int i = 0; i < 4; i++) {
						List<Model>& models = (List<Model>&) design->models[i]; // cast-away const

						ListIter<Model> model_iter = models;
						while (++model_iter) {
							Model* model = model_iter.value();
							if (!all_models.contains(model)) {
								all_models.append(model);
								//model->GetAllTextures(all_textures);

								ListIter<Surface> surf_iter = model->GetSurfaces();
								while (++surf_iter) {
									Surface* surface = surf_iter.value();
									video->PreloadSurface(surface);
								}
							}
						}
					}
				}
			}

			/*
		if (video && all_textures.size() > 0) {
			::Print("Preloading %d textures into video texture cache\n", all_textures.size());
			ListIter<Bitmap> bmp_iter = all_textures;
			while (++bmp_iter) {
			Bitmap* bmp = bmp_iter.value();
			video->PreloadTexture(bmp);
			}
		}
		*/
		}
	}
}

void
Sim::ExecMission()
{
	cam_dir = CameraDirector::GetInstance();

	if (!mission) {
		Print("Sim::ExecMission() - No mission to execute.\n");
		return;
	}

	if (elements.size() || finished.size()) {
		Print("Sim::ExecMission(%s) mission is already executing.\n", mission->Name());
		return;
	}

	Print("\nExec Mission: '%s'\n", (const char*) mission->Name());

	if (cam_dir)
	cam_dir->Reset();

	if (mission->Stardate() > 0)
	StarSystem::SetBaseTime(mission->Stardate(), true);

	star_system = mission->GetStarSystem();
	star_system->Activate(scene);

	int dust_factor = 0;

	if (Starshatter::GetInstance())
	dust_factor = Starshatter::GetInstance()->Dust();

	if (star_system->NumDust() * dust_factor) {
		dust = new(__FILE__,__LINE__) Dust(star_system->NumDust() * 2*(dust_factor+1), dust_factor > 1);
		scene.AddGraphic(dust);
	}

	CreateRegions();
	BuildLinks();
	CreateElements();
	CopyEvents();

	if (netgame) {
		delete netgame;
		netgame = 0;
	}

	first_frame = true;
	start_time  = Game::GameTime();

	AudioConfig::SetTraining(mission->Type() == Mission::TRAINING);
}

// +--------------------------------------------------------------------+

void
Sim::CreateRegions()
{
	const char* active_region_name = 0;

	if (mission)
		active_region_name = mission->GetRegion();
	else return;

	ListIter<StarSystem> iter = mission->GetSystemList();
	while (++iter) {
		StarSystem* sys = iter.value();

		// insert objects from star system:
		ListIter<OrbitalBody> star = sys->Bodies();
		while (++star) {
			ListIter<OrbitalBody> planet = star->Satellites();
			while (++planet) {
				ListIter<OrbitalBody> moon = planet->Satellites();
				while (++moon) {
					ListIter<OrbitalRegion> rgn = moon->Regions();
					while (++rgn) {
						SimRegion* sim_region = new(__FILE__,__LINE__) SimRegion(this, rgn.value());
						regions.append(sim_region);
						if (!strcmp(active_region_name, sim_region->Name())) {
							ActivateRegion(sim_region);
						}
					}
				}
				
				ListIter<OrbitalRegion> rgn = planet->Regions();
				while (++rgn) {
					SimRegion* sim_region = new(__FILE__,__LINE__) SimRegion(this, rgn.value());
					regions.append(sim_region);
					if (!strcmp(active_region_name, sim_region->Name())) {
						ActivateRegion(sim_region);
					}
				}
			}
			
			ListIter<OrbitalRegion> rgn = star->Regions();
			while (++rgn) {
				SimRegion* sim_region = new(__FILE__,__LINE__) SimRegion(this, rgn.value());
				regions.append(sim_region);
				if (!strcmp(active_region_name, sim_region->Name())) {
					ActivateRegion(sim_region);
				}
			}
		}
	}
}

// +--------------------------------------------------------------------+

void
Sim::BuildLinks()
{
	ListIter<SimRegion> iter = regions;
	while (++iter) {
		SimRegion*     rgn = iter.value();
		OrbitalRegion* orb = rgn->GetOrbitalRegion();

		if (orb) {
			ListIter<Text> lnk_iter = orb->Links();
			while (++lnk_iter) {
				Text* t = lnk_iter.value();

				SimRegion* tgt = FindRegion(*t);

				if (tgt && !rgn->Links().contains(tgt))
				rgn->Links().append(tgt);
			}
		}
	}
}

void
Sim::CreateElements()
{
	ListIter<MissionElement> e_iter = mission->GetElements();
	while (++e_iter) {
		MissionElement* msn_elem = e_iter.value();

		// add element to a carrier?
		if (msn_elem->IsSquadron()) {
			Ship* carrier = FindShip(msn_elem->Carrier());
			if (carrier) {
				Hangar* hangar = carrier->GetHangar();

				if (hangar) {
					int* def_load = 0;

					if (msn_elem->Loadouts().size()) {
						MissionLoad* m = msn_elem->Loadouts().at(0);

						if (m->GetName().length()) {
							ShipDesign* dsn = (ShipDesign*) msn_elem->GetDesign();
							ListIter<ShipLoad> sl_iter = dsn->loadouts;
							while (++sl_iter) {
								ShipLoad* sl = sl_iter.value();

								if (m->GetName() == sl->name)
								def_load = sl->load;
							}
						}

						if (!def_load) {
							def_load = m->GetStations();
						}
					}

					hangar->CreateSquadron(msn_elem->Name(), msn_elem->GetCombatGroup(),
					msn_elem->GetDesign(), msn_elem->Count(),
					msn_elem->GetIFF(),
					def_load, msn_elem->MaintCount(), msn_elem->DeadCount());

					Element* element = CreateElement(msn_elem->Name(),
					msn_elem->GetIFF(),
					msn_elem->MissionRole());

					element->SetCarrier(carrier);
					element->SetCombatGroup(msn_elem->GetCombatGroup());
					element->SetCombatUnit(msn_elem->GetCombatUnit());
					element->SetCount(msn_elem->Count());
					element->SetRogue(false);
					element->SetPlayable(false);
					element->SetLoadout(def_load);
				}
			}
		}

		// create the element in space:
		else {
			Ship*       carrier  = 0;
			Hangar*     hangar   = 0;
			int         squadron = -1;
			int         slot     = 0;

			// first create the package element:
			Element* element = CreateElement(msn_elem->Name(),
			msn_elem->GetIFF(),
			msn_elem->MissionRole());

			element->SetPlayer(msn_elem->Player());
			element->SetCombatGroup(msn_elem->GetCombatGroup());
			element->SetCombatUnit(msn_elem->GetCombatUnit());
			element->SetCommandAILevel(msn_elem->CommandAI());
			element->SetHoldTime(msn_elem->HoldTime());
			element->SetZoneLock(msn_elem->ZoneLock() ? true : false);
			element->SetRogue(msn_elem->IsRogue());
			element->SetPlayable(msn_elem->IsPlayable());
			element->SetIntelLevel(msn_elem->IntelLevel());

			// if this is the player's element, make sure to activate the region:
			if (msn_elem->Player()) {
				SimRegion* rgn = FindRegion(msn_elem->Region());

				if (rgn && rgn != active_region)
				ActivateRegion(rgn);
			}

			// if element belongs to a squadron, 
			// find the carrier, squadron, flight deck, etc.:
			if (msn_elem->Squadron().length() > 0) {
				MissionElement* squadron_elem = mission->FindElement(msn_elem->Squadron());

				if (squadron_elem) {
					element->SetSquadron(msn_elem->Squadron());

					Element* cmdr  = FindElement(squadron_elem->Carrier());

					if (cmdr) {
						element->SetCommander(cmdr);
						carrier = cmdr->GetShip(1);

						if (carrier) {
							element->SetCarrier(carrier);
							hangar = carrier->GetHangar();

							for (int s = 0; s < hangar->NumSquadrons(); s++) {
								if (hangar->SquadronName(s) == msn_elem->Squadron()) {
									squadron = s;
									break;
								}
							}
						}
					}
				}
			}

			else if (msn_elem->Commander().length() > 0) {
				Element* cmdr = FindElement(msn_elem->Commander());

				if (cmdr) {
					element->SetCommander(cmdr);
				}
			}

			ListIter<Instruction> obj = msn_elem->Objectives();
			while (++obj) {
				Instruction* o     = obj.value();
				Instruction* instr = 0;

				instr = new(__FILE__,__LINE__) Instruction(*o);

				element->AddObjective(instr);
			}

			if (msn_elem->Instructions().size() > 0) {
				ListIter<Text> instr = msn_elem->Instructions();
				while (++instr) {
					element->AddInstruction(*instr);
				}
			}

			ListIter<Instruction> nav = msn_elem->NavList();
			while (++nav) {
				SimRegion* rgn = FindRegion(nav->RegionName());

				if (!rgn)
				rgn = FindRegion(msn_elem->Region());

				if (rgn) {
					Instruction* npt = new(__FILE__,__LINE__)
					Instruction(rgn, nav->Location(), nav->Action());

					npt->SetStatus(nav->Status());
					npt->SetEMCON(nav->EMCON());
					npt->SetFormation(nav->Formation());
					npt->SetSpeed(nav->Speed());
					npt->SetTarget(nav->TargetName());
					npt->SetHoldTime(nav->HoldTime());
					npt->SetFarcast(nav->Farcast());

					element->AddNavPoint(npt);
				}
			}

			bool  alertPrep = false;
			int*  loadout   = 0;
			int   respawns  = msn_elem->RespawnCount();

			// if ships are to start on alert,
			// spot them onto the appropriate launch deck:
			if (hangar && element && msn_elem->Count() > 0 && msn_elem->IsAlert()) {
				FlightDeck*       deck  = 0;
				int               queue = 1000;
				const ShipDesign* dsn   = msn_elem->GetDesign();

				if (dsn) {
					for (int i = 0; i < carrier->NumFlightDecks(); i++) {
						FlightDeck* d  = carrier->GetFlightDeck(i);
						int         dq = hangar->PreflightQueue(d);

						if (d && d->IsLaunchDeck() && d->SpaceLeft(dsn->type) && dq < queue) {
							queue = dq;
							deck  = d;
						}
					}
				}

				if (deck) {
					alertPrep = true;

					// choose best loadout:
					if (msn_elem->Loadouts().size()) {
						MissionLoad* l = msn_elem->Loadouts().at(0);
						if (l->GetName().length()) {
							ListIter<ShipLoad> sl = ((ShipDesign*) dsn)->loadouts;
							while (++sl) {
								if (!_stricmp(sl->name, l->GetName()))
								loadout = sl->load;
							}
						}

						else {
							loadout = l->GetStations();
						}
					}

					element->SetLoadout(loadout);

					for (int i = 0; i < msn_elem->Count(); i++) {
						int   squadron = -1;
						int   slot     = -1;

						if (hangar->FindAvailSlot(msn_elem->GetDesign(), squadron, slot)) {
							alertPrep = alertPrep &&
							hangar->GotoAlert(squadron,
							slot,
							deck,
							element,
							loadout,
							true,    // package for launch
							true);   // expedite

							HangarSlot* s         = (HangarSlot*) hangar->GetSlot(squadron, slot);
							Ship*       alertShip = hangar->GetShip(s);

							if (alertShip) {
								alertShip->SetRespawnCount(respawns);

								if (msn_elem->Player() == i+1) {
									if (alertShip->GetRegion()) {
										alertShip->GetRegion()->SetPlayerShip(alertShip);
									}
									else {
										::Print("WARNING: alert ship '%s' region is null\n", alertShip->Name());
									}
								}
							}
						}
					}
				}
			}

			if (!alertPrep) {
				// then, create the ships:
				for (int i = 0; i < msn_elem->Count(); i++) {
					MissionShip*   msn_ship = 0;
					Text           sname    = msn_elem->GetShipName(i);
					Text           rnum     = msn_elem->GetRegistry(i);
					Text           rgn_name = msn_elem->Region();

					if (msn_elem->Ships().size() > i) {
						msn_ship = msn_elem->Ships()[i];
						sname    = msn_ship->Name();
						rnum     = msn_ship->RegNum();
						rgn_name = msn_ship->Region();
					}

					Point l2 = msn_elem->Location();

					if (msn_ship && fabs(msn_ship->Location().x) < 1e9) {
						l2 = msn_ship->Location();
					}
					else if (i) {
						Point offset = RandomPoint();
						offset.z = Random(-1e3, 1e3);

						if (msn_elem->Count() < 5)
						offset *= 0.3;

						l2 += offset;
					}

					// choose best loadout:
					ListIter<MissionLoad> l = msn_elem->Loadouts();
					while (++l) {
						if ((l->GetShip() == i) || (l->GetShip() < 0 && loadout == 0)) {
							if (l->GetName().length()) {
								ListIter<ShipLoad> sl = ((ShipDesign*) msn_elem->GetDesign())->loadouts;
								while (++sl) {
									if (!_stricmp(sl->name, l->GetName()))
									loadout = sl->load;
								}
							}

							else {
								loadout = l->GetStations();
							}
						}
					}

					element->SetLoadout(loadout);

					Ship* ship = CreateShip(sname, rnum, 
					(ShipDesign*) msn_elem->GetDesign(), 
					rgn_name, l2,
					msn_elem->GetIFF(),
					msn_elem->CommandAI(),
					loadout);

					if (ship) {
						double      heading  = msn_elem->Heading();
						const Skin* skin     = msn_elem->GetSkin();

						if (msn_ship) {
							heading  = msn_ship->Heading();

							if (msn_ship->GetSkin())
							skin = msn_ship->GetSkin();
						}

						ship->SetRogue(msn_elem->IsRogue());
						ship->SetInvulnerable(msn_elem->IsInvulnerable());
						ship->SetHeading(0, 0, heading + PI);
						ship->SetRespawnCount(respawns);
						ship->UseSkin(skin);

						if (!netgame)
						ship->SetRespawnLoc(RandomPoint() * 2);

						if (ship->IsStarship())
						ship->SetHelmHeading(heading);

						else if (ship->IsAirborne() && ship->AltitudeAGL() > 25)
						ship->SetVelocity(ship->Heading() * 250);

						if (element)
						element->AddShip(ship);

						if (hangar)
						hangar->FindSlot(ship, squadron, slot, Hangar::ACTIVE);

						if (ship->GetRegion() && msn_elem->Player() == i+1)
						ship->GetRegion()->SetPlayerShip(ship);

						if (ship->NumFlightDecks()) {
							for (int i = 0; i < ship->NumFlightDecks(); i++) {
								FlightDeck* deck = ship->GetFlightDeck(i);
								if (deck)
								deck->Orient(ship);
							}
						}

						if (msn_ship) {
							ship->SetVelocity(msn_ship->Velocity().OtherHand());
							ship->SetIntegrity((float) msn_ship->Integrity());
							ship->SetRespawnCount(msn_ship->Respawns());

							if (msn_ship->Ammo()[0] > -10) {
								for (int i = 0; i < 64; i++) {
									Weapon* w = ship->GetWeaponByIndex(i+1);
									if (w)
									w->SetAmmo(msn_ship->Ammo()[i]);
									else
									break;
								}
							}

							if (msn_ship->Fuel()[0] > -10) {
								for (int i = 0; i < 4; i++) {
									if (ship->Reactors().size() > i) {
										PowerSource* p = ship->Reactors()[i];
										p->SetCapacity(msn_ship->Fuel()[i]);
									}
								}
							}

							if (msn_ship->Decoys() > -10) {
								Weapon* w = ship->GetDecoy();
								if (w)
								w->SetAmmo(msn_ship->Decoys());
							}

							if (msn_ship->Probes() > -10) {
								Weapon* w = ship->GetProbeLauncher();
								if (w)
								w->SetAmmo(msn_ship->Probes());
							}
						}

						Shield* shield = ship->GetShield();

						if (shield) {
							shield->SetPowerLevel(50);
						}

						if (ship->Class() > Ship::FRIGATE) {
							ListIter<WeaponGroup> iter = ship->Weapons();
							while (++iter) {
								WeaponGroup* weapon = iter.value();

								// anti-air weapon?
								if (weapon->GetDesign()->target_type & Ship::DRONE) {
									weapon->SetFiringOrders(Weapon::POINT_DEFENSE);
								}
								else {
									weapon->SetFiringOrders(Weapon::MANUAL);
								}
							}
						}

						if (ship->Class() > Ship::DRONE && ship->Class() < Ship::STATION) {
							ShipStats* stats = ShipStats::Find(sname);
							if (stats) {
								char design[64];
								sprintf_s(design, "%s %s", ship->Abbreviation(), ship->Design()->display_name);
								stats->SetType(design);
								stats->SetShipClass(ship->Class());
								stats->SetRole(Mission::RoleName(msn_elem->MissionRole()));
								stats->SetIFF(ship->GetIFF());
								stats->SetRegion(msn_elem->Region());
								stats->SetCombatGroup(msn_elem->GetCombatGroup());
								stats->SetCombatUnit(msn_elem->GetCombatUnit());
								stats->SetPlayer(msn_elem->Player() == i+1);
								stats->SetElementIndex(ship->GetElementIndex());
							}
						}
					}  // ship
				}     // count
			}
		}
	}
}

void
Sim::CopyEvents()
{
	events.destroy();

	if (mission) {
		ListIter<MissionEvent> iter = mission->GetEvents();
		while (++iter) {
			MissionEvent* orig = iter.value();
			MissionEvent* event = new(__FILE__,__LINE__) MissionEvent(*orig);
			events.append(event);
		}
	}
}

// +--------------------------------------------------------------------+

const char*
Sim::FindAvailCallsign(int IFF)
{
	const char* call = "Unidentified";

	for (int i = 0; i < 32; i++) {
		call = Callsign::GetCallsign(IFF);

		if (!FindElement(call))
		break;
	}

	return call;
}

Element*
Sim::CreateElement(const char* callsign, int IFF, int type)
{
	Element* elem = new(__FILE__,__LINE__) Element(callsign, IFF, type);
	elements.append(elem);
	return elem;
}

void
Sim::DestroyElement(Element* elem)
{
	if (elements.contains(elem))
	elements.remove(elem);

	delete elem;
}

Element*
Sim::FindElement(const char* name)
{
	ListIter<Element> iter = elements;

	while (++iter) {
		Element* elem  = iter.value();
		Text     ename = elem->Name();

		if (ename == name)
		return elem;
	}

	return 0;
}

// +--------------------------------------------------------------------+

int
Sim::GetAssignedElements(Element* elem, List<Element>& assigned)
{
	assigned.clear();

	if (elem) {
		for (int i = 0; i < elements.size(); i++) {
			Element* e = elements.at(i);
			if (!e->IsSquadron() && e->GetAssignment() == elem)
			assigned.append(e);
		}
	}

	return assigned.size();
}

// +--------------------------------------------------------------------+

Ship*
Sim::CreateShip(const char* name, const char* reg_num, ShipDesign* design, const char* rgn_name, const Point& loc, int IFF, int cmd_ai, const int* loadout)
{
	if (!design) {
		Print("WARNING: CreateShip(%s): invalid design\n", name);
		return 0;
	}

	SimRegion* rgn = FindRegion(rgn_name);

	if (!rgn) {
		return 0;
	}

	Ship* ship = new(__FILE__,__LINE__) Ship(name, reg_num, design, IFF, cmd_ai, loadout);
	ship->MoveTo(loc.OtherHand());

	if (rgn) {
		Print("Inserting Ship(%s) into Region(%s) (%s)\n", ship->Name(), rgn->Name(), FormatGameTime());
		rgn->InsertObject(ship);

		if (ship->IsAirborne() && ship->AltitudeAGL() > 25)
		ship->SetVelocity(ship->Heading() * 250);
	}

	return ship;
}

Ship*
Sim::FindShip(const char* name, const char* rgn_name)
{
	Ship* ship = 0;

	if (rgn_name) {
		SimRegion* rgn = FindRegion(rgn_name);
		if (rgn)
		ship = rgn->FindShip(name);
	}

	if (!ship) {
		ListIter<SimRegion> rgn = regions;
		while (++rgn && !ship)
		ship = rgn->FindShip(name);
	}

	return ship;
}

void
Sim::DestroyShip(Ship* ship)
{
	SimRegion* rgn = ship->GetRegion();
	if (rgn)
	rgn->DestroyShip(ship);
}

void
Sim::NetDockShip(Ship* ship, Ship* carrier, FlightDeck* deck)
{
	SimRegion* rgn = ship->GetRegion();
	if (rgn)
	rgn->NetDockShip(ship, carrier, deck);
}

Ship*
Sim::FindShipByObjID(DWORD objid)
{
	Ship* ship = 0;

	ListIter<SimRegion> rgn = regions;
	while (++rgn && !ship)
	ship = rgn->FindShipByObjID(objid);

	return ship;
}

Shot*
Sim::FindShotByObjID(DWORD objid)
{
	Shot* shot = 0;

	ListIter<SimRegion> rgn = regions;
	while (++rgn && !shot)
	shot = rgn->FindShotByObjID(objid);

	return shot;
}

// +--------------------------------------------------------------------+

Orbital*
Sim::FindOrbitalBody(const char* name)
{
	Orbital* body = 0;

	if (mission) {
		ListIter<StarSystem> iter = mission->GetSystemList();
		while (++iter && !body) {
			StarSystem* sys = iter.value();
			body = sys->FindOrbital(name);
		}
	}

	return body;
}


// +--------------------------------------------------------------------+

Shot*
Sim::CreateShot(const Point& pos, const Camera& shot_cam, WeaponDesign* design, const Ship* ship, SimRegion* rgn)
{
	Shot* shot = 0;

	if (design->drone)
	shot = new(__FILE__,__LINE__) Drone(pos, shot_cam, design, ship);
	else
	shot = new(__FILE__,__LINE__) Shot( pos, shot_cam, design, ship);

	if (rgn)
	rgn->InsertObject(shot);

	else if (active_region)
	active_region->InsertObject(shot);

	return shot;
}

// +--------------------------------------------------------------------+

Explosion*
Sim::CreateExplosion(const Point& pos, const Point& vel, int type, float exp_scale, float part_scale, SimRegion* rgn, SimObject* source, System* sys)
{
	// don't bother creating explosions that can't be seen:
	if (!rgn || !active_region || rgn != active_region)
	return 0;

	Explosion* exp = new(__FILE__,__LINE__) Explosion(type, pos, vel, exp_scale, part_scale, rgn, source);

	if (rgn)
	rgn->InsertObject(exp);

	else if (active_region)
	active_region->InsertObject(exp);

	return exp;
}

// +--------------------------------------------------------------------+

Debris*
Sim::CreateDebris(const Point& pos, const Point& vel, Model* model, double mass, SimRegion* rgn)
{
	Debris* debris = new(__FILE__,__LINE__) Debris(model, pos, vel, mass);

	if (rgn)
	rgn->InsertObject(debris);

	else if (active_region)
	active_region->InsertObject(debris);

	return debris;
}

// +--------------------------------------------------------------------+

Asteroid*
Sim::CreateAsteroid(const Point& pos, int t, double mass, SimRegion* rgn)
{
	Asteroid* asteroid = new(__FILE__,__LINE__) Asteroid(t, pos, mass);

	if (rgn)
	rgn->InsertObject(asteroid);

	else if (active_region)
	active_region->InsertObject(asteroid);

	return asteroid;
}

// +--------------------------------------------------------------------+

void
Sim::CreateSplashDamage(Ship* ship)
{
	if (ship && ship->GetRegion() && ship->Design()->splash_radius > 1) {
		SimSplash* splash = new(__FILE__,__LINE__)
		SimSplash(ship->GetRegion(),
		ship->Location(),
		ship->Design()->integrity / 4,
		ship->Design()->splash_radius);

		splash->owner_name = ship->Name();
		splashlist.append(splash);
	}
}

// +--------------------------------------------------------------------+

void
Sim::CreateSplashDamage(Shot* shot)
{
	if (shot && shot->GetRegion()) {
		double damage = shot->Damage();
		if (damage < shot->Design()->damage)
		damage = shot->Design()->damage;

		SimSplash* splash = new(__FILE__,__LINE__)
		SimSplash(shot->GetRegion(),
		shot->Location(),
		damage,
		shot->Design()->lethal_radius);

		if (shot->Owner())
		splash->owner_name = shot->Owner()->Name();

		splash->missile = shot->IsMissile();

		splashlist.append(splash);
		CreateExplosion(shot->Location(), Point(), Explosion::SHOT_BLAST, 20.0f, 1.0f, shot->GetRegion());
	}
}

// +--------------------------------------------------------------------+

void
Sim::ShowGrid(int show)
{
	Player* player = Player::GetCurrentPlayer();

	if (player && player->GridMode() == 0) {
		show       = 0;
		grid_shown = false;
	}

	ListIter<SimRegion> rgn = regions;
	while (++rgn) {
		rgn->ShowGrid(show);
	}

	grid_shown = show?true:false;
}

bool
Sim::GridShown() const
{
	return grid_shown;
}

// +--------------------------------------------------------------------+

List<StarSystem>&
Sim::GetSystemList()
{
	if (mission)
	return mission->GetSystemList();

	static List<StarSystem> dummy_system_list;
	return dummy_system_list;
}

// +--------------------------------------------------------------------+

void
Sim::NextView()
{
	if (active_region)
	active_region->NextView();
}

Ship*
Sim::GetPlayerShip()
{
	if (active_region)
	return active_region->GetPlayerShip();

	Starshatter* stars = Starshatter::GetInstance();
	if (stars && stars->InCutscene()) {
		Ship* player = 0;

		ListIter<SimRegion> rgn = regions;
		while (++rgn && !player) {
			player = rgn->GetPlayerShip();
		}

		return player;
	}

	return 0;
}

Element*
Sim::GetPlayerElement()
{
	Element* elem = 0;

	for (int i = 0; i < elements.size(); i++) {
		Element* e = elements[i];

		if (e->Player() > 0)
		elem = e;
	}

	return elem;
}

bool
Sim::IsSelected(Ship* s)
{
	if (active_region)
	return active_region->IsSelected(s);

	return false;
}

ListIter<Ship>
Sim::GetSelection()
{
	if (active_region)
	return active_region->GetSelection();

	static List<Ship> empty;
	return empty;
}

void
Sim::ClearSelection()
{
	if (active_region)
	active_region->ClearSelection();
}

void
Sim::AddSelection(Ship* s)
{
	if (active_region)
	active_region->AddSelection(s);
}

void
Sim::SetSelection(Ship* newsel)
{
	if (active_region)
	active_region->SetSelection(newsel);
}

// +--------------------------------------------------------------------+

void
Sim::SetTestMode(bool t)
{
	test_mode    = t;
	Ship* pship  = GetPlayerShip();

	if (pship)
	if (IsTestMode())
	pship->SetControls(0);
	else
	pship->SetControls(ctrl);
}

// +--------------------------------------------------------------------+

SimRegion*
Sim::FindRegion(const char* name)
{
	ListIter<SimRegion> rgn = regions;
	while (++rgn)
	if (rgn->name == name)
	return rgn.value();

	return 0;
}

SimRegion*
Sim::FindRegion(OrbitalRegion* orgn)
{
	ListIter<SimRegion> rgn = regions;
	while (++rgn)
	if (rgn->orbital_region == orgn)
	return rgn.value();

	return 0;
}

// +--------------------------------------------------------------------+

SimRegion*
Sim::FindNearestSpaceRegion(SimObject* object)
{
	return FindNearestRegion(object, REAL_SPACE);
}

SimRegion*
Sim::FindNearestTerrainRegion(SimObject* object)
{
	return FindNearestRegion(object, AIR_SPACE);
}

SimRegion*
Sim::FindNearestRegion(SimObject* object, int type)
{
	if (!object) return 0;

	SimRegion*  result   = 0;
	double      distance = 1.0e40;
	Point       objloc   = object->Location();

	objloc = objloc.OtherHand();

	if (object->GetRegion())
	objloc += object->GetRegion()->Location();

	ListIter<SimRegion> rgn = regions;
	while (++rgn) {
		if (rgn->Type() == type) {
			OrbitalRegion* orgn = rgn->GetOrbitalRegion();
			if (orgn) {
				double test = fabs((orgn->Location() - objloc).length());
				if (test < distance) {
					result = rgn.value();
					distance = test;
				}
			}
		}
	}
	
	return result;
}

SimRegion*
Sim::FindNearestSpaceRegion(Orbital* body)
{
	SimRegion*  result   = 0;

	if (!body)
	return result;

	ListIter<SimRegion> rgn = regions;
	while (++rgn && !result) {
		if (rgn->IsOrbital()) {
			OrbitalRegion* orgn = rgn->GetOrbitalRegion();
			if (orgn) {
				ListIter<OrbitalRegion> iter = body->Regions();
				while (++iter) {
					if (iter.value() == orgn)
					result = rgn.value();
				}
			}
		}
	}
	
	return result;
}

// +--------------------------------------------------------------------+

bool
Sim::ActivateRegion(SimRegion* rgn)
{
	if (rgn && active_region != rgn && regions.contains(rgn)) {
		if (active_region)
		active_region->Deactivate();

		if (!active_region || active_region->System() != rgn->System()) {
			if (active_region)
			active_region->System()->Deactivate();
			rgn->System()->Activate(scene);
		}

		active_region = rgn;
		star_system   = active_region->System();

		if (star_system) {
			star_system->SetActiveRegion(active_region->orbital_region);
		}
		else {
			::Print("WARNING: Sim::ActivateRegion() No star system found for rgn '%s'", rgn->Name());
		}

		active_region->Activate();
		return true;
	}

	return false;
}

// +--------------------------------------------------------------------+

void
Sim::RequestHyperJump(Ship* obj, SimRegion* rgn, const Point& loc,
int type, Ship* fc1, Ship* fc2)
{
	bool hyperdrive = false;

	if (obj->GetQuantumDrive() && obj->GetQuantumDrive()->Subtype() == QuantumDrive::HYPER)
	hyperdrive = true;

	jumplist.append(new(__FILE__,__LINE__) SimHyper(obj, rgn, loc, type, hyperdrive, fc1, fc2));
}

// +--------------------------------------------------------------------+

void
Sim::ExecFrame(double seconds)
{
	if (first_frame) {
		first_frame = false;
		netgame = NetGame::Create();
	}

	if (netgame)
	netgame->ExecFrame();

	if (regions.isEmpty()) {
		active_region = 0;
		rgn_queue.clear();
		jumplist.destroy();
		scene.Collect();
		return;
	}

	ListIter<Element> elem = elements;
	while (++elem)
	if (!elem->IsSquadron())
	elem->ExecFrame(seconds);

	ListIter<SimRegion> rgn = regions;
	while (++rgn)
	if (rgn.value() != active_region && rgn->NumShips() && !rgn_queue.contains(rgn.value()))
	rgn_queue.append(rgn.value());

	// execframe for one inactive sim region:
	if (rgn_queue.size()) {
		SimRegion* exec_rgn = rgn_queue.removeIndex(0);

		while (exec_rgn && (exec_rgn->NumShips() == 0 || exec_rgn == active_region))
		if (rgn_queue.size())
		exec_rgn = rgn_queue.removeIndex(0);
		else
		exec_rgn = 0;

		if (exec_rgn)
		exec_rgn->ExecFrame(seconds);
	}

	if (active_region)
	active_region->ExecFrame(seconds);

	ExecEvents(seconds);
	ResolveHyperList();
	ResolveSplashList();

	// GC all the dead objects:
	scene.Collect();

	if (!IsTestMode()) {
		ListIter<Element> e_iter = elements;
		while (++e_iter) {
			Element* elem = e_iter.value();
			if (!elem->IsSquadron() && elem->IsFinished()) {
				finished.append(e_iter.removeItem());
			}
		}
	}

	// setup music
	if (!MusicDirector::IsNoMusic()) {
		Starshatter* stars = Starshatter::GetInstance();
		if (stars && stars->GetGameMode() == Starshatter::PLAY_MODE) {
			Ship* player_ship = GetPlayerShip();
			if (player_ship) {
				int phase = player_ship->GetFlightPhase();

				if (phase < Ship::ACTIVE) {
					MusicDirector::SetMode(MusicDirector::LAUNCH);
				}

				else if (phase > Ship::ACTIVE) {
					MusicDirector::SetMode(MusicDirector::RECOVERY);
				}

				else {
					if (player_ship->IsInCombat()) {
						MusicDirector::SetMode(MusicDirector::COMBAT);
					}

					else {
						MusicDirector::SetMode(MusicDirector::FLIGHT);
					}
				}
			}
		}
	}
}

void
Sim::ExecEvents(double seconds)
{
	ListIter<MissionEvent> iter = events;
	while (++iter) {
		MissionEvent* event = iter.value();
		event->ExecFrame(seconds);
	}
}

void
Sim::ResolveHyperList()
{
	// resolve the hyper space transitions:
	if (jumplist.size()) {
		Ship* pship = GetPlayerShip();

		ListIter<SimHyper> j_iter = jumplist;
		while (++j_iter) {
			SimHyper*   jump     = j_iter.value();
			Ship*       jumpship = jump->ship;

			if (jumpship) {
				SimRegion* dest = jump->rgn;

				if (!dest)
				dest = FindNearestSpaceRegion(jumpship);

				if (dest) {
					// bring along fighters on deck:
					ListIter<FlightDeck> deck = jumpship->FlightDecks();
					while (++deck) {
						for (int i = 0; i < deck->NumSlots(); i++) {
							Ship* s = deck->GetShip(i);

							if (s) {
								dest->InsertObject(s);
								s->ClearTrack();
							}
						}
					}

					if (jump->type == 0 && !jump->hyperdrive) {
						// bring along nearby ships:
						// have to do it in two parts, because inserting the ships
						// into the destination corrupts the iter over the current
						// region's list of ships...

						// part one: gather the ships that will be jumping:
						List<Ship> riders;
						ListIter<Ship> neighbor = jumpship->GetRegion()->Ships();
						while (++neighbor) {
							if (neighbor->IsDropship()) {
								Ship* s = neighbor.value();
								if (s == jumpship) continue;

								Point delta = s->Location() - jumpship->Location();

								if (delta.length() < 5e3) {
									riders.append(s);
								}
							}
						}

						// part two: now transfer the list to the destination:
						for (int i = 0; i < riders.size(); i++) {
							Ship* s = riders[i];
							Point delta = s->Location() - jumpship->Location();
							dest->InsertObject(s);
							s->MoveTo(jump->loc.OtherHand() + delta);
							s->ClearTrack();

							if (jump->fc_dst) {
								double r = jump->fc_dst->Roll();
								double p = jump->fc_dst->Pitch();
								double w = jump->fc_dst->Yaw();

								s->SetAbsoluteOrientation(r, p, w);
								s->SetVelocity(jump->fc_dst->Heading() * 500);
							}

							ProcessEventTrigger(MissionEvent::TRIGGER_JUMP, 0, s->Name());
						}
					}

					// now it is safe to move the main jump ship:
					dest->InsertObject(jumpship);
					jumpship->MoveTo(jump->loc.OtherHand());
					jumpship->ClearTrack();

					ProcessEventTrigger(MissionEvent::TRIGGER_JUMP, 0, jumpship->Name());
					NetUtil::SendObjHyper(jumpship, dest->Name(), jump->loc, jump->fc_src, jump->fc_dst, jump->type);

					// if using farcaster:
					if (jump->fc_src) {
						::Print("Ship '%s' farcast to '%s'\n", jumpship->Name(), dest->Name());
						CreateExplosion(jumpship->Location(), Point(0,0,0), Explosion::QUANTUM_FLASH, 1.0f, 0, dest);

						if (jump->fc_dst) {
							double r = jump->fc_dst->Roll();
							double p = jump->fc_dst->Pitch();
							double w = jump->fc_dst->Yaw();

							jumpship->SetAbsoluteOrientation(r, p, w);
							jumpship->SetVelocity(jump->fc_dst->Heading() * 500);
						}

						jumpship->SetHelmHeading(jumpship->CompassHeading());
						jumpship->SetHelmPitch(0);
					}

					// break orbit:
					else if (jump->type == Ship::TRANSITION_DROP_ORBIT) {
						::Print("Ship '%s' broke orbit to '%s'\n", jumpship->Name(), dest->Name());
						jumpship->SetAbsoluteOrientation(0,PI/4,0);
						jumpship->SetVelocity(jumpship->Heading() * 1.0e3);
					}

					// make orbit:
					else if (jump->type == Ship::TRANSITION_MAKE_ORBIT) {
						::Print("Ship '%s' achieved orbit '%s'\n", jumpship->Name(), dest->Name());
						jumpship->LookAt(Point(0,0,0));
						jumpship->SetVelocity(jumpship->Heading() * 500.0);
					}

					// hyper jump:
					else {
						::Print("Ship '%s' quantum to '%s'\n", jumpship->Name(), dest->Name());

						if (jump->hyperdrive)
						CreateExplosion(jumpship->Location(), Point(0,0,0), Explosion::HYPER_FLASH,   1, 1, dest);
						else
						CreateExplosion(jumpship->Location(), Point(0,0,0), Explosion::QUANTUM_FLASH, 1, 0, dest);

						jumpship->LookAt(Point(0,0,0));
						jumpship->SetVelocity(jumpship->Heading() * 500.0);
						jumpship->SetHelmHeading(jumpship->CompassHeading());
						jumpship->SetHelmPitch(0);
					}
				}

				else if (regions.size() > 1) {
					::Print("Warning: Unusual jump request for ship '%s'\n", jumpship->Name());
					regions[1]->InsertObject(jumpship);
				}

				Sensor* sensor = jumpship->GetSensor();
				if (sensor)
				sensor->ClearAllContacts();
			}
		}

		jumplist.destroy();
		
		if (pship && pship->GetRegion()) {
			if (active_region != pship->GetRegion()) {
				pship->GetRegion()->SetPlayerShip(pship);
			}
		}
	}
}

void
Sim::ResolveSplashList()
{
	if (splashlist.size()) {
		ListIter<SimSplash> iter = splashlist;
		while (++iter) {
			SimSplash* splash = iter.value();

			if (!splash->rgn)
			continue;

			// damage ships:
			ListIter<Ship> s_iter = splash->rgn->Ships();
			while (++s_iter) {
				Ship* ship = s_iter.value();

				double distance = (ship->Location() - splash->loc).length();

				if (distance > 1 && distance < splash->range) {
					double damage = splash->damage * (1 - distance/splash->range);
					if (!NetGame::IsNetGameClient()) {
						ship->InflictDamage(damage);
					}

					int ship_destroyed = (!ship->InTransition() && ship->Integrity() < 1.0f);

					// then delete the ship:
					if (ship_destroyed) {
						NetUtil::SendObjKill(ship, 0, NetObjKill::KILL_MISC);
						Print("    %s Killed %s (%s)\n", (const char*) splash->owner_name, ship->Name(), FormatGameTime());

						// record the kill
						ShipStats* killer = ShipStats::Find(splash->owner_name);
						if (killer) {
							if (splash->missile)
							killer->AddEvent(SimEvent::MISSILE_KILL, ship->Name());
							else
							killer->AddEvent(SimEvent::GUNS_KILL, ship->Name());
						}

						Ship* owner = FindShip(splash->owner_name, splash->rgn->Name());
						if (owner && owner->GetIFF() != ship->GetIFF()) {
							if (ship->GetIFF() > 0 || owner->GetIFF() > 1) {
								killer->AddPoints(ship->Value());

								Element* elem = owner->GetElement();
								if (elem) {
									if (owner->GetElementIndex() > 1) {
										Ship* s = elem->GetShip(1);

										if (s) {
											ShipStats* cmdr_stats = ShipStats::Find(s->Name());
											if (cmdr_stats) {
												cmdr_stats->AddCommandPoints(ship->Value()/2);
											}
										}
									}

									Element* cmdr = elem->GetCommander();
									if (cmdr) {
										Ship* s = cmdr->GetShip(1);

										if (s) {
											ShipStats* cmdr_stats = ShipStats::Find(s->Name());
											if (cmdr_stats) {
												cmdr_stats->AddCommandPoints(ship->Value()/2);
											}
										}
									}
								}
							}
						}

						ShipStats* killee = ShipStats::Find(ship->Name());
						if (killee)
						killee->AddEvent(SimEvent::DESTROYED, splash->owner_name);

						ship->DeathSpiral();
					}
				}
			}

			// damage drones:
			ListIter<Drone> drone_iter = splash->rgn->Drones();
			while (++drone_iter) {
				Drone* drone = drone_iter.value();

				double distance = (drone->Location() - splash->loc).length();

				if (distance > 1 && distance < splash->range) {
					double damage = splash->damage * (1 - distance/splash->range);
					drone->InflictDamage(damage);

					int destroyed = (drone->Integrity() < 1.0f);

					// then mark the drone for deletion:
					if (destroyed) {
						NetUtil::SendWepDestroy(drone);
						sim->CreateExplosion(drone->Location(), drone->Velocity(), 21 /* was LARGE_EXP */, 1.0f, 1.0f, splash->rgn);
						drone->SetLife(0);
					}
				}
			}
		}

		splashlist.destroy();
	}
}

// +--------------------------------------------------------------------+

void
Sim::ProcessEventTrigger(int type, int event_id, const char* ship, int param)
{
	Text ship_name = ship;

	ListIter<MissionEvent> iter = events;
	while (++iter) {
		MissionEvent* event = iter.value();

		if (event->IsPending() && event->Trigger() == type) {
			switch (type) {
			case MissionEvent::TRIGGER_DAMAGE:
			case MissionEvent::TRIGGER_DESTROYED:
			case MissionEvent::TRIGGER_JUMP:
			case MissionEvent::TRIGGER_LAUNCH:
			case MissionEvent::TRIGGER_DOCK:
			case MissionEvent::TRIGGER_TARGET:
				if (event->TriggerParam() <= param) {
					if (ship_name.indexOf(event->TriggerShip()) == 0)
					event->Activate();
				}
				break;

			case MissionEvent::TRIGGER_NAVPT:
				if (event->TriggerParam() == param) {
					if (ship_name.indexOf(event->TriggerShip()) == 0)
					event->Activate();
				}
				break;

			case MissionEvent::TRIGGER_EVENT:
			case MissionEvent::TRIGGER_SKIPPED:
				if (event->TriggerParam() == event_id)
				event->Activate();
				break;
			}
		}
	}
}

double
Sim::MissionClock() const
{
	return (Game::GameTime() - start_time) / 1000.0;
}

// +--------------------------------------------------------------------+

void
Sim::SkipCutscene()
{
	Starshatter* stars = Starshatter::GetInstance();
	if (stars && stars->InCutscene()) {
		ListIter<MissionEvent>  iter     = events;
		bool                    end      = false;
		double                  end_time = 0;

		while (++iter && !end) {
			MissionEvent* event = iter.value();

			if (event->IsPending() || event->IsActive()) {
				if (event->Event() == MissionEvent::END_SCENE ||
						event->Event() == MissionEvent::END_MISSION) {
					end = true;
					end_time = event->Time();
				}

				if (event->Event() == MissionEvent::FIRE_WEAPON) {
					event->Skip();
				}

				else {
					event->Activate();
					event->Execute(true);
				}
			}
		}

		double skip_time = end_time - MissionClock();
		if (skip_time > 0) {
			Game::SkipGameTime(skip_time);
		}
	}
}

// +--------------------------------------------------------------------+

void
Sim::ResolveTimeSkip(double seconds)
{
	double skipped   = 0;

	// allow elements to process hold time, and release as needed:
	ListIter<Element> elem = elements;
	while (++elem)
	elem->ExecFrame(seconds);

	// step through the skip, ten seconds at a time:
	if (active_region) {
		double total_skip = seconds;
		double frame_skip = 10;
		Ship*  player     = GetPlayerShip();

		while (total_skip > frame_skip) {
			if (active_region->CanTimeSkip()) {
				active_region->ResolveTimeSkip(frame_skip);
				total_skip -= frame_skip;
				skipped    += frame_skip;
			}
			// break out early if player runs into bad guys...
			else {
				total_skip = 0;
			}
		}

		if (total_skip > 0)
		active_region->ResolveTimeSkip(total_skip);
		skipped += total_skip;
	}

	// give player control after time skip:
	Ship* player_ship = GetPlayerShip();
	if (player_ship) {
		player_ship->SetAutoNav(false);
		player_ship->SetThrottle(75);

		HUDView* hud = HUDView::GetInstance();
		if (hud)
		hud->SetHUDMode(HUDView::HUD_MODE_TAC);

		if (IsTestMode())
		player_ship->SetControls(0);
	}

	Game::SkipGameTime(skipped);
	CameraDirector::SetCameraMode(CameraDirector::MODE_COCKPIT);
}

// +--------------------------------------------------------------------+

ListIter<MissionElement>
Sim::GetMissionElements()
{
	mission_elements.destroy();

	ListIter<Element> iter = elements;
	while (++iter) {
		Element* elem = iter.value();

		int num_live_ships = 0;

		for (int i = 0; i < elem->NumShips(); i++) {
			Ship* s = elem->GetShip(i+1);

			if (s && !s->IsDying() && !s->IsDead())
			num_live_ships++;
		}

		if (elem->IsSquadron() || num_live_ships > 0) {
			MissionElement* msn_elem = CreateMissionElement(elem);

			if (msn_elem)
			mission_elements.append(msn_elem);
		}
	}

	return mission_elements;
}

MissionElement*
Sim::CreateMissionElement(Element* elem)
{
	MissionElement* msn_elem = 0;

	if (elem->IsSquadron()) {
		if (!elem->GetCarrier() || elem->GetCarrier()->Integrity() < 1)
		return msn_elem;
	}

	if (elem && !elem->IsNetObserver()) {
		msn_elem = new(__FILE__,__LINE__) MissionElement;

		msn_elem->SetName(elem->Name());
		msn_elem->SetIFF(elem->GetIFF());
		msn_elem->SetMissionRole(elem->Type());

		if (elem->IsSquadron() && elem->GetCarrier()) {
			Ship* carrier = elem->GetCarrier();

			msn_elem->SetCarrier(carrier->Name());
			msn_elem->SetCount(elem->GetCount());
			msn_elem->SetLocation(carrier->Location().OtherHand());

			if (carrier->GetRegion())
			msn_elem->SetRegion(carrier->GetRegion()->Name());

			int      squadron_index = 0;
			Hangar*  hangar = FindSquadron(elem->Name(), squadron_index);

			if (hangar) {
				msn_elem->SetDeadCount(hangar->NumShipsDead(squadron_index));
				msn_elem->SetMaintCount(hangar->NumShipsMaint(squadron_index));

				const ShipDesign* design = hangar->SquadronDesign(squadron_index);
				msn_elem->SetDesign(design);

				Text design_path = design->path_name;
				design_path.setSensitive(false);

				if (design_path.indexOf("/Mods/Ships") == 0) {
					design_path = design_path.substring(11, 1000);
					msn_elem->SetPath(design_path);
				}
			}
		}

		else {
			msn_elem->SetSquadron(elem->GetSquadron());
			msn_elem->SetCount(elem->NumShips());
		}

		if (elem->GetCommander())
		msn_elem->SetCommander(elem->GetCommander()->Name());

		msn_elem->SetCombatGroup(elem->GetCombatGroup());
		msn_elem->SetCombatUnit(elem->GetCombatUnit());

		Ship* ship = elem->GetShip(1);
		if (ship) {
			if (ship->GetRegion())
			msn_elem->SetRegion(ship->GetRegion()->Name());

			msn_elem->SetLocation(ship->Location().OtherHand());
			msn_elem->SetDesign(ship->Design());

			msn_elem->SetPlayer(elem->Player());
			msn_elem->SetCommandAI(elem->GetCommandAILevel());
			msn_elem->SetHoldTime((int) elem->GetHoldTime());
			msn_elem->SetZoneLock(elem->GetZoneLock());
			msn_elem->SetHeading(ship->CompassHeading());

			msn_elem->SetPlayable(elem->IsPlayable());
			msn_elem->SetRogue(elem->IsRogue());
			msn_elem->SetIntelLevel(elem->IntelLevel());

			Text design_path = ship->Design()->path_name;
			design_path.setSensitive(false);

			if (design_path.indexOf("/Mods/Ships") == 0) {
				design_path = design_path.substring(11, 1000);
				msn_elem->SetPath(design_path);
			}

			msn_elem->SetRespawnCount(ship->RespawnCount());
		}

		MissionLoad* loadout = new(__FILE__,__LINE__) MissionLoad;
		CopyMemory(loadout->GetStations(), elem->Loadout(), 16 * sizeof(int));

		msn_elem->Loadouts().append(loadout);

		int num_obj = elem->NumObjectives();
		for (int i = 0; i < num_obj; i++) {
			Instruction* o     = elem->GetObjective(i);
			Instruction* instr = 0;

			instr = new(__FILE__,__LINE__) Instruction(*o);

			msn_elem->AddObjective(instr);
		}

		int num_inst = elem->NumInstructions();
		for (int i = 0; i < num_inst; i++) {
			Text instr = elem->GetInstruction(i);
			msn_elem->AddInstruction(instr);
		}

		ListIter<Instruction> nav_iter = elem->GetFlightPlan();
		while (++nav_iter) {
			Instruction* nav = nav_iter.value();
			Instruction* npt = new(__FILE__,__LINE__)
			Instruction(nav->RegionName(), nav->Location(), nav->Action());

			npt->SetFormation(nav->Formation());
			npt->SetSpeed(nav->Speed());
			npt->SetTarget(nav->TargetName());
			npt->SetHoldTime(nav->HoldTime());
			npt->SetFarcast(nav->Farcast());
			npt->SetStatus(nav->Status());

			msn_elem->AddNavPoint(npt);
		}

		for (int i = 0; i < elem->NumShips(); i++) {
			ship = elem->GetShip(i+1);

			if (ship) {
				MissionShip* s = new(__FILE__,__LINE__) MissionShip;

				s->SetName(ship->Name());
				s->SetRegNum(ship->Registry());
				s->SetRegion(ship->GetRegion()->Name());
				s->SetLocation(ship->Location().OtherHand());
				s->SetVelocity(ship->Velocity().OtherHand());

				s->SetRespawns(ship->RespawnCount());
				s->SetHeading(ship->CompassHeading());
				s->SetIntegrity(ship->Integrity());

				if (ship->GetDecoy())
				s->SetDecoys(ship->GetDecoy()->Ammo());

				if (ship->GetProbeLauncher())
				s->SetProbes(ship->GetProbeLauncher()->Ammo());

				int n;
				int ammo[16];
				int fuel[4];

				for (n = 0; n < 16; n++) {
					Weapon* w = ship->GetWeaponByIndex(n+1);

					if (w)
					ammo[n] = w->Ammo();
					else
					ammo[n] = -10;
				}

				for (n = 0; n < 4; n++) {
					if (ship->Reactors().size() > n)
					fuel[n] = ship->Reactors()[n]->Charge();
					else
					fuel[n] = -10;
				}

				s->SetAmmo(ammo);
				s->SetFuel(fuel);

				msn_elem->Ships().append(s);
			}
		}
	}

	return msn_elem;
}

Hangar*
Sim::FindSquadron(const char* name, int& index)
{
	Hangar* hangar = 0;

	ListIter<SimRegion> iter = regions;
	while (++iter && !hangar) {
		SimRegion* rgn = iter.value();

		ListIter<Ship> s_iter = rgn->Carriers();
		while (++s_iter && !hangar) {
			Ship*    carrier = s_iter.value();
			Hangar*  h       = carrier->GetHangar();

			for (int i = 0; i < h->NumSquadrons() && !hangar; i++) {
				if (h->SquadronName(i) == name) {
					hangar = h;
					index  = i;
				}
			}
		}
	}

	return hangar;
}

// +===================================================================-+

SimRegion::SimRegion(Sim* s, const char* n, int t)
: sim(s), name(n), type(t), orbital_region(0), star_system(0)
, player_ship(0), grid(0), active(false), current_view(0), sim_time(0)
, ai_index(0), terrain(0)
{
	if (sim) {
		star_system = sim->GetStarSystem();
	}
}

SimRegion::SimRegion(Sim* s, OrbitalRegion* r)
: sim(s), orbital_region(r), type(REAL_SPACE), star_system(0)
, player_ship(0), grid(0), active(false), current_view(0), sim_time(0)
, ai_index(0), terrain(0)
{
	if (r) {
		star_system = r->System();
	}

	if (orbital_region) {
		name = orbital_region->Name();
		grid = new(__FILE__,__LINE__) Grid((int) orbital_region->Radius(),
		(int) orbital_region->GridSpace());


		if (orbital_region->Type() == Orbital::TERRAIN) {
			TerrainRegion* trgn = (TerrainRegion*) orbital_region;
			terrain = new(__FILE__,__LINE__) Terrain(trgn);

			type = AIR_SPACE;
		}

		else if (orbital_region->Asteroids() > 0) {
			int asteroids = orbital_region->Asteroids();

			for (int i = 0; i < asteroids; i++) {
				Point init_loc((rand()-16384.0f) * 30,
				(rand()-16384.0f) * 3,
				(rand()-16384.0f) * 30);
				sim->CreateAsteroid(init_loc, i, Random(1e7, 1e8), this); 
			}
		}
	}
	else {
		name = Game::GetText("Unknown");
	}
}

SimRegion::~SimRegion()
{
	GRAPHIC_DESTROY(grid);
	delete terrain;
	explosions.destroy();
	shots.destroy();
	ships.destroy();
	debris.destroy();
	asteroids.destroy();
	dead_ships.destroy();

	for (int i = 0; i < 5; i++)
	track_database[i].destroy();
}

int
SimRegion::operator < (const SimRegion& r) const
{
	return (orbital_region && r.orbital_region && *orbital_region <  *r.orbital_region);
}

int
SimRegion::operator <= (const SimRegion& r) const
{
	return (orbital_region && r.orbital_region && *orbital_region <= *r.orbital_region);
}

// +--------------------------------------------------------------------+

void
SimRegion::SetPlayerShip(Ship* ship)
{
	// there can only be a player ship when playing the game locally
	if (Starshatter::GetInstance()) {
		int player_index = ships.index(ship);

		if (player_index >= 0) {
			if (sim->GetActiveRegion() != this)
			sim->ActivateRegion(this);

			AttachPlayerShip(player_index);
		}

		else {
			Print("SimRegion %s could not set player ship '%s' - not in region\n",
			name.data(), ship ? ship->Name() : "(null)");
		}
	}

	// if this is a stand-alone server, set player ship to null
	else {
		if (player_ship)
		player_ship->SetControls(0);

		current_view = -1;
		player_ship = 0;
	}
}

void
SimRegion::AttachPlayerShip(int index)
{
	if (player_ship)
	player_ship->SetControls(0);

	current_view = index;
	player_ship  = ships[current_view];

	CameraDirector* cam_dir = CameraDirector::GetInstance();
	if (cam_dir)
	cam_dir->SetShip(player_ship);

	if (sim->dust)
	sim->dust->Reset(player_ship->Location());

	if (!sim->IsTestMode())
	player_ship->SetControls(sim->ctrl);

	MouseController* mouse_con = MouseController::GetInstance();
	if (mouse_con)
	mouse_con->SetActive(false);
}

void
SimRegion::NextView()
{
	if (ships.size()) {
		int original_view = current_view;

		do {
			current_view++;
			if (current_view >= ships.size()) {
				current_view = 0;
			}
		}
		while (ships[current_view]->Life() == 0 && current_view != original_view);

		if (current_view != original_view) {
			ClearSelection();

			if (!sim->IsTestMode())
			player_ship->SetControls(0);

			if (player_ship->Rep())
			player_ship->Rep()->Show();

			AttachPlayerShip(current_view);
		}
	}
}

bool
SimRegion::IsSelected(Ship* s)
{
	return selection.contains(s);
}

ListIter<Ship>
SimRegion::GetSelection()
{
	return selection;
}

void
SimRegion::SetSelection(Ship* newsel)
{
	selection.clear();
	selection.append(newsel);
}

void
SimRegion::ClearSelection()
{
	selection.clear();
}

void
SimRegion::AddSelection(Ship* newsel)
{
	if (!newsel || 
			newsel->GetFlightPhase() <  Ship::ACTIVE ||
			newsel->GetFlightPhase() >= Ship::RECOVERY)
	return;

	if (!selection.contains(newsel))
	selection.append(newsel);
}

// +--------------------------------------------------------------------+

void
SimRegion::Activate()
{
	if (!sim) return;

	ListIter<Ship> ship = ships;
	while (++ship)
	ship->Activate(sim->scene);

	ListIter<Shot> shot = shots;
	while (++shot)
	shot->Activate(sim->scene);

	ListIter<Explosion> exp = explosions;
	while (++exp)
	exp->Activate(sim->scene);

	ListIter<Debris> deb = debris;
	while (++deb)
	deb->Activate(sim->scene);

	ListIter<Asteroid> a = asteroids;
	while (++a)
	a->Activate(sim->scene);

	if (grid)
	sim->scene.AddGraphic(grid);

	if (terrain)
	terrain->Activate(sim->scene);

	player_ship = 0;
	active      = true;
}

// +--------------------------------------------------------------------+

void
SimRegion::Deactivate()
{
	if (!sim) return;

	ListIter<Ship> ship = ships;
	while (++ship)
	ship->Deactivate(sim->scene);

	ListIter<Shot> shot = shots;
	while (++shot)
	shot->Deactivate(sim->scene);

	ListIter<Explosion> exp = explosions;
	while (++exp)
	exp->Deactivate(sim->scene);

	ListIter<Debris> deb = debris;
	while (++deb)
	deb->Deactivate(sim->scene);

	ListIter<Asteroid> a = asteroids;
	while (++a)
	a->Deactivate(sim->scene);

	if (grid)
	sim->scene.DelGraphic(grid);

	if (terrain)
	terrain->Deactivate(sim->scene);

	player_ship = 0;
	active      = false;

	for (int i = 0; i < 5; i++)
	track_database[i].destroy();
}

// +--------------------------------------------------------------------+

void
SimRegion::ExecFrame(double secs)
{
	if (!sim) return;

	double seconds    = secs;

	// DON'T REALLY KNOW WHAT PURPOSE THIS SERVES....
	if (!active) {
		double max_frame  = 3 * Game::GetMaxFrameLength();
		long   new_time   = Game::GameTime();
		double delta      = new_time - sim_time;
		seconds    = delta / 1000.0;

		if (seconds > max_frame)
		seconds = max_frame;
	}

	sim_time = Game::GameTime();

	if (orbital_region)
	location = orbital_region->Location();

	CameraDirector* cam_dir = CameraDirector::GetInstance();

	Point ref;

	if (active && cam_dir) {
		ref = cam_dir->GetCamera()->Pos();
		UpdateSky(seconds, ref);
	}

	if (terrain)
	terrain->ExecFrame(seconds);

	UpdateTracks(seconds);
	UpdateShips(seconds);
	UpdateShots(seconds);
	UpdateExplosions(seconds);

	if (!Game::Paused()) {
		DamageShips();
		DockShips();

		if (active) {
			CollideShips();
			CrashShips();
		}

		DestroyShips();
	}

	if (active && cam_dir && player_ship) {
		Sound::SetListener(*(cam_dir->GetCamera()), player_ship->Velocity());
	}
}

// +--------------------------------------------------------------------+

void
SimRegion::ShowGrid(int show)
{
	if (grid) {
		if (show)
		grid->Show();
		else
		grid->Hide();
	}
}

// +--------------------------------------------------------------------+

void
SimRegion::UpdateSky(double seconds, const Point& ref)
{
	Dust* dust = sim->dust;

	if (dust) {
		if (orbital_region && orbital_region->Type() == Orbital::TERRAIN) {
			dust->Hide();
		}
		else {
			dust->Show();

			dust->ExecFrame(seconds, ref);

			if (player_ship && dust->Hidden()) {
				dust->Reset(player_ship->Location());
				dust->Show();
			}
		}
	}

	ListIter<Asteroid> a = asteroids;
	while (++a) {
		a->ExecFrame(seconds);
	}
}

// +--------------------------------------------------------------------+

void
SimRegion::UpdateShips(double seconds)
{
	int ship_index = 0;
	if (ai_index > ships.size())
	ai_index = 0;

	ListIter<Ship> ship_iter = ships;
	Ship* ship = 0;

	while (++ship_iter) {
		ship = ship_iter.value();
		
		if (ship_index == ai_index || ship == player_ship)
		ship->SetAIMode(2);
		else
		ship->SetAIMode(1);

		ship->ExecFrame(seconds);
		ship_index++;
	}

	ai_index++;
}

// +--------------------------------------------------------------------+

void
SimRegion::UpdateShots(double seconds)
{
	ListIter<Shot> shot_iter = shots;
	while (++shot_iter) {
		Shot* shot = shot_iter.value();
		shot->ExecFrame(seconds);

		if (shot->Design()->flak) {
			SeekerAI* seeker = (SeekerAI*) shot->GetDirector();

			if (shot->Life() < 0.02 || seeker && seeker->Overshot()) {
				shot->SetFuse(0.001); // set lifetime to ~zero
				sim->CreateSplashDamage(shot);
			}
		}
		
		if (shot->Life() < 0.01) {  // died of old age
			NetUtil::SendWepDestroy(shot);

			if (shot->IsDrone())
			drones.remove((Drone*) shot);

			shot_iter.removeItem();
			delete shot;
			shot = 0;
		}
	}
}

// +--------------------------------------------------------------------+

void
SimRegion::UpdateExplosions(double seconds)
{
	ListIter<Explosion> exp_iter = explosions;
	while (++exp_iter) {
		Explosion* exp = exp_iter.value();
		exp->ExecFrame(seconds);

		if (exp->Life() < 0.01) {  // died of old age
			exp_iter.removeItem();
			delete exp;
		}
	}

	ListIter<Debris> debris_iter = debris;
	while (++debris_iter) {
		Debris* d = debris_iter.value();
		d->ExecFrame(seconds);

		if (d->Life() < 0.01) {  // died of old age
			debris_iter.removeItem();
			delete d;
		}
	}
}

// +--------------------------------------------------------------------+
// Check for collisions between ships and shots, and apply damage.
// Also look for damage to drones and debris.

void
SimRegion::DamageShips()
{
	if (ships.size() == 0 || shots.size() == 0)
	return;

	Point impact;

	// FOR EACH SHOT IN THE REGION:
	ListIter<Shot> shot_iter = shots;
	while (++shot_iter) {
		Shot*       shot  = shot_iter.value();
		const Ship* owner = shot->Owner();
		const char* owner_name;

		if (owner)
		owner_name = owner->Name();
		else
		owner_name = "[KIA]";

		// CHECK FOR COLLISION WITH A SHIP:
		ListIter<Ship> ship_iter = ships;
		while (shot && ++ship_iter) {
			Ship* ship = ship_iter.value();
			int   hit  = ship->HitBy(shot, impact);

			if (hit) {
				// recon imager:
				if (shot->Damage() < 0) {
					ShipStats* shooter = ShipStats::Find(owner_name);
					if (shooter) {
						shooter->AddEvent(SimEvent::SCAN_TARGET, ship->Name());
					}
				}

				// live round:
				else if (shot->Damage() > 0) {
					int ship_destroyed = (!ship->InTransition() && ship->Integrity() < 1.0f);

					// then delete the ship:
					if (ship_destroyed) {
						NetUtil::SendObjKill(ship, owner, shot->IsMissile() ? NetObjKill::KILL_SECONDARY : NetObjKill::KILL_PRIMARY);
                        Director* director;
                        
						Print("    %s Killed %s (%s)\n", owner_name, ship->Name(), FormatGameTime());

						if (owner)
                            director = owner->GetDirector();
                        
                        // alert the killer
						if (director && director->Type() > SteerAI::SEEKER && director->Type() < SteerAI::GROUND) {
							ShipAI* shipAI = (ShipAI*) director;
							shipAI->Splash(ship);
						}

						// record the kill
						ShipStats* killer = ShipStats::Find(owner_name);
						if (killer) {
							if (shot->IsMissile())
							killer->AddEvent(SimEvent::MISSILE_KILL, ship->Name());
							else
							killer->AddEvent(SimEvent::GUNS_KILL, ship->Name());
						}

						if (owner && owner->GetIFF() != ship->GetIFF()) {
							if (ship->GetIFF() > 0 || owner->GetIFF() > 1) {
								killer->AddPoints(ship->Value());

								Element* elem = owner->GetElement();
								if (elem) {
									if (owner->GetElementIndex() > 1) {
										Ship* s = elem->GetShip(1);

										if (s) {
											ShipStats* cmdr_stats = ShipStats::Find(s->Name());
											if (cmdr_stats) {
												cmdr_stats->AddCommandPoints(ship->Value()/2);
											}
										}
									}

									Element* cmdr = elem->GetCommander();
									if (cmdr) {
										Ship* s = cmdr->GetShip(1);

										if (s) {
											ShipStats* cmdr_stats = ShipStats::Find(s->Name());
											if (cmdr_stats) {
												cmdr_stats->AddCommandPoints(ship->Value()/2);
											}
										}
									}
								}
							}
						}

						ShipStats* killee = ShipStats::Find(ship->Name());
						if (killee)
						killee->AddEvent(SimEvent::DESTROYED, owner_name);

						ship->DeathSpiral();
					}
				}

				// finally, consume the shot:
				if (!shot->IsBeam()) {
					if (owner) {
						ShipStats* stats = ShipStats::Find(owner_name);
						if (shot->Design()->primary)
						stats->AddGunHit();
						else if (shot->Damage() > 0)
						stats->AddMissileHit();
					}

					NetUtil::SendWepDestroy(shot);

					if (shot->IsDrone())
					drones.remove((Drone*) shot);

					shot_iter.removeItem();
					delete shot;
					shot = 0;
				}
				else if (!shot->HitTarget()) {
					shot->SetHitTarget(true);

					if (owner) {
						ShipStats* stats = ShipStats::Find(owner_name);
						if (shot->Design()->primary)
						stats->AddGunHit();
					}
				}
			}
		}

		// CHECK FOR COLLISION WITH A DRONE:
		if (shot && shot->Design()->target_type & Ship::DRONE) {
			ListIter<Drone> drone_iter = drones;
			while (shot && ++drone_iter) {
				Drone* d = drone_iter.value();

				if (d == shot || d->Owner() == owner)
				continue;

				int hit = d->HitBy(shot, impact);
				if (hit) {
					int destroyed = (d->Integrity() < 1.0f);

					// then mark the drone for deletion:
					if (destroyed) {
						NetUtil::SendWepDestroy(d);
						sim->CreateExplosion(d->Location(), d->Velocity(), 21, 1.0f, 1.0f, this);
						d->SetLife(0);
					}

					// finally, consume the shot:
					if (!shot->IsBeam()) {
						if (owner) {
							ShipStats* stats = ShipStats::Find(owner_name);
							if (shot->Design()->primary)
							stats->AddGunHit();
							else
							stats->AddMissileHit();
						}

						NetUtil::SendWepDestroy(shot);

						if (shot->IsDrone())
						drones.remove((Drone*) shot);
						
						shot_iter.removeItem();
						delete shot;
						shot = 0;
					}
				}
			}
		}

		// CHECK FOR COLLISION WITH DEBRIS:
		ListIter<Debris> debris_iter = debris;
		while (shot && ++debris_iter) {
			Debris* d = debris_iter.value();

			if (d->Radius() < 50)
			continue;

			int hit = d->HitBy(shot, impact);
			if (hit) {
				int destroyed = (d->Integrity() < 1.0f);

				// then delete the debris:
				if (destroyed) {
					sim->CreateExplosion(d->Location(), d->Velocity(), Explosion::LARGE_EXPLOSION, 1.0f, 1.0f, this);
					debris_iter.removeItem();
					delete d;
				}

				// finally, consume the shot:
				if (!shot->IsBeam()) {
					NetUtil::SendWepDestroy(shot);
					if (shot->IsDrone())
					drones.remove((Drone*) shot);
					
					shot_iter.removeItem();
					delete shot;
					shot = 0;
				}
			}
		}

		// CHECK FOR COLLISION WITH ASTEROIDS:
		ListIter<Asteroid> a_iter = asteroids;
		while (shot && ++a_iter) {
			Asteroid* a = a_iter.value();

			int hit = a->HitBy(shot, impact);
			if (hit) {
				if (!shot->IsBeam()) {
					if (shot->IsDrone())
					drones.remove((Drone*) shot);
					
					shot_iter.removeItem();
					delete shot;
					shot = 0;
				}
			}
		}
	}
}

// +--------------------------------------------------------------------+

void
SimRegion::CollideShips()
{
	if (ships.size() < 2 && debris.size() < 1)
	return;

	List<Ship> kill_list;

	int s_index = 0;

	ListIter<Ship> ship_iter = ships;
	while (++ship_iter) {
		Ship* ship = ship_iter.value();
		
		if (ship->InTransition()                  || 
				ship->GetFlightPhase() < Ship::ACTIVE ||
				ship->MissionClock() < 10000          ||
				ship->IsNetObserver())
		continue;

		int t_index = 0;
		ListIter<Ship> targ_iter = ships;
		while (++targ_iter) {
			Ship* targ = targ_iter.value();

			if (t_index++ <= s_index) continue;

			if (targ == ship) continue;

			if (targ->InTransition()                  || 
					targ->GetFlightPhase() < Ship::ACTIVE ||
					targ->MissionClock() < 10000          ||
					targ->IsNetObserver())
			continue;

			// ignore AI fighter collisions:
			if (ship->IsDropship()     &&
					ship != player_ship    &&
					targ->IsDropship()     &&
					targ != player_ship)
			continue;

			// don't collide with own runway!
			if (ship->IsAirborne() && ship->GetCarrier() == targ)
			continue;
			if (targ->IsAirborne() && targ->GetCarrier() == ship)
			continue;

			// impact:
			if (ship->CollidesWith(*targ)) {
				Vec3 tv1 = targ->Velocity();
				Vec3 sv1 = ship->Velocity();

				Physical::SemiElasticCollision(*ship, *targ);

				Vec3 tv2 = targ->Velocity();
				Vec3 sv2 = ship->Velocity();

				double dvs = (sv2-sv1).length();
				double dvt = (tv2-tv1).length();

				if (dvs > 20) dvs *= dvs;
				if (dvt > 20) dvt *= dvt;

				if (!NetGame::IsNetGameClient()) {
					double old_integrity = ship->Integrity();
					ship->InflictDamage(dvs);
					double hull_damage = old_integrity - ship->Integrity();
					NetUtil::SendObjDamage(ship, hull_damage);

					old_integrity = targ->Integrity();
					targ->InflictDamage(dvt);
					hull_damage = old_integrity - targ->Integrity();
					NetUtil::SendObjDamage(targ, hull_damage);
				}

				// then delete the ship:
				if (targ->Integrity() < 1.0f) {
					NetUtil::SendObjKill(targ, ship, NetObjKill::KILL_COLLISION);
					Print("   ship %s died in collision with %s (%s)\n", targ->Name(), ship->Name(), FormatGameTime());
					if (!kill_list.contains(targ)) {
						ShipStats* r = ShipStats::Find(targ->Name());
						if (r) r->AddEvent(SimEvent::COLLIDE, ship->Name());

						if (targ->GetIFF() > 0 && ship->GetIFF() != targ->GetIFF()) {
							r = ShipStats::Find(ship->Name());
							if (r) r->AddPoints(targ->Value());
						}

						kill_list.insert(targ);
					}
				}

				if (ship->Integrity() < 1.0f) {
					NetUtil::SendObjKill(ship, targ, NetObjKill::KILL_COLLISION);
					Print("   ship %s died in collision with %s (%s)\n", ship->Name(), targ->Name(), FormatGameTime());
					if (!kill_list.contains(ship)) {
						ShipStats* r = ShipStats::Find(ship->Name());
						if (r) r->AddEvent(SimEvent::COLLIDE, targ->Name());

						if (ship->GetIFF() > 0 && ship->GetIFF() != targ->GetIFF()) {
							r = ShipStats::Find(targ->Name());
							if (r) r->AddPoints(ship->Value());
						}

						kill_list.insert(ship);
					}
				}
			}
		}

		ListIter<Debris> debris_iter = debris;
		while (++debris_iter) {
			Debris* d = debris_iter.value();

			if (d->Radius() < 50)
			continue;

			if (ship->CollidesWith(*d)) {
				Vec3 tv1 = d->Velocity();
				Vec3 sv1 = ship->Velocity();

				Physical::SemiElasticCollision(*ship, *d);

				Vec3 tv2 = d->Velocity();
				Vec3 sv2 = ship->Velocity();

				if (!NetGame::IsNetGameClient()) {
					ship->InflictDamage((sv2-sv1).length());
				}

				d->InflictDamage((tv2-tv1).length());

				// then delete the debris:
				if (d->Integrity() < 1.0f) {
					sim->CreateExplosion(d->Location(), d->Velocity(), Explosion::LARGE_EXPLOSION, 1.0f, 1.0f, this);
					debris_iter.removeItem();
					delete d;
				}

				if (ship->Integrity() < 1.0f) {
					if (!kill_list.contains(ship)) {
						ShipStats* r = ShipStats::Find(ship->Name());
						if (r) r->AddEvent(SimEvent::COLLIDE, Game::GetText("DEBRIS"));

						kill_list.insert(ship);
					}
				}
			}
		}

		ListIter<Asteroid> a_iter = asteroids;
		while (++a_iter) {
			Asteroid* a = a_iter.value();

			if (ship->CollidesWith(*a)) {
				Vec3 sv1 = ship->Velocity();
				Physical::SemiElasticCollision(*ship, *a);
				Vec3 sv2 = ship->Velocity();

				if (!NetGame::IsNetGameClient()) {
					ship->InflictDamage((sv2-sv1).length() * 10);
				}

				if (ship->Integrity() < 1.0f) {
					if (!kill_list.contains(ship)) {
						ShipStats* r = ShipStats::Find(ship->Name());
						if (r) r->AddEvent(SimEvent::COLLIDE, Game::GetText("ASTEROID"));

						kill_list.insert(ship);
					}
				}
			}
		}

		s_index++;
	}

	ListIter<Ship> killed(kill_list);
	while (++killed) {
		Ship* kill = killed.value();
		kill->DeathSpiral();
	}
}

// +--------------------------------------------------------------------+

void
SimRegion::CrashShips()
{
	if (type != AIR_SPACE || NetGame::IsNetGameClient())
	return;

	ListIter<Ship> ship_iter = ships;
	while (++ship_iter) {
		Ship* ship = ship_iter.value();

		if (!ship->IsGroundUnit() && 
				!ship->InTransition() && 
				ship->Class() != Ship::LCA &&
				ship->AltitudeAGL() < ship->Radius()/2) {
			if (ship->GetFlightPhase() == Ship::ACTIVE || ship->GetFlightPhase() == Ship::APPROACH) {
				ship->InflictDamage(1e6);

				if (ship->Integrity() < 1.0f) {
					Print("    ship destroyed by crash: %s (%s)\n", ship->Name(), FormatGameTime());
					ShipStats* r = ShipStats::Find(ship->Name());
					if (r) r->AddEvent(SimEvent::CRASH);

					ship->DeathSpiral();
				}
			}
		}
	}

	ListIter<Shot> shot_iter = shots;
	while (++shot_iter) {
		Shot* shot = shot_iter.value();

		if (shot->IsBeam() || shot->IsDecoy())
		continue;

		if (shot->AltitudeMSL() < 5e3 &&
				shot->AltitudeAGL() < 5) {

			// shot hit the ground, destroy it:
			NetUtil::SendWepDestroy(shot);

			if (shot->IsDrone())
			drones.remove((Drone*) shot);

			shot_iter.removeItem();
			delete shot;
		}
	}
}

// +--------------------------------------------------------------------+

void
SimRegion::DestroyShips()
{
	ListIter<Ship> ship_iter = ships;
	while (++ship_iter) {
		Ship*    ship = ship_iter.value();

		if (ship->IsDead()) {
			// must use the iterator to remove the current
			// item from the container:
			ship_iter.removeItem();
			DestroyShip(ship);
		}
	}
}

// +--------------------------------------------------------------------+

void
SimRegion::DestroyShip(Ship* ship)
{
	if (!ship) return;

	Ship*    spawn = 0;

	ships.remove(ship);
	carriers.remove(ship);
	selection.remove(ship);

	Text rgn_name;
	if (ship->GetRegion())
	rgn_name = ship->GetRegion()->Name();

	bool player_destroyed = (player_ship == ship);

	char        ship_name[64];
	char        ship_reg[64];
	strcpy_s(ship_name, ship->Name());
	strcpy_s(ship_reg,  ship->Registry());

	ShipDesign* ship_design = (ShipDesign*) ship->Design();
	int         ship_iff    = ship->GetIFF();
	int         cmd_ai      = ship->GetCommandAILevel();
	bool        respawn     = sim->IsTestMode() && !ship->IsGroundUnit();
	bool        observe     = false;

	if (!respawn)
	respawn = ship->RespawnCount() > 0;

	if (sim->netgame) {
		if (!respawn)
		observe = player_destroyed;
	}

	if (respawn || observe) {
		if (!sim->netgame || !respawn)
		ship->SetRespawnLoc(RandomPoint() * 2);

		Point spawn_loc = ship->RespawnLoc();

		if (ship->IsAirborne() && spawn_loc.z < 5e3)
		spawn_loc.z = Random(8e3, 10e3);

		spawn = sim->CreateShip(ship_name, ship_reg, ship_design, rgn_name, spawn_loc, ship_iff, cmd_ai, observe ? 0 : ship->GetLoadout());
		spawn->SetRespawnCount(ship->RespawnCount() - 1);
		spawn->SetNetObserver(observe);

		if (sim->netgame && respawn)
		sim->netgame->Respawn(ship->GetObjID(), spawn);

		int n = strlen(ship_name); 
		if (n > 2) {
			if (ship_name[n-2] == ' ' && isdigit(ship_name[n-1]))
				ship_name[n-2] = 0;
		}

		Element* elem = sim->FindElement(ship_name);
		if (elem)
		elem->AddShip(spawn, ship->GetOrigElementIndex());
		else
		Print("Warning: No Element found for '%s' on respawn.\n", ship_name);

		if (player_destroyed)
		SetPlayerShip(spawn);
	}
	else {
		// close mission, return to menu:
		if (player_destroyed) {
			Starshatter* stars = Starshatter::GetInstance();
			if (stars)
			stars->SetGameMode(Starshatter::PLAN_MODE);
		}
	}

	sim->ProcessEventTrigger(MissionEvent::TRIGGER_DESTROYED, 0, ship->Name());

	dead_ships.insert(ship);
	ship->Destroy();
}

// +--------------------------------------------------------------------+

void
SimRegion::NetDockShip(Ship* ship, Ship* carrier, FlightDeck* deck)
{
	if (!ship || !carrier || !deck) return;

	deck->Dock(ship);
}

// +--------------------------------------------------------------------+

Ship*
SimRegion::FindShip(const char* ship_name)
{
	Ship* ship = 0;

	if (ship_name && *ship_name) {
		int   name_len = strlen(ship_name);

		ListIter<Ship> ship_iter = ships;
		while (++ship_iter && !ship) {
			Ship* test = ship_iter.value();
			if (!strncmp(test->Name(), ship_name, name_len)) {
				int test_len = strlen(test->Name());

				// The only fuzzy match is for element indices.
				// The desired name "Alpha" matches "Alpha 1" and "Alpha 2"
				// but not "Alpha-Centauri"

				if (test_len > name_len && test->Name()[name_len] != ' ')
				continue;

				ship = test;
			}
		}
	}

	return ship;
}

Ship*
SimRegion::FindShipByObjID(DWORD objid)
{
	Ship* ship = 0;

	ListIter<Ship> ship_iter = ships;
	while (++ship_iter && !ship) {
		Ship* test = ship_iter.value();
		if (test->GetObjID() == objid)
		ship = test;
	}

	return ship;
}

Shot*
SimRegion::FindShotByObjID(DWORD objid)
{
	Shot* shot = 0;

	ListIter<Shot> shot_iter = shots;
	while (++shot_iter && !shot) {
		Shot* test = shot_iter.value();
		if (test->GetObjID() == objid)
		shot = test;
	}

	if (!shot) {
		ListIter<Drone> drone_iter = drones;
		while (++drone_iter && !shot) {
			Drone* test = drone_iter.value();
			if (test->GetObjID() == objid)
			shot = test;
		}
	}

	return shot;
}

// +--------------------------------------------------------------------+

void
SimRegion::DockShips()
{
	if (ships.size() == 0)
	return;

	ListIter<Ship> ship_iter = ships;
	while (++ship_iter) {
		Ship* ship   = ship_iter.value();
		int   docked = (ship->GetFlightPhase() == Ship::DOCKED);

		if (docked) {
			sim->ProcessEventTrigger(MissionEvent::TRIGGER_DOCK, 0, ship->Name());

			// who did this ship dock with?
			Ship* carrier = ship->GetCarrier();

			if (carrier) {
				ShipStats* s = ShipStats::Find(ship->Name());
				if (s) {
					if (ship->IsAirborne())
					s->AddEvent(SimEvent::LAND, carrier->Name());
					else
					s->AddEvent(SimEvent::DOCK, carrier->Name());
				}

				ShipStats* c = ShipStats::Find(carrier->Name());
				if (c) c->AddEvent(SimEvent::RECOVER_SHIP, ship->Name());
			}

			// then delete the ship:
			int  player_docked = (player_ship == ship);
			char ship_name[33];
			strcpy_s(ship_name, ship->Name());

			selection.remove(ship);
			dead_ships.insert(ship_iter.removeItem());
			ship->Destroy();

			if (player_docked) {
				// close mission, return to menu:
				Starshatter* stars = Starshatter::GetInstance();
				if (stars)
				stars->SetGameMode(Starshatter::PLAN_MODE);
			}

			if (carrier)
			Print("    %s Docked with %s\n", ship_name, carrier->Name());
		}
	}
}

// +--------------------------------------------------------------------+

void
SimRegion::InsertObject(Ship* ship)
{
	if (!ship) return;

	SimRegion* orig = ship->GetRegion();

	if (orig != this) {
		if (orig != 0) {
			if (orig->active)
			ship->Deactivate(sim->scene);

			orig->ships.remove(ship);
			orig->carriers.remove(ship);
			orig->selection.remove(ship);
		}

		ships.append(ship);

		if (ship->NumFlightDecks())
		carriers.append(ship);

		TranslateObject(ship);
		ship->SetRegion(this);
		
		if (active)
		ship->Activate(sim->scene);
	}
}

void
SimRegion::InsertObject(Shot* shot)
{
	if (!shot) return;

	SimRegion* orig = shot->GetRegion();

	if (orig != this) {
		if (orig != 0)
		orig->shots.remove(shot);

		shots.append(shot);
		if (shot->IsDrone())
		drones.append((Drone*) shot);

		TranslateObject(shot);
		shot->SetRegion(this);
		
		if (active)
		shot->Activate(sim->scene);
	}
}

void
SimRegion::InsertObject(Explosion* exp)
{
	if (!exp) return;

	SimRegion* orig = exp->GetRegion();

	if (orig != this) {
		if (orig != 0)
		orig->explosions.remove(exp);

		explosions.append(exp);
		TranslateObject(exp);
		exp->SetRegion(this);
		
		if (active)
		exp->Activate(sim->scene);
	}
}

void
SimRegion::InsertObject(Debris* d)
{
	if (!d) return;

	SimRegion* orig = d->GetRegion();

	if (orig != this) {
		if (orig != 0)
		orig->debris.remove(d);

		debris.append(d);
		TranslateObject(d);
		d->SetRegion(this);
		
		if (active)
		d->Activate(sim->scene);
	}
}

void
SimRegion::InsertObject(Asteroid* a)
{
	if (!a) return;

	SimRegion* orig = a->GetRegion();

	if (orig != this) {
		if (orig != 0)
		orig->asteroids.remove(a);

		asteroids.append(a);
		TranslateObject(a);
		a->SetRegion(this);
		
		if (active)
		a->Activate(sim->scene);
	}
}

// +--------------------------------------------------------------------+

void
SimRegion::TranslateObject(SimObject* object)
{
	if (orbital_region)
	location = orbital_region->Location();

	if (object) {
		SimRegion* orig = object->GetRegion();
		if (orig) {
			Point delta = Location() - orig->Location();
			delta = delta.OtherHand();
			object->TranslateBy(delta);
		}
	}
}

// +--------------------------------------------------------------------+

List<Contact>&
SimRegion::TrackList(int iff)
{
	if (iff >= 0 && iff < 5)
	return track_database[iff];

	static List<Contact> empty;
	return empty;
}

void
SimRegion::UpdateTracks(double seconds)
{
	for (int i = 0; i < 5; i++) {
		ListIter<Contact> track_iter = track_database[i];

		while (++track_iter) {
			Contact* t        = track_iter.value();
			Ship*    c_ship   = t->GetShip();
			Shot*    c_shot   = t->GetShot();
			double   c_life   = 0;

			if (c_ship) {
				c_life = c_ship->Life();

				// look for quantum jumps and orbit transitions:
				if (c_ship->GetRegion() != this || c_ship->IsNetObserver())
				c_life = 0;
			}

			else if (c_shot)
			c_life = c_shot->Life();

			if (t->Age() < 0 || c_life == 0) {
				track_iter.removeItem();
				delete t;
			}

			else {
				t->Reset();
			}
		}
	}
}

// +--------------------------------------------------------------------+

bool
SimRegion::CanTimeSkip() const
{
	bool ok = false;

	if (player_ship) {
		ok = true;

		for (int i = 0; ok && i < ships.size(); i++) {
			Ship* s = ships[i];

			if (s != player_ship && s->GetIFF() && s->GetIFF() != player_ship->GetIFF()) {
				double dist = Point(s->Location() - player_ship->Location()).length();

				if (s->IsStarship())
				ok = dist > 60e3;
				else
				ok = dist > 30e3;
			}
		}
	}

	return ok;
}

// +--------------------------------------------------------------------+

void
SimRegion::ResolveTimeSkip(double seconds)
{
	for (int i = 0; i < ships.size(); i++) {
		Ship* ship = ships[i];
		Ship* ward = ship->GetWard();

		// remember to burn fuel and fix stuff...
		ship->ExecSystems(seconds);
		ship->ExecMaintFrame(seconds);

		ship->ClearTrack();
		ListIter<Contact> contact = ship->ContactList();
		while (++contact)
		contact->ClearTrack();

		if (ship->IsStatic())
		continue;

		// if ship is cleared inbound, land him:
		InboundSlot* inbound = ship->GetInbound();
		if (inbound) {
			if (inbound->Cleared()) {
				FlightDeck* deck = inbound->GetDeck();

				if (deck) {
					ship->SetCarrier((Ship*) deck->GetCarrier(), deck);
					ship->SetFlightPhase(Ship::DOCKED);
					ship->Stow();
					deck->Clear(inbound->Index());
				}
			}

			// cleared or not, once you're inbound, don't seek navpoints:
			continue;
		}

		if (ship->GetHangar()) {
			ship->GetHangar()->ExecFrame(seconds);

			List<FlightDeck>& flight_decks = ship->FlightDecks();
			for (int n = 0; n < flight_decks.size(); n++)
			flight_decks[n]->ExecFrame(seconds);
		}

		Instruction* navpt = ship->GetNextNavPoint();
		Point        dest  = ship->Location();
		double       speed = 500;
		double       space = 2.0e3 * (ship->GetElementIndex() - 1);

		if (ship->IsStarship())
		space *= 5;

		if (navpt && navpt->Action() == Instruction::LAUNCH) {
			ship->SetNavptStatus(navpt, Instruction::COMPLETE);
			navpt = ship->GetNextNavPoint();
		}

		if (navpt) {
			dest  = navpt->Location().OtherHand();
			speed = navpt->Speed();
		}

		else if (ward) {
			Point delta = ship->Location() - ward->Location();
			delta.y = 0;

			if (delta.length() > 25e3) {
				delta.Normalize();
				dest = ward->Location() + delta * 25e3;
			}
		}

		Point    delta = dest - ship->Location();
		Point    unit  = delta;
		double   dist  = unit.Normalize() - space;

		if (dist > 1e3) {
			if (speed < 50)
			speed = 500;

			double etr = dist / speed;

			if (etr > seconds)
			etr = seconds;

			Point trans = unit * (speed * etr);

			if (ship->GetFuelLevel() > 1) {
				ship->MoveTo(ship->Location() + trans);
				ship->SetVelocity(unit * speed);
			}
			ship->LookAt(dest);

			if (ship->IsStarship()) {
				ship->SetFLCSMode(Ship::FLCS_HELM);
				ship->SetHelmHeading(ship->CompassHeading());
				ship->SetHelmPitch(ship->CompassPitch());
			}
		}

		else if (navpt && navpt->Status() <= Instruction::ACTIVE) {
			ship->SetNavptStatus(navpt, Instruction::COMPLETE);
		}

		if (ward) {
			Point ward_heading = ward->Heading();
			ward_heading.y = 0;
			ward_heading.Normalize();

			if (ship->GetFuelLevel() > 1) {
				ship->SetVelocity(ward->Velocity());
			}
			ship->LookAt(ship->Location() + ward_heading * 1e6);

			if (ship->IsStarship()) {
				ship->SetFLCSMode(Ship::FLCS_HELM);
				ship->SetHelmHeading(ship->CompassHeading());
				ship->SetHelmPitch(ship->CompassPitch());
			}
		}

		if (dist > 1 || ward) {
			for (int j = 0; j < ships.size(); j++) {
				Ship* test = ships[j];

				if (ship != test && test->Mass() >= ship->Mass()) {
					Point delta = ship->Location() - test->Location();

					if (delta.length() < ship->Radius() * 2 + test->Radius() * 2) {
						ship->MoveTo(test->Location() + RandomPoint().OtherHand());
					}
				}
			}
		}
	}

	DockShips();
}

// +--------------------------------------------------------------------+

void
SimRegion::CommitMission()
{
	for (int i = 0; i < dead_ships.size(); i++) {
		Ship* s = dead_ships[i];

		if (s->GetCombatUnit() && s->GetFlightPhase() != Ship::DOCKED)
		s->GetCombatUnit()->Kill(1);
	}

	for (int i = 0; i < ships.size(); i++) {
		Ship*       s = ships[i];
		CombatUnit* u = s->GetCombatUnit();

		if (u) {
			Point u_loc = s->Location().OtherHand();
			if (u_loc.z > 20e3)
			u_loc.z = 20e3;
			else if (u_loc.z < -20e3)
			u_loc.z = -20e3;

			if (u->IsStarship()) {
				u->SetRegion(s->GetRegion()->Name());
				u->MoveTo(u_loc);
			}

			if (!u->IsDropship()) {
				if (s->Integrity() < 1)
				u->Kill(1);
				else
				u->SetSustainedDamage(s->Design()->integrity - s->Integrity());
			}

			CombatGroup* g = u->GetCombatGroup();
			if (g && g->Type() > CombatGroup::FLEET && g->GetFirstUnit() == u) {
				if (!g->IsZoneLocked())
				g->SetRegion(s->GetRegion()->Name());
				else
				u->SetRegion(g->GetRegion());

				g->MoveTo(u_loc);
			}
		}
	}
}

// +--------------------------------------------------------------------+

const char* FormatGameTime()
{
	static char txt[64];

	int t = Game::GameTime();

	int h = ( t                        / 3600000);
	int m = ((t - h*3600000)           /   60000);
	int s = ((t - h*3600000 - m*60000) /    1000);
	int e = ( t - h*3600000 - m*60000 - s*1000);

	if (h > 0)
	sprintf_s(txt, "%02d:%02d:%02d.%03d", h,m,s,e);
	else
	sprintf_s(txt, "%02d:%02d.%03d", m,s,e);

	return txt;
}
