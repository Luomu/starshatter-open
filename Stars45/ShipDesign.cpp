/*  Project Starshatter 5.0
    Destroyer Studios LLC
    Copyright © 1997-2007. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         ShipDesign.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Starship Design parameters class
*/

#include "MemDebug.h"
#include "ShipDesign.h"
#include "Ship.h"
#include "Shot.h"
#include "Power.h"
#include "HardPoint.h"
#include "Weapon.h"
#include "WeaponDesign.h"
#include "Shield.h"
#include "Sensor.h"
#include "NavLight.h"
#include "NavSystem.h"
#include "Drive.h"
#include "QuantumDrive.h"
#include "Farcaster.h"
#include "Thruster.h"
#include "FlightDeck.h"
#include "LandingGear.h"
#include "Computer.h"
#include "SystemDesign.h"
#include "Component.h"

#include "Game.h"
#include "Solid.h"
#include "Skin.h"
#include "Sprite.h"
#include "Light.h"
#include "Bitmap.h"
#include "Sound.h"
#include "DataLoader.h"
#include "ParseUtil.h"

// +--------------------------------------------------------------------+

const char* ship_design_class_name[32] = {
   "Drone",          "Fighter",
   "Attack",         "LCA",
   "Courier",        "Cargo",
   "Corvette",       "Freighter",

   "Frigate",        "Destroyer",
   "Cruiser",        "Battleship",
   "Carrier",        "Dreadnaught",

   "Station",        "Farcaster",

   "Mine",           "DEFSAT",
   "COMSAT",         "SWACS",

   "Building",       "Factory",
   "SAM",            "EWR",
   "C3I",            "Starbase",

   "0x04000000",     "0x08000000",
   "0x10000000",     "0x20000000",
   "0x40000000",     "0x80000000"
};

// +--------------------------------------------------------------------+

static const int     NAMELEN = 64;
static bool          degrees = false;

struct ShipCatalogEntry {
   static const char* TYPENAME() { return "ShipCatalogEntry"; }

   ShipCatalogEntry() : hide(false), design(0) {}

   ShipCatalogEntry(const char* n, const char* t, const char* p, const char* f, bool h=false) :
      name(n), type(t), path(p), file(f), hide(h), design(0) {}

   ~ShipCatalogEntry() { delete design; }

   Text        name;
   Text        type;
   Text        path;
   Text        file;
   bool        hide;

   ShipDesign* design;
};

static List<ShipCatalogEntry> catalog;
static List<ShipCatalogEntry> mod_catalog;

// +--------------------------------------------------------------------+

#define GET_DEF_BOOL(n) if (defname==(#n)) GetDefBool((n),   def, filename)
#define GET_DEF_TEXT(n) if (defname==(#n)) GetDefText((n),   def, filename)
#define GET_DEF_NUM(n)  if (defname==(#n)) GetDefNumber((n), def, filename)
#define GET_DEF_VEC(n)  if (defname==(#n)) GetDefVec((n),    def, filename)

static char        cockpit_name[80];
static List<Text>  detail[4];
static List<Point> offset[4];

static char errmsg[256];

// +--------------------------------------------------------------------+

ShipLoad::ShipLoad()
{
   ZeroMemory(name, sizeof(name));
   ZeroMemory(load, sizeof(load));
   mass = 0;
}

ShipSquadron::ShipSquadron()
{
   name[0] = 0;
   design  = 0;
   count   = 4;
   avail   = 4;
}

static void PrepareModel(Model& model)
{
   bool uses_bumps = false;

   ListIter<Material> iter = model.GetMaterials();
   while (++iter && !uses_bumps) {
      Material* mtl = iter.value();
      if (mtl->tex_bumpmap != 0 && mtl->bump != 0)
         uses_bumps = true;
   }

   if (uses_bumps)
      model.ComputeTangents();
}

// +--------------------------------------------------------------------+

ShipDesign::ShipDesign()
   : sensor(0), navsys(0), shield(0), type(0), decoy(0),
     probe(0), gear(0), valid(false), secret(false), auto_roll(1), cockpit_model(0),
     bolt_hit_sound_resource(0), beam_hit_sound_resource(0), lod_levels(0)
{
   ZeroMemory(filename,       sizeof(filename));
   ZeroMemory(path_name,      sizeof(path_name));
   ZeroMemory(name,           sizeof(name));
   ZeroMemory(display_name,   sizeof(display_name));
   ZeroMemory(abrv,           sizeof(abrv));

   for (int i = 0; i < 4; i++)
      feature_size[i] = 0.0f;
}

// +--------------------------------------------------------------------+

ShipDesign::ShipDesign(const char* n, const char* p, const char* fname, bool s)
   : sensor(0), navsys(0), shield(0), type(0),
     quantum_drive(0), farcaster(0), thruster(0), shield_model(0), decoy(0),
     probe(0), gear(0), valid(false), secret(s), auto_roll(1), cockpit_model(0),
     bolt_hit_sound_resource(0), beam_hit_sound_resource(0), lod_levels(0)
{
   ZeroMemory(filename,       sizeof(filename));
   ZeroMemory(path_name,      sizeof(path_name));
   ZeroMemory(name,           sizeof(name));
   ZeroMemory(display_name,   sizeof(display_name));
   ZeroMemory(abrv,           sizeof(abrv));

   strcpy(name, n);

   if (!strstr(fname, ".def"))
      sprintf(filename, "%s.def", fname);
   else
      strcpy(filename, fname);

   for (int i = 0; i < 4; i++)
      feature_size[i] = 0.0f;

   scale       = 1.0f;

   agility     = 2e2f;
   air_factor  = 0.1f;
   vlimit      = 8e3f;
   drag        = 2.5e-5f;
   arcade_drag = 1.0f;
   roll_drag   = 5.0f;
   pitch_drag  = 5.0f;
   yaw_drag    = 5.0f;

   roll_rate   = 0.0f;
   pitch_rate  = 0.0f;
   yaw_rate    = 0.0f;

   trans_x     = 0.0f;
   trans_y     = 0.0f;
   trans_z     = 0.0f;

   turn_bank   = (float) (PI/8);

   CL          = 0.0f;
   CD          = 0.0f;
   stall       = 0.0f;

   prep_time      = 30.0f;
   avoid_time     = 0.0f;
   avoid_fighter  = 0.0f;
   avoid_strike   = 0.0f;
   avoid_target   = 0.0f;
   commit_range   = 0.0f;

   splash_radius  = -1.0f;
   scuttle        = 5e3f;
   repair_speed   = 1.0f;
   repair_teams   = 2;
   repair_auto    = true;
   repair_screen  = true;
   wep_screen     = true;

   chase_vec      = Vec3(0, -100, 20);
   bridge_vec     = Vec3(0,    0,  0);
   beauty_cam     = Vec3(0,    0,  0);
   cockpit_scale  = 1.0f;

   radius      =   1.0f;
   integrity   = 500.0f;

   primary     = 0;
   secondary   = 1;
   main_drive  = -1;

   pcs         = 3.0f;
   acs         = 1.0f;
   detet       = 250.0e3f;
   e_factor[0] = 0.1f;
   e_factor[1] = 0.3f;
   e_factor[2] = 1.0f;

   explosion_scale   = 0.0f;
   death_spiral_time = 3.0f;

   if (!secret)
      Print("Loading ShipDesign '%s'\n", name);

   strcpy(path_name, p);
   if (path_name[strlen(path_name)-1] != '/')
      strcat(path_name, "/");

   // Load Design File:
   DataLoader* loader = DataLoader::GetLoader();
   loader->SetDataPath(path_name);

   BYTE* block;
   int blocklen = loader->LoadBuffer(filename, block, true);

   // file not found:
   if (blocklen <= 4) {
      valid = false;
      return;
   }

   Parser parser(new(__FILE__,__LINE__) BlockReader((const char*) block, blocklen));
   Term*  term = parser.ParseTerm();

   if (!term) {
      Print("ERROR: could not parse '%s'\n", filename);
      valid = false;
      return;
   }
   else {
      TermText* file_type = term->isText();
      if (!file_type || file_type->value() != "SHIP") {
         Print("ERROR: invalid ship design file '%s'\n", filename);
         valid = false;
         return;
      }
   }

   cockpit_name[0] = 0;
   valid           = true;
   degrees         = false;

   do {
      delete term;

      term = parser.ParseTerm();
      
      if (term) {
         TermDef* def = term->isDef();
         if (def) {
            ParseShip(def);
         }
         else {
            Print("WARNING: term ignored in '%s'\n", filename);
            term->print();
         }
      }
   }
   while (term);
   
   for (i = 0; i < 4; i++) {
      int n = 0;
      ListIter<Text> iter = detail[i];
      while (++iter) {
         const char* model_name = iter.value()->data();

         Model* model = new(__FILE__,__LINE__) Model;
         if (!model->Load(model_name, scale)) {
            Print("ERROR: Could not load detail %d, model '%s'\n", i, model_name);
            delete model;
            model = 0;
            valid = false;
         }

         else {
            lod_levels = i+1;

            if (model->Radius() > radius)
               radius = (float) model->Radius();

            models[i].append(model);
            PrepareModel(*model);

            if (offset[i].size()) {
               *offset[i].at(n) *= scale;
               offsets[i].append(offset[i].at(n)); // transfer ownership
            }
            else
               offsets[i].append(new(__FILE__,__LINE__) Point);

            n++;
         }
      }

      detail[i].destroy();
   }

   if (!secret)
      Print("   Ship Design Radius = %f\n", radius);

   if (cockpit_name[0]) {
      const char* model_name = cockpit_name;

      cockpit_model = new(__FILE__,__LINE__) Model;
      if (!cockpit_model->Load(model_name, cockpit_scale)) {
         Print("ERROR: Could not load cockpit model '%s'\n", model_name);
         delete cockpit_model;
         cockpit_model = 0;
      }
      else {
         if (!secret)
            Print("   Loaded cockpit model '%s', preparing tangents\n", model_name);
         PrepareModel(*cockpit_model);
      }
   }

   if (beauty.Width() < 1 && loader->FindFile("beauty.pcx"))
      loader->LoadBitmap("beauty.pcx", beauty);

   if (hud_icon.Width() < 1 && loader->FindFile("hud_icon.pcx"))
      loader->LoadBitmap("hud_icon.pcx", hud_icon);

   loader->ReleaseBuffer(block);
   loader->SetDataPath(0);

   if (abrv[0] == 0) {
      switch (type) {
      case Ship::DRONE:       strcpy(abrv, "DR");     break;
      case Ship::FIGHTER:     strcpy(abrv, "F");      break;
      case Ship::ATTACK:      strcpy(abrv, "F/A");    break;
      case Ship::LCA:         strcpy(abrv, "LCA");    break;
      case Ship::CORVETTE:    strcpy(abrv, "FC");     break;
      case Ship::COURIER:
      case Ship::CARGO:
      case Ship::FREIGHTER:   strcpy(abrv, "MV");     break;
      case Ship::FRIGATE:     strcpy(abrv, "FF");     break;
      case Ship::DESTROYER:   strcpy(abrv, "DD");     break;
      case Ship::CRUISER:     strcpy(abrv, "CA");     break;
      case Ship::BATTLESHIP:  strcpy(abrv, "BB");     break;
      case Ship::CARRIER:     strcpy(abrv, "CV");     break;
      case Ship::DREADNAUGHT: strcpy(abrv, "DN");     break;
      case Ship::MINE:        strcpy(abrv, "MINE");   break;
      case Ship::COMSAT:      strcpy(abrv, "COMS");   break;
      case Ship::DEFSAT:      strcpy(abrv, "DEFS");   break;
      case Ship::SWACS:       strcpy(abrv, "SWAC");   break;
      default:                                        break;
      }
   }

   if (scuttle < 1)
      scuttle = 1;

   if (splash_radius < 0)
      splash_radius = radius * 12.0f;

   if (repair_speed <= 1e-6)
      repair_speed = 1.0e-6f;

   if (commit_range <= 0) {
      if (type <= Ship::LCA)
         commit_range =  80.0e3f;
      else
         commit_range = 200.0e3f;
   }

   // calc standard loadout weights:
   ListIter<ShipLoad> sl = loadouts;
   while (++sl) {
      for (int i = 0; i < hard_points.size(); i++) {
         HardPoint* hp = hard_points[i];
         sl->mass += hp->GetCarryMass(sl->load[i]);
      }
   }
}

// +--------------------------------------------------------------------+

ShipDesign::~ShipDesign()
{
   delete bolt_hit_sound_resource;
   delete beam_hit_sound_resource;
   delete cockpit_model;
   delete navsys;
   delete sensor;
   delete shield;
   delete thruster;
   delete farcaster;
   delete quantum_drive;
   delete decoy;
   delete probe;
   delete gear;

   navlights.destroy();
   flight_decks.destroy();
   hard_points.destroy();
   computers.destroy();
   weapons.destroy();
   drives.destroy();
   reactors.destroy();
   loadouts.destroy();
   map_sprites.destroy();

   delete shield_model;
   for (int i = 0; i < 4; i++) {
      models[i].destroy();
      offsets[i].destroy();
   }

   spin_rates.destroy();

   for (i = 0; i < 10; i++) {
      delete debris[i].model;
   }
}

const char*
ShipDesign::DisplayName() const
{
   if (display_name[0])
      return display_name;

   return name;
}

// +--------------------------------------------------------------------+

void AddModCatalogEntry(const char* design_name, const char* design_path)
{
   if (!design_name || !*design_name)
      return;

   ShipCatalogEntry* entry  = 0;

   for (int i = 0; i < catalog.size(); i++) {
      ShipCatalogEntry* e = catalog[i];
      if (e->name == design_name) {
         if (design_path && *design_path && e->path != design_path)
            continue;
         entry = e;
         return;
      }
   }

   for (i = 0; i < mod_catalog.size(); i++) {
      ShipCatalogEntry* e = mod_catalog[i];
      if (e->name == design_name) {
         if (design_path && *design_path) {
            Text full_path = "Mods/Ships/";
            full_path += design_path;
            
            if (e->path != full_path)
               continue;
         }

         entry = e;
         return;
      }
   }

   // still here? not found yet:
   Text file = Text(design_name) + ".def";
   Text path = Text("Mods/Ships/");
   Text name;
   Text type;
   bool valid = false;
   
   if (design_path && *design_path)
      path += design_path;
   else
      path += design_name;

   path += "/";

   DataLoader* loader   = DataLoader::GetLoader();
   loader->SetDataPath(path);

   BYTE* block;
   int blocklen = loader->LoadBuffer(file, block, true);

   // file not found:
   if (blocklen <= 4) {
      return;
   }

   Parser parser(new(__FILE__,__LINE__) BlockReader((const char*) block, blocklen));
   Term*  term = parser.ParseTerm();

   if (!term) {
      Print("ERROR: could not parse '%s'\n", file);
      delete block;
      return;
   }
   else {
      TermText* file_type = term->isText();
      if (!file_type || file_type->value() != "SHIP") {
         Print("ERROR: invalid ship design file '%s'\n", file);
         delete block;
         return;
      }
   }

   valid = true;

   do {
      delete term;

      term = parser.ParseTerm();
      
      if (term) {
         TermDef* def = term->isDef();
         if (def) {
            Text defname = def->name()->value();
            defname.setSensitive(false);

            if (defname == "class") {
               if (!GetDefText(type, def, file)) {
                  Print("WARNING: invalid or missing ship class in '%s'\n", file);
                  valid = false;
               }
            }

            else if (defname == "name") {
               if (!GetDefText(name, def, file)) {
                  Print("WARNING: invalid or missing ship name in '%s'\n", file);
                  valid = false;
               }
            }
         }
         else {
            Print("WARNING: term ignored in '%s'\n", file);
            term->print();
         }
      }
   }
   while (term && valid && (name.length() < 1 || type.length() < 1));

   delete block;

   if (valid && name.length() && type.length()) {
      Print("Add Mod Catalog Entry '%s' Class '%s'\n", name.data(), type.data());

      ShipCatalogEntry* entry = new(__FILE__,__LINE__) ShipCatalogEntry(name, type, path, file);
      mod_catalog.append(entry);
   }
}

void
ShipDesign::Initialize()
{
   if (catalog.size()) return;

   LoadCatalog("Ships/", "catalog.def");
   LoadSkins("Mods/Skins/");

   List<Text>  mod_designs;
   DataLoader* loader = DataLoader::GetLoader();
   loader->SetDataPath("Mods/Ships/");
   loader->ListFiles("*.def", mod_designs, true);

   for (int i = 0; i < mod_designs.size(); i++) {
      Text full_name = *mod_designs[i];
      full_name.setSensitive(false);

      if (full_name.contains('/') && !full_name.contains("catalog")) {
         char path[1024];
         strcpy(path, full_name.data());

         char* name = path + full_name.length();
         while (*name != '/')
            name--;

         *name++ = 0;

         char* p = strrchr(name, '.');
         if (p && strlen(p) > 3) {
            if ((p[1] == 'd' || p[1] == 'D') &&
                (p[2] == 'e' || p[2] == 'E') &&
                (p[3] == 'f' || p[3] == 'F')) {

               *p = 0;
            }
         }

         // Just do a quick parse of the def file and add the
         // info to the catalog.  DON'T preload all of the models,
         // textures, and weapons at this time.  That takes way
         // too long with some of the larger user mods.

         AddModCatalogEntry(name, path);
      }
   }

   mod_designs.destroy();
   loader->SetDataPath(0);
}

void
ShipDesign::Close()
{
   mod_catalog.destroy();
   catalog.destroy();
}

// +--------------------------------------------------------------------+

int
ShipDesign::LoadCatalog(const char* path, const char* fname, bool mod)
{
   int result = 0;

   // Load Design Catalog File:
   DataLoader* loader = DataLoader::GetLoader();
   loader->SetDataPath(path);

   char filename[NAMELEN];
   ZeroMemory(filename, NAMELEN);
   strncpy(filename, fname, NAMELEN-1);

   Print("Loading ship design catalog: %s%s\n", path, filename);

   BYTE* block;
   int blocklen = loader->LoadBuffer(filename, block, true);
   Parser parser(new(__FILE__,__LINE__) BlockReader((const char*) block, blocklen));
   Term*  term = parser.ParseTerm();
   
   if (!term) {
      Print("ERROR: could not parse '%s'\n", filename);
      loader->ReleaseBuffer(block);
      loader->SetDataPath(0);
      return result;
   }
   else {
      TermText* file_type = term->isText();
      if (!file_type || file_type->value() != "SHIPCATALOG") {
         Print("ERROR: invalid ship catalog file '%s'\n", filename);
         loader->ReleaseBuffer(block);
         loader->SetDataPath(0);
         return result;
      }
   }

   do {
      delete term;

      term = parser.ParseTerm();

      Text name, type, fname, path;
      bool hide = false;
      
      if (term) {
         TermDef* def = term->isDef();
         if (def && def->term() && def->term()->isStruct()) {
            TermStruct* val = def->term()->isStruct();

            for (int i = 0; i < val->elements()->size(); i++) {
               TermDef* pdef = val->elements()->at(i)->isDef();
               if (pdef) {
                  Text defname = pdef->name()->value();
                  defname.setSensitive(false);

                  if (defname == "name") {
                     if (!GetDefText(name, pdef, filename))
                        Print("WARNING: invalid or missing ship name in '%s'\n", filename);
                  }
                  else if (defname == "type") {
                     if (!GetDefText(type, pdef, filename))
                        Print("WARNING: invalid or missing ship type in '%s'\n", filename);
                  }
                  else if (defname == "path") {
                     if (!GetDefText(path, pdef, filename))
                        Print("WARNING: invalid or missing ship path in '%s'\n", filename);
                  }
                  else if (defname == "file") {
                     if (!GetDefText(fname, pdef, filename))
                        Print("WARNING: invalid or missing ship file in '%s'\n", filename);
                  }
                  else if (defname == "hide" || defname == "secret") {
                     GetDefBool(hide, pdef, filename);
                  }
               }
            }

            ShipCatalogEntry* entry = new(__FILE__,__LINE__) ShipCatalogEntry(name, type, path, fname, hide);

            if (mod) mod_catalog.append(entry);
            else     catalog.append(entry);

            result++;
         }
         else {
            Print("WARNING: term ignored in '%s'\n", filename);
            term->print();
         }
      }
   }
   while (term);

   loader->ReleaseBuffer(block);
   loader->SetDataPath(0);

   return result;
}

// +--------------------------------------------------------------------+

void
ShipDesign::LoadSkins(const char* path, const char* archive)
{
   // Load MOD Skin Files:
   List<Text>  list;
   DataLoader* loader = DataLoader::GetLoader();
   bool        oldfs  = loader->IsFileSystemEnabled();

   loader->UseFileSystem(true);
   loader->SetDataPath(path);
   loader->ListArchiveFiles(archive, "*.def", list);

   ListIter<Text> iter = list;
   while (++iter) {
      Text  filename = *iter.value();
      BYTE* block;
      int   blocklen = loader->LoadBuffer(filename, block, true);

      // file not found:
      if (blocklen <= 4) {
         continue;
      }

      Parser      parser(new(__FILE__,__LINE__) BlockReader((const char*) block, blocklen));
      Term*       term   = parser.ParseTerm();
      ShipDesign* design = 0;

      if (!term) {
         Print("ERROR: could not parse '%s'\n", filename);
         return;
      }
      else {
         TermText* file_type = term->isText();
         if (!file_type || file_type->value() != "SKIN") {
            Print("ERROR: invalid skin file '%s'\n", filename);
            return;
         }
      }

      do {
         delete term;

         term = parser.ParseTerm();
      
         if (term) {
            TermDef* def = term->isDef();
            if (def) {
               Text defname = def->name()->value();
               defname.setSensitive(false);

               if (defname == "name") {
                  Text name;
                  GetDefText(name, def, filename);
                  design = Get(name);
               }

               else if (defname == "skin" && design != 0) {
                  if (!def->term() || !def->term()->isStruct()) {
                     Print("WARNING: skin struct missing in '%s'\n", filename);
                  }
                  else {
                     TermStruct* val = def->term()->isStruct();
                     Skin* skin = design->ParseSkin(val);

                     if (skin)
                        skin->SetPath(archive);
                  }
               }
            }
         }
      }
      while (term);
   }

   loader->UseFileSystem(oldfs);
}

// +--------------------------------------------------------------------+

int
ShipDesign::StandardCatalogSize()
{
   return catalog.size();
}

void
ShipDesign::PreloadCatalog(int index)
{
   if (index >= 0 && index < catalog.size()) {
      ShipCatalogEntry* entry = catalog[index];

      if (entry->hide)
         return;

      int ship_class = ClassForName(entry->type);
      if (ship_class > Ship::STARSHIPS)
         return;

      if (!entry->path.contains("Alliance_"))
         return;

      if (!entry->design) {
         entry->design = new(__FILE__,__LINE__) ShipDesign(entry->name,
                                                           entry->path,
                                                           entry->file,
                                                           entry->hide);
      }
   }

   else {
      ListIter<ShipCatalogEntry> iter = catalog;
      while (++iter) {
         ShipCatalogEntry* entry = iter.value();

         if (!entry->design) {
            entry->design = new(__FILE__,__LINE__) ShipDesign(entry->name,
                                                              entry->path,
                                                              entry->file,
                                                              entry->hide);
         }
      }
   }
}

// +--------------------------------------------------------------------+

bool
ShipDesign::CheckName(const char* design_name)
{
   ShipCatalogEntry* entry  = 0;

   for (int i = 0; i < catalog.size(); i++) {
      if (catalog.at(i)->name == design_name) {
         entry  = catalog.at(i);
         break;
      }
   }

   if (!entry) {
      for (int i = 0; i < mod_catalog.size(); i++) {
         if (mod_catalog.at(i)->name == design_name) {
            entry  = mod_catalog.at(i);
            break;
         }
      }
   }

   return entry != 0;
}

// +--------------------------------------------------------------------+

ShipDesign*
ShipDesign::Get(const char* design_name, const char* design_path)
{
   if (!design_name || !*design_name)
      return 0;

   ShipCatalogEntry* entry  = 0;

   for (int i = 0; i < catalog.size(); i++) {
      ShipCatalogEntry* e = catalog[i];
      if (e->name == design_name) {
         if (design_path && *design_path && e->path != design_path)
            continue;
         entry = e;
         break;
      }
   }

   if (!entry) {
      for (int i = 0; i < mod_catalog.size(); i++) {
         ShipCatalogEntry* e = mod_catalog[i];
         if (e->name == design_name) {
            if (design_path && *design_path) {
               Text full_path = "Mods/Ships/";
               full_path += design_path;
               
               if (e->path != full_path)
                  continue;
            }

            entry = e;
            break;
         }
      }
   }

   if (entry) {
      if (!entry->design) {
         entry->design = new(__FILE__,__LINE__) ShipDesign(entry->name,
                                                           entry->path,
                                                           entry->file,
                                                           entry->hide);
      }
      return entry->design;
   }
   else {
      Print("ShipDesign: no catalog entry for design '%s', checking mods...\n", design_name);
      return ShipDesign::FindModDesign(design_name, design_path);
   }
}

ShipDesign*
ShipDesign::FindModDesign(const char* design_name, const char* design_path)
{
   Text file = Text(design_name) + ".def";
   Text path = Text("Mods/Ships/");
   
   if (design_path && *design_path)
      path += design_path;
   else
      path += design_name;

   DataLoader* loader   = DataLoader::GetLoader();
   loader->SetDataPath(path);

   ShipDesign* design = new(__FILE__,__LINE__) ShipDesign(design_name, path, file);

   if (design->valid) {
      Print("ShipDesign: found mod design '%s'\n", design->name);

      ShipCatalogEntry* entry = new(__FILE__,__LINE__) ShipCatalogEntry(design->name,
                                                                        ClassName(design->type), 
                                                                        path, 
                                                                        file);
      mod_catalog.append(entry);
      entry->design = design;
      return entry->design;
   }
   else {
      delete design;
   }

   return 0;
}

void
ShipDesign::ClearModCatalog()
{
   mod_catalog.destroy();

   for (int i = 0; i < catalog.size(); i++) {
      ShipCatalogEntry* e = catalog[i];

      if (e && e->design) {
         ListIter<Skin> iter = e->design->skins;

         while (++iter) {
            Skin* skin = iter.value();
            if (*skin->Path())
               iter.removeItem();
         }
      }
   }
}

// +--------------------------------------------------------------------+

int
ShipDesign::GetDesignList(int type, List<Text>& designs)
{
   designs.clear();

   for (int i = 0; i < catalog.size(); i++) {
      ShipCatalogEntry* e = catalog[i];

      int etype = ClassForName(e->type);
      if (etype & type) {
         if (!e->design)
            e->design = new(__FILE__,__LINE__) ShipDesign(e->name,
                                                          e->path,
                                                          e->file,
                                                          e->hide);

         if (e->hide || !e->design || !e->design->valid || e->design->secret)
            continue;

         designs.append(&e->name);
      }
   }

   for (i = 0; i < mod_catalog.size(); i++) {
      ShipCatalogEntry* e = mod_catalog[i];

      int etype = ClassForName(e->type);
      if (etype & type) {
         designs.append(&e->name);
      }
   }

   return designs.size();
}

// +--------------------------------------------------------------------+

int
ShipDesign::ClassForName(const char* name)
{
   if (!name || !name[0])
      return 0;

   for (int i = 0; i < 32; i++) { 
      if (!stricmp(name, ship_design_class_name[i])) {
         return 1 << i;
      }
   }

   return 0;
}

const char*
ShipDesign::ClassName(int type)
{
   if (type != 0) {
      int index = 0;

      while (!(type & 1)) {
         type >>= 1;
         index++;
      }

      if (index >= 0 && index < 32)
         return ship_design_class_name[index];
   }

   return "Unknown";
}

// +--------------------------------------------------------------------+

void
ShipDesign::ParseShip(TermDef* def)
{
   char           detail_name[NAMELEN];
   Vec3           off_loc;
   Vec3           spin;
   Text           defname = def->name()->value();

   defname.setSensitive(false);

   if (defname == "cockpit_model") {
      if (!GetDefText(cockpit_name, def, filename))
         Print("WARNING: invalid or missing cockpit_model in '%s'\n", filename);
   }

   else if (defname == "model" || defname == "detail_0") {
      if (!GetDefText(detail_name, def, filename))
         Print("WARNING: invalid or missing model in '%s'\n", filename);

      detail[0].append(new(__FILE__,__LINE__) Text(detail_name));
   }

   else if (defname == "detail_1") {
      if (!GetDefText(detail_name, def, filename))
         Print("WARNING: invalid or missing detail_1 in '%s'\n", filename);

      detail[1].append(new(__FILE__,__LINE__) Text(detail_name));
   }

   else if (defname == "detail_2") {
      if (!GetDefText(detail_name, def, filename))
         Print("WARNING: invalid or missing detail_2 in '%s'\n", filename);

      detail[2].append(new(__FILE__,__LINE__) Text(detail_name));
   }

   else if (defname == "detail_3") {
      if (!GetDefText(detail_name, def, filename))
         Print("WARNING: invalid or missing detail_3 in '%s'\n", filename);

      detail[3].append(new(__FILE__,__LINE__) Text(detail_name));
   }

   else if (defname == "spin") {
      if (!GetDefVec(spin, def, filename))
         Print("WARNING: invalid or missing spin in '%s'\n", filename);

      spin_rates.append(new(__FILE__,__LINE__) Point(spin));
   }

   else if (defname == "offset_0") {
      if (!GetDefVec(off_loc, def, filename))
         Print("WARNING: invalid or missing offset_0 in '%s'\n", filename);

      offset[0].append(new(__FILE__,__LINE__) Point(off_loc));
   }

   else if (defname == "offset_1") {
      if (!GetDefVec(off_loc, def, filename))
         Print("WARNING: invalid or missing offset_1 in '%s'\n", filename);

      offset[1].append(new(__FILE__,__LINE__) Point(off_loc));
   }

   else if (defname == "offset_2") {
      if (!GetDefVec(off_loc, def, filename))
         Print("WARNING: invalid or missing offset_2 in '%s'\n", filename);

      offset[2].append(new(__FILE__,__LINE__) Point(off_loc));
   }

   else if (defname == "offset_3") {
      if (!GetDefVec(off_loc, def, filename))
         Print("WARNING: invalid or missing offset_3 in '%s'\n", filename);

      offset[3].append(new(__FILE__,__LINE__) Point(off_loc));
   }

   else if (defname == "beauty") {
      if (def->term() && def->term()->isArray()) {
         GetDefVec(beauty_cam, def, filename);

         if (degrees) {
            beauty_cam.x *= (float) DEGREES;
            beauty_cam.y *= (float) DEGREES;
         }
      }

      else {
         char beauty_name[64];
         if (!GetDefText(beauty_name, def, filename))
            Print("WARNING: invalid or missing beauty in '%s'\n", filename);

         DataLoader* loader = DataLoader::GetLoader();
         loader->LoadBitmap(beauty_name, beauty);
      }
   }

   else if (defname == "hud_icon") {
      char hud_icon_name[64];
      if (!GetDefText(hud_icon_name, def, filename))
         Print("WARNING: invalid or missing hud_icon in '%s'\n", filename);

      DataLoader* loader = DataLoader::GetLoader();
      loader->LoadBitmap(hud_icon_name, hud_icon);
   }

   else if (defname == "feature_0") {
      if (!GetDefNumber(feature_size[0], def, filename))
         Print("WARNING: invalid or missing feature_0 in '%s'\n", filename);
   }

   else if (defname == "feature_1") {
      if (!GetDefNumber(feature_size[1], def, filename))
         Print("WARNING: invalid or missing feature_1 in '%s'\n", filename);
   }

   else if (defname == "feature_2") {
      if (!GetDefNumber(feature_size[2], def, filename))
         Print("WARNING: invalid or missing feature_2 in '%s'\n", filename);
   }

   else if (defname == "feature_3") {
      if (!GetDefNumber(feature_size[3], def, filename))
         Print("WARNING: invalid or missing feature_3 in '%s'\n", filename);
   }


   else if (defname == "class") {
      char typestr[64];
      if (!GetDefText(typestr, def, filename))
         Print("WARNING: invalid or missing ship class in '%s'\n", filename);

      type = ClassForName(typestr);

      if (type <= Ship::LCA) {
         repair_auto   = false;
         repair_screen = false;
         wep_screen    = false;
      }
   }

   else GET_DEF_TEXT(name);
   else GET_DEF_TEXT(description);
   else GET_DEF_TEXT(display_name);
   else GET_DEF_TEXT(abrv);
   else GET_DEF_NUM(pcs);
   else GET_DEF_NUM(acs);
   else GET_DEF_NUM(detet);
   else GET_DEF_NUM(scale);
   else GET_DEF_NUM(explosion_scale);
   else GET_DEF_NUM(mass);
   else GET_DEF_NUM(vlimit);
   else GET_DEF_NUM(agility);
   else GET_DEF_NUM(air_factor);
   else GET_DEF_NUM(roll_rate);
   else GET_DEF_NUM(pitch_rate);
   else GET_DEF_NUM(yaw_rate);
   else GET_DEF_NUM(integrity);
   else GET_DEF_NUM(drag);
   else GET_DEF_NUM(arcade_drag);
   else GET_DEF_NUM(roll_drag);
   else GET_DEF_NUM(pitch_drag);
   else GET_DEF_NUM(yaw_drag);
   else GET_DEF_NUM(trans_x);
   else GET_DEF_NUM(trans_y);
   else GET_DEF_NUM(trans_z);
   else GET_DEF_NUM(turn_bank);
   else GET_DEF_NUM(cockpit_scale);
   else GET_DEF_NUM(auto_roll);

   else GET_DEF_NUM(CL);
   else GET_DEF_NUM(CD);
   else GET_DEF_NUM(stall);

   else GET_DEF_NUM(prep_time);
   else GET_DEF_NUM(avoid_time);
   else GET_DEF_NUM(avoid_fighter);
   else GET_DEF_NUM(avoid_strike);
   else GET_DEF_NUM(avoid_target);
   else GET_DEF_NUM(commit_range);

   else GET_DEF_NUM(splash_radius);
   else GET_DEF_NUM(scuttle);
   else GET_DEF_NUM(repair_speed);
   else GET_DEF_NUM(repair_teams);
   else GET_DEF_BOOL(secret);
   else GET_DEF_BOOL(repair_auto);
   else GET_DEF_BOOL(repair_screen);
   else GET_DEF_BOOL(wep_screen);
   else GET_DEF_BOOL(degrees);

   else if (defname == "emcon_1") {
      GetDefNumber(e_factor[0], def, filename);
   }

   else if (defname == "emcon_2") {
      GetDefNumber(e_factor[1], def, filename);
   }

   else if (defname == "emcon_3") {
      GetDefNumber(e_factor[2], def, filename);
   }

   else if (defname == "chase") {
      if (!GetDefVec(chase_vec, def, filename))
         Print("WARNING: invalid or missing chase cam loc in '%s'\n", filename);

      chase_vec *= (float) scale;
   }
   
   else if (defname == "bridge") {
      if (!GetDefVec(bridge_vec, def, filename))
         Print("WARNING: invalid or missing bridge cam loc in '%s'\n", filename);

      bridge_vec *= (float) scale;
   }
   
   else if (defname == "power") {
      if (!def->term() || !def->term()->isStruct()) {
         Print("WARNING: power source struct missing in '%s'\n", filename);
      }
      else {
         TermStruct* val = def->term()->isStruct();
         ParsePower(val);
      }
   }
   
   else if (defname == "main_drive" || defname == "drive") {
      if (!def->term() || !def->term()->isStruct()) {
         Print("WARNING: main drive struct missing in '%s'\n", filename);
      }
      else {
         TermStruct* val = def->term()->isStruct();
         ParseDrive(val);
      }
   }

   else if (defname == "quantum" || defname == "quantum_drive") {
      if (!def->term() || !def->term()->isStruct()) {
         Print("WARNING: quantum_drive struct missing in '%s'\n", filename);
      }
      else {
         TermStruct* val = def->term()->isStruct();
         ParseQuantumDrive(val);
      }
   }

   else if (defname == "sender" || defname == "farcaster") {
      if (!def->term() || !def->term()->isStruct()) {
         Print("WARNING: farcaster struct missing in '%s'\n", filename);
      }
      else {
         TermStruct* val = def->term()->isStruct();
         ParseFarcaster(val);
      }
   }

   else if (defname == "thruster") {
      if (!def->term() || !def->term()->isStruct()) {
         Print("WARNING: thruster struct missing in '%s'\n", filename);
      }
      else {
         TermStruct* val = def->term()->isStruct();
         ParseThruster(val);
      }
   }

   else if (defname == "navlight") {
      if (!def->term() || !def->term()->isStruct()) {
         Print("WARNING: navlight struct missing in '%s'\n", filename);
      }
      else {
         TermStruct* val = def->term()->isStruct();
         ParseNavlight(val);
      }
   }

   else if (defname == "flightdeck") {
      if (!def->term() || !def->term()->isStruct()) {
         Print("WARNING: flightdeck struct missing in '%s'\n", filename);
      }
      else {
         TermStruct* val = def->term()->isStruct();
         ParseFlightDeck(val);
      }
   }

   else if (defname == "gear") {
      if (!def->term() || !def->term()->isStruct()) {
         Print("WARNING: gear struct missing in '%s'\n", filename);
      }
      else {
         TermStruct* val = def->term()->isStruct();
         ParseLandingGear(val);
      }
   }

   else if (defname == "weapon") {
      if (!def->term() || !def->term()->isStruct()) {
         Print("WARNING: weapon struct missing in '%s'\n", filename);
      }
      else {
         TermStruct* val = def->term()->isStruct();
         ParseWeapon(val);
      }
   }

   else if (defname == "hardpoint") {
      if (!def->term() || !def->term()->isStruct()) {
         Print("WARNING: hardpoint struct missing in '%s'\n", filename);
      }
      else {
         TermStruct* val = def->term()->isStruct();
         ParseHardPoint(val);
      }
   }

   else if (defname == "loadout") {
      if (!def->term() || !def->term()->isStruct()) {
         Print("WARNING: loadout struct missing in '%s'\n", filename);
      }
      else {
         TermStruct* val = def->term()->isStruct();
         ParseLoadout(val);
      }
   }

   else if (defname == "decoy") {
      if (!def->term() || !def->term()->isStruct()) {
         Print("WARNING: decoy struct missing in '%s'\n", filename);
      }
      else {
         TermStruct* val = def->term()->isStruct();
         ParseWeapon(val);
      }
   }

   else if (defname == "probe") {
      if (!def->term() || !def->term()->isStruct()) {
         Print("WARNING: probe struct missing in '%s'\n", filename);
      }
      else {
         TermStruct* val = def->term()->isStruct();
         ParseWeapon(val);
      }
   }

   else if (defname == "sensor") {
      if (!def->term() || !def->term()->isStruct()) {
         Print("WARNING: sensor struct missing in '%s'\n", filename);
      }
      else {
         TermStruct* val = def->term()->isStruct();
         ParseSensor(val);
      }
   }

   else if (defname == "nav") {
      if (!def->term() || !def->term()->isStruct()) {
         Print("WARNING: nav struct missing in '%s'\n", filename);
      }
      else {
         TermStruct* val = def->term()->isStruct();
         ParseNavsys(val);
      }
   }

   else if (defname == "computer") {
      if (!def->term() || !def->term()->isStruct()) {
         Print("WARNING: computer struct missing in '%s'\n", filename);
      }
      else {
         TermStruct* val = def->term()->isStruct();
         ParseComputer(val);
      }
   }

   else if (defname == "shield") {
      if (!def->term() || !def->term()->isStruct()) {
         Print("WARNING: shield struct missing in '%s'\n", filename);
      }
      else {
         TermStruct* val = def->term()->isStruct();
         ParseShield(val);
      }
   }

   else if (defname == "death_spiral") {
      if (!def->term() || !def->term()->isStruct()) {
         Print("WARNING: death spiral struct missing in '%s'\n", filename);
      }
      else {
         TermStruct* val = def->term()->isStruct();
         ParseDeathSpiral(val);
      }
   }

   else if (defname == "map") {
      if (!def->term() || !def->term()->isStruct()) {
         Print("WARNING: map struct missing in '%s'\n", filename);
      }
      else {
         TermStruct* val = def->term()->isStruct();
         ParseMap(val);
      }
   }

   else if (defname == "squadron") {
      if (!def->term() || !def->term()->isStruct()) {
         Print("WARNING: squadron struct missing in '%s'\n", filename);
      }
      else {
         TermStruct* val = def->term()->isStruct();
         ParseSquadron(val);
      }
   }

   else if (defname == "skin") {
      if (!def->term() || !def->term()->isStruct()) {
         Print("WARNING: skin struct missing in '%s'\n", filename);
      }
      else {
         TermStruct* val = def->term()->isStruct();
         ParseSkin(val);
      }
   }

   else {
      Print("WARNING: unknown parameter '%s' in '%s'\n",
               defname.data(), filename);
   }

   if (description.length())
      description = Game::GetText(description);
}

// +--------------------------------------------------------------------+

void
ShipDesign::ParsePower(TermStruct* val)
{
   int   stype  = 0;
   float output = 1000.0f;
   float fuel   = 0.0f;
   Vec3  loc(0.0f, 0.0f, 0.0f);
   float size   = 0.0f;
   float hull   = 0.5f;
   Text  design_name;
   Text  pname;
   Text  pabrv;
   int   etype  = 0;
   int   emcon_1 = -1;
   int   emcon_2 = -1;
   int   emcon_3 = -1;

   for (int i = 0; i < val->elements()->size(); i++) {
      TermDef* pdef = val->elements()->at(i)->isDef();
      if (pdef) {
         Text defname = pdef->name()->value();
         defname.setSensitive(false);

         if (defname == "type") {
            TermText* tname = pdef->term()->isText();
            
            if (tname) {
               if      (tname->value()[0] == 'B') stype = PowerSource::BATTERY;
               else if (tname->value()[0] == 'A') stype = PowerSource::AUX;
               else if (tname->value()[0] == 'F') stype = PowerSource::FUSION;
               else Print("WARNING: unknown power source type '%s' in '%s'\n", tname->value().data(), filename);
            }
         }

         else if (defname == "name") {
            GetDefText(pname, pdef, filename);
         }

         else if (defname == "abrv") {
            GetDefText(pabrv, pdef, filename);
         }

         else if (defname == "design") {
            GetDefText(design_name, pdef, filename);
         }

         else if (defname == "max_output") {
            GetDefNumber(output, pdef, filename);
         }
         else if (defname == "fuel_range") {
            GetDefNumber(fuel, pdef, filename);
         }

         else if (defname == "loc") {
            GetDefVec(loc, pdef, filename);
            loc *= (float) scale;
         }
         else if (defname == "size") {
            GetDefNumber(size, pdef, filename);
            size *= (float) scale;
         }
         else if (defname == "hull_factor") {
            GetDefNumber(hull, pdef, filename);
         }

         else if (defname == "explosion") {
            GetDefNumber(etype, pdef, filename);
         }

         else if (defname == "emcon_1") {
            GetDefNumber(emcon_1, pdef, filename);
         }

         else if (defname == "emcon_2") {
            GetDefNumber(emcon_2, pdef, filename);
         }

         else if (defname == "emcon_3") {
            GetDefNumber(emcon_3, pdef, filename);
         }
      }
   }
   
   PowerSource* source = new(__FILE__,__LINE__) PowerSource((PowerSource::SUBTYPE) stype, output);
   if (pname.length()) source->SetName(pname);
   if (pabrv.length()) source->SetName(pabrv);
   source->SetFuelRange(fuel);
   source->Mount(loc, size, hull);
   source->SetExplosionType(etype);

   if (design_name.length()) {
      SystemDesign* sd = SystemDesign::Find(design_name);
      if (sd)
         source->SetDesign(sd);
   }

   if (emcon_1 >= 0 && emcon_1 <= 100)
      source->SetEMCONPower(1, emcon_1);

   if (emcon_2 >= 0 && emcon_2 <= 100)
      source->SetEMCONPower(1, emcon_2);

   if (emcon_3 >= 0 && emcon_3 <= 100)
      source->SetEMCONPower(1, emcon_3);

   reactors.append(source);
}

// +--------------------------------------------------------------------+

void
ShipDesign::ParseDrive(TermStruct* val)
{
   Text  dname;
   Text  dabrv;
   int   dtype  = 0;
   int   etype  = 0;
   float dthrust = 1.0f;
   float daug    = 0.0f;
   float dscale = 1.0f;
   Vec3  loc(0.0f, 0.0f, 0.0f);
   float size   = 0.0f;
   float hull   = 0.5f;
   Text  design_name;
   int   emcon_1 = -1;
   int   emcon_2 = -1;
   int   emcon_3 = -1;
   bool  trail   = true;
   Drive* drive = 0;

   for (int i = 0; i < val->elements()->size(); i++) {
      TermDef* pdef = val->elements()->at(i)->isDef();
      if (pdef) {
         Text defname = pdef->name()->value();
         defname.setSensitive(false);

         if (defname == "type") {
            TermText* tname = pdef->term()->isText();

            if (tname) {
               Text tval = tname->value();
               tval.setSensitive(false);

               if (tval == "Plasma")       dtype = Drive::PLASMA;
               else if (tval == "Fusion")  dtype = Drive::FUSION;
               else if (tval == "Alien")   dtype = Drive::GREEN;
               else if (tval == "Green")   dtype = Drive::GREEN;
               else if (tval == "Red")     dtype = Drive::RED;
               else if (tval == "Blue")    dtype = Drive::BLUE;
               else if (tval == "Yellow")  dtype = Drive::YELLOW;
               else if (tval == "Stealth") dtype = Drive::STEALTH;

               else Print("WARNING: unknown drive type '%s' in '%s'\n", tname->value().data(), filename);
            }
         }
         else if (defname == "name") {
            if (!GetDefText(dname, pdef, filename))
               Print("WARNING: invalid or missing name for drive in '%s'\n", filename);
         }

         else if (defname == "abrv") {
            if (!GetDefText(dabrv, pdef, filename))
               Print("WARNING: invalid or missing abrv for drive in '%s'\n", filename);
         }

         else if (defname == "design") {
            if (!GetDefText(design_name, pdef, filename))
               Print("WARNING: invalid or missing design for drive in '%s'\n", filename);
         }

         else if (defname == "thrust") {
            if (!GetDefNumber(dthrust, pdef, filename))
               Print("WARNING: invalid or missing thrust for drive in '%s'\n", filename);
         }

         else if (defname == "augmenter") {
            if (!GetDefNumber(daug, pdef, filename))
               Print("WARNING: invalid or missing augmenter for drive in '%s'\n", filename);
         }

         else if (defname == "scale") {
            if (!GetDefNumber(dscale, pdef, filename))
               Print("WARNING: invalid or missing scale for drive in '%s'\n", filename);
         }

         else if (defname == "port") {
            Vec3  port;
            float flare_scale = 0;

            if (pdef->term()->isArray()) {
               GetDefVec(port, pdef, filename);
               port *= scale;
               flare_scale = dscale;
            }

            else if (pdef->term()->isStruct()) {
               TermStruct* val = pdef->term()->isStruct();

               for (int i = 0; i < val->elements()->size(); i++) {
                  TermDef* pdef2 = val->elements()->at(i)->isDef();
                  if (pdef2) {
                     if (pdef2->name()->value() == "loc") {
                        GetDefVec(port, pdef2, filename);
                        port *= scale;
                     }

                     else if (pdef2->name()->value() == "scale") {
                        GetDefNumber(flare_scale, pdef2, filename);
                     }
                  }
               }

               if (flare_scale <= 0)
                  flare_scale = dscale;
            }

            if (!drive)
               drive = new(__FILE__,__LINE__) Drive((Drive::SUBTYPE) dtype, dthrust, daug, trail);

            drive->AddPort(port, flare_scale);
         }
         
         else if (defname == "loc") {
            if (!GetDefVec(loc, pdef, filename))
               Print("WARNING: invalid or missing loc for drive in '%s'\n", filename);
            loc *= (float) scale;
         }
         
         else if (defname == "size") {
            if (!GetDefNumber(size, pdef, filename))
               Print("WARNING: invalid or missing size for drive in '%s'\n", filename);
            size *= (float) scale;
         }

         else if (defname == "hull_factor") {
            if (!GetDefNumber(hull, pdef, filename))
               Print("WARNING: invalid or missing hull_factor for drive in '%s'\n", filename);
         }

         else if (defname == "explosion") {
            if (!GetDefNumber(etype, pdef, filename))
               Print("WARNING: invalid or missing explosion for drive in '%s'\n", filename);
         }

         else if (defname == "emcon_1") {
            GetDefNumber(emcon_1, pdef, filename);
         }

         else if (defname == "emcon_2") {
            GetDefNumber(emcon_2, pdef, filename);
         }

         else if (defname == "emcon_3") {
            GetDefNumber(emcon_3, pdef, filename);
         }

         else if (defname == "trail" || defname == "show_trail") {
            GetDefBool(trail, pdef, filename);
         }
      }
   }

   if (!drive)
      drive = new(__FILE__,__LINE__) Drive((Drive::SUBTYPE) dtype, dthrust, daug, trail);

   drive->SetSourceIndex(reactors.size()-1);
   drive->Mount(loc, size, hull);
   if (dname.length()) drive->SetName(dname);
   if (dabrv.length()) drive->SetAbbreviation(dabrv);
   drive->SetExplosionType(etype);

   if (design_name.length()) {
      SystemDesign* sd = SystemDesign::Find(design_name);
      if (sd)
         drive->SetDesign(sd);
   }

   if (emcon_1 >= 0 && emcon_1 <= 100)
      drive->SetEMCONPower(1, emcon_1);

   if (emcon_2 >= 0 && emcon_2 <= 100)
      drive->SetEMCONPower(1, emcon_2);

   if (emcon_3 >= 0 && emcon_3 <= 100)
      drive->SetEMCONPower(1, emcon_3);

   main_drive = drives.size();
   drives.append(drive);
}

// +--------------------------------------------------------------------+

void
ShipDesign::ParseQuantumDrive(TermStruct* val)
{
   double   capacity    = 250e3;
   double   consumption = 1e3;
   Vec3     loc(0.0f, 0.0f, 0.0f);
   float    size        = 0.0f;
   float    hull        = 0.5f;
   float    countdown   = 5.0f;
   Text     design_name;
   Text     type_name;
   Text     abrv;
   int      subtype = QuantumDrive::QUANTUM;
   int      emcon_1 = -1;
   int      emcon_2 = -1;
   int      emcon_3 = -1;

   for (int i = 0; i < val->elements()->size(); i++) {
      TermDef* pdef = val->elements()->at(i)->isDef();
      if (pdef) {
         Text defname = pdef->name()->value();
         defname.setSensitive(false);

         if (defname == "design") {
            GetDefText(design_name, pdef, filename);
         }
         else if (defname == "abrv") {
            GetDefText(abrv, pdef, filename);
         }
         else if (defname == "type") {
            GetDefText(type_name, pdef, filename);
            type_name.setSensitive(false);

            if (type_name.contains("hyper")) {
               subtype = QuantumDrive::HYPER;
            }
         }
         else if (defname == "capacity") {
            GetDefNumber(capacity, pdef, filename);
         }
         else if (defname == "consumption") {
            GetDefNumber(consumption, pdef, filename);
         }
         else if (defname == "loc") {
            GetDefVec(loc, pdef, filename);
            loc *= (float) scale;
         }
         else if (defname == "size") {
            GetDefNumber(size, pdef, filename);
            size *= (float) scale;
         }
         else if (defname == "hull_factor") {
            GetDefNumber(hull, pdef, filename);
         }
         else if (defname == "jump_time") {
            GetDefNumber(countdown, pdef, filename);
         }
         else if (defname == "countdown") {
            GetDefNumber(countdown, pdef, filename);
         }

         else if (defname == "emcon_1") {
            GetDefNumber(emcon_1, pdef, filename);
         }

         else if (defname == "emcon_2") {
            GetDefNumber(emcon_2, pdef, filename);
         }

         else if (defname == "emcon_3") {
            GetDefNumber(emcon_3, pdef, filename);
         }
      }
   }

   QuantumDrive* drive = new(__FILE__,__LINE__) QuantumDrive((QuantumDrive::SUBTYPE) subtype, capacity, consumption);
   drive->SetSourceIndex(reactors.size()-1);
   drive->Mount(loc, size, hull);
   drive->SetCountdown(countdown);

   if (design_name.length()) {
      SystemDesign* sd = SystemDesign::Find(design_name);
      if (sd)
         drive->SetDesign(sd);
   }

   if (abrv.length())
      drive->SetAbbreviation(abrv);

   if (emcon_1 >= 0 && emcon_1 <= 100)
      drive->SetEMCONPower(1, emcon_1);

   if (emcon_2 >= 0 && emcon_2 <= 100)
      drive->SetEMCONPower(1, emcon_2);

   if (emcon_3 >= 0 && emcon_3 <= 100)
      drive->SetEMCONPower(1, emcon_3);

   quantum_drive = drive;
}

// +--------------------------------------------------------------------+

void
ShipDesign::ParseFarcaster(TermStruct* val)
{
   Text     design_name;
   double   capacity    = 300e3;
   double   consumption =  15e3;  // twenty second recharge
   int      napproach   =     0;
   Vec3     approach[Farcaster::NUM_APPROACH_PTS];
   Vec3     loc(0.0f, 0.0f, 0.0f);
   Vec3     start(0.0f, 0.0f, 0.0f);
   Vec3     end(0.0f, 0.0f, 0.0f);
   float    size        = 0.0f;
   float    hull        = 0.5f;
   int      emcon_1     = -1;
   int      emcon_2     = -1;
   int      emcon_3     = -1;

   for (int i = 0; i < val->elements()->size(); i++) {
      TermDef* pdef = val->elements()->at(i)->isDef();
      if (pdef) {
         Text defname = pdef->name()->value();
         defname.setSensitive(false);

         if (defname == "design") {
            GetDefText(design_name, pdef, filename);
         }
         else if (defname == "capacity") {
            GetDefNumber(capacity, pdef, filename);
         }
         else if (defname == "consumption") {
            GetDefNumber(consumption, pdef, filename);
         }
         else if (defname == "loc") {
            GetDefVec(loc, pdef, filename);
            loc *= (float) scale;
         }
         else if (defname == "size") {
            GetDefNumber(size, pdef, filename);
            size *= (float) scale;
         }
         else if (defname == "hull_factor") {
            GetDefNumber(hull, pdef, filename);
         }

         else if (defname == "start") {
            GetDefVec(start, pdef, filename);
            start *= (float) scale;
         }
         else if (defname == "end") {
            GetDefVec(end, pdef, filename);
            end *= (float) scale;
         }
         else if (defname == "approach") {
            if (napproach < Farcaster::NUM_APPROACH_PTS) {
               GetDefVec(approach[napproach], pdef, filename);
               approach[napproach++] *= (float) scale;
            }
            else {
               Print("WARNING: farcaster approach point ignored in '%s' (max=%d)\n",
                  filename, Farcaster::NUM_APPROACH_PTS);
            }
         }

         else if (defname == "emcon_1") {
            GetDefNumber(emcon_1, pdef, filename);
         }

         else if (defname == "emcon_2") {
            GetDefNumber(emcon_2, pdef, filename);
         }

         else if (defname == "emcon_3") {
            GetDefNumber(emcon_3, pdef, filename);
         }
      }
   }

   Farcaster* caster = new(__FILE__,__LINE__) Farcaster(capacity, consumption);
   caster->SetSourceIndex(reactors.size()-1);
   caster->Mount(loc, size, hull);

   if (design_name.length()) {
      SystemDesign* sd = SystemDesign::Find(design_name);
      if (sd)
         caster->SetDesign(sd);
   }

   caster->SetStartPoint(start);
   caster->SetEndPoint(end);

   for (i = 0; i < napproach; i++)
      caster->SetApproachPoint(i, approach[i]);

   if (emcon_1 >= 0 && emcon_1 <= 100)
      caster->SetEMCONPower(1, emcon_1);

   if (emcon_2 >= 0 && emcon_2 <= 100)
      caster->SetEMCONPower(1, emcon_2);

   if (emcon_3 >= 0 && emcon_3 <= 100)
      caster->SetEMCONPower(1, emcon_3);

   farcaster = caster;
}

// +--------------------------------------------------------------------+

void
ShipDesign::ParseThruster(TermStruct* val)
{
   if (thruster) {
      Print("WARNING: additional thruster ignored in '%s'\n", filename);
      return;
   }

   double thrust   = 100;

   Vec3  loc(0.0f, 0.0f, 0.0f);
   float size   = 0.0f;
   float hull   = 0.5f;
   Text  design_name;
   float tscale = 1.0f;
   int   emcon_1 = -1;
   int   emcon_2 = -1;
   int   emcon_3 = -1;
   int   dtype   = 0;

   Thruster* drive = 0;

   for (int i = 0; i < val->elements()->size(); i++) {
      TermDef* pdef = val->elements()->at(i)->isDef();
      if (pdef) {
         Text defname = pdef->name()->value();
         defname.setSensitive(false);


         if (defname == "type") {
            TermText* tname = pdef->term()->isText();

            if (tname) {
               Text tval = tname->value();
               tval.setSensitive(false);

               if (tval == "Plasma")       dtype = Drive::PLASMA;
               else if (tval == "Fusion")  dtype = Drive::FUSION;
               else if (tval == "Alien")   dtype = Drive::GREEN;
               else if (tval == "Green")   dtype = Drive::GREEN;
               else if (tval == "Red")     dtype = Drive::RED;
               else if (tval == "Blue")    dtype = Drive::BLUE;
               else if (tval == "Yellow")  dtype = Drive::YELLOW;
               else if (tval == "Stealth") dtype = Drive::STEALTH;

               else Print("WARNING: unknown thruster type '%s' in '%s'\n", tname->value().data(), filename);
            }
         }

         else if (defname == "thrust") {
            GetDefNumber(thrust, pdef, filename);
         }

         else if (defname == "design") {
            GetDefText(design_name, pdef, filename);
         }

         else if (defname == "loc") {
            GetDefVec(loc, pdef, filename);
            loc *= (float) scale;
         }
         else if (defname == "size") {
            GetDefNumber(size, pdef, filename);
            size *= (float) scale;
         }
         else if (defname == "hull_factor") {
            GetDefNumber(hull, pdef, filename);
         }
         else if (defname == "scale") {
            GetDefNumber(tscale, pdef, filename);
         }
         else if (defname.contains("port") && pdef->term()) {
            Vec3  port;
            float port_scale = 0;
            DWORD fire = 0;

            if (pdef->term()->isArray()) {
               GetDefVec(port, pdef, filename);
               port *= scale;
               port_scale = tscale;
            }

            else if (pdef->term()->isStruct()) {
               TermStruct* val = pdef->term()->isStruct();

               for (int i = 0; i < val->elements()->size(); i++) {
                  TermDef* pdef2 = val->elements()->at(i)->isDef();
                  if (pdef2) {
                     if (pdef2->name()->value() == "loc") {
                        GetDefVec(port, pdef2, filename);
                        port *= scale;
                     }

                     else if (pdef2->name()->value() == "fire") {
                        GetDefNumber(fire, pdef2, filename);
                     }

                     else if (pdef2->name()->value() == "scale") {
                        GetDefNumber(port_scale, pdef2, filename);
                     }
                  }
               }

               if (port_scale <= 0)
                  port_scale = tscale;
            }

            if (!drive)
               drive = new(__FILE__,__LINE__) Thruster(dtype, thrust, tscale);

            if (defname == "port" || defname == "port_bottom")
               drive->AddPort(Thruster::BOTTOM, port, fire, port_scale);

            else if (defname == "port_top")
               drive->AddPort(Thruster::TOP, port, fire, port_scale);

            else if (defname == "port_left")
               drive->AddPort(Thruster::LEFT, port, fire, port_scale);

            else if (defname == "port_right")
               drive->AddPort(Thruster::RIGHT, port, fire, port_scale);

            else if (defname == "port_fore")
               drive->AddPort(Thruster::FORE, port, fire, port_scale);

            else if (defname == "port_aft")
               drive->AddPort(Thruster::AFT, port, fire, port_scale);
         }

         else if (defname == "emcon_1") {
            GetDefNumber(emcon_1, pdef, filename);
         }

         else if (defname == "emcon_2") {
            GetDefNumber(emcon_2, pdef, filename);
         }

         else if (defname == "emcon_3") {
            GetDefNumber(emcon_3, pdef, filename);
         }
      }
   }

   if (!drive)
      drive = new(__FILE__,__LINE__) Thruster(dtype, thrust, tscale);
   drive->SetSourceIndex(reactors.size()-1);
   drive->Mount(loc, size, hull);

   if (design_name.length()) {
      SystemDesign* sd = SystemDesign::Find(design_name);
      if (sd)
         drive->SetDesign(sd);
   }

   if (emcon_1 >= 0 && emcon_1 <= 100)
      drive->SetEMCONPower(1, emcon_1);

   if (emcon_2 >= 0 && emcon_2 <= 100)
      drive->SetEMCONPower(1, emcon_2);

   if (emcon_3 >= 0 && emcon_3 <= 100)
      drive->SetEMCONPower(1, emcon_3);

   thruster = drive;
}

// +--------------------------------------------------------------------+

void
ShipDesign::ParseNavlight(TermStruct* val)
{
   Text  dname;
   Text  dabrv;
   Text  design_name;
   int   nlights = 0;
   float dscale = 1.0f;
   float period = 10.0f;
   Vec3  bloc[NavLight::MAX_LIGHTS];
   int   btype[NavLight::MAX_LIGHTS];
   DWORD pattern[NavLight::MAX_LIGHTS];

   for (int i = 0; i < val->elements()->size(); i++) {
      TermDef* pdef = val->elements()->at(i)->isDef();
      if (pdef) {
         Text defname = pdef->name()->value();
         defname.setSensitive(false);

         if (defname == "name")
            GetDefText(dname, pdef, filename);
         else if (defname == "abrv")
            GetDefText(dabrv, pdef, filename);

         else if (defname == "design") {
            GetDefText(design_name, pdef, filename);
         }

         else if (defname == "scale") {
            GetDefNumber(dscale, pdef, filename);
         }
         else if (defname == "period") {
            GetDefNumber(period, pdef, filename);
         }
         else if (defname == "light") {
            if (!pdef->term() || !pdef->term()->isStruct()) {
               Print("WARNING: light struct missing for ship '%s' in '%s'\n", name, filename);
            }
            else {
               TermStruct* val = pdef->term()->isStruct();

               Vec3  loc;
               int   t     = 0;
               DWORD ptn   = 0;

               for (int i = 0; i < val->elements()->size(); i++) {
                  TermDef* pdef = val->elements()->at(i)->isDef();
                  if (pdef) {
                     Text defname = pdef->name()->value();
                     defname.setSensitive(false);

                     if (defname == "type") {
                        GetDefNumber(t, pdef, filename);
                     }
                     else if (defname == "loc") {
                        GetDefVec(loc, pdef, filename);
                     }
                     else if (defname == "pattern") {
                        GetDefNumber(ptn, pdef, filename);
                     }
                  }
               }

               if (t < 1 || t > 4)
                  t = 1;

               if (nlights < NavLight::MAX_LIGHTS) {
                  bloc[nlights]    = loc * scale;
                  btype[nlights]   = t-1;
                  pattern[nlights] = ptn;
                  nlights++;
               }
               else {
                  Print("WARNING: Too many lights ship '%s' in '%s'\n", name, filename);
               }
            }
         }
      }
   }

   NavLight* nav = new(__FILE__,__LINE__) NavLight(period, dscale);
   if (dname.length()) nav->SetName(dname);
   if (dabrv.length()) nav->SetAbbreviation(dabrv);

   if (design_name.length()) {
      SystemDesign* sd = SystemDesign::Find(design_name);
      if (sd)
         nav->SetDesign(sd);
   }

   for (i = 0; i < nlights; i++)
      nav->AddBeacon(bloc[i], pattern[i], btype[i]);

   navlights.append(nav);
}

// +--------------------------------------------------------------------+

void
ShipDesign::ParseFlightDeck(TermStruct* val)
{
   Text  dname;
   Text  dabrv;
   Text  design_name;
   float dscale = 1.0f;
   float az     = 0.0f;
   int   etype  = 0;

   bool  launch   = false;
   bool  recovery = false;
   int   nslots   = 0;
   int   napproach = 0;
   int   nrunway  = 0;
   DWORD filters[10];
   Vec3  spots[10];
   Vec3  approach[FlightDeck::NUM_APPROACH_PTS];
   Vec3  runway[2];
   Vec3  loc(0,0,0);
   Vec3  start(0,0,0);
   Vec3  end(0,0,0);
   Vec3  cam(0,0,0);
   Vec3  box(0,0,0);
   float cycle_time  = 0.0f;
   float size        = 0.0f;
   float hull        = 0.5f;

   float light       = 0.0f;

   for (int i = 0; i < val->elements()->size(); i++) {
      TermDef* pdef = val->elements()->at(i)->isDef();
      if (pdef) {
         Text defname = pdef->name()->value();
         defname.setSensitive(false);

         if (defname == "name")
            GetDefText(dname, pdef, filename);
         else if (defname == "abrv")
            GetDefText(dabrv, pdef, filename);
         else if (defname == "design")
            GetDefText(design_name, pdef, filename);

         else if (defname == "start") {
            GetDefVec(start, pdef, filename);
            start *= (float) scale;
         }
         else if (defname == "end") {
            GetDefVec(end, pdef, filename);
            end *= (float) scale;
         }
         else if (defname == "cam") {
            GetDefVec(cam, pdef, filename);
            cam *= (float) scale;
         }
         else if (defname == "box" || defname == "bounding_box") {
            GetDefVec(box, pdef, filename);
            box *= (float) scale;
         }
         else if (defname == "approach") {
            if (napproach < FlightDeck::NUM_APPROACH_PTS) {
               GetDefVec(approach[napproach], pdef, filename);
               approach[napproach++] *= (float) scale;
            }
            else {
               Print("WARNING: flight deck approach point ignored in '%s' (max=%d)\n",
                  filename, FlightDeck::NUM_APPROACH_PTS);
            }
         }
         else if (defname == "runway") {
            GetDefVec(runway[nrunway], pdef, filename);
            runway[nrunway++] *= (float) scale;
         }
         else if (defname == "spot") {
            if (pdef->term()->isStruct()) {
               TermStruct* s = pdef->term()->isStruct();
               for (int i = 0; i < s->elements()->size(); i++) {
                  TermDef* d = s->elements()->at(i)->isDef();
                  if (d) {
                     if (d->name()->value() == "loc") {
                        GetDefVec(spots[nslots], d, filename);
                        spots[nslots] *= (float) scale;
                     }
                     else if (d->name()->value() == "filter") {
                        GetDefNumber(filters[nslots], d, filename);
                     }
                  }
               }

               nslots++;
            }

            else if (pdef->term()->isArray()) {
               GetDefVec(spots[nslots], pdef, filename);
               spots[nslots] *= (float) scale;
               filters[nslots++] = 0xf;
            }
         }

         else if (defname == "light") {
            GetDefNumber(light, pdef, filename);
         }

         else if (defname == "cycle_time") {
            GetDefNumber(cycle_time, pdef, filename);
         }

         else if (defname == "launch") {
            GetDefBool(launch, pdef, filename);
         }

         else if (defname == "recovery") {
            GetDefBool(recovery, pdef, filename);
         }

         else if (defname == "azimuth") {
            GetDefNumber(az, pdef, filename);
            if (degrees) az *= (float) DEGREES;
         }

         else if (defname == "loc") {
            GetDefVec(loc, pdef, filename);
            loc *= (float) scale;
         }
         else if (defname == "size") {
            GetDefNumber(size, pdef, filename);
            size *= (float) scale;
         }
         else if (defname == "hull_factor") {
            GetDefNumber(hull, pdef, filename);
         }
         else if (defname == "explosion") {
            GetDefNumber(etype, pdef, filename);
         }
      }
   }

   FlightDeck* deck = new(__FILE__,__LINE__) FlightDeck();
   deck->Mount(loc, size, hull);
   if (dname.length()) deck->SetName(dname);
   if (dabrv.length()) deck->SetAbbreviation(dabrv);

   if (design_name.length()) {
      SystemDesign* sd = SystemDesign::Find(design_name);
      if (sd)
         deck->SetDesign(sd);
   }

   if (launch)
      deck->SetLaunchDeck();
   else if (recovery)
      deck->SetRecoveryDeck();

   deck->SetAzimuth(az);
   deck->SetBoundingBox(box);
   deck->SetStartPoint(start);
   deck->SetEndPoint(end);
   deck->SetCamLoc(cam);
   deck->SetExplosionType(etype);

   if (light > 0)
      deck->SetLight(light);

   for (i = 0; i < napproach; i++)
      deck->SetApproachPoint(i, approach[i]);

   for (i = 0; i < nrunway; i++)
      deck->SetRunwayPoint(i, runway[i]);

   for (i = 0; i < nslots; i++)
      deck->AddSlot(spots[i], filters[i]);

   if (cycle_time > 0)
      deck->SetCycleTime(cycle_time);

   flight_decks.append(deck);
}

// +--------------------------------------------------------------------+

void
ShipDesign::ParseLandingGear(TermStruct* val)
{
   Text  dname;
   Text  dabrv;
   Text  design_name;
   int   ngear  = 0;
   Vec3  start[LandingGear::MAX_GEAR];
   Vec3  end[LandingGear::MAX_GEAR];
   Model* model[LandingGear::MAX_GEAR];

   for (int i = 0; i < val->elements()->size(); i++) {
      TermDef* pdef = val->elements()->at(i)->isDef();
      if (pdef) {
         Text defname = pdef->name()->value();
         defname.setSensitive(false);

         if (defname == "name")
            GetDefText(dname, pdef, filename);
         else if (defname == "abrv")
            GetDefText(dabrv, pdef, filename);

         else if (defname == "design") {
            GetDefText(design_name, pdef, filename);
         }

         else if (defname == "gear") {
            if (!pdef->term() || !pdef->term()->isStruct()) {
               Print("WARNING: gear struct missing for ship '%s' in '%s'\n", name, filename);
            }
            else {
               TermStruct* val = pdef->term()->isStruct();

               Vec3  v1, v2;
               char  mod_name[256];

               ZeroMemory(mod_name, sizeof(mod_name));

               for (int i = 0; i < val->elements()->size(); i++) {
                  TermDef* pdef = val->elements()->at(i)->isDef();
                  if (pdef) {
                     defname = pdef->name()->value();
                     defname.setSensitive(false);

                     if (defname == "model") {
                        GetDefText(mod_name, pdef, filename);
                     }
                     else if (defname == "start") {
                        GetDefVec(v1, pdef, filename);
                     }
                     else if (defname == "end") {
                        GetDefVec(v2, pdef, filename);
                     }
                  }
               }
               
               if (ngear < LandingGear::MAX_GEAR) {
                  Model* m = new(__FILE__,__LINE__) Model;
                  if (!m->Load(mod_name, scale)) {
                     Print("WARNING: Could not load landing gear model '%s'\n", mod_name);
                     delete m;
                     m = 0;
                  }
                  else {
                     model[ngear] = m;
                     start[ngear] = v1 * scale;
                     end[ngear]   = v2 * scale;
                     ngear++;
                  }
               }
               else {
                  Print("WARNING: Too many landing gear ship '%s' in '%s'\n", name, filename);
               }
            }
         }
      }
   }

   gear = new(__FILE__,__LINE__) LandingGear();
   if (dname.length()) gear->SetName(dname);
   if (dabrv.length()) gear->SetAbbreviation(dabrv);

   if (design_name.length()) {
      SystemDesign* sd = SystemDesign::Find(design_name);
      if (sd)
         gear->SetDesign(sd);
   }

   for (i = 0; i < ngear; i++)
      gear->AddGear(model[i], start[i], end[i]);
}

// +--------------------------------------------------------------------+

void
ShipDesign::ParseWeapon(TermStruct* val)
{
   Text  wtype;
   Text  wname;
   Text  wabrv;
   Text  design_name;
   Text  group_name;
   int   nmuz = 0;
   Vec3  muzzles[Weapon::MAX_BARRELS];
   Vec3  loc(0.0f, 0.0f, 0.0f);
   float size   = 0.0f;
   float hull   = 0.5f;
   float az     = 0.0f;
   float el     = 0.0f;
   float az_max  = 1e6f;
   float az_min  = 1e6f;
   float el_max  = 1e6f;
   float el_min  = 1e6f;
   float az_rest = 1e6f;
   float el_rest = 1e6f;
   int   etype  = 0;
   int   emcon_1 = -1;
   int   emcon_2 = -1;
   int   emcon_3 = -1;

   for (int i = 0; i < val->elements()->size(); i++) {
      TermDef* pdef = val->elements()->at(i)->isDef();
      if (pdef) {
         Text defname = pdef->name()->value();
         defname.setSensitive(false);

         if (defname == "type")
            GetDefText(wtype, pdef, filename);
         else if (defname == "name")
            GetDefText(wname, pdef, filename);
         else if (defname == "abrv")
            GetDefText(wabrv, pdef, filename);
         else if (defname == "design")
            GetDefText(design_name, pdef, filename);
         else if (defname == "group")
            GetDefText(group_name, pdef, filename);

         else if (defname == "muzzle") {
            if (nmuz < Weapon::MAX_BARRELS) {
               GetDefVec(muzzles[nmuz], pdef, filename);
               nmuz++;
            }
            else {
               Print("WARNING: too many muzzles (max=%d) for weapon in '%s'\n", filename, Weapon::MAX_BARRELS);
            }
         }
         else if (defname == "loc") {
            GetDefVec(loc, pdef, filename);
            loc *= (float) scale;
         }
         else if (defname == "size") {
            GetDefNumber(size, pdef, filename);
            size *= (float) scale;
         }
         else if (defname == "hull_factor") {
            GetDefNumber(hull, pdef, filename);
         }
         else if (defname == "azimuth") {
            GetDefNumber(az, pdef, filename);
            if (degrees) az *= (float) DEGREES;
         }
         else if (defname == "elevation") {
            GetDefNumber(el, pdef, filename);
            if (degrees) el *= (float) DEGREES;
         }

         else if (defname==("aim_az_max")) {
            GetDefNumber(az_max,pdef,filename);
            if (degrees) az_max *= (float) DEGREES;
            az_min = 0.0f - az_max;
         }

         else if (defname==("aim_el_max")) {
            GetDefNumber(el_max,pdef,filename);
            if (degrees) el_max *= (float) DEGREES;
            el_min = 0.0f - el_max;
         }

         else if (defname==("aim_az_min")) {
            GetDefNumber(az_min,pdef,filename);
            if (degrees) az_min *= (float) DEGREES;
         }

         else if (defname==("aim_el_min")) {
            GetDefNumber(el_min,pdef,filename);
            if (degrees) el_min *= (float) DEGREES;
         }

         else if (defname==("aim_az_rest")) {
            GetDefNumber(az_rest,pdef,filename);
            if (degrees) az_rest *= (float) DEGREES;
         }

         else if (defname==("aim_el_rest")) {
            GetDefNumber(el_rest,pdef,filename);
            if (degrees) el_rest *= (float) DEGREES;
         }

         else if (defname == "rest_azimuth") {
            GetDefNumber(az_rest, pdef, filename);
            if (degrees) az_rest *= (float) DEGREES;
         }
         else if (defname == "rest_elevation") {
            GetDefNumber(el_rest, pdef, filename);
            if (degrees) el_rest *= (float) DEGREES;
         }
         else if (defname == "explosion") {
            GetDefNumber(etype, pdef, filename);
         }

         else if (defname == "emcon_1") {
            GetDefNumber(emcon_1, pdef, filename);
         }

         else if (defname == "emcon_2") {
            GetDefNumber(emcon_2, pdef, filename);
         }

         else if (defname == "emcon_3") {
            GetDefNumber(emcon_3, pdef, filename);
         }
         else {
            Print("WARNING: unknown weapon parameter '%s' in '%s'\n",
                  defname.data(), filename);
         }
      }
   }

   WeaponDesign* meta = WeaponDesign::Find(wtype);
   if (!meta) {
      Print("WARNING: unusual weapon name '%s' in '%s'\n", (const char*) wtype, filename);
   }
   else {
      // non-turret weapon muzzles are relative to ship scale:
      if (meta->turret_model == 0) {
         for (int i = 0; i < nmuz; i++)
            muzzles[i] *= (float) scale;
      }

      // turret weapon muzzles are relative to weapon scale:
      else {
         for (int i = 0; i < nmuz; i++)
            muzzles[i] *= (float) meta->scale;
      }

      Weapon* gun = new(__FILE__,__LINE__) Weapon(meta, nmuz, muzzles, az, el);
      gun->SetSourceIndex(reactors.size()-1);
      gun->Mount(loc, size, hull);

      if (az_max < 1e6)    gun->SetAzimuthMax(az_max);
      if (az_min < 1e6)    gun->SetAzimuthMin(az_min);
      if (az_rest < 1e6)   gun->SetRestAzimuth(az_rest);

      if (el_max < 1e6)    gun->SetElevationMax(el_max);
      if (el_min < 1e6)    gun->SetElevationMin(el_min);
      if (el_rest < 1e6)   gun->SetRestElevation(el_rest);

      if (emcon_1 >= 0 && emcon_1 <= 100)
         gun->SetEMCONPower(1, emcon_1);

      if (emcon_2 >= 0 && emcon_2 <= 100)
         gun->SetEMCONPower(1, emcon_2);

      if (emcon_3 >= 0 && emcon_3 <= 100)
         gun->SetEMCONPower(1, emcon_3);

      if (wname.length()) gun->SetName(wname);
      if (wabrv.length()) gun->SetAbbreviation(wabrv);

      if (design_name.length()) {
         SystemDesign* sd = SystemDesign::Find(design_name);
         if (sd)
            gun->SetDesign(sd);
      }

      if (group_name.length()) {
         gun->SetGroup(group_name);
      }

      gun->SetExplosionType(etype);

      if (meta->decoy_type && !decoy)
         decoy = gun;
      else if (meta->probe && !probe)
         probe = gun;
      else
         weapons.append(gun);
   }

   DataLoader* loader = DataLoader::GetLoader();
   loader->SetDataPath(path_name);
}

// +--------------------------------------------------------------------+

void
ShipDesign::ParseHardPoint(TermStruct* val)
{
   Text  wtypes[8];
   Text  wname;
   Text  wabrv;
   Text  design;
   Vec3  muzzle;
   Vec3  loc(0.0f, 0.0f, 0.0f);
   float size   = 0.0f;
   float hull   = 0.5f;
   float az     = 0.0f;
   float el     = 0.0f;
   int   ntypes = 0;
   int   emcon_1 = -1;
   int   emcon_2 = -1;
   int   emcon_3 = -1;

   for (int i = 0; i < val->elements()->size(); i++) {
      TermDef* pdef = val->elements()->at(i)->isDef();
      if (pdef) {
         Text defname = pdef->name()->value();
         defname.setSensitive(false);

         if (defname == "type")
            GetDefText(wtypes[ntypes++], pdef, filename);
         else if (defname == "name")
            GetDefText(wname, pdef, filename);
         else if (defname == "abrv")
            GetDefText(wabrv, pdef, filename);
         else if (defname == "design")
            GetDefText(design, pdef, filename);

         else if (defname == "muzzle") {
            GetDefVec(muzzle, pdef, filename);
            muzzle *= (float) scale;
         }
         else if (defname == "loc") {
            GetDefVec(loc, pdef, filename);
            loc *= (float) scale;
         }
         else if (defname == "size") {
            GetDefNumber(size, pdef, filename);
            size *= (float) scale;
         }
         else if (defname == "hull_factor") {
            GetDefNumber(hull, pdef, filename);
         }
         else if (defname == "azimuth") {
            GetDefNumber(az, pdef, filename);
            if (degrees) az *= (float) DEGREES;
         }
         else if (defname == "elevation") {
            GetDefNumber(el, pdef, filename);
            if (degrees) el *= (float) DEGREES;
         }

         else if (defname == "emcon_1") {
            GetDefNumber(emcon_1, pdef, filename);
         }

         else if (defname == "emcon_2") {
            GetDefNumber(emcon_2, pdef, filename);
         }

         else if (defname == "emcon_3") {
            GetDefNumber(emcon_3, pdef, filename);
         }
         else {
            Print("WARNING: unknown weapon parameter '%s' in '%s'\n",
                  defname.data(), filename);
         }
      }
   }

   HardPoint* hp = new(__FILE__,__LINE__) HardPoint(muzzle, az, el);
   if (hp) {
      for (int i = 0; i < ntypes; i++) {
         WeaponDesign* meta = WeaponDesign::Find(wtypes[i]);
         if (!meta) {
            Print("WARNING: unusual weapon name '%s' in '%s'\n", (const char*) wtypes[i], filename);
         }
         else {
            hp->AddDesign(meta);
         }
      }

      hp->Mount(loc, size, hull);
      if (wname.length())  hp->SetName(wname);
      if (wabrv.length())  hp->SetAbbreviation(wabrv);
      if (design.length()) hp->SetDesign(design);

      hard_points.append(hp);
   }

   DataLoader* loader = DataLoader::GetLoader();
   loader->SetDataPath(path_name);
}

// +--------------------------------------------------------------------+

void
ShipDesign::ParseLoadout(TermStruct* val)
{
   ShipLoad* load = new(__FILE__,__LINE__) ShipLoad;

   if (!load) return;

   for (int i = 0; i < val->elements()->size(); i++) {
      TermDef* pdef = val->elements()->at(i)->isDef();
      if (pdef) {
         Text defname = pdef->name()->value();
         defname.setSensitive(false);

         if (defname == "name")
            GetDefText(load->name, pdef, filename);

         else if (defname == "stations")
            GetDefArray(load->load, 16, pdef, filename);

         else
            Print("WARNING: unknown loadout parameter '%s' in '%s'\n",
                  defname.data(), filename);
      }
   }

   loadouts.append(load);
}

// +--------------------------------------------------------------------+

void
ShipDesign::ParseSensor(TermStruct* val)
{
   Text  design_name;
   Vec3  loc(0.0f, 0.0f, 0.0f);
   float size   = 0.0f;
   float hull   = 0.5f;
   int   nranges = 0;
   float ranges[8];
   int   emcon_1 = -1;
   int   emcon_2 = -1;
   int   emcon_3 = -1;

   ZeroMemory(ranges, sizeof(ranges));

   for (int i = 0; i < val->elements()->size(); i++) {
      TermDef* pdef = val->elements()->at(i)->isDef();
      if (pdef) {
         Text defname = pdef->name()->value();
         defname.setSensitive(false);

         if (defname == "range") {
            GetDefNumber(ranges[nranges++], pdef, filename);
         }
         else if (defname == "loc") {
            GetDefVec(loc, pdef, filename);
            loc *= (float) scale;
         }
         else if (defname == "size") {
            size *= (float) scale;
            GetDefNumber(size, pdef, filename);
         }
         else if (defname == "hull_factor") {
            GetDefNumber(hull, pdef, filename);
         }
         else if (defname == "design") {
            GetDefText(design_name, pdef, filename);
         }
         else if (defname == "emcon_1") {
            GetDefNumber(emcon_1, pdef, filename);
         }

         else if (defname == "emcon_2") {
            GetDefNumber(emcon_2, pdef, filename);
         }

         else if (defname == "emcon_3") {
            GetDefNumber(emcon_3, pdef, filename);
         }
      }
   }

   if (!sensor) {
      sensor = new(__FILE__,__LINE__) Sensor();

      if (design_name.length()) {
         SystemDesign* sd = SystemDesign::Find(design_name);
         if (sd)
            sensor->SetDesign(sd);
      }

      for (int i = 0; i < nranges; i++)
         sensor->AddRange(ranges[i]);

      if (emcon_1 >= 0 && emcon_1 <= 100)
         sensor->SetEMCONPower(1, emcon_1);

      if (emcon_2 >= 0 && emcon_2 <= 100)
         sensor->SetEMCONPower(1, emcon_2);

      if (emcon_3 >= 0 && emcon_3 <= 100)
         sensor->SetEMCONPower(1, emcon_3);

      sensor->Mount(loc, size, hull);
      sensor->SetSourceIndex(reactors.size()-1);
   }
   else {
      Print("WARNING: additional sensor ignored in '%s'\n", filename);
   }
}

// +--------------------------------------------------------------------+

void
ShipDesign::ParseNavsys(TermStruct* val)
{
   Text  design_name;
   Vec3  loc(0.0f, 0.0f, 0.0f);
   float size   = 0.0f;
   float hull   = 0.5f;

   for (int i = 0; i < val->elements()->size(); i++) {
      TermDef* pdef = val->elements()->at(i)->isDef();
      if (pdef) {
         Text defname = pdef->name()->value();
         defname.setSensitive(false);

         if (defname == "loc") {
            GetDefVec(loc, pdef, filename);
            loc *= (float) scale;
         }
         else if (defname == "size") {
            size *= (float) scale;
            GetDefNumber(size, pdef, filename);
         }
         else if (defname == "hull_factor") {
            GetDefNumber(hull, pdef, filename);
         }
         else if (defname == "design")
            GetDefText(design_name, pdef, filename);
      }
   }

   if (!navsys) {
      navsys = new(__FILE__,__LINE__) NavSystem;

      if (design_name.length()) {
         SystemDesign* sd = SystemDesign::Find(design_name);
         if (sd)
            navsys->SetDesign(sd);
      }

      navsys->Mount(loc, size, hull);
      navsys->SetSourceIndex(reactors.size()-1);
   }
   else {
      Print("WARNING: additional nav system ignored in '%s'\n", filename);
   }
}

// +--------------------------------------------------------------------+

void
ShipDesign::ParseComputer(TermStruct* val)
{
   Text  comp_name("Computer");
   Text  comp_abrv("Comp");
   Text  design_name;
   int   comp_type = 1;
   Vec3  loc(0.0f, 0.0f, 0.0f);
   float size   = 0.0f;
   float hull   = 0.5f;

   for (int i = 0; i < val->elements()->size(); i++) {
      TermDef* pdef = val->elements()->at(i)->isDef();
      if (pdef) {
         Text defname = pdef->name()->value();
         defname.setSensitive(false);

         if (defname == "name") {
            GetDefText(comp_name, pdef, filename);
         }
         else if (defname == "abrv") {
            GetDefText(comp_abrv, pdef, filename);
         }
         else if (defname == "design") {
            GetDefText(design_name, pdef, filename);
         }
         else if (defname == "type") {
            GetDefNumber(comp_type, pdef, filename);
         }
         else if (defname == "loc") {
            GetDefVec(loc, pdef, filename);
            loc *= (float) scale;
         }
         else if (defname == "size") {
            size *= (float) scale;
            GetDefNumber(size, pdef, filename);
         }
         else if (defname == "hull_factor") {
            GetDefNumber(hull, pdef, filename);
         }
      }
   }

   Computer* comp = new(__FILE__,__LINE__) Computer(comp_type, comp_name);
   comp->Mount(loc, size, hull);
   comp->SetAbbreviation(comp_abrv);
   comp->SetSourceIndex(reactors.size()-1);

   if (design_name.length()) {
      SystemDesign* sd = SystemDesign::Find(design_name);
      if (sd)
         comp->SetDesign(sd);
   }

   computers.append(comp);
}

// +--------------------------------------------------------------------+

void
ShipDesign::ParseShield(TermStruct* val)
{
   Text     dname;
   Text     dabrv;
   Text     design_name;
   Text     model_name;
   double   factor      = 0;
   double   capacity    = 0;
   double   consumption = 0;
   double   cutoff      = 0;
   double   curve       = 0;
   double   def_cost    = 1;
   int      shield_type = 0;
   Vec3     loc(0.0f, 0.0f, 0.0f);
   float    size   = 0.0f;
   float    hull   = 0.5f;
   int      etype  = 0;
   bool     shield_capacitor = false;
   bool     shield_bubble    = false;
   int   emcon_1 = -1;
   int   emcon_2 = -1;
   int   emcon_3 = -1;

   for (int i = 0; i < val->elements()->size(); i++) {
      TermDef* pdef = val->elements()->at(i)->isDef();
      if (pdef) {
         Text defname = pdef->name()->value();
         defname.setSensitive(false);

         if (defname == "type") {
            GetDefNumber(shield_type, pdef, filename);
         }
         else if (defname == "name")
            GetDefText(dname, pdef, filename);
         else if (defname == "abrv")
            GetDefText(dabrv, pdef, filename);
         else if (defname == "design")
            GetDefText(design_name, pdef, filename);
         else if (defname == "model")
            GetDefText(model_name, pdef, filename);

         else if (defname == "loc") {
            GetDefVec(loc, pdef, filename);
            loc *= (float) scale;
         }
         else if (defname == "size") {
            GetDefNumber(size, pdef, filename);
            size *= (float) scale;
         }
         else if (defname == "hull_factor")
            GetDefNumber(hull, pdef, filename);

         else if (defname.contains("factor"))
            GetDefNumber(factor, pdef, filename);
         else if (defname.contains("cutoff"))
            GetDefNumber(cutoff, pdef, filename);
         else if (defname.contains("curve"))
            GetDefNumber(curve, pdef, filename);
         else if (defname.contains("capacitor"))
            GetDefBool(shield_capacitor, pdef, filename);
         else if (defname.contains("bubble"))
            GetDefBool(shield_bubble, pdef, filename);
         else if (defname == "capacity")
            GetDefNumber(capacity, pdef, filename);
         else if (defname == "consumption")
            GetDefNumber(consumption, pdef, filename);
         else if (defname == "deflection_cost")
            GetDefNumber(def_cost, pdef, filename);
         else if (defname == "explosion")
            GetDefNumber(etype, pdef, filename);

         else if (defname == "emcon_1") {
            GetDefNumber(emcon_1, pdef, filename);
         }

         else if (defname == "emcon_2") {
            GetDefNumber(emcon_2, pdef, filename);
         }

         else if (defname == "emcon_3") {
            GetDefNumber(emcon_3, pdef, filename);
         }

         else if (defname == "bolt_hit_sound") {
            GetDefText(bolt_hit_sound, pdef, filename);
         }

         else if (defname == "beam_hit_sound") {
            GetDefText(beam_hit_sound, pdef, filename);
         }
      }
   }

   if (!shield) {
      if (shield_type) {
         shield = new(__FILE__,__LINE__) Shield((Shield::SUBTYPE) shield_type);
         shield->SetSourceIndex(reactors.size()-1);
         shield->Mount(loc, size, hull);
         if (dname.length()) shield->SetName(dname);
         if (dabrv.length()) shield->SetAbbreviation(dabrv);

         if (design_name.length()) {
            SystemDesign* sd = SystemDesign::Find(design_name);
            if (sd)
               shield->SetDesign(sd);
         }

         shield->SetExplosionType(etype);
         shield->SetShieldCapacitor(shield_capacitor);
         shield->SetShieldBubble(shield_bubble);

         if (factor      > 0) shield->SetShieldFactor(factor);
         if (capacity    > 0) shield->SetCapacity(capacity);
         if (cutoff      > 0) shield->SetShieldCutoff(cutoff);
         if (consumption > 0) shield->SetConsumption(consumption);
         if (def_cost    > 0) shield->SetDeflectionCost(def_cost);
         if (curve       > 0) shield->SetShieldCurve(curve);

         if (emcon_1 >= 0 && emcon_1 <= 100)
            shield->SetEMCONPower(1, emcon_1);

         if (emcon_2 >= 0 && emcon_2 <= 100)
            shield->SetEMCONPower(1, emcon_2);

         if (emcon_3 >= 0 && emcon_3 <= 100)
            shield->SetEMCONPower(1, emcon_3);

         if (model_name.length()) {
            shield_model = new(__FILE__,__LINE__) Model;
            if (!shield_model->Load(model_name, scale)) {
               Print("ERROR: Could not load shield model '%s'\n", model_name);
               delete shield_model;
               shield_model = 0;
               valid = false;
            }
            else {
               shield_model->SetDynamic(true);
               shield_model->SetLuminous(true);
            }
         }

         DataLoader* loader      = DataLoader::GetLoader();
         DWORD       SOUND_FLAGS = Sound::LOCALIZED | Sound::LOC_3D;

         if (bolt_hit_sound.length()) {
            if (!loader->LoadSound(bolt_hit_sound, bolt_hit_sound_resource, SOUND_FLAGS, true)) {
               loader->SetDataPath("Sounds/");
               loader->LoadSound(bolt_hit_sound, bolt_hit_sound_resource, SOUND_FLAGS);
               loader->SetDataPath(path_name);
            }
         }

         if (beam_hit_sound.length()) {
            if (!loader->LoadSound(beam_hit_sound, beam_hit_sound_resource, SOUND_FLAGS, true)) {
               loader->SetDataPath("Sounds/");
               loader->LoadSound(beam_hit_sound, beam_hit_sound_resource, SOUND_FLAGS);
               loader->SetDataPath(path_name);
            }
         }
      }
      else {
         Print("WARNING: invalid shield type in '%s'\n", filename);
      }
   }
   else {
      Print("WARNING: additional shield ignored in '%s'\n", filename);
   }
}

// +--------------------------------------------------------------------+

void
ShipDesign::ParseDeathSpiral(TermStruct* val)
{
   int   exp_index    = -1;
   int   debris_index = -1;
   int   fire_index   = -1;

   for (int i = 0; i < val->elements()->size(); i++) {
      TermDef* def = val->elements()->at(i)->isDef();
      if (def) {
         Text defname = def->name()->value();
         defname.setSensitive(false);

         if (defname == "time") {
            GetDefNumber(death_spiral_time, def, filename);
         }

         else if (defname == "explosion") {
            if (!def->term() || !def->term()->isStruct()) {
               Print("WARNING: explosion struct missing in '%s'\n", filename);
            }
            else {
               TermStruct* val = def->term()->isStruct();
               ParseExplosion(val, ++exp_index);
            }
         }

         // BACKWARD COMPATIBILITY:
         else if (defname == "explosion_type") {
            GetDefNumber(explosion[++exp_index].type, def, filename);
         }

         else if (defname == "explosion_time") {
            GetDefNumber(explosion[exp_index].time, def, filename);
         }

         else if (defname == "explosion_loc") {
            GetDefVec(explosion[exp_index].loc, def, filename);
            explosion[exp_index].loc *= (float) scale;
         }

         else if (defname == "final_type") {
            GetDefNumber(explosion[++exp_index].type, def, filename);
            explosion[exp_index].final = true;
         }

         else if (defname == "final_loc") {
            GetDefVec(explosion[exp_index].loc, def, filename);
            explosion[exp_index].loc *= (float) scale;
         }


         else if (defname == "debris") {
            if (def->term() && def->term()->isText()) {
               Text model_name;
               GetDefText(model_name, def, filename);
               Model* model = new(__FILE__,__LINE__) Model;
               if (!model->Load(model_name, scale)) {
                  Print("Could not load debris model '%s'\n", model_name);
                  delete model;
                  return;
               }

               PrepareModel(*model);
               debris[++debris_index].model = model;
               fire_index = -1;
            }
            else if (!def->term() || !def->term()->isStruct()) {
               Print("WARNING: debris struct missing in '%s'\n", filename);
            }
            else {
               TermStruct* val = def->term()->isStruct();
               ParseDebris(val, ++debris_index);
            }
         }

         else if (defname == "debris_mass") {
            GetDefNumber(debris[debris_index].mass, def, filename);
         }

         else if (defname == "debris_speed") {
            GetDefNumber(debris[debris_index].speed, def, filename);
         }

         else if (defname == "debris_drag") {
            GetDefNumber(debris[debris_index].drag, def, filename);
         }

         else if (defname == "debris_loc") {
            GetDefVec(debris[debris_index].loc, def, filename);
            debris[debris_index].loc *= (float) scale;
         }

         else if (defname == "debris_count") {
            GetDefNumber(debris[debris_index].count, def, filename);
         }

         else if (defname == "debris_life") {
            GetDefNumber(debris[debris_index].life, def, filename);
         }

         else if (defname == "debris_fire") {
            if (++fire_index < 5) {
               GetDefVec(debris[debris_index].fire_loc[fire_index], def, filename);
               debris[debris_index].fire_loc[fire_index] *= (float) scale;
            }
         }

         else if (defname == "debris_fire_type") {
            GetDefNumber(debris[debris_index].fire_type, def, filename);
         }
      }
   }
}

// +--------------------------------------------------------------------+

void
ShipDesign::ParseExplosion(TermStruct* val, int index)
{
   ShipExplosion* exp = &explosion[index];

   for (int i = 0; i < val->elements()->size(); i++) {
      TermDef* def = val->elements()->at(i)->isDef();
      if (def) {
         Text defname = def->name()->value();
         defname.setSensitive(false);

         if (defname == "time") {
            GetDefNumber(exp->time, def, filename);
         }

         else if (defname == "type") {
            GetDefNumber(exp->type, def, filename);
         }

         else if (defname == "loc") {
            GetDefVec(exp->loc, def, filename);
            exp->loc *= (float) scale;
         }

         else if (defname == "final") {
            GetDefBool(exp->final, def, filename);
         }
      }
   }
}

// +--------------------------------------------------------------------+

void
ShipDesign::ParseDebris(TermStruct* val, int index)
{
   char        model_name[NAMELEN];
   int         fire_index = 0;
   ShipDebris* deb = &debris[index];

   for (int i = 0; i < val->elements()->size(); i++) {
      TermDef* def = val->elements()->at(i)->isDef();
      if (def) {
         Text defname = def->name()->value();

         if (defname == "model") {
            GetDefText(model_name, def, filename);
            Model* model = new(__FILE__,__LINE__) Model;
            if (!model->Load(model_name, scale)) {
               Print("Could not load debris model '%s'\n", model_name);
               delete model;
               return;
            }

            PrepareModel(*model);
            deb->model = model;
         }

         else if (defname == "mass") {
            GetDefNumber(deb->mass, def, filename);
         }

         else if (defname == "speed") {
            GetDefNumber(deb->speed, def, filename);
         }

         else if (defname == "drag") {
            GetDefNumber(deb->drag, def, filename);
         }

         else if (defname == "loc") {
            GetDefVec(deb->loc, def, filename);
            deb->loc *= (float) scale;
         }

         else if (defname == "count") {
            GetDefNumber(deb->count, def, filename);
         }

         else if (defname == "life") {
            GetDefNumber(deb->life, def, filename);
         }

         else if (defname == "fire") {
            if (fire_index < 5) {
               GetDefVec(deb->fire_loc[fire_index], def, filename);
               deb->fire_loc[fire_index] *= (float) scale;
               fire_index++;
            }
         }

         else if (defname == "fire_type") {
            GetDefNumber(deb->fire_type, def, filename);
         }
      }
   }
}

// +--------------------------------------------------------------------+

void
ShipDesign::ParseMap(TermStruct* val)
{
   char  sprite_name[NAMELEN];

   for (int i = 0; i < val->elements()->size(); i++) {
      TermDef* pdef = val->elements()->at(i)->isDef();
      if (pdef) {
         Text defname = pdef->name()->value();
         defname.setSensitive(false);

         if (defname == "sprite") {
            GetDefText(sprite_name, pdef, filename);

            Bitmap*     sprite = new(__FILE__,__LINE__) Bitmap();
            DataLoader* loader = DataLoader::GetLoader();
            loader->LoadBitmap(sprite_name, *sprite, Bitmap::BMP_TRANSLUCENT);

            map_sprites.append(sprite);
         }
      }
   }
}

// +--------------------------------------------------------------------+

void
ShipDesign::ParseSquadron(TermStruct* val)
{
   char  name[NAMELEN];
   char  design[NAMELEN];
   int   count = 4;
   int   avail = 4;

   name[0] = 0;
   design[0] = 0;

   for (int i = 0; i < val->elements()->size(); i++) {
      TermDef* pdef = val->elements()->at(i)->isDef();
      if (pdef) {
         Text defname = pdef->name()->value();
         defname.setSensitive(false);

         if (defname == "name") {
            GetDefText(name, pdef, filename);
         }
         else if (defname == "design") {
            GetDefText(design, pdef, filename);
         }
         else if (defname == "count") {
            GetDefNumber(count, pdef, filename);
         }
         else if (defname == "avail") {
            GetDefNumber(avail, pdef, filename);
         }
      }
   }

   ShipSquadron* s = new(__FILE__,__LINE__) ShipSquadron;
   strcpy(s->name, name);

   s->design = Get(design);
   s->count  = count;
   s->avail  = avail;

   squadrons.append(s);
}

// +--------------------------------------------------------------------+

Skin*
ShipDesign::ParseSkin(TermStruct* val)
{
   Skin* skin = 0;
   char  name[NAMELEN];

   name[0] = 0;

   for (int i = 0; i < val->elements()->size(); i++) {
      TermDef* def = val->elements()->at(i)->isDef();
      if (def) {
         Text defname = def->name()->value();
         defname.setSensitive(false);

         if (defname == "name") {
            GetDefText(name, def, filename);

            skin = new(__FILE__,__LINE__) Skin(name);
         }
         else if (defname == "material" || defname == "mtl") {
            if (!def->term() || !def->term()->isStruct()) {
               Print("WARNING: skin struct missing in '%s'\n", filename);
            }
            else {
               TermStruct* val = def->term()->isStruct();
               ParseSkinMtl(val, skin);
            }
         }
      }
   }

   if (skin && skin->NumCells()) {
      skins.append(skin);
   }

   else if (skin) {
      delete skin;
      skin = 0;
   }

   return skin;
}

void
ShipDesign::ParseSkinMtl(TermStruct* val, Skin* skin)
{
   Material* mtl = new(__FILE__,__LINE__) Material;

   for (int i = 0; i < val->elements()->size(); i++) {
      TermDef* def = val->elements()->at(i)->isDef();
      if (def) {
         Text defname = def->name()->value();
         defname.setSensitive(false);

         if (defname == "name") {
            GetDefText(mtl->name, def, filename);
         }
         else if (defname == "Ka") {
            GetDefColor(mtl->Ka, def, filename);
         }
         else if (defname == "Kd") {
            GetDefColor(mtl->Kd, def, filename);
         }
         else if (defname == "Ks") {
            GetDefColor(mtl->Ks, def, filename);
         }
         else if (defname == "Ke") {
            GetDefColor(mtl->Ke, def, filename);
         }
         else if (defname == "Ns" || defname == "power") {
            GetDefNumber(mtl->power, def, filename);
         }
         else if (defname == "bump") {
            GetDefNumber(mtl->bump, def, filename);
         }
         else if (defname == "luminous") {
            GetDefBool(mtl->luminous, def, filename);
         }

         else if (defname == "blend") {
            if (def->term() && def->term()->isNumber())
               GetDefNumber(mtl->blend, def, filename);

            else if (def->term() && def->term()->isText()) {
               Text val;
               GetDefText(val, def, filename);
               val.setSensitive(false);

               if (val == "alpha" || val == "translucent")
                  mtl->blend = Material::MTL_TRANSLUCENT;

               else if (val == "additive")
                  mtl->blend = Material::MTL_ADDITIVE;

               else
                  mtl->blend = Material::MTL_SOLID;
            }
         }

         else if (defname.indexOf("tex_d") == 0) {
            char tex_name[64];
            if (!GetDefText(tex_name, def, filename))
               Print("WARNING: invalid or missing tex_diffuse in '%s'\n", filename);

            DataLoader* loader = DataLoader::GetLoader();
            loader->LoadTexture(tex_name, mtl->tex_diffuse);
         }

         else if (defname.indexOf("tex_s") == 0) {
            char tex_name[64];
            if (!GetDefText(tex_name, def, filename))
               Print("WARNING: invalid or missing tex_specular in '%s'\n", filename);

            DataLoader* loader = DataLoader::GetLoader();
            loader->LoadTexture(tex_name, mtl->tex_specular);
         }

         else if (defname.indexOf("tex_b") == 0) {
            char tex_name[64];
            if (!GetDefText(tex_name, def, filename))
               Print("WARNING: invalid or missing tex_bumpmap in '%s'\n", filename);

            DataLoader* loader = DataLoader::GetLoader();
            loader->LoadTexture(tex_name, mtl->tex_bumpmap);
         }

         else if (defname.indexOf("tex_e") == 0) {
            char tex_name[64];
            if (!GetDefText(tex_name, def, filename))
               Print("WARNING: invalid or missing tex_emissive in '%s'\n", filename);

            DataLoader* loader = DataLoader::GetLoader();
            loader->LoadTexture(tex_name, mtl->tex_emissive);
         }
      }
   }

   if (skin && mtl)
      skin->AddMaterial(mtl);
}

const Skin*
ShipDesign::FindSkin(const char* skin_name) const
{
   int n = skins.size();

   for (int i = 0; i < n; i++) {
      Skin* s = skins[n-1-i];

      if (!strcmp(s->Name(), skin_name))
         return s;
   }

   return 0;
}
