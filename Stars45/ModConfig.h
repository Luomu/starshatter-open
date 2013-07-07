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
    FILE:         ModConfig.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Mod file deployment configuration and manager
*/


#ifndef ModConfig_h
#define ModConfig_h

#include "Types.h"
#include "Bitmap.h"
#include "Text.h"
#include "List.h"

// +-------------------------------------------------------------------+

class ModConfig;
class ModInfo;
class ModCampaign;

// +-------------------------------------------------------------------+

class ModConfig
{
public:
    static const char* TYPENAME() { return "ModConfig"; }

    ModConfig();
    ~ModConfig();

    int operator == (const ModConfig& cfg) const { return this == &cfg; }

    static void          Initialize();
    static void          Close();
    static ModConfig*    GetInstance();

    void                 Load();
    void                 Save();
    void                 FindMods();

    bool                 IsDeployed(const char* name);
    void                 Deploy();
    void                 Undeploy();
    void                 Redeploy();

    // these methods change the configuration only
    // you must Redeploy() to have them take effect:

    void                 EnableMod(const char* name);
    void                 DisableMod(const char* name);
    void                 IncreaseModPriority(int mod_index);
    void                 DecreaseModPriority(int mod_index);

    List<Text>&          EnabledMods()     { return enabled;    }
    List<Text>&          DisabledMods()    { return disabled;   }
    List<ModInfo>&       GetModInfoList()  { return mods;       }

    ModInfo*             GetModInfo(const char* filename);

private:
    List<Text>     enabled;
    List<Text>     disabled;
    List<ModInfo>  mods;
};

#endif ModConfig_h