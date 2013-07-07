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
