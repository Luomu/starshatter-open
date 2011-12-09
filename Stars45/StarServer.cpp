/*  Project Starshatter 5.0
	Destroyer Studios LLC
	Copyright © 1997-2007. All Rights Reserved.

	SUBSYSTEM:    Stars
	FILE:         StarServer.cpp
	AUTHOR:       John DiCamillo

*/

#include "MemDebug.h"

#include "StarServer.h"
#include "Campaign.h"
#include "CombatRoster.h"
#include "Galaxy.h"
#include "Mission.h"
#include "Sim.h"
#include "SimEvent.h"
#include "Ship.h"
#include "Contact.h"
#include "QuantumDrive.h"
#include "Power.h"
#include "SystemDesign.h"
#include "WeaponDesign.h"
#include "Shot.h"
#include "Drive.h"
#include "Explosion.h"
#include "FlightDeck.h"
#include "RadioMessage.h"
#include "RadioTraffic.h"
#include "Random.h"
#include "ModConfig.h"

#include "NetLayer.h"
#include "NetGame.h"
#include "NetHost.h"
#include "NetServer.h"
#include "HttpServer.h"
#include "HttpServletExec.h"
#include "NetAdminServer.h"
#include "NetLobbyServer.h"
#include "NetServerConfig.h"

#include "Token.h"
#include "MachineInfo.h"
#include "Game.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "EventDispatch.h"
#include "MultiController.h"
#include "DataLoader.h"
#include "ParseUtil.h"
#include "Resource.h"

// +--------------------------------------------------------------------+

StarServer*       StarServer::instance = 0;

static Mission*   current_mission      = 0;
static double     time_til_change      = 0;
static bool       exit_latch           = true;

extern const char* versionInfo;

// +--------------------------------------------------------------------+

StarServer::StarServer()
: loader(0), time_mark(0), minutes(0), game_mode(MENU_MODE),
admin_server(0), lobby_server(0)
{
	if (!instance)
	instance = this;

	app_name     = "Starserver 5.0";
	title_text   = "Starserver";
	palette_name = "alpha";

	Game::server      = true;
	Game::show_mouse  = true;

	DataLoader::Initialize();
	loader       = DataLoader::GetLoader();
	int loadstat = loader->EnableDatafile("shatter.dat");

	if (loadstat != DataLoader::DATAFILE_OK) {
		const char* err_msg = loadstat == DataLoader::DATAFILE_INVALID ?
		"The file 'shatter.dat' appears to have been damaged.  Please re-install Starshatter." :
		"Starshatter cannot open the file 'shatter.dat'.  Please re-install Starshatter.";

		::MessageBox(hwnd, err_msg, "Starshatter - Error", MB_OK);
		::Print(err_msg);
		::Print("\n\nFATAL ERROR: EXIT.");
		exit(-1);
	}

#ifndef STARSHATTER_DEMO_RELEASE
	if (loader->FindFile("start.dat"))
	loader->EnableDatafile("start.dat");
#endif

	// no images or sounds in server mode:
	loader->EnableMedia(false);
}

StarServer::~StarServer()
{
	delete admin_server;
	delete lobby_server;

	admin_server = 0;
	lobby_server = 0;

	// delete all the ships and stuff
	// BEFORE getting rid of the system
	// and weapons catalogs!
	delete world;
	world = 0; // don't let base class double delete the world

	Drive::Close();
	Explosion::Close();
	FlightDeck::Close();
	Campaign::Close();
	CombatRoster::Close();
	Galaxy::Close();
	RadioTraffic::Close();
	Ship::Close();
	WeaponDesign::Close();
	SystemDesign::Close();
	DataLoader::Close();
	NetServerConfig::Close();
	ModConfig::Close();

	instance = 0;

	Game::server = false;
}

// +--------------------------------------------------------------------+

bool
StarServer::Init(HINSTANCE hi, HINSTANCE hpi, LPSTR cmdline, int nCmdShow)
{
	if (loader)
	loader->UseFileSystem(false);

	return Game::Init(hi, hpi, cmdline, nCmdShow);
}

// +--------------------------------------------------------------------+

bool
StarServer::InitGame()
{
	if (!Game::InitGame())
	return false;

	RandomInit();
	ModConfig::Initialize();
	NetServerConfig::Initialize();
	SystemDesign::Initialize("sys.def");
	WeaponDesign::Initialize("wep.def");
	Ship::Initialize();
	Galaxy::Initialize();
	CombatRoster::Initialize();
	Campaign::Initialize();

	Drive::Initialize();
	Explosion::Initialize();
	FlightDeck::Initialize();
	Ship::Initialize();
	Shot::Initialize();
	RadioTraffic::Initialize();

	time_mark = Game::GameTime();
	minutes   = 0;

	NetServerConfig* server_config = NetServerConfig::GetInstance();
	if (!server_config)
	return false;

	::Print("\n\n\nStarshatter Server Init\n");
	::Print("-----------------------\n");
	::Print("Server Name:       %s\n", (const char*) server_config->Name());
	::Print("Server Type:       %d\n", server_config->GetGameType());

	if (server_config->GetMission().length() > 0)
	::Print("Server Mission:    %s\n", (const char*) server_config->GetMission());

	::Print("Lobby Server Port: %d\n", server_config->GetLobbyPort());
	::Print("Admin Server Port: %d\n", server_config->GetAdminPort());
	::Print("-----------------------\n");

	NetLobbyServer* nls = new(__FILE__,__LINE__) NetLobbyServer;
	NetAdminServer* nas = NetAdminServer::GetInstance(server_config->GetAdminPort());
	nas->SetServerName(server_config->Name());

	lobby_server = nls;
	admin_server = nas;

	return true;
}

// +--------------------------------------------------------------------+

void
StarServer::SetGameMode(int m)
{
	if (game_mode == m)
	return;

	if (m == LOAD_MODE) {
		Print("  game_mode = LOAD_MODE\n");
		paused = true;
	}

	else if (m == PLAY_MODE) {
		Print("  game_mode = PLAY_MODE\n");

		if (!world) {
			CreateWorld();
			InstantiateMission();
		}

		// stand alone server should wait for players to connect
		// before unpausing the simulation...
		SetTimeCompression(1);
		Pause(true);
	}

	else if (m == MENU_MODE) {
		Print("  game_mode = MENU_MODE\n");
		paused = true;

		Sim* sim = (Sim*) world;

		if (sim)
		sim->UnloadMission();
	}

	game_mode = m;
}

// +--------------------------------------------------------------------+

void
StarServer::SetNextMission(const char* script)
{
	if (lobby_server)
	lobby_server->SetServerMission(script);
}

// +--------------------------------------------------------------------+

void
StarServer::CreateWorld()
{
	RadioTraffic::Initialize();

	// create world
	if (!world) {
		Sim* sim = new(__FILE__,__LINE__) Sim(0);
		world = sim;
		Print("  World Created.\n");
	}
}

void
StarServer::InstantiateMission()
{
	Memory::Check();

	current_mission = 0;

	if (Campaign::GetCampaign()) {
		current_mission = Campaign::GetCampaign()->GetMission();
	}

	Sim* sim = (Sim*) world;

	if (sim) {
		sim->UnloadMission();

		if (current_mission) {
			sim->LoadMission(current_mission);
			sim->ExecMission();
			sim->SetTestMode(false);

			Print("  Mission Instantiated.\n");
		}

		else {
			Print("  *** WARNING: StarServer::InstantiateMission() - no mission selected ***\n");
		}
	}

	Memory::Check();
}

// +--------------------------------------------------------------------+

bool
StarServer::GameLoop()
{
	if (active && paused) {
		UpdateWorld();
		GameState();
	}

	else if (!active) {
		UpdateWorld();
		GameState();
		Sleep(10);
	}

	Game::GameLoop();
	return false;  // must return false to keep processing
	// true tells the outer loop to sleep until a
	// windows event is available
}

// +--------------------------------------------------------------------+

void
StarServer::UpdateWorld()
{
	long   new_time      = real_time;
	double delta         = new_time - frame_time;
	seconds       = max_frame_length;
	gui_seconds   = delta * 0.001;

	if (frame_time == 0)
	gui_seconds = 0;

	time_comp = 1;

	if (delta < max_frame_length * 1000)
	seconds = delta * 0.001;

	frame_time = new_time;

	Galaxy* galaxy = Galaxy::GetInstance();
	if (galaxy) galaxy->ExecFrame();

	Campaign* campaign = Campaign::GetCampaign();
	if (campaign) campaign->ExecFrame();

	if (paused) {
		if (world)
		world->ExecFrame(0);
	}

	else {
		game_time += (DWORD) (seconds * 1000);

		Drive::StartFrame();

		if (world)
		world->ExecFrame(seconds);
	}

	static DWORD refresh_time = 0;
	if (RealTime() - refresh_time > 1000) {
		refresh_time = RealTime();
		RedrawWindow(hwnd, 0, 0, RDW_ERASE|RDW_INVALIDATE);
	}
}

// +--------------------------------------------------------------------+

void
StarServer::GameState()
{
	if (lobby_server) {
		lobby_server->ExecFrame();

		if (lobby_server->GetStatus() == NetServerInfo::PERSISTENT)
		paused = NetGame::NumPlayers() < 1;
	}

	if (game_mode == MENU_MODE) {
		Sleep(30);
	}

	else if (game_mode == LOAD_MODE) {
		CreateWorld();
		InstantiateMission();

		SetGameMode(PLAY_MODE);
	}

	else if (game_mode == PLAY_MODE) {
		if (Game::GameTime() - time_mark > 60000) {
			time_mark = Game::GameTime();
			minutes++;
			if (minutes > 60)
			Print("  TIME %2d:%02d:00\n", minutes/60, minutes%60);
			else
			Print("  TIME    %2d:00\n", minutes);
		}

		Sleep(10);
	}

	Memory::Check();
}

// +--------------------------------------------------------------------+

bool
StarServer::OnPaint()
{
	PAINTSTRUCT paintstruct;
	HDC hdc = BeginPaint(hwnd, &paintstruct);

	Text txt_title = title_text;
	Text txt_mode;
	Text txt_users = Game::GetText("server.no-users");
	char buf[256];

	txt_title += " ";
	txt_title += versionInfo;

	switch (game_mode) {
	case LOAD_MODE:
	case MENU_MODE:  
		txt_mode = Game::GetText("server.mode.lobby");

		if (lobby_server) {
			sprintf(buf, Game::GetText("server.users").data(), lobby_server->NumUsers());
			txt_users = buf;
		}
		break;

	case PLAY_MODE:  
		txt_mode = Game::GetText("server.mode.active");
		if (lobby_server) {
			sprintf(buf, Game::GetText("server.users-and-players").data(), lobby_server->NumUsers(), NetGame::NumPlayers());
		}
		else {
			sprintf(buf, Game::GetText("server.players").data(), NetGame::NumPlayers());
		}
		txt_users = buf;
		break;

	default:
		txt_mode = Game::GetText("server.mode.other");
		break;
	}

	if (lobby_server && lobby_server->GetStatus() == NetServerInfo::PERSISTENT)
	txt_mode += " " + Game::GetText("server.alt.persistent");

	if (paused)
	txt_mode += " " + Game::GetText("server.alt.paused");

	TextOut(hdc, 4,  4, txt_title, txt_title.length());
	TextOut(hdc, 4, 22, txt_mode,  txt_mode.length());
	TextOut(hdc, 4, 40, txt_users, txt_users.length());

	Sim* sim = Sim::GetSim();
	if (sim && sim->GetMission()) {
		Mission* mission = sim->GetMission();
		Text txt_msn = Game::GetText("server.mission");
		txt_msn += mission->Name();
		TextOut(hdc, 4, 58, txt_msn, txt_msn.length());
	}

	EndPaint(hwnd, &paintstruct);
	return true;
}

// +--------------------------------------------------------------------+

DWORD WINAPI StarServerShutdownProc(LPVOID link)
{
	StarServer* stars = (StarServer*) link;

	Sleep(3000);

	if (stars) {
		stars->Exit();
		return 0;
	}

	return (DWORD) E_POINTER;
}

DWORD WINAPI StarServerRestartProc(LPVOID link)
{
	StarServer* stars = (StarServer*) link;

	Sleep(3000);

#ifdef STARSHATTER_DEMO_RELEASE

	if (stars) {
		char cmdline[256];
		strcpy(cmdline, "StarDemo -server");

		STARTUPINFO s;
		ZeroMemory(&s, sizeof(s));
		s.cb = sizeof(s);

		PROCESS_INFORMATION pi;
		ZeroMemory(&pi, sizeof(pi));

		CreateProcess("StarDemo.exe", cmdline, 0, 0, 0, 0, 0, 0, &s, &pi);
		stars->Exit();
		return 0;
	}

#else

	if (stars) {
		char cmdline[256];
		strcpy(cmdline, "stars -server");

		STARTUPINFO s;
		ZeroMemory(&s, sizeof(s));
		s.cb = sizeof(s);

		PROCESS_INFORMATION pi;
		ZeroMemory(&pi, sizeof(pi));

		CreateProcess("stars.exe", cmdline, 0, 0, 0, 0, 0, 0, &s, &pi);
		stars->Exit();
		return 0;
	}

#endif

	return (DWORD) E_POINTER;
}

void
StarServer::Shutdown(bool restart)
{
	DWORD thread_id = 0;

	if (restart)
	CreateThread(0, 4096, StarServerRestartProc,  (LPVOID) this, 0, &thread_id);
	else
	CreateThread(0, 4096, StarServerShutdownProc, (LPVOID) this, 0, &thread_id);
}