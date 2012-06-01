/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         NetAdminChat.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	HTTP Servlet Engine for Multiplayer Admin
*/


#ifndef NetAdminChat_h
#define NetAdminChat_h

#include "NetAdminServer.h"

// +-------------------------------------------------------------------+

class NetAdminChat : public NetAdminServlet
{
public:
	NetAdminChat();
	virtual ~NetAdminChat()  { }

	virtual bool      DoGet(HttpRequest& request, HttpResponse& response);
	virtual Text      GetContent();
};

#endif NetAdminChat_h