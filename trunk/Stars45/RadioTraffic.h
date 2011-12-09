/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         RadioTraffic.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	RadioTraffic maintains a history of all messages sent between ships
	in the simulation.  This class also handles displaying relevant
	traffic to the player.
*/

#ifndef RadioTraffic_h
#define RadioTraffic_h

#include "Types.h"
#include "Geometry.h"
#include "SimObject.h"
#include "List.h"
#include "Text.h"

// +--------------------------------------------------------------------+

class Element;
class RadioMessage;
class Ship;
class SimObject;

// +--------------------------------------------------------------------+

class RadioTraffic
{
public:
	RadioTraffic();
	~RadioTraffic();

	// accessors:
	static void          Initialize();
	static void          Close();

	static RadioTraffic* GetInstance()  { return radio_traffic; }

	static void          SendQuickMessage(Ship* ship, int msg);
	static void          Transmit(RadioMessage* msg);
	static void          DiscardMessages();
	static Text          TranslateVox(const char* phrase);

	void                 SendMessage(RadioMessage* msg);
	void                 DisplayMessage(RadioMessage* msg);


protected:
	List<RadioMessage>   traffic;

	static RadioTraffic* radio_traffic;
};

// +--------------------------------------------------------------------+

#endif RadioTraffic_h

