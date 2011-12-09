/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         NetUser.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	This class represents a user connecting to the multiplayer lobby
*/


#include "MemDebug.h"
#include "NetUser.h"
#include "NetAuth.h"
#include "NetLobby.h"
#include "Player.h"
#include "FormatUtil.h"

// +-------------------------------------------------------------------+

static Color user_colors[] = {
	Color::BrightBlue,
	Color::BrightRed,
	Color::BrightGreen,
	Color::Cyan,
	Color::Orange,
	Color::Magenta,
	Color::Yellow,
	Color::White,
	Color::Gray,
	Color::DarkRed,
	Color::Tan,
	Color::Violet
};

static int user_color_index = 0;

// +-------------------------------------------------------------------+

NetUser::NetUser(const char* n)
: name(n), netid(0), host(false),
rank(0), flight_time(0), missions(0), kills(0), losses(0),
auth_state(NetAuth::NET_AUTH_INITIAL),
auth_level(NetAuth::NET_AUTH_MINIMAL)
{
	if (user_color_index < 0 || user_color_index >= 12)
	user_color_index = 0;

	color = user_colors[user_color_index++];

	ZeroMemory(salt, sizeof(salt));
}

NetUser::NetUser(const Player* player)
: netid(0), host(false),
rank(0), flight_time(0), missions(0), kills(0), losses(0),
auth_state(NetAuth::NET_AUTH_INITIAL),
auth_level(NetAuth::NET_AUTH_MINIMAL)
{
	if (user_color_index < 0 || user_color_index >= 12)
	user_color_index = 0;

	color = user_colors[user_color_index++];

	if (player) {
		name        = player->Name();
		pass        = player->Password();
		signature   = player->Signature();
		squadron    = player->Squadron();
		rank        = player->Rank();
		flight_time = player->FlightTime();
		missions    = player->Missions();
		kills       = player->Kills();
		losses      = player->Losses();
	}

	ZeroMemory(salt, sizeof(salt));
}

NetUser::~NetUser()
{ }

// +-------------------------------------------------------------------+

Text
NetUser::GetDescription()
{
	Text content;

	content += "name \"";
	content += SafeQuotes(name);
	content += "\" sig \"";
	content += SafeQuotes(signature);
	content += "\" squad \"";
	content += SafeQuotes(squadron);

	char buffer[256];
	sprintf_s(buffer, "\" rank %d time %d miss %d kill %d loss %d host %s ",
	rank, flight_time, missions, kills, losses,
	host ? "true" : "false");

	content += buffer;

	return content;
}

// +-------------------------------------------------------------------+

bool
NetUser::IsAuthOK() const
{
	return auth_state == NetAuth::NET_AUTH_OK;
}