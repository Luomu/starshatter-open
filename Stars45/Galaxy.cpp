/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         Galaxy.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Galaxy (list of star systems) for a single campaign.
*/

#include "MemDebug.h"
#include "Galaxy.h"
#include "StarSystem.h"
#include "Starshatter.h"

#include "Game.h"
#include "Solid.h"
#include "Sprite.h"
#include "Light.h"
#include "Bitmap.h"
#include "DataLoader.h"
#include "ParseUtil.h"

static Galaxy* galaxy = 0;

// +--------------------------------------------------------------------+

Galaxy::Galaxy(const char* n)
   : name(n), radius(10)
{ }

// +--------------------------------------------------------------------+

Galaxy::~Galaxy()
{
   Print("  Destroying Galaxy %s\n", (const char*) name);
   systems.destroy();
   stars.destroy();
}

// +--------------------------------------------------------------------+

void
Galaxy::Initialize()
{
   if (galaxy) delete galaxy;
   galaxy = new(__FILE__,__LINE__) Galaxy("Galaxy");
   galaxy->Load();
}

void
Galaxy::Close()
{
   delete galaxy;
   galaxy = 0;
}

Galaxy*
Galaxy::GetInstance()
{
   return galaxy;
}

// +--------------------------------------------------------------------+

void
Galaxy::Load()
{
   DataLoader* loader = DataLoader::GetLoader();
   loader->SetDataPath("Galaxy/");
   sprintf(filename, "%s.def", (const char*) name);
   Load(filename);

   // load mod galaxies:
   List<Text> mod_galaxies;
   loader->SetDataPath("Mods/Galaxy/");
   loader->ListFiles("*.def", mod_galaxies);

   ListIter<Text> iter = mod_galaxies;
   while (++iter) {
      Text* name = iter.value();

      if (!name->contains("/")) {
         loader->SetDataPath("Mods/Galaxy/");
         Load(name->data());
      }
   }
}

void
Galaxy::Load(const char* filename)
{
   Print("\nLoading Galaxy: %s\n", filename);

   BYTE*       block  = 0;
   DataLoader* loader = DataLoader::GetLoader();
   loader->LoadBuffer(filename, block, true);

   Parser parser(new(__FILE__,__LINE__) BlockReader((const char*) block));

   Term*  term = parser.ParseTerm();
   
   if (!term) {
      Print("WARNING: could not parse '%s'\n", filename);
      return;
   }
   else {
      TermText* file_type = term->isText();
      if (!file_type || file_type->value() != "GALAXY") {
         Print("WARNING: invalid galaxy file '%s'\n", filename);
         return;
      }
   }

   // parse the galaxy:
   do {
      delete term;
      term = parser.ParseTerm();
      
      if (term) {
         TermDef* def = term->isDef();
         if (def) {
            if (def->name()->value() == "radius") {
               GetDefNumber(radius, def, filename);
            }

            else if (def->name()->value() == "system") {
               if (!def->term() || !def->term()->isStruct()) {
                  Print("WARNING: system struct missing in '%s'\n", filename);
               }
               else {
                  TermStruct* val = def->term()->isStruct();

                  char  sys_name[32];
                  char  classname[32];
                  Vec3  sys_loc;
                  int   sys_iff = 0;
                  int   star_class = Star::G;

                  sys_name[0] = 0;
               
                  for (int i = 0; i < val->elements()->size(); i++) {
                     TermDef* pdef = val->elements()->at(i)->isDef();
                     if (pdef) {
                        if (pdef->name()->value() == "name")
                           GetDefText(sys_name, pdef, filename);

                        else if (pdef->name()->value() == "loc")
                           GetDefVec(sys_loc, pdef, filename);

                        else if (pdef->name()->value() == "iff")
                           GetDefNumber(sys_iff, pdef, filename);

                        else if (pdef->name()->value() == "class") {
                           GetDefText(classname, pdef, filename);

                           switch (classname[0]) {
                           case 'O':   star_class = Star::O;            break;
                           case 'B':   star_class = Star::B;            break;
                           case 'A':   star_class = Star::A;            break;
                           case 'F':   star_class = Star::F;            break;
                           case 'G':   star_class = Star::G;            break;
                           case 'K':   star_class = Star::K;            break;
                           case 'M':   star_class = Star::M;            break;
                           case 'R':   star_class = Star::RED_GIANT;    break;
                           case 'W':   star_class = Star::WHITE_DWARF;  break;
                           case 'Z':   star_class = Star::BLACK_HOLE;   break;
                           }
                        }
                     }
                  }

                  if (sys_name[0]) {
                     StarSystem* star_system = new(__FILE__,__LINE__) StarSystem(sys_name, sys_loc, sys_iff, star_class);
                     star_system->Load();
                     systems.append(star_system);

                     Star* star = new(__FILE__,__LINE__) Star(sys_name, sys_loc, star_class);
                     stars.append(star);
                  }
               }
            }

            else if (def->name()->value() == "star") {
               if (!def->term() || !def->term()->isStruct()) {
                  Print("WARNING: star struct missing in '%s'\n", filename);
               }
               else {
                  TermStruct* val = def->term()->isStruct();

                  char  star_name[32];
                  char  classname[32];
                  Vec3  star_loc;
                  int   star_class = Star::G;

                  star_name[0] = 0;
               
                  for (int i = 0; i < val->elements()->size(); i++) {
                     TermDef* pdef = val->elements()->at(i)->isDef();
                     if (pdef) {
                        if (pdef->name()->value() == "name")
                           GetDefText(star_name, pdef, filename);

                        else if (pdef->name()->value() == "loc")
                           GetDefVec(star_loc, pdef, filename);

                        else if (pdef->name()->value() == "class") {
                           GetDefText(classname, pdef, filename);

                           switch (classname[0]) {
                           case 'O':   star_class = Star::O;            break;
                           case 'B':   star_class = Star::B;            break;
                           case 'A':   star_class = Star::A;            break;
                           case 'F':   star_class = Star::F;            break;
                           case 'G':   star_class = Star::G;            break;
                           case 'K':   star_class = Star::K;            break;
                           case 'M':   star_class = Star::M;            break;
                           case 'R':   star_class = Star::RED_GIANT;    break;
                           case 'W':   star_class = Star::WHITE_DWARF;  break;
                           case 'Z':   star_class = Star::BLACK_HOLE;   break;
                           }
                        }
                     }
                  }

                  if (star_name[0]) {
                     Star* star = new(__FILE__,__LINE__) Star(star_name, star_loc, star_class);
                     stars.append(star);
                  }
               }
            }
         }
      }
   }
   while (term);
   
   loader->ReleaseBuffer(block);
   loader->SetDataPath(0);
}

// +--------------------------------------------------------------------+

void
Galaxy::ExecFrame()
{
   ListIter<StarSystem> sys = systems;
   while (++sys) {
      sys->ExecFrame();
   }
}

// +--------------------------------------------------------------------+

StarSystem*
Galaxy::GetSystem(const char* name)
{
   ListIter<StarSystem> sys = systems;
   while (++sys) {
      if (!strcmp(sys->Name(), name))
         return sys.value();
   }

   return 0;
}

// +--------------------------------------------------------------------+

StarSystem*
Galaxy::FindSystemByRegion(const char* rgn_name)
{
   ListIter<StarSystem> iter = systems;
   while (++iter) {
      StarSystem* sys = iter.value();
      if (sys->FindRegion(rgn_name))
         return sys;
   }

   return 0;
}
