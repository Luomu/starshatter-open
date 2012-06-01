/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         RadioHandler.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	RadioHandler (radio comms) class declaration
*/

#ifndef RadioHandler_h
#define RadioHandler_h

#include "Types.h"
#include "Geometry.h"
#include "SimObject.h"

// +--------------------------------------------------------------------+

class RadioMessage;
class Ship;

// +--------------------------------------------------------------------+

class RadioHandler
{
public:
	RadioHandler();
	virtual ~RadioHandler();

	virtual bool      ProcessMessage(RadioMessage* msg,       Ship*   s);
	virtual void      AcknowledgeMessage(RadioMessage* msg,   Ship*   s);

protected:
	virtual bool      IsOrder(int action);
	virtual bool      ProcessMessageOrders(RadioMessage* msg, Ship*   s);
	virtual bool      ProcessMessageAction(RadioMessage* msg, Ship*   s);

	virtual bool      Inbound(RadioMessage* msg, Ship* s);
	virtual bool      Picture(RadioMessage* msg, Ship* s);
	virtual bool      Support(RadioMessage* msg, Ship* s);
	virtual bool      SkipNavpoint(RadioMessage* msg, Ship* s);
	virtual bool      LaunchProbe(RadioMessage* msg, Ship* s);
};

// +--------------------------------------------------------------------+

#endif RadioHandler_h

