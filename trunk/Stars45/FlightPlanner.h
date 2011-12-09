/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         FlightPlanner.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Flight Planning class for creating navpoint routes for fighter elements.
	Used both by the CarrierAI class and the Flight Dialog.
*/

#ifndef FlightPlanner_h
#define FlightPlanner_h

#include "Types.h"
#include "Director.h"

// +--------------------------------------------------------------------+

class Sim;
class Ship;
class ShipAI;
class Instruction;
class Hangar;
class Element;

// +--------------------------------------------------------------------+

class FlightPlanner
{
public:
	FlightPlanner(Ship* s);
	virtual ~FlightPlanner();

	virtual void      CreatePatrolRoute(Element* elem,   int index);
	virtual void      CreateStrikeRoute(Element* strike, Element* target);
	virtual void      CreateEscortRoute(Element* escort, Element* ward);

	Sim*              sim;
	Ship*             ship;
	float             patrol_range;
};

// +--------------------------------------------------------------------+

#endif FlightPlanner_h

