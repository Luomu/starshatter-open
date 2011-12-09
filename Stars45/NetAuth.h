/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         NetAuth.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	This class authenticates a user connecting to the multiplayer lobby
*/


#ifndef NetAuth_h
#define NetAuth_h

#include "Types.h"
#include "NetAddr.h"
#include "NetLobby.h"
#include "Text.h"

// +-------------------------------------------------------------------+

class NetAuth
{
public:
	enum AUTH_STATE {
		NET_AUTH_INITIAL  = 0,
		NET_AUTH_FAILED   = 1,
		NET_AUTH_OK       = 2
	};

	enum AUTH_LEVEL {
		NET_AUTH_MINIMAL  = 0,
		NET_AUTH_STANDARD = 1,
		NET_AUTH_SECURE   = 2
	};

	static int     AuthLevel();
	static void    SetAuthLevel(int n);

	static Text    CreateAuthRequest(NetUser* u);
	static Text    CreateAuthResponse(int level, const char* salt);
	static bool    AuthUser(NetUser* u, Text response);
};

// +-------------------------------------------------------------------+

#endif NetAuth_h