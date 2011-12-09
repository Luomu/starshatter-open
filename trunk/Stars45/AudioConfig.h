/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         AudioConfig.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Audio Configuration class
*/

#ifndef AudioConfig_h
#define AudioConfig_h

#include "Types.h"

// +--------------------------------------------------------------------+

class AudioConfig
{
public:
	AudioConfig();
	~AudioConfig();

	static void          Initialize();
	static void          Close();
	static AudioConfig*  GetInstance();

	void     Load();
	void     Save();

	static int  MenuMusic();
	static int  GameMusic();
	static int  EfxVolume();
	static int  GuiVolume();
	static int  WrnVolume();
	static int  VoxVolume();
	static int  Silence();
	static void SetTraining(bool t);

	int      GetMenuMusic() const { return menu_music; }
	int      GetGameMusic() const { return game_music; }
	int      GetEfxVolume() const { return efx_volume; }
	int      GetGuiVolume() const { return gui_volume; }
	int      GetWrnVolume() const { return wrn_volume; }
	int      GetVoxVolume() const { return vox_volume; }

	void     SetMenuMusic(int v);
	void     SetGameMusic(int v);
	void     SetEfxVolume(int v);
	void     SetGuiVolume(int v);
	void     SetWrnVolume(int v);
	void     SetVoxVolume(int v);

protected:
	int      menu_music;
	int      game_music;

	int      efx_volume;
	int      gui_volume;
	int      wrn_volume;
	int      vox_volume;

	bool     training;
};

#endif AudioConfig_h

