/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         RadioVox.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	View class for Radio Communications HUD Overlay
*/

#ifndef RadioVox_h
#define RadioVox_h

#include "Types.h"
#include "List.h"
#include "Text.h"

// +--------------------------------------------------------------------+

class Element;
class Ship;
class RadioMessage;
class Sound;

// +--------------------------------------------------------------------+

class RadioVox
{
	friend class RadioVoxController;

public:
	static const char* TYPENAME() { return "RadioVox"; }

	RadioVox(int channel, const char* path, const char* message=0);
	virtual ~RadioVox();

	// Operations:
	virtual bool      AddPhrase(const char* key);
	virtual bool      Start();

	static void       Initialize();
	static void       Close();

protected:
	virtual bool      Update();

	Text              path;
	Text              message;
	List<Sound>       sounds;
	int               index;
	int               channel;
};

#endif RadioVox_h

