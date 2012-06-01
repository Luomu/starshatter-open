/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

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