/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         NetFileServlet.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	HTTP Servlet for File Transfer
*/


#ifndef NetFileServlet_h
#define NetFileServlet_h

#include "Types.h"
#include "NetAdminServer.h"
#include "NetUser.h"

// +-------------------------------------------------------------------+

class Campaign;
class File;

// +-------------------------------------------------------------------+

class NetFileServlet : public NetAdminServlet
{
public:
	NetFileServlet()           { }
	virtual ~NetFileServlet()  { }

	virtual bool DoGet(HttpRequest& request,  HttpResponse& response);
};

// +-------------------------------------------------------------------+

class NetWebServlet : public NetAdminServlet
{
public:
	NetWebServlet()           { }
	virtual ~NetWebServlet()  { }

	virtual bool DoGet(HttpRequest& request,  HttpResponse& response);
};

#endif NetFileServlet_h