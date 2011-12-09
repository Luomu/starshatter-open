/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars
	FILE:         NetServerConfig.cpp
	AUTHOR:       John DiCamillo

*/

#include "MemDebug.h"
#include "NetServerConfig.h"
#include "NetUser.h"

#include "NetLayer.h"
#include "NetAddr.h"
#include "NetGame.h"
#include "NetHost.h"
#include "NetServer.h"
#include "HttpServer.h"
#include "HttpServletExec.h"
#include "NetLobbyServer.h"
#include "NetAuth.h"

#include "token.h"
#include "Game.h"
#include "DataLoader.h"
#include "ParseUtil.h"
#include "Resource.h"

// +--------------------------------------------------------------------+

NetServerConfig*     NetServerConfig::instance = 0;
extern const char*   versionInfo;

// +--------------------------------------------------------------------+

NetServerConfig::NetServerConfig()
{
	instance = this;

	name              = "Starshatter ";
	admin_name        = "system";
	admin_pass        = "manager";
	admin_port        = 11111;
	lobby_port        = 11100;
	game_port         = 11101;
	game_type         = NET_GAME_PUBLIC;
	auth_level        = NetAuth::NET_AUTH_STANDARD;
	poolsize          = 8;
	session_timeout   = 300;

	name += versionInfo;

	Load();
}

NetServerConfig::~NetServerConfig()
{
	instance = 0;

	banned_addrs.destroy();
	banned_names.destroy();
}

// +--------------------------------------------------------------------+

void
NetServerConfig::Initialize()
{
	if (!instance)
	instance = new(__FILE__,__LINE__) NetServerConfig();
}

void
NetServerConfig::Close()
{
	delete instance;
	instance = 0;
}

// +--------------------------------------------------------------------+

void
NetServerConfig::Load()
{
	// read the config file:
	BYTE*       block    = 0;
	int         blocklen = 0;
	int         port     = 0;

	char        filename[64];
	strcpy(filename, "server.cfg");

	FILE* f = ::fopen(filename, "rb");

	if (f) {
		::fseek(f, 0, SEEK_END);
		blocklen = ftell(f);
		::fseek(f, 0, SEEK_SET);

		block = new(__FILE__,__LINE__) BYTE[blocklen+1];
		block[blocklen] = 0;

		::fread(block, blocklen, 1, f);
		::fclose(f);
	}

	if (blocklen == 0) {
		delete [] block;
		return;
	}

	Parser parser(new(__FILE__,__LINE__) BlockReader((const char*) block, blocklen));
	Term*  term = parser.ParseTerm();

	if (!term) {
		Print("ERROR: could not parse '%s'.\n", filename);
		delete [] block;
		return;
	}
	else {
		TermText* file_type = term->isText();
		if (!file_type || file_type->value() != "SERVER_CONFIG") {
			Print("WARNING: invalid '%s' file.  Using defaults\n", filename);
			delete [] block;
			return;
		}
	}

	do {
		delete term;

		term = parser.ParseTerm();
		
		if (term) {
			TermDef* def = term->isDef();
			if (def) {
				if (def->name()->value() == "name") {
					GetDefText(instance->name, def, filename);
				}

				else if (def->name()->value() == "admin_name") {
					GetDefText(instance->admin_name, def, filename);
				}

				else if (def->name()->value() == "admin_pass") {
					GetDefText(instance->admin_pass, def, filename);
				}

				else if (def->name()->value() == "game_pass") {
					GetDefText(instance->game_pass, def, filename);
				}

				else if (def->name()->value() == "mission") {
					GetDefText(instance->mission, def, filename);
				}

				else if (def->name()->value() == "auth_level") {
					Text level;

					if (def->term() && def->term()->isText()) {
						GetDefText(level, def, filename);

						level.toLower();

						if (level.indexOf("min") == 0)
						instance->auth_level = NetAuth::NET_AUTH_MINIMAL;

						else if (level == "standard" || level == "std")
						instance->auth_level = NetAuth::NET_AUTH_STANDARD;

						else if (level == "secure")
						instance->auth_level = NetAuth::NET_AUTH_SECURE;
					}

					else {
						GetDefNumber(instance->auth_level, def, filename);
					}
				}

				else if (def->name()->value() == "admin_port") {
					GetDefNumber(port, def, filename);
					if (port > 1024 && port < 48000)
					instance->admin_port = (WORD) port;
				}

				else if (def->name()->value() == "lobby_port") {
					GetDefNumber(port, def, filename);
					if (port > 1024 && port < 48000)
					instance->lobby_port = (WORD) port;
				}

				else if (def->name()->value() == "game_port") {
					GetDefNumber(port, def, filename);
					if (port > 1024 && port < 48000)
					instance->game_port = (WORD) port;
				}

				else if (def->name()->value() == "game_type") {
					Text type;
					GetDefText(type, def, filename);
					type.setSensitive(false);

					if (type == "LAN")
					instance->game_type = NET_GAME_LAN;

					else if (type == "private")
					instance->game_type = NET_GAME_PRIVATE;

					else
					instance->game_type = NET_GAME_PUBLIC;
				}

				else if (def->name()->value() == "poolsize") {
					GetDefNumber(instance->poolsize, def, filename);
				}

				else if (def->name()->value() == "session_timeout") {
					GetDefNumber(instance->session_timeout, def, filename);
				}

				else
				Print("WARNING: unknown label '%s' in '%s'\n",
				def->name()->value().data(), filename);
			}
			else {
				Print("WARNING: term ignored in '%s'\n", filename);
				term->print();
			}
		}
	}
	while (term);

	delete [] block;

	LoadBanList();
}

// +--------------------------------------------------------------------+

void
NetServerConfig::Save()
{
	FILE* f = fopen("server.cfg", "w");
	if (f) {
		fprintf(f, "SERVER_CONFIG\n\n");
		fprintf(f, "name:            \"%s\"\n", instance->name.data());
		fprintf(f, "admin_name:      \"%s\"\n", instance->admin_name.data());
		fprintf(f, "admin_pass:      \"%s\"\n", instance->admin_pass.data());
		fprintf(f, "game_pass:       \"%s\"\n", instance->game_pass.data());
		fprintf(f, "\n");
		fprintf(f, "admin_port:      %d\n", instance->admin_port);
		fprintf(f, "lobby_port:      %d\n", instance->lobby_port);
		fprintf(f, "game_port:       %d\n", instance->game_port);

		switch (instance->game_type) {
		case NET_GAME_LAN:
			fprintf(f, "game_type:       LAN\n");
			break;

		case NET_GAME_PRIVATE:
			fprintf(f, "game_type:       private\n");
			break;

		case NET_GAME_PUBLIC:
		default:
			fprintf(f, "game_type:       public\n");
			break;
		}

		switch (instance->auth_level) {
		case NetAuth::NET_AUTH_MINIMAL:
			fprintf(f, "auth_level:      minimal\n");
			break;

		case NetAuth::NET_AUTH_STANDARD:
		default:
			fprintf(f, "auth_level:      standard\n");
			break;

		case NetAuth::NET_AUTH_SECURE:
			fprintf(f, "auth_level:      secure\n");
			break;
		}

		fprintf(f, "\n");
		fprintf(f, "poolsize:        %d\n", instance->poolsize);
		fprintf(f, "session_timeout: %d\n", instance->session_timeout);

		if (mission.length() > 0) {
			fprintf(f, "\nmission:         \"%s\"\n", instance->mission.data());
		}

		fclose(f);
	}
}

// +--------------------------------------------------------------------+

Text
NetServerConfig::Clean(const char* s)
{
	if (!s || !*s) return Text();

	int len = strlen(s);
	char* buff = new(__FILE__,__LINE__) char[len+1];
	ZeroMemory(buff, len+1);

	char* p = buff;

	for (int i = 0; i < len; i++) {
		char c = s[i];

		if (c >= 32 && c < 127)
		*p++ = c;
	}

	Text result(buff);
	delete [] buff;

	return result;
}

// +--------------------------------------------------------------------+

void
NetServerConfig::LoadBanList()
{
	// read the config file:
	BYTE*       block    = 0;
	int         blocklen = 0;
	int         port     = 0;

	char        filename[64];
	strcpy(filename, "banned.cfg");

	FILE* f = ::fopen(filename, "rb");

	if (f) {
		::fseek(f, 0, SEEK_END);
		blocklen = ftell(f);
		::fseek(f, 0, SEEK_SET);

		block = new(__FILE__,__LINE__) BYTE[blocklen+1];
		block[blocklen] = 0;

		::fread(block, blocklen, 1, f);
		::fclose(f);
	}

	if (blocklen == 0) {
		delete [] block;
		return;
	}

	Parser parser(new(__FILE__,__LINE__) BlockReader((const char*) block, blocklen));
	Term*  term = parser.ParseTerm();

	if (!term) {
		Print("ERROR: could not parse '%s'.\n", filename);
		delete [] block;
		return;
	}
	else {
		TermText* file_type = term->isText();
		if (!file_type || file_type->value() != "BANNED_CONFIG") {
			Print("WARNING: invalid '%s' file.\n", filename);
			delete [] block;
			return;
		}
	}

	banned_addrs.destroy();
	banned_names.destroy();

	do {
		delete term;

		term = parser.ParseTerm();
		
		if (term) {
			TermDef* def = term->isDef();
			if (def) {
				if (def->name()->value() == "name") {
					Text name;
					GetDefText(name, def, filename);
					banned_names.append(new(__FILE__,__LINE__) Text(name));
				}

				else if (def->name()->value() == "addr") {
					DWORD addr;
					GetDefNumber(addr, def, filename);
					banned_addrs.append(new(__FILE__,__LINE__) NetAddr(addr));
				}
			}

			else {
				Print("WARNING: term ignored in '%s'\n", filename);
				term->print();
			}
		}
	}
	while (term);

	delete [] block;
}

void
NetServerConfig::BanUser(NetUser* user)
{
	if (!user || IsUserBanned(user))
	return;

	NetAddr* user_addr = new(__FILE__,__LINE__) NetAddr(user->GetAddress().IPAddr());
	Text*    user_name = new(__FILE__,__LINE__) Text(user->Name());

	banned_addrs.append(user_addr);
	banned_names.append(user_name);

	FILE* f = fopen("banned.cfg", "w");
	if (f) {
		fprintf(f, "BANNED_CONFIG\n\n");

		ListIter<NetAddr> a_iter = banned_addrs;
		while (++a_iter) {
			NetAddr* addr = a_iter.value();
			fprintf(f, "addr: 0x%08x  // %d.%d.%d.%d\n",
			addr->IPAddr(),
			addr->B1(),
			addr->B2(),
			addr->B3(),
			addr->B4());
		}

		fprintf(f, "\n");

		ListIter<Text> n_iter = banned_names;
		while (++n_iter) {
			Text* name = n_iter.value();
			fprintf(f, "name: \"%s\"\n", name->data());
		}

		fclose(f);
	}
}

bool
NetServerConfig::IsUserBanned(NetUser* user)
{
	if (user) {
		NetAddr  user_addr = user->GetAddress();
		Text     user_name = user->Name();

		user_addr.SetPort(0);

		return banned_addrs.contains(&user_addr) ||
		banned_names.contains(&user_name);
	}

	return false;
}

