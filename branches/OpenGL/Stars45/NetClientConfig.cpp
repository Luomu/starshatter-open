/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars
	FILE:         NetClientConfig.cpp
	AUTHOR:       John DiCamillo

*/

#include "MemDebug.h"

#include "NetClientConfig.h"
#include "NetLobbyClient.h"
#include "NetBrokerClient.h"

#include "NetLayer.h"
#include "NetAddr.h"
#include "NetHost.h"

#include "Token.h"
#include "Game.h"
#include "DataLoader.h"
#include "ParseUtil.h"
#include "Resource.h"

// +--------------------------------------------------------------------+

NetClientConfig*     NetClientConfig::instance = 0;

// +--------------------------------------------------------------------+

NetClientConfig::NetClientConfig()
: server_index(-1), host_request(false), conn(0)
{
	instance = this;
	Load();
}

NetClientConfig::~NetClientConfig()
{
	Logout();

	instance = 0;
	servers.destroy();
}

// +--------------------------------------------------------------------+

void
NetClientConfig::Initialize()
{
	if (!instance)
	instance = new(__FILE__,__LINE__) NetClientConfig();
}

void
NetClientConfig::Close()
{
	delete instance;
	instance = 0;
}

// +--------------------------------------------------------------------+

void
NetClientConfig::AddServer(const char* name, const char* addr, WORD port, const char* pass, bool save)
{
	if (!addr || !*addr || port < 1024 || port > 48000)
	return;

	char buffer[1024];
	if (name && *name)
	strcpy_s(buffer, name);
	else
	sprintf_s(buffer, "%s:%d", addr, port);

	NetServerInfo* server = new(__FILE__,__LINE__) NetServerInfo;
	server->name     = buffer;
	server->hostname = addr;
	server->addr     = NetAddr(addr, port);
	server->port     = port;
	server->password = pass;
	server->save     = save;

	if (server->addr.IPAddr() == 0) {
		Print("NetClientConfig::AddServer(%s, %s, %d) failed to resolve IP Addr\n",
		name, addr, port);
	}

	servers.append(server);
}

void
NetClientConfig::DelServer(int index)
{
	if (index >= 0 && index < servers.size()) {
		delete servers.removeIndex(index);
	}
}

// +--------------------------------------------------------------------+

NetServerInfo*
NetClientConfig::GetServerInfo(int n)
{
	if (n >= 0 && n < servers.size())
	return servers.at(n);

	return 0;
}

// +--------------------------------------------------------------------+

NetServerInfo*
NetClientConfig::GetSelectedServer()
{
	if (server_index >= 0 && server_index < servers.size())
	return servers.at(server_index);

	return 0;
}

// +--------------------------------------------------------------------+

void
NetClientConfig::Download()
{
	Load();

	List<NetServerInfo> list;
	if (NetBrokerClient::GameList("Starshatter", list)) {
		servers.append(list);
	}
}

// +--------------------------------------------------------------------+

void
NetClientConfig::Load()
{
	server_index = -1;

	// read the config file:
	BYTE*       block    = 0;
	int         blocklen = 0;

	char        filename[64];
	strcpy_s(filename, "client.cfg");

	FILE* f;
	::fopen_s(&f, filename, "rb");

	if (f) {
		::fseek(f, 0, SEEK_END);
		blocklen = ftell(f);
		::fseek(f, 0, SEEK_SET);

		block = new(__FILE__,__LINE__) BYTE[blocklen+1];
		block[blocklen] = 0;

		::fread(block, blocklen, 1, f);
		::fclose(f);
	}

	if (blocklen == 0)
	return;

	servers.destroy();

	Parser parser(new(__FILE__,__LINE__) BlockReader((const char*) block, blocklen));
	Term*  term = parser.ParseTerm();

	if (!term) {
		Print("ERROR: could not parse '%s'.\n", filename);
		return;
	}
	else {
		TermText* file_type = term->isText();
		if (!file_type || file_type->value() != "CLIENT_CONFIG") {
			Print("WARNING: invalid '%s' file.  Using defaults\n", filename);
			return;
		}
	}

	do {
		delete term;

		term = parser.ParseTerm();
		
		if (term) {
			TermDef* def = term->isDef();
			if (def) {
				if (def->name()->value() == "server") {
					if (!def->term() || !def->term()->isStruct()) {
						Print("WARNING: server struct missing in '%s'\n", filename);
					}
					else {
						TermStruct* val = def->term()->isStruct();

						Text name;
						Text addr;
						Text pass;
						int  port;

						for (int i = 0; i < val->elements()->size(); i++) {
							TermDef* pdef = val->elements()->at(i)->isDef();
							if (pdef) {
								if (pdef->name()->value() == "name")
								GetDefText(name, pdef, filename);
								else if (pdef->name()->value() == "addr")
								GetDefText(addr, pdef, filename);
								else if (pdef->name()->value() == "pass")
								GetDefText(pass, pdef, filename);
								else if (pdef->name()->value() == "port")
								GetDefNumber(port, pdef, filename);
							}
						}

						AddServer(name, addr, (WORD) port, pass, true);
					}
				}
				else {
					Print("WARNING: unknown label '%s' in '%s'\n",
					def->name()->value().data(), filename);
				}
			}
		}
	}
	while (term);

	delete [] block;
}

// +--------------------------------------------------------------------+

void
NetClientConfig::Save()
{
	FILE* f;
	fopen_s(&f, "client.cfg", "w");
	if (f) {
		fprintf(f, "CLIENT_CONFIG\n\n");

		ListIter<NetServerInfo> iter = servers;
		while (++iter) {
			NetServerInfo* server = iter.value();

			if (server->save) {
				int port = (int) server->port;
				fprintf(f, "server: {\n");
				fprintf(f, "  name: \"%s\",\n", (const char*) server->name);
				fprintf(f, "  addr: \"%s\",\n", (const char*) server->hostname);
				fprintf(f, "  port: %d,\n",                   port);

				if (server->password.length())
				fprintf(f, "  pass: \"%s\",\n", (const char*) server->password);

				fprintf(f, "}\n\n");
			}
		}

		fclose(f);
	}
}

// +--------------------------------------------------------------------+

void
NetClientConfig::CreateConnection()
{
	NetServerInfo* s = GetSelectedServer();

	if (s) {
		NetAddr addr = s->addr;

		if (conn) {
			if (conn->GetServerAddr().IPAddr() != addr.IPAddr() ||
					conn->GetServerAddr().Port()   != addr.Port()) {
				conn->Logout();
				DropConnection();
			}
		}

		if (addr.IPAddr() && addr.Port() && !conn) {
			conn = new(__FILE__,__LINE__) NetLobbyClient; // (addr);
		}
	}

	else if (conn) {
		conn->Logout();
		DropConnection();
	}
}

NetLobbyClient*
NetClientConfig::GetConnection()
{
	return conn;
}

bool
NetClientConfig::Login()
{
	bool result = false;

	if (!conn)
	CreateConnection();

	if (conn)
	result = conn->Login(host_request);

	return result;
}

bool
NetClientConfig::Logout()
{
	bool result = false;

	if (conn) {
		result = conn->Logout();
		DropConnection();
	}

	return result;
}

void
NetClientConfig::DropConnection()
{
	delete conn;
	conn = 0;
}
