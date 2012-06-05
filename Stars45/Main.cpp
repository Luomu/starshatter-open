/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         main.cpp
	AUTHOR:       John DiCamillo
*/


#include "MemDebug.h"
#include "Starshatter.h"
#include "StarServer.h"
#include "Authorization.h"
#include "HUDView.h"

#include "NetHost.h"
#include "NetAddr.h"
#include "NetLayer.h"
#include "NetBrokerClient.h"
#include "NetClient.h"
#include "HttpClient.h"

#include "Color.h"
#include "DataLoader.h"
#include "Pcx.h"
#include "MachineInfo.h"
#include "Encrypt.h"
#include "FormatUtil.h"
#include "ParseUtil.h"
#include "Random.h"

// +--------------------------------------------------------------------+
// WinMain
// +--------------------------------------------------------------------+

extern FILE* ErrLog;
extern int   VD3D_describe_things;
int          dump_missions = 0;

const char*  versionInfo = "5.1.66";

static void PrintLogHeader()
{
	Text sTime = FormatTimeString();

	Print("+====================================================================+\n");
	Print("| STARSHATTER %-25s%29s |\n", versionInfo, sTime.data());

	Memory::SetLevel(Memory::MAXIMAL);
	Memory::OpenLog();
	MachineInfo::DescribeMachine();
}

int APIENTRY WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
LPSTR lpCmdLine, int nCmdShow)
{
	int   result    = 0;
	int   test_mode = 0;
	int   do_server = 0;

	if (strstr(lpCmdLine, "-server"))
		ErrLog = fopen("serverlog.txt", "w");
	else
		ErrLog = fopen("errlog.txt", "w");

	PrintLogHeader();

	if (strstr(lpCmdLine, "-test")) {
		Print("  Request TEST mode\n");
		test_mode = 1;
	}

	if (strstr(lpCmdLine, "-fps")) {
		HUDView::ShowFPS(true);
	}

	if (strstr(lpCmdLine, "-dump")) {
		Print("  Request dump dynamic missions\n");
		dump_missions = 1;
	}

	if (strstr(lpCmdLine, "-lan")) {
		Print("  Request LAN ONLY mode\n");
		NetBrokerClient::Disable();
	}

	if (strstr(lpCmdLine, "-server")) {
		do_server = 1;
		Print("  Request Standalone Server Mode\n");
	}

	char* d3dinfo = strstr(lpCmdLine, "-d3d");
	if (d3dinfo) {
		int n = d3dinfo[4] - '0';

		if (n >= 0 && n <= 5)
		VD3D_describe_things = n;

		Print("  D3D Info Level: %d\n", VD3D_describe_things);
	}
	else {
		VD3D_describe_things = 0;
	}


	// FREE VERSION - AUTHORIZATION DISABLED
	/*
::Print("  Checking authorization codes...\n");
if (!Authorization::IsUserAuthorized()) {
	if (!DataLoader::GetLoader()) {
		DataLoader::Initialize();
		DataLoader::GetLoader()->EnableDatafile("content.dat");
	}

	Game* game = new Game();
	game->InitContent();

	MessageBox(0, FormatTextEscape(Game::GetText("main.auth-invalid")).data(), 
					Game::GetText("main.title.error").data(), MB_OK);
	::Print("  Not authorized.\n");

	delete game;
	DataLoader::Close();
}
else {
	::Print("  Authorized\n");
	*/
	try {
		NetLayer net;

		if (do_server) {
			StarServer* server = new(__FILE__,__LINE__) StarServer();

			if (server->Init(hInstance, hPrevInstance, lpCmdLine, nCmdShow))
			result = server->Run();

			Print("\n+====================================================================+\n");
			Print("  Begin Shutdown...\n");

			delete server;
		}

		else {
			Starshatter* stars = 0;
			
			stars = new(__FILE__,__LINE__) Starshatter;
			stars->SetTestMode(test_mode);

			if (stars->Init(hInstance, hPrevInstance, lpCmdLine, nCmdShow))
			result = stars->Run();

			Print("\n+====================================================================+\n");
			Print("  Begin Shutdown...\n");

			delete stars;
		}

		Token::close();

		if (*Game::GetPanicMessage())
		MessageBox(0, Game::GetPanicMessage(), "Starshatter - Error", MB_OK);
	}

	catch (const char* msg) {
		Print("  FATAL EXCEPTION: '%s'\n", msg);
	}
	/* } */

	Memory::Stats();
	Memory::DumpLeaks();
	Memory::CloseLog();

	Print("+====================================================================+\n");
	Print(" END OF LINE.\n");

	fclose(ErrLog);

	return result;
}


