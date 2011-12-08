/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         Callsign.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Package Callsign catalog class
*/

#include "MemDebug.h"
#include "Callsign.h"

// +----------------------------------------------------------------------+

static int  callsign_index = -1;

static char civilian_catalog[32][16] = {
   "Aleph",    "Vehan",    "Galvin",   "Caleb",
   "Mercury",  "Lancet",   "Hera",     "Zeus",
   "Odin",     "Thor",     "Nereus",   "Klono",
   "Athena",   "Helios",   "Leto",     "Nivas",

   "Selene",   "Proteus",  "Triton",   "Thetis",
   "Aurora",   "Ceres",    "Rana",     "Doradus",
   "Perseus",  "Corina",   "Cygnus",   "Lago",
   "Andil",    "Galen",    "Temas",    "Dalan"
};

static char alliance_catalog[32][16] = {
   "Alpha",    "Bravo",    "Delta",    "Echo",
   "Ranger",   "Magic",    "Falcon",   "Omega",
   "Vulcan",   "Hammer",   "Nomad",    "Dragon",
   "Sierra",   "Tango",    "Victor",   "Zulu",

   "Sentry",   "Wolf",     "Zeta",     "Jackal",
   "Merlin",   "Eagle",    "Blade",    "Tiger",
   "Raptor",   "Ares",     "Condor",   "Rogue",
   "Hornet",   "Gold",     "Mustang",  "Voodoo"
};

static char hegemony_catalog[32][16] = {
   "Nagal",    "Nalak",    "Olkar",    "Kalar",
   "Narom",    "Tranor",   "Orrin",    "Orlak",
   "Nardik",   "Sorrin",   "Amnar",    "Kizek",
   "Orten",    "Ronar",    "Molkar",   "Ternal",

   "Martak",   "Komar",    "Malik",    "Morgul",
   "Kliek",    "Torlan",   "Arvin",    "Artak",
   "Ralka",    "Sernal",   "Roten",    "Reza",
   "Tinet",    "Aliek",    "Salar",    "Sona"
};

static char pirate_catalog[32][16] = {
   "Raider",   "Skull",    "Black",    "Blood",
   "Roger",    "Hook",     "Galleon",  "Privateer",
   "Cutlass",  "Sabre",    "Pike",     "Blackbeard",
   "Pistol",   "Cortez",   "Pirate",   "Buccaneer",

   "Raider",   "Skull",    "Black",    "Blood",
   "Morgan",   "Redbeard", "Cutlass",  "Sabre",
   "Iron",     "Stocks",   "Quarter",  "Gray",
   "Ruby",     "Cross",    "Pirate",   "Raider"
};

static char zolon_catalog[32][16] = {
   "Cancer",   "Krill",    "Bluefin",  "Scylla",
   "Charybdis","Finback",  "Mantis",   "Skate",
   "Sealion",  "Lamprey",  "Tarpon",   "Hammerhead",
   "Orca",     "Skipjack", "Sculpin",  "Thresher",

   "Devilray", "Chinook",  "Moray",    "Seastar",
   "Heron",    "Puffin",   "Rockeye",  "Tiburon",
   "Coho",     "Stingray", "Mako",     "Conger",
   "Scad",     "Pompano",  "Tusk",     "Nautilus"
};


// +----------------------------------------------------------------------+

const char*
Callsign::GetCallsign(int IFF)
{
   if (callsign_index < 0)
      callsign_index = rand()/1000;

   if (callsign_index > 31)
      callsign_index = 0;

   switch (IFF) {
   case 0:     return civilian_catalog[callsign_index++];
   default:
   case 1:     return alliance_catalog[callsign_index++];
   case 2:     return hegemony_catalog[callsign_index++];
   case 3:     return pirate_catalog[callsign_index++];
   case 4:     return zolon_catalog[callsign_index++];
   }
}
