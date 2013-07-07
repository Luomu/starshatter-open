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
    FILE:         ModConfig.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Mod file deployment configuration and manager
*/


#include "MemDebug.h"
#include "ModConfig.h"
#include "ModInfo.h"
#include "Campaign.h"
#include "ShipDesign.h"
#include "WeaponDesign.h"
#include "Starshatter.h"
#include "ParseUtil.h"
#include "DataLoader.h"

// +-------------------------------------------------------------------+

static ModConfig* mod_config = 0;

// +-------------------------------------------------------------------+

ModConfig::ModConfig()
{
    mod_config = this;

    Load();
    FindMods();
    Deploy();
}

ModConfig::~ModConfig()
{
    if (mod_config == this)
    mod_config = 0;

    Undeploy();

    enabled.destroy();
    disabled.destroy();
    mods.destroy();
}

// +-------------------------------------------------------------------+

void
ModConfig::Initialize()
{
    mod_config = new(__FILE__,__LINE__) ModConfig;
}

void
ModConfig::Close()
{
    delete mod_config;
    mod_config = 0;
}

// +-------------------------------------------------------------------+

ModConfig*
ModConfig::GetInstance()
{
    return mod_config;
}

// +-------------------------------------------------------------------+

void
ModConfig::Load()
{
    // read the config file:
    BYTE*       block    = 0;
    int         blocklen = 0;

    char        filename[64];
    strcpy_s(filename, "mod.cfg");

    FILE* f;
    ::fopen_s(&f, filename, "rb");

    if (f) {
        ::fseek(f, 0, SEEK_END);
        blocklen = ftell(f);
        ::fseek(f, 0, SEEK_SET);

        block = new(__FILE__,__LINE__) BYTE[blocklen+1];
        block[blocklen] = 0;

        ::fread(block, blocklen, 1, f);
        ::fclose(f);
    }

    if (blocklen == 0)
    return;

    Parser parser(new(__FILE__,__LINE__) BlockReader((const char*) block, blocklen));
    Term*  term = parser.ParseTerm();

    if (!term) {
        Print("ERROR: could not parse '%s'.\n", filename);
        return;
    }
    else {
        TermText* file_type = term->isText();
        if (!file_type || file_type->value() != "MOD_CONFIG") {
            Print("WARNING: invalid '%s' file. No mods deployed\n", filename);
            return;
        }
    }

    enabled.destroy();

    do {
        delete term;

        term = parser.ParseTerm();

        if (term) {
            TermDef* def = term->isDef();
            if (def) {
                Text name;
                GetDefText(name, def, filename);
                enabled.append(new(__FILE__,__LINE__) Text(name));
            }
        }
    }
    while (term);

    delete [] block;
}

void
ModConfig::Save()
{
    FILE* f;
    fopen_s(&f, "mod.cfg", "w");
    if (f) {
        fprintf(f, "MOD_CONFIG\n\n");

        ListIter<Text> iter = enabled;
        while (++iter) {
            Text* name = iter.value();
            fprintf(f, "mod: \"%s\"\n", name->data());
        }

        fclose(f);
    }
}

void
ModConfig::FindMods()
{
    disabled.destroy();

    DataLoader* loader = DataLoader::GetLoader();

    if (loader) {
        loader->UseFileSystem(true);
        loader->ListFiles("*.dat", disabled, true);
        loader->UseFileSystem(Starshatter::UseFileSystem());

        ListIter<Text> iter = disabled;
        while (++iter) {
            Text* name = iter.value();
            name->setSensitive(false);

            if (*name == "shatter.dat"      || 
                    *name == "beta.dat"         || 
                    *name == "start.dat"        ||
                    *name == "irunin.dat"       ||
                    *name == "vox.dat"          ||
                    name->contains("uninstall") ||
                    enabled.contains(name))
            delete iter.removeItem();
        }
    }
}

// +-------------------------------------------------------------------+

ModInfo*
ModConfig::GetModInfo(const char* filename)
{
    for (int i = 0; i < mods.size(); i++) {
        ModInfo* mod_info = mods[i];

        if (mod_info->Filename() == filename && mod_info->IsEnabled())
        return mod_info;
    }

    return NULL;
}

// +-------------------------------------------------------------------+

bool
ModConfig::IsDeployed(const char* name)
{
    for (int i = 0; i < mods.size(); i++) {
        ModInfo* mod_info = mods[i];

        if (mod_info->Name() == name && mod_info->IsEnabled())
        return true;
    }

    return false;
}

// +-------------------------------------------------------------------+

void
ModConfig::Deploy()
{
    Save();

    if (enabled.size() < 1)
    return;

    Print("\nDEPLOYING MODS\n--------------\n");

    int i = 1;
    ListIter<Text> iter = enabled;
    while (++iter) {
        Text* name = iter.value();

        if (IsDeployed(name->data())) {
            Print("  %d. %s is already deployed (skipping)\n", i++, name->data());
            continue;
        }

        Print("  %d. %s\n", i++, name->data());

        ModInfo* mod_info = new(__FILE__,__LINE__) ModInfo;
        
        if (mod_info->Load(name->data()) && mod_info->Enable()) {
            mods.append(mod_info);
        }
        else {
            Print("     Could not deploy '%s' - disabling\n", name->data());

            delete mod_info;
            iter.removeItem();
            disabled.append(name);
        }
    }

    Print("\n");
    Game::UseLocale(0);
}

void
ModConfig::Undeploy()
{
    Print("UNDEPLOYING MODS\n");
    mods.destroy();

    ShipDesign::ClearModCatalog();
    WeaponDesign::ClearModCatalog();
}

void
ModConfig::Redeploy()
{
    Undeploy();
    Deploy();

    Campaign::Close();
    Campaign::Initialize();
    Campaign::SelectCampaign("Single Missions");
}

// +-------------------------------------------------------------------+

void
ModConfig::EnableMod(const char* name)
{
    if (!name || !*name)
    return;

    Text* mod_name;

    ListIter<Text> iter = disabled;
    while (++iter) {
        Text* t = iter.value();

        if (*t == name) {
            mod_name = t;
            iter.removeItem();
            break;
        }
    }

    if (mod_name) {
        enabled.append(mod_name);

        if (!IsDeployed(*mod_name)) {
            ModInfo* mod_info = new(__FILE__,__LINE__) ModInfo;
            
            if (mod_info->Load(*mod_name) && mod_info->Enable()) {
                mods.append(mod_info);
            }
        }
    }
}

void
ModConfig::DisableMod(const char* name)
{
    if (!name || !*name)
    return;

    Text* mod_name;

    ListIter<Text> iter = enabled;
    while (++iter) {
        Text* t = iter.value();

        if (*t == name) {
            mod_name = t;
            iter.removeItem();
            break;
        }
    }

    if (mod_name) {
        disabled.append(mod_name);

        ListIter<ModInfo> iter = mods;
        while (++iter) {
            ModInfo* mod_info = iter.value();
            if (mod_info->Name() == *mod_name) {
                delete iter.removeItem();
                break;
            }
        }
    }
}

// +-------------------------------------------------------------------+

void
ModConfig::IncreaseModPriority(int mod_index)
{
    if (mod_index > 0 && mod_index < enabled.size()) {
        Text* mod1 = enabled.at(mod_index-1);
        Text* mod2 = enabled.at(mod_index);

        enabled.at(mod_index-1) = mod2;
        enabled.at(mod_index)   = mod1;
    }
}

void
ModConfig::DecreaseModPriority(int mod_index)
{
    if (mod_index >= 0 && mod_index < enabled.size()-1) {
        Text* mod1 = enabled.at(mod_index);
        Text* mod2 = enabled.at(mod_index+1);

        enabled.at(mod_index)   = mod2;
        enabled.at(mod_index+1) = mod1;
    }
}

