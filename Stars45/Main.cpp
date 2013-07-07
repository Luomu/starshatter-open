/*  Starshatter OpenSource Distribution
    Copyright (c) 1997-2004, Destroyer Studios LLC.
    All Rights Reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.
    * Neither the name "Destroyer Studios" nor the names of its contributors
      may be used to endorse or promote products derived from this software
      without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

    SUBSYSTEM:    Stars.exe
    FILE:         main.cpp
    AUTHOR:       John DiCamillo
*/


#include "MemDebug.h"
#include "Starshatter.h"
#include "StarServer.h"
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

const char*  versionInfo = "5.1.87 EX";

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


