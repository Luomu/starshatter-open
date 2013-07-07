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
    FILE:         Starshatter.cpp
    AUTHOR:       John DiCamillo

*/


#include "MemDebug.h"
#include "Starshatter.h"

#include "MenuScreen.h"
#include "LoadScreen.h"
#include "PlanScreen.h"
#include "CmpnScreen.h"

#include "AudioConfig.h"
#include "MusicDirector.h"
#include "HUDSounds.h"
#include "Player.h"

#include "Shot.h"
#include "Drive.h"
#include "LandingGear.h"
#include "Explosion.h"
#include "FlightDeck.h"
#include "NavLight.h"
#include "Debris.h"
#include "Contact.h"
#include "QuantumDrive.h"
#include "Sensor.h"
#include "Power.h"
#include "SystemDesign.h"
#include "WeaponDesign.h"

#include "Campaign.h"
#include "CampaignSaveGame.h"
#include "CombatRoster.h"
#include "CombatZone.h"
#include "CampaignPlan.h"

#include "Galaxy.h"
#include "StarSystem.h"
#include "Mission.h"
#include "Sim.h"
#include "SimEvent.h"
#include "Element.h"
#include "Ship.h"
#include "ShipCtrl.h"
#include "ShipDesign.h"
#include "HUDView.h"
#include "MFD.h"
#include "RadioMessage.h"
#include "RadioTraffic.h"
#include "RadioVox.h"
#include "CameraDirector.h"
#include "ModConfig.h"
#include "KeyMap.h"

#include "GameScreen.h"
#include "QuantumView.h"
#include "QuitView.h"
#include "RadioView.h"
#include "TacticalView.h"
#include "DisplayView.h"

#include "LoadDlg.h"
#include "TacRefDlg.h"
#include "CmpLoadDlg.h"
#include "Terrain.h"

#include "NetClientConfig.h"
#include "NetServerConfig.h"
#include "NetLayer.h"
#include "NetLobbyClient.h"
#include "NetLobbyServer.h"
#include "NetGame.h"
#include "NetUtil.h"

#include "ParseUtil.h"
#include "Token.h"

#include "MachineInfo.h"
#include "Game.h"
#include "VideoFactory.h"
#include "Screen.h"
#include "Window.h"
#include "ActiveWindow.h"
#include "Button.h"
#include "CameraView.h"
#include "ImgView.h"
#include "FadeView.h"
#include "Color.h"
#include "Bitmap.h"
#include "Font.h"
#include "FontMgr.h"
#include "Keyboard.h"
#include "Joystick.h"
#include "MouseController.h"
#include "Mouse.h"
#include "TrackIR.h"
#include "EventDispatch.h"
#include "MultiController.h"
#include "Archive.h"
#include "DataLoader.h"
#include "Random.h"
#include "Resource.h"
#include "Universe.h"
#include "Video.h"
#include "VideoSettings.h"
#include "WebBrowser.h"

// +--------------------------------------------------------------------+

int            quick_mode = 0;
char           quick_mission_name[64];
Mission*       quick_mission = 0;

int            Starshatter::keymap[256];
int            Starshatter::keyalt[256];
Starshatter*   Starshatter::instance = 0;

static Mission*   current_mission   = 0;
static Mission*   cutscene_mission  = 0;
static double     cutscene_basetime = 0;
static int        cut_efx_volume    = 100;
static int        cut_wrn_volume    = 100;
static double     time_til_change   = 0;
static bool       exit_latch        = true;
static bool       show_missions     = false;
static bool       use_file_system   = false;
static bool       no_splash         = false;

enum CHAT_MODES {
    CHAT_BROADCAST = 1,
    CHAT_TEAM      = 2,
    CHAT_WING      = 3,
    CHAT_UNIT      = 4
};

// +--------------------------------------------------------------------+

Starshatter::Starshatter()
: gamewin(0), menuscreen(0), loadscreen(0), planscreen(0),
cmpnscreen(0), gamescreen(0), splash(0), splash_index(0), 
input(0), loader(0), cam_dir(0), music_dir(0),
field_of_view(2), time_mark(0), minutes(0), 
player_ship(0), net_lobby(0),
spinning(false), tactical(false), mouse_x(0), mouse_y(0),
game_mode(MENU_MODE), mouse_input(0), head_tracker(0),
terminal(0), verdana(0), limerick18(0), limerick12(0),
HUDfont(0), GUIfont(0), GUI_small_font(0), title_font(0),
ocrb(0), req_change_video(0), video_changed(0),
lens_flare(true), corona(true), nebula(true), dust(0),
load_step(0), load_progress(0),
chat_mode(0), exit_time(1.2), cutscene(0)
{
    if (!instance)
    instance = this;

    app_name     = "Starshatter: The Gathering Storm";
    title_text   = "STARSHATTER";
    palette_name = "alpha";

    gamma        = 128; // default - flat gamma ramp

    if (!DataLoader::GetLoader())
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

    if (loader->FindFile("vox.dat"))
    loader->EnableDatafile("vox.dat");

    if (loader->FindFile("start.dat"))
    loader->EnableDatafile("start.dat");

    if (loader->FindFile("content.dat"))
    loader->EnableDatafile("content.dat");

    //if (loadstat != DataLoader::DATAFILE_OK) {
    //    const char* err_msg = loadstat == DataLoader::DATAFILE_INVALID ?
    //    "The file 'content.dat' appears to have been damaged.  Please re-install the latest Starshatter update." :
    //    "Starshatter cannot open the file 'content.dat'.  Please re-install the latest Starshatter update.";

    //    ::MessageBox(hwnd, err_msg, "Starshatter - Error", MB_OK);
    //    ::Print(err_msg);
    //    ::Print("\n\nFATAL ERROR: EXIT.");
    //    exit(-1);
    //}

    LoadVideoConfig("video.cfg");

    // create the fonts
    loader->SetDataPath("Fonts/");

    HUDfont = new(__FILE__,__LINE__) Font("HUDfont");
    FontMgr::Register("HUD", HUDfont);

    GUIfont = new(__FILE__,__LINE__) Font("GUIfont");
    FontMgr::Register("GUI", GUIfont);

    GUI_small_font = new(__FILE__,__LINE__) Font("GUIsmall");
    FontMgr::Register("GUIsmall", GUI_small_font);

    limerick12  = new(__FILE__,__LINE__) Font("Limerick12");
    limerick18  = new(__FILE__,__LINE__) Font("Limerick18");
    terminal    = new(__FILE__,__LINE__) Font("Terminal");
    verdana     = new(__FILE__,__LINE__) Font("Verdana");
    ocrb        = new(__FILE__,__LINE__) Font("OCRB");

    FontMgr::Register("Limerick12",  limerick12);
    FontMgr::Register("Limerick18",  limerick18);
    FontMgr::Register("Terminal",    terminal);
    FontMgr::Register("Verdana",     verdana);
    FontMgr::Register("OCRB",        ocrb);

    loader->SetDataPath(0);

    ZeroMemory(keymap, sizeof(keymap));
    ZeroMemory(keyalt, sizeof(keyalt));
}

Starshatter::~Starshatter()
{
    if (video_changed) {
        SaveVideoConfig("video.cfg");
    }

    DeleteFile("video2.cfg");
    StopLobby();

    if (Status() <= EXIT)
    Player::Save();

    delete menuscreen;
    delete loadscreen;
    delete planscreen;
    delete gamescreen;
    delete cmpnscreen;

    menuscreen = 0;
    loadscreen = 0;
    planscreen = 0;
    gamescreen = 0;
    cmpnscreen = 0;

    music_dir  = 0;

    // delete all the ships and stuff
    // BEFORE getting rid of the system
    // and weapons catalogs!
    delete world;
    world = 0; // don't let base class double delete the world

    delete quick_mission;

    AudioConfig::Close();
    HUDSounds::Close();
    MusicDirector::Close();

    Player::Close();
    Drive::Close();
    LandingGear::Close();
    MFD::Close();
    Explosion::Close();
    FlightDeck::Close();
    Campaign::Close();
    CombatRoster::Close();
    Galaxy::Close();
    RadioTraffic::Close();
    RadioVox::Close();
    Ship::Close();
    WeaponDesign::Close();
    SystemDesign::Close();
    ModConfig::Close();
    NetClientConfig::Close();
    TacticalView::Close();
    QuantumView::Close();
    QuitView::Close();
    RadioView::Close();
    NetServerConfig::Close();

    Mouse::Close();
    EventDispatch::Close();
    FontMgr::Close();
    Button::Close();
    DataLoader::Close();

    delete ocrb;
    delete limerick12;
    delete limerick18;
    delete verdana;
    delete terminal;
    delete HUDfont;
    delete GUIfont;
    delete GUI_small_font;
    delete input;
    delete head_tracker;

    instance = 0;
}

void
Starshatter::Exit()
{
    MusicDirector::SetMode(MusicDirector::NONE);
    SetGameMode(EXIT_MODE);
}

// +--------------------------------------------------------------------+

bool
Starshatter::OnHelp()
{
    WebBrowser browser;
    browser.OpenURL("http://matrixgames.com/support");
    return true;
}

// +--------------------------------------------------------------------+

void
Starshatter::MapKeys()
{
    int nkeys = keycfg.GetNumKeys();

    if (nkeys > 0) {
        Starshatter::MapKeys(&keycfg, nkeys);
        input->MapKeys(keycfg.GetMapping(), nkeys);
    }
}

void
Starshatter::MapKeys(KeyMap* mapping, int nkeys)
{
    for (int i = 0; i < nkeys; i++) {
        KeyMapEntry* k = mapping->GetKeyMap(i);
        
        if (k->act >= KEY_MAP_FIRST && k->act <= KEY_MAP_LAST)
        MapKey(k->act, k->key, k->alt);
    }
}

void
Starshatter::MapKey(int act, int key, int alt)
{
    keymap[act] = key;
    keyalt[act] = alt;

    GetAsyncKeyState(key);
    GetAsyncKeyState(alt);
}

// +--------------------------------------------------------------------+

bool
Starshatter::Init(HINSTANCE hi, HINSTANCE hpi, LPSTR cmdline, int nCmdShow)
{
    if (strstr(cmdline, "-win") || strstr(cmdline, "-dbg")) {
        if (video_settings) {
            video_settings->is_windowed   = true;

            Print("   STARSHATTER RUNNING IN WINDOW MODE\n");
        }
    }

    if (strstr(cmdline, "-filesys")) {
        use_file_system = true;
        Print("   FILE SYSTEM ENABLED\n");
    }

    if (strstr(cmdline, "-nosplash")) {
        no_splash = true;
    }

    if (loader)
    loader->UseFileSystem(use_file_system);

    return Game::Init(hi, hpi, cmdline, nCmdShow);
}

// +--------------------------------------------------------------------+

bool
Starshatter::InitGame()
{
    if (!Game::InitGame())
    return false;

    RandomInit();

    AudioConfig::Initialize();
    ModConfig::Initialize();

    InitMouse();

    Button::Initialize();
    EventDispatch::Create();
    NetClientConfig::Initialize();
    Player::Initialize();
    HUDSounds::Initialize();

    int nkeys = keycfg.LoadKeyMap("key.cfg", 256);

    if (nkeys)
    Print("  Loaded key.cfg\n\n");

    // create the appropriate motion controller and player_ship
    input = new(__FILE__,__LINE__) MultiController;
    Keyboard* k = new(__FILE__,__LINE__) Keyboard;
    input->AddController(k);
    ActivateKeyboardLayout(GetKeyboardLayout(0), 0);

    mouse_input = new(__FILE__,__LINE__) MouseController;
    input->AddController(mouse_input);

    ::Print("\nStarshatter::InitGame() create joystick\n");
    Joystick* j = new(__FILE__,__LINE__) Joystick;
    j->SetSensitivity(15, 5000);
    input->AddController(j);

    Joystick::EnumerateDevices();
    ::Print("\n");

    head_tracker = new(__FILE__,__LINE__) TrackIR();
    MapKeys();

    SystemDesign::Initialize("sys.def");
    WeaponDesign::Initialize("wep.def");
    MusicDirector::Initialize();

    // if no splashes, we need to initialize the campaign engine now
    if (no_splash) {
        Ship::Initialize();
        Galaxy::Initialize();
        CombatRoster::Initialize();
        Campaign::Initialize();
    }

    // otherwise, the campaign engine will get initialized during the splashes
    else {
        SetupSplash();
    }

    time_mark = Game::GameTime();
    minutes   = 0;

    return true;
}

void
Starshatter::InitMouse()
{
    if (loader) {
        loader->SetDataPath(0);

        Mouse::Create(screen);
        Mouse::Show(false);
        Mouse::LoadCursor(Mouse::ARROW, "MouseArrow.pcx", Mouse::HOTSPOT_NW);
        Mouse::LoadCursor(Mouse::CROSS, "MouseCross.pcx", Mouse::HOTSPOT_CTR);
        Mouse::LoadCursor(Mouse::DRAG,  "MouseDrag.pcx",  Mouse::HOTSPOT_NW);
        Mouse::SetCursor(Mouse::ARROW);
    }
}

// +--------------------------------------------------------------------+

void
Starshatter::RequestChangeVideo()
{
    req_change_video = true;
}

int
Starshatter::GetScreenWidth()
{
    if (video_settings)
    return video_settings->GetWidth();

    return 0;
}

// +--------------------------------------------------------------------+

int
Starshatter::GetScreenHeight()
{
    if (video_settings)
    return video_settings->GetHeight();

    return 0;
}

// +--------------------------------------------------------------------+

void
Starshatter::StartOrResumeGame()
{
    if (game_mode != MENU_MODE && game_mode != CMPN_MODE)
    return;

    Player* p = Player::GetCurrentPlayer();
    if (!p)
    return;

    List<Campaign>& list    = Campaign::GetAllCampaigns();
    Campaign*       c       = 0;
    Text            saved   = CampaignSaveGame::GetResumeFile();


    // resume saved game?
    if (saved.length()) {
        CampaignSaveGame  savegame;
        savegame.Load(saved);
        c = savegame.GetCampaign();
    }

    // start training campaign?
    else if (p->Trained() < 255) {
        c = list[0];
        c->Load();
    }

    // start new dynamic campaign sequence?
    else {
        c = list[1];
        c->Load();
    }

    if (c)
    Campaign::SelectCampaign(c->Name());

    Mouse::Show(false);
    SetGameMode(CLOD_MODE);
}

// +--------------------------------------------------------------------+

bool
Starshatter::UseFileSystem()
{
    return use_file_system;
}

// +--------------------------------------------------------------------+

void
Starshatter::OpenTacticalReference()
{
    if (menuscreen && game_mode == MENU_MODE) {
        menuscreen->ShowLoadDlg();

        LoadDlg* load_dlg = menuscreen->GetLoadDlg();

        if (load_dlg && load_dlg->IsShown()) {
            load_activity = Game::GetText("Starshatter.load.tac-ref");
            load_progress = 1;
            catalog_index = 0;
        }
        else {
            menuscreen->ShowTacRefDlg();
        }
    }
}

// +--------------------------------------------------------------------+

void
Starshatter::SetGameMode(int m)
{
    if (game_mode == m)
    return;

    const char* mode_name[] = {
        "MENU_MODE",  // main menu
        "CLOD_MODE",  // loading campaign
        "CMPN_MODE",  // operational command for dynamic campaign
        "PREP_MODE",  // loading mission info for planning
        "PLAN_MODE",  // mission briefing
        "LOAD_MODE",  // loading mission into simulator
        "PLAY_MODE",  // active simulation
        "EXIT_MODE"   // shutting down
    };

    if (m >= MENU_MODE && m <= EXIT_MODE)
    Print(">>> Starshatter::SetGameMode(%d) (%s)\n", m, mode_name[m]);
    else
    Print(">>> Starshatter::SetGameMode(%d) (UNKNOWN MODE)\n", m);

    MouseController* mouse_con = MouseController::GetInstance();
    if (mouse_con)
    mouse_con->SetActive(false);

    if (m == CLOD_MODE || m == PREP_MODE || m == LOAD_MODE) {
        load_step     = 0;
        load_progress = 0;
        load_activity = Game::GetText("Starshatter.load.general");
        paused = true;
    }

    else if (m == CMPN_MODE) {
        load_step     = 0;
        load_progress = 100;
        load_activity = Game::GetText("Starshatter.load.complete");
        paused = false;
    }

    else if (m == PLAY_MODE) {
        Print("  Starting Game...\n");

        player_ship   = 0;
        load_progress = 100;
        load_activity = Game::GetText("Starshatter.load.complete");

        if (!world) {
            CreateWorld();
            InstantiateMission();
        }

        if (gamescreen)
        gamescreen->SetFieldOfView(field_of_view);

        HUDView::ClearMessages();
        RadioView::ClearMessages();

        SetTimeCompression(1);
        Pause(false);

        Print("  Stardate: %.1f\n", StarSystem::GetBaseTime());
    }

    else if (m == PLAN_MODE) {
        if (game_mode == PLAY_MODE) {
            Print("  Returning to Plan Mode...\n");
            if (soundcard)
            soundcard->StopSoundEffects();

            StopNetGame();
            Pause(true);
            Print("  Stardate: %.1f\n", StarSystem::GetBaseTime());
        }
    }

    else if (m == MENU_MODE) {
        Print("  Returning to Main Menu...\n");

        if (game_mode == PLAN_MODE || game_mode == PLAY_MODE) {
            if (soundcard)
            soundcard->StopSoundEffects();

            StopNetGame();
        }

        paused = true;
    }

    if (m == EXIT_MODE) {
        Print("  Shutting Down (Returning to Windows)...\n");

        if (game_mode == PLAN_MODE || game_mode == PLAY_MODE) {
            if (soundcard)
            soundcard->StopSoundEffects();

            StopNetGame();
        }

        Print("  Stardate: %.1f\n", StarSystem::GetBaseTime());
        Print("  Bitmap Cache Footprint: %d KB\n", Bitmap::CacheMemoryFootprint() / 1024);

        paused = true;
    }

    FlushKeys();
    game_mode = m;
}

// +--------------------------------------------------------------------+

bool
Starshatter::ChangeVideo()
{
    bool  result = false;

    if (menuscreen) {
        delete menuscreen;
        menuscreen = 0;
    }

    if (loadscreen) {
        delete loadscreen;
        loadscreen = 0;
    }

    if (planscreen) {
        delete planscreen;
        planscreen = 0;
    }

    if (gamescreen) {
        delete gamescreen;
        gamescreen = 0;
    }

    if (cmpnscreen) {
        delete cmpnscreen;
        cmpnscreen = 0;
    }

    loader->SetDataPath(0);

    LoadVideoConfig("video2.cfg");

    result = ResetVideo();

    InitMouse();

    req_change_video = false;
    video_changed    = true;

    return result;
}

bool
Starshatter::ResizeVideo()
{
    if (Game::ResizeVideo()) {
        InitMouse();
        Mouse::Show(true);

        return true;
    }

    return false;
}

// +--------------------------------------------------------------------+

void
Starshatter::CreateWorld()
{
    RadioTraffic::Initialize();
    RadioView::Initialize();
    RadioVox::Initialize();
    QuantumView::Initialize();
    QuitView::Initialize();
    TacticalView::Initialize();

    // create world
    if (!world) {
        Sim* sim = new(__FILE__,__LINE__) Sim(input);
        world = sim;
        Print("  World Created.\n");
    }

    cam_dir = CameraDirector::GetInstance();
}

void
Starshatter::InstantiateMission()
{
    Memory::Check();

    current_mission = 0;

    if (Campaign::GetCampaign()) {
        current_mission = Campaign::GetCampaign()->GetMission();
    }

    Sim* sim = (Sim*) world;

    if (sim) {
        bool        dynamic  = false;
        Campaign*   campaign = Campaign::GetCampaign();

        if (campaign && campaign->IsDynamic())
        dynamic = true;

        sim->UnloadMission();
        sim->LoadMission(current_mission);
        sim->ExecMission();
        sim->SetTestMode(test_mode && !dynamic ? true : false);

        Print("  Mission Instantiated.\n");
    }

    Memory::Check();
}

// +--------------------------------------------------------------------+

int
Starshatter::KeyDown(int action) const
{
    int k = Joystick::KeyDownMap(action) ||
    Keyboard::KeyDownMap(action);

    return k;
}

// +--------------------------------------------------------------------+

bool
Starshatter::GameLoop()
{
    cam_dir = CameraDirector::GetInstance();

    if (active && paused) {
        // Route Events to EventTargets
        EventDispatch* ed = EventDispatch::GetInstance();
        if (ed)
        ed->Dispatch();

        UpdateWorld();
        GameState();
        UpdateScreen();
        CollectStats();

        /***
    static DWORD vmf_time = 0;

    if (real_time() - vmf_time > 5000) {
        vmf_time = real_time();
        DWORD vmf = video->VidMemFree() / (1024 * 1024);
        ::Print("\n###### %02d:%02d - Video Memory Free: %d MB\n\n",
                vmf_time / 60000,
                vmf_time /  1000,
                vmf);
    }
    ***/
    }

    Game::GameLoop();
    return false;  // must return false to keep processing
    // true tells the outer loop to sleep until a
    // windows event is available
}

// +--------------------------------------------------------------------+

void
Starshatter::UpdateWorld()
{
    long   new_time      = real_time;
    double delta         = new_time - frame_time; // in milliseconds
    seconds       = max_frame_length;      // in seconds
    gui_seconds   = delta * 0.001;

    if (frame_time == 0)
    gui_seconds = 0;

    if (delta < time_comp * max_frame_length * 1000) {
        seconds = time_comp * delta * 0.001;
    }
    else {
        seconds = time_comp * max_frame_length;
    }

    frame_time = new_time;

    Galaxy* galaxy = Galaxy::GetInstance();
    if (galaxy) galaxy->ExecFrame();

    // Cutscene missions have a tendency to mess with the stardate
    // to manage time-of-day and camera effects.  It's a bad idea to
    // evaluate campaign actions and events while the cutscene is
    // changing the time base.
    if (!cutscene_mission) {
        Campaign* campaign = Campaign::GetCampaign();
        if (campaign) campaign->ExecFrame();
    }

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

    if (game_mode == PLAY_MODE || InCutscene()) {
        if (cam_dir) {
            if (head_tracker && head_tracker->IsRunning() && !InCutscene()) {
                head_tracker->ExecFrame();
                cam_dir->VirtualHead(head_tracker->GetAzimuth(), head_tracker->GetElevation());
                cam_dir->VirtualHeadOffset(head_tracker->GetX(), head_tracker->GetY(), head_tracker->GetZ());
            }

            cam_dir->ExecFrame(gui_seconds);
        }

        Sim*        sim   = Sim::GetSim();
        SimRegion*  rgn   = sim ? sim->GetActiveRegion() : 0;

        if (rgn) {
            ListIter<Ship> iter = rgn->Ships();
            while (++iter) {
                Ship* s = iter.value();
                s->SelectDetail(seconds);
            }
        }
    }
}

// +--------------------------------------------------------------------+

void
Starshatter::GameState()
{
    if (splash) {
        static bool quick_splash = false;

        if (GetKey() != 0)
        quick_splash = true;

        if (quick_splash) {
            splash->FadeIn(0);
            splash->StopHold();
            splash->FadeOut(0);
        }

        if (splash->Done()) {
            splash = 0; // this will get deleted along with gamewin
            splash_index++;

            if (gamewin) {
                screen->DelWindow(gamewin);
                delete gamewin;
                gamewin = 0;
            }

            if (splash_index < 2) {
                Ship::Initialize();
                Galaxy::Initialize();
                SetupSplash();
            }
            else {
                CombatRoster::Initialize();
                Campaign::Initialize();
                SetupMenuScreen();
            }

            FlushKeys();
        }
    }

    else if (game_mode == MENU_MODE) {
        bool campaign_select = false;

        if (cmpnscreen) {
            campaign_select = cmpnscreen->IsShown();
            cmpnscreen->Hide();
        }

        if (gamescreen)
        gamescreen->Hide();

        if (planscreen)
        planscreen->Hide();

        if (loadscreen)
        loadscreen->Hide();

        if (!menuscreen) {
            SetupMenuScreen();
        }
        else {
            menuscreen->Show();

            if (campaign_select)
            menuscreen->ShowCmpSelectDlg();
        }

        if (MusicDirector::GetInstance() &&
                MusicDirector::GetInstance()->GetMode() != MusicDirector::CREDITS)
        MusicDirector::SetMode(MusicDirector::MENU);

        DoMenuScreenFrame();
    }

    else if (game_mode == CLOD_MODE ||
            game_mode == PREP_MODE ||
            game_mode == LOAD_MODE) {
        if (menuscreen)
        menuscreen->Hide();

        if (planscreen)
        planscreen->Hide();

        if (cmpnscreen)
        cmpnscreen->Hide();

        if (!loadscreen)
        SetupLoadScreen();
        else
        loadscreen->Show();

        if (game_mode == CLOD_MODE)
        MusicDirector::SetMode(MusicDirector::MENU);
        else
        MusicDirector::SetMode(MusicDirector::BRIEFING);

        DoLoadScreenFrame();
    }

    else if (game_mode == PLAN_MODE) {
        if (menuscreen)
        menuscreen->Hide();

        if (cmpnscreen)
        menuscreen->Hide();

        if (loadscreen)
        loadscreen->Hide();

        if (gamescreen)
        gamescreen->Hide();

        if (!planscreen)
        SetupPlanScreen();
        else
        planscreen->Show();

        Player* p = Player::GetCurrentPlayer();
        if (p && p->ShowAward()) {
            if (!planscreen->IsAwardShown())
            planscreen->ShowAwardDlg();
        }

        else if (ShipStats::NumStats()) {
            if (!planscreen->IsDebriefShown()) {
                planscreen->ShowDebriefDlg();
                show_missions = true;
            }
        }

        else {
            if (!planscreen->IsMsnShown() && !planscreen->IsNavShown())
            planscreen->ShowMsnDlg();
        }

        MusicDirector::SetMode(MusicDirector::BRIEFING);

        DoPlanScreenFrame();
    }


    else if (game_mode == CMPN_MODE) {
        if (menuscreen)
        menuscreen->Hide();

        if (planscreen)
        planscreen->Hide();

        if (loadscreen)
        loadscreen->Hide();

        if (gamescreen)
        gamescreen->Hide();

        if (!cmpnscreen)
        SetupCmpnScreen();
        else
        cmpnscreen->Show();

        DoCmpnScreenFrame();
    }

    else if (game_mode == PLAY_MODE) {
        if (menuscreen)
        menuscreen->Hide();

        if (cmpnscreen)
        cmpnscreen->Hide();

        if (planscreen)
        planscreen->Hide();

        if (loadscreen)
        loadscreen->Hide();

        if (!gamescreen)
        SetupGameScreen();
        else
        gamescreen->Show();

        DoGameScreenFrame();
    }

    if (game_mode == EXIT_MODE) {
        exit_time -= Game::GUITime();

        if (exit_time <= 0)
        Game::Exit();
    }

    if (net_lobby)
    net_lobby->ExecFrame();

    if (music_dir)
    music_dir->ExecFrame();

    Memory::Check();
}

// +--------------------------------------------------------------------+

void
Starshatter::DoMenuScreenFrame()
{
    if (!Mouse::RButton()) {
        Mouse::SetCursor(Mouse::ARROW);
        Mouse::Show(true);
    }

    if (time_til_change > 0)
    time_til_change -= Game::GUITime();

    if (!menuscreen)
    return;

    if (KeyDown(KEY_EXIT)) {
        if (time_til_change <= 0) {
            time_til_change = 0.5;

            if (!exit_latch && !menuscreen->CloseTopmost()) {
                menuscreen->ShowExitDlg();
            }
        }

        exit_latch = true;
    }
    else {
        exit_latch = false;
    }

    LoadDlg* load_dlg = menuscreen->GetLoadDlg();

    if (load_dlg && load_dlg->IsShown()) {
        // load all ship designs in the standard catalog
        if (catalog_index < ShipDesign::StandardCatalogSize()) {
            ShipDesign::PreloadCatalog(catalog_index++);
            load_activity = Game::GetText("Starshatter.load.tac-ref");

            if (load_progress < 95)
            load_progress++;
        }

        else {
            menuscreen->ShowTacRefDlg();
        }
    }

    if (show_missions) {
        if (net_lobby)
        menuscreen->ShowNetLobbyDlg();
        else
        menuscreen->ShowMsnSelectDlg();

        show_missions = false;
    }

    menuscreen->ExecFrame();

    if (req_change_video) {
        ChangeVideo();
        SetupMenuScreen();
    }
}

// +--------------------------------------------------------------------+

void
Starshatter::DoPlanScreenFrame()
{
    Mouse::SetCursor(Mouse::ARROW);

    if (time_til_change > 0)
    time_til_change -= Game::GUITime();

    if (KeyDown(KEY_EXIT)) {
        if (time_til_change <= 0) {
            time_til_change = 1;

            if (!exit_latch && !planscreen->CloseTopmost()) {
                Campaign* campaign = Campaign::GetCampaign();
                if (campaign && (campaign->IsDynamic() || campaign->IsTraining()))
                SetGameMode(CMPN_MODE);
                else
                SetGameMode(MENU_MODE);
            }
        }

        exit_latch = true;
    }
    else {
        exit_latch = false;
    }

    planscreen->ExecFrame();
    show_missions = true;
}

// +--------------------------------------------------------------------+

void
Starshatter::DoCmpnScreenFrame()
{
    Mouse::SetCursor(Mouse::ARROW);

    if (time_til_change > 0)
    time_til_change -= Game::GUITime();

    exit_latch = KeyDown(KEY_EXIT) ? true : false;

    if (InCutscene() && player_ship) {
        // warp effect:
        if (player_ship->WarpFactor() > 1) {
            if (player_ship->WarpFactor() > field_of_view)
            cmpnscreen->SetFieldOfView(player_ship->WarpFactor());
            else
            cmpnscreen->SetFieldOfView(field_of_view);
        }

        else {
            if (cmpnscreen->GetFieldOfView() != field_of_view)
            cmpnscreen->SetFieldOfView(field_of_view);
        }
    }

    if (InCutscene() && exit_latch) {
        time_til_change = 1;
        EndCutscene();
        EndMission();
        cmpnscreen->SetFieldOfView(field_of_view);
    }

    else if (time_til_change <= 0 && exit_latch) {
        time_til_change = 1;

        if (!cmpnscreen || !cmpnscreen->CloseTopmost()) {
            SetGameMode(MENU_MODE);
        }
    }

    // time control for campaign mode:
    else if (game_mode == CMPN_MODE) {
        if (time_til_change <= 0) {
            if (KeyDown(KEY_PAUSE)) {
                time_til_change = 1;
                Pause(!paused);
            }

            else if (KeyDown(KEY_TIME_COMPRESS)) {
                time_til_change = 1;

                switch (TimeCompression()) {
                case 1:  SetTimeCompression(2);  break;
                case 2:  SetTimeCompression(4);  break;
                case 4:  SetTimeCompression(8);  break;
                }
            }

            else if (KeyDown(KEY_TIME_EXPAND)) {
                time_til_change = 1;

                switch (TimeCompression()) {
                case  8: SetTimeCompression( 4); break;
                case  4: SetTimeCompression( 2); break;
                default: SetTimeCompression( 1); break;
                }
            }
        }
    }

    if (show_missions && !InCutscene()) {
        cmpnscreen->ShowCmdMissionsDlg();
        show_missions = false;
    }

    cmpnscreen->ExecFrame();
}

// +--------------------------------------------------------------------+

void
Starshatter::DoLoadScreenFrame()
{
    Mouse::Show(false);
    Mouse::SetCursor(Mouse::ARROW);

    if (game_mode == CLOD_MODE) {
        CmpLoadDlg* dlg = loadscreen->GetCmpLoadDlg();

        switch (load_step) {
        case 0:
            load_activity = Game::GetText("Starshatter.load.campaign");
            load_progress = 1;
            catalog_index = 0;
            break;

        case 1:
            // load all ship designs in the standard catalog
            if (catalog_index < ShipDesign::StandardCatalogSize()) {
                ShipDesign::PreloadCatalog(catalog_index++);
                load_activity = Game::GetText("Starshatter.load.campaign");

                if (load_progress < 80)
                load_progress++;

                load_step = 0; // force return to current step on next frame
            }
            else {
                load_activity = Game::GetText("Starshatter.load.start");
                load_progress = 80;
            }
            break;

        case 2:
            if (Campaign::GetCampaign())
            Campaign::GetCampaign()->Start();
            break;

        default:
            if (dlg && load_progress < 100) {
                load_progress++;
            }
            else {
                load_activity = Game::GetText("Starshatter.load.ready");
                load_progress = 100;
                SetGameMode(CMPN_MODE);
            }
            break;
        }
    }
    else {
        switch (load_step) {
        case 0:
            load_activity = Game::GetText("Starshatter.load.drives");
            load_progress = 0;
            break;

        case 1:
            Drive::Initialize();
            LandingGear::Initialize();
            load_activity = Game::GetText("Starshatter.load.explosions");
            load_progress = 5;
            break;

        case 2:
            Explosion::Initialize();
            load_activity = Game::GetText("Starshatter.load.systems");
            load_progress = 10;
            break;

        case 3:
            FlightDeck::Initialize();
            NavLight::Initialize();
            load_activity = Game::GetText("Starshatter.load.ships");
            load_progress = 15;
            break;

        case 4:
            Ship::Initialize();
            Shot::Initialize();
            load_activity = Game::GetText("Starshatter.load.hud");
            load_progress = 20;
            break;

        case 5:
            MFD::Initialize();
            load_activity = Game::GetText("Starshatter.load.menus");
            load_progress = 25;
            break;

        case 6:
            RadioTraffic::Initialize();
            RadioView::Initialize();
            TacticalView::Initialize();
            
            if (!gamescreen && game_mode != PREP_MODE)
            SetupGameScreen();

            load_activity = Game::GetText("Starshatter.load.mission");
            load_progress = 65;
            break;

        case 7:
            if (game_mode == PREP_MODE) {
                if (Campaign::GetCampaign())
                Campaign::GetCampaign()->GetMission();
                SetGameMode(PLAN_MODE);
                load_activity = Game::GetText("Starshatter.load.loaded");
                load_progress = 100;
                Button::PlaySound(4);
            }
            else {
                CreateWorld();
                load_activity = Game::GetText("Starshatter.load.simulation");
                load_progress = 75;
            }
            break;

        case 8:
            InstantiateMission();
            load_activity = Game::GetText("Starshatter.load.viewscreen");
            load_progress = 90;
            break;

        default:
            SetGameMode(PLAY_MODE);
            load_activity = Game::GetText("Starshatter.load.ready");
            load_progress = 100;
            break;
        }
    }

    load_step++;
    loadscreen->ExecFrame();
}

// +--------------------------------------------------------------------+

void
Starshatter::PlayerCam(int mode)
{
    if (player_ship && !player_ship->InTransition() && player_ship->GetFlightPhase() >= Ship::LOCKED) {
        gamescreen->CloseTopmost();
        if (mode == CameraDirector::MODE_DROP) {
            player_ship->DropCam(15, 0);
            cam_dir->SetShip(player_ship);
            cam_dir->SetMode(CameraDirector::MODE_DROP, 0);
        }

        else {
            cam_dir->SetMode(mode);

            if (mode == CameraDirector::MODE_ORBIT) {
                MouseController* mouse_con = MouseController::GetInstance();
                if (mouse_con)
                mouse_con->SetActive(false);
            }
        }
    }
}

void
Starshatter::ViewSelection()
{
    Sim* sim = (Sim*) world;
    List<Ship>& seln = sim->GetSelection().container();

    if (cam_dir) {
        if (seln.isEmpty())
        cam_dir->SetViewObject(player_ship);

        else if (seln.size() == 1)
        cam_dir->SetViewObject(seln[0]);

        else
        cam_dir->SetViewObjectGroup(seln);
    }
}

// +--------------------------------------------------------------------+

void
Starshatter::DoGameScreenFrame()
{
    Sim* sim = (Sim*) world;

    if (!gamescreen || !sim)
    return;

    if (InCutscene()) {
        if (player_ship) {
            // warp effect:
            if (player_ship->WarpFactor() > 1) {
                if (player_ship->WarpFactor() > field_of_view)
                gamescreen->SetFieldOfView(player_ship->WarpFactor());
                else
                gamescreen->SetFieldOfView(field_of_view);
            }

            else {
                if (gamescreen->GetFieldOfView() != field_of_view)
                gamescreen->SetFieldOfView(field_of_view);
            }
        }

        gamescreen->FrameRate(frame_rate);
        gamescreen->ExecFrame();

        if (KeyDown(KEY_EXIT)) {
            gamescreen->SetFieldOfView(field_of_view);
            exit_latch      = true;
            time_til_change = 1;

            sim->SkipCutscene();
        }

        return;
    }

    if (time_til_change > 0)
    time_til_change -= Game::GUITime();

    if (exit_latch && !KeyDown(KEY_EXIT))
    exit_latch = false;

    DoMouseFrame();

    HUDView*       hud_view = HUDView::GetInstance();

    // changing to a new ship?
    if (player_ship != sim->GetPlayerShip()) {
        gamescreen->HideNavDlg();
        gamescreen->HideFltDlg();
        gamescreen->HideEngDlg();

        Ship* new_player = sim->GetPlayerShip();

        if (new_player) {
            if (new_player->IsDropship() && Ship::GetFlightModel() < 2 && Ship::GetControlModel() < 1)
            input->SwapYawRoll(true);
            else
            input->SwapYawRoll(false);

            if (hud_view) {
                hud_view->SetHUDMode(HUDView::HUD_MODE_TAC);
                hud_view->HideHUDWarn();
            }
        }
    }

    player_ship = sim->GetPlayerShip();
    
    if (player_ship) {
        // warp effect:
        if (player_ship->WarpFactor() > 1) {
            if (player_ship->WarpFactor() > field_of_view)
            gamescreen->SetFieldOfView(player_ship->WarpFactor());
            else
            gamescreen->SetFieldOfView(field_of_view);
        }

        else {
            if (gamescreen->GetFieldOfView() != field_of_view)
            gamescreen->SetFieldOfView(field_of_view);
        }

        gamescreen->ShowExternal();   
        
        if (CameraDirector::GetCameraMode() >= CameraDirector::MODE_ORBIT && !player_ship->InTransition())
        tactical = true;
        else
        tactical = false;

        if (player_ship->InTransition())
        gamescreen->CloseTopmost();
    }

    if (chat_mode) {
        DoChatMode();
    }

    else {
        DoGameKeys();
    }

    gamescreen->FrameRate(frame_rate);
    gamescreen->ExecFrame();

    if (Game::GameTime() - time_mark > 60000) {
        time_mark = Game::GameTime();
        minutes++;
        if (minutes > 60)
        Print("  TIME %2d:%02d:00\n", minutes/60, minutes%60);
        else
        Print("  TIME    %2d:00\n", minutes);
    }
}

void
Starshatter::DoGameKeys()
{
    Sim*     sim      = (Sim*) world;
    HUDView* hud_view = HUDView::GetInstance();

    if (time_til_change <= 0) {
        if (KeyDown(KEY_CAM_BRIDGE)) {
            PlayerCam(CameraDirector::MODE_COCKPIT);
            time_til_change = 0.5;
        }

        else if (KeyDown(KEY_CAM_VIRT)) {
            PlayerCam(CameraDirector::MODE_VIRTUAL);
            time_til_change = 0.5;
        }

        else if (KeyDown(KEY_CAM_CHASE)) {
            PlayerCam(CameraDirector::MODE_CHASE);
            time_til_change = 0.5;
        }

        else if (KeyDown(KEY_CAM_DROP)) {
            PlayerCam(CameraDirector::MODE_DROP);
            time_til_change = 0.5;
        }

        else if (KeyDown(KEY_CAM_EXTERN)) {
            PlayerCam(CameraDirector::MODE_ORBIT);
            time_til_change = 0.5;
        }
        
        else if (KeyDown(KEY_TARGET_PADLOCK)) {
            PlayerCam(CameraDirector::MODE_TARGET);
            time_til_change = 0.5;
        }

        else if (KeyDown(KEY_SWAP_ROLL_YAW)) {
            input->SwapYawRoll(!input->GetSwapYawRoll());
            time_til_change = 0.5;
        }

        else if (KeyDown(KEY_ZOOM_WIDE)) {
            time_til_change = 0.5;
            if (gamescreen->GetFieldOfView() <= 2)
            field_of_view = 3; // wide
            else
            field_of_view = 2; // normal

            // don't mess with fov during warp:
            if (player_ship && player_ship->WarpFactor() <= 1)
            gamescreen->SetFieldOfView(field_of_view);
        }
        
        else if (!exit_latch && KeyDown(KEY_EXIT)) {
            exit_latch      = true;
            time_til_change = 0.5;

            if (!gamescreen->CloseTopmost()) {
                QuitView* quit = QuitView::GetInstance();
                if (quit)
                quit->ShowMenu();
                else
                SetGameMode(Starshatter::PLAN_MODE);
            }
        }

        else if (KeyDown(KEY_PAUSE)) {
            Pause(!paused);
            time_til_change = 0.5;
        }


        else if (KeyDown(KEY_TIME_COMPRESS)) {
            time_til_change = 0.5;
            if (NetGame::IsNetGame())
            SetTimeCompression(1);
            else
            switch (TimeCompression()) {
            case 1:  SetTimeCompression(2);  break;
            default: SetTimeCompression(4);  break;
            }
        }

        else if (KeyDown(KEY_TIME_EXPAND)) {
            time_til_change = 0.5;

            if (NetGame::IsNetGame())
            SetTimeCompression(1);
            else
            switch (TimeCompression()) {
            case  4: SetTimeCompression( 2); break;
            default: SetTimeCompression( 1); break;
            }
        }

        else if (KeyDown(KEY_TIME_SKIP)) {
            time_til_change = 0.5;

            if (player_ship && !NetGame::IsNetGame()) {
                player_ship->TimeSkip();
            }
        }

        else if (KeyDown(KEY_COMMAND_MODE)) {
            if (player_ship) {
                time_til_change = 0.5;
                player_ship->CommandMode();
            }
        }

        /*** For Debug Convenience Only: ***/
        else if (KeyDown(KEY_INC_STARDATE)) {
            StarSystem::SetBaseTime(StarSystem::GetBaseTime() + 600, true);
        }

        else if (KeyDown(KEY_DEC_STARDATE)) {
            StarSystem::SetBaseTime(StarSystem::GetBaseTime() - 600, true);
        }
        /***/
    }

    if (gamescreen && time_til_change <= 0) {
        if (KeyDown(KEY_MFD1)) {
            gamescreen->CycleMFDMode(0);
            time_til_change = 0.5;
        }

        else if (KeyDown(KEY_MFD2)) {
            gamescreen->CycleMFDMode(1);
            time_til_change = 0.5;
        }

        else if (KeyDown(KEY_MFD3)) {
            gamescreen->CycleMFDMode(2);
            time_til_change = 0.5;
        }

        else if (KeyDown(KEY_MFD4)) {
            gamescreen->CycleMFDMode(3);
            time_til_change = 0.5;
        }

        else if (KeyDown(KEY_RADIO_MENU)) {
            RadioView* radio_view = RadioView::GetInstance();
            if (radio_view) {
                if (radio_view->IsMenuShown())
                radio_view->CloseMenu();
                else
                radio_view->ShowMenu();
            }
            time_til_change = 0.5;
        }

        else if (KeyDown(KEY_QUANTUM_MENU)) {
            QuantumView* quantum_view = QuantumView::GetInstance();
            if (quantum_view) {
                if (quantum_view->IsMenuShown())
                quantum_view->CloseMenu();
                else
                quantum_view->ShowMenu();
            }
            time_til_change = 0.5;
        }

        else if (KeyDown(KEY_CAM_VIEW_SELECTION)) {
            time_til_change = 0.5;
            ViewSelection();
        }

        else if (KeyDown(KEY_HUD_MODE)) {
            time_til_change = 0.5;
            if (hud_view)
            hud_view->CycleHUDMode();
        }

        else if (KeyDown(KEY_HUD_COLOR)) {
            time_til_change = 0.5;
            if (hud_view)
            hud_view->CycleHUDColor();
        }

        else if (KeyDown(KEY_HUD_WARN)) {
            time_til_change = 0.5;
            if (hud_view)
            hud_view->CycleHUDWarn();
        }

        else if (KeyDown(KEY_HUD_INST)) {
            time_til_change = 0.5;
            if (hud_view)
            hud_view->CycleHUDInst();
        }

        else if (KeyDown(KEY_SELF_DESTRUCT)) {
            time_til_change = 0.5;
            
            if (player_ship && !player_ship->InTransition()) {
                double damage = player_ship->Design()->scuttle;

                if (NetGame::IsNetGameClient()) {
                    NetUtil::SendSelfDestruct(player_ship, damage);
                }
                else {
                    Point scuttle_loc = player_ship->Location() + RandomDirection() * player_ship->Radius();
                    player_ship->InflictDamage(damage, 0, 1, scuttle_loc);
                }

                if (player_ship->Integrity() < 1) {
                    ::Print("  %s 0-0-0-Destruct-0\n\n", player_ship->Name());

                    ShipStats* s = ShipStats::Find(player_ship->Name());
                    if (s)
                    s->AddEvent(SimEvent::DESTROYED, player_ship->Name());

                    player_ship->DeathSpiral();
                }
            }
        }
    }

    if (gamescreen && player_ship && time_til_change <= 0 && !::GetAsyncKeyState(VK_SHIFT) && !::GetAsyncKeyState(VK_MENU)) {
        if (KeyDown(KEY_NAV_DLG)) {
            gamescreen->ShowNavDlg();
            time_til_change = 0.5;
        }

        else if (KeyDown(KEY_WEP_DLG)) {
            if (player_ship && player_ship->Design()->wep_screen)
            gamescreen->ShowWeaponsOverlay();
            time_til_change = 0.5;
        }

        else if (KeyDown(KEY_FLT_DLG)) {
            if (player_ship && player_ship->NumFlightDecks() > 0)
            gamescreen->ShowFltDlg();
            time_til_change = 0.5;
        }

        else if (KeyDown(KEY_ENG_DLG)) {
            if (player_ship && player_ship->Design()->repair_screen)
            gamescreen->ShowEngDlg();
            time_til_change = 0.5;
        }
    }

    if (cam_dir) {
        double spin = (PI/2) * Game::FrameTime(); // Game::GUITime();

        if (avi_file)
        spin /= 6;

        if (KeyDown(KEY_CAM_EXT_PLUS_AZ))
        cam_dir->ExternalAzimuth(spin);

        else if (KeyDown(KEY_CAM_EXT_MINUS_AZ))
        cam_dir->ExternalAzimuth(-spin);

        if (KeyDown(KEY_CAM_EXT_PLUS_EL))
        cam_dir->ExternalElevation(spin);

        else if (KeyDown(KEY_CAM_EXT_MINUS_EL))
        cam_dir->ExternalElevation(-spin);

        if (KeyDown(KEY_CAM_VIRT_PLUS_AZ))
        cam_dir->VirtualAzimuth(-spin);

        else if (KeyDown(KEY_CAM_VIRT_MINUS_AZ))
        cam_dir->VirtualAzimuth(spin);

        if (KeyDown(KEY_CAM_VIRT_PLUS_EL))
        cam_dir->VirtualElevation(spin);

        else if (KeyDown(KEY_CAM_VIRT_MINUS_EL))
        cam_dir->VirtualElevation(-spin);

        if (KeyDown(KEY_CAM_EXT_PLUS_RANGE)){
            if (!gamescreen->IsNavShown()) {
                cam_dir->ExternalRange((float) (1 + 1.5 * Game::FrameTime())); // 1.1f);
            }
        }

        else if (KeyDown(KEY_CAM_EXT_MINUS_RANGE)) {
            if (!gamescreen->IsNavShown()) {
                cam_dir->ExternalRange((float) (1 - 1.5 * Game::FrameTime())); // 0.9f);
            }
        }

        if (tactical && !gamescreen->IsFormShown()) {
            if (Mouse::Wheel()) {
                int w = Mouse::Wheel();

                if (w < 0) {
                    while (w < 0) {
                        cam_dir->ExternalRange(1.25f);
                        w += 120;
                    }
                }
                else {
                    while (w > 0) {
                        cam_dir->ExternalRange(0.75f);
                        w -= 120;
                    }
                }
            }

            else if (Mouse::LButton() && Mouse::RButton()) {
                if (mouse_dy < 0)
                cam_dir->ExternalRange(0.85f);
                else if (mouse_dy > 0)
                cam_dir->ExternalRange(1.15f);
            }

            else if (Mouse::MButton() && time_til_change <= 0) {
                time_til_change = 0.5;
                ViewSelection();
            }

            else {
                if (mouse_dx || mouse_dy) {
                    if (CameraDirector::GetCameraMode() == CameraDirector::MODE_VIRTUAL) {
                        cam_dir->VirtualAzimuth(  mouse_dx * 0.2 * DEGREES);
                        cam_dir->VirtualElevation(mouse_dy * 0.2 * DEGREES);
                    }
                    else {
                        cam_dir->ExternalAzimuth(  mouse_dx * 0.2 * DEGREES);
                        cam_dir->ExternalElevation(mouse_dy * 0.2 * DEGREES);
                    }
                }
            }
        }
    }

    // radio message hot keys:
    static bool comm_key = false;

    if (KeyDown(KEY_COMM_ATTACK_TGT)) {
        if (!comm_key)
        RadioTraffic::SendQuickMessage(player_ship, RadioMessage::ATTACK);
        comm_key = true;
    }

    else if (KeyDown(KEY_COMM_ESCORT_TGT)) {
        if (!comm_key)
        RadioTraffic::SendQuickMessage(player_ship, RadioMessage::ESCORT);
        comm_key = true;
    }

    else if (KeyDown(KEY_COMM_WEP_FREE)) {
        if (!comm_key)
        RadioTraffic::SendQuickMessage(player_ship, RadioMessage::WEP_FREE);
        comm_key = true;
    }

    else if (KeyDown(KEY_COMM_WEP_HOLD)) {
        if (!comm_key)
        RadioTraffic::SendQuickMessage(player_ship, RadioMessage::FORM_UP);
        comm_key = true;
    }

    else if (KeyDown(KEY_COMM_COVER_ME)) {
        if (!comm_key)
        RadioTraffic::SendQuickMessage(player_ship, RadioMessage::COVER_ME);
        comm_key = true;
    }

    else if (KeyDown(KEY_COMM_SKIP_NAV)) {
        if (!comm_key)
        RadioTraffic::SendQuickMessage(player_ship, RadioMessage::SKIP_NAVPOINT);
        comm_key = true;
    }

    else if (KeyDown(KEY_COMM_RETURN_TO_BASE)) {
        if (!comm_key)
        RadioTraffic::SendQuickMessage(player_ship, RadioMessage::RTB);
        comm_key = true;
    }

    else if (KeyDown(KEY_COMM_CALL_INBOUND)) {
        if (!comm_key)
        RadioTraffic::SendQuickMessage(player_ship, RadioMessage::CALL_INBOUND);
        comm_key = true;
    }

    else if (KeyDown(KEY_COMM_REQUEST_PICTURE)) {
        if (!comm_key)
        RadioTraffic::SendQuickMessage(player_ship, RadioMessage::REQUEST_PICTURE);
        comm_key = true;
    }

    else if (KeyDown(KEY_COMM_REQUEST_SUPPORT)) {
        if (!comm_key)
        RadioTraffic::SendQuickMessage(player_ship, RadioMessage::REQUEST_SUPPORT);
        comm_key = true;
    }

    else if (KeyDown(KEY_CHAT_BROADCAST)) {
        if (!comm_key)
        SetChatMode(CHAT_BROADCAST);
        comm_key = true;
    }

    else if (KeyDown(KEY_CHAT_TEAM)) {
        if (!comm_key)
        SetChatMode(CHAT_TEAM);
        comm_key = true;
    }

    else if (KeyDown(KEY_CHAT_WING)) {
        if (!comm_key)
        SetChatMode(CHAT_WING);
        comm_key = true;
    }

    else if (KeyDown(KEY_CHAT_UNIT) && sim->GetSelection().container().size()) {
        if (!comm_key)
        SetChatMode(CHAT_UNIT);
        comm_key = true;
    }

    else {
        comm_key = false;
    }
}

void
Starshatter::DoChatMode()
{
    Player*  p         = Player::GetCurrentPlayer();
    bool     send_chat = false;
    Text     name      = "Player";

    if (player_ship)
    name = player_ship->Name();

    if (p)
    name = p->Name();

    if (chat_text.length()) {
        if (chat_text[0] >= '0' && chat_text[0] <= '9') {
            if (p) {
                chat_text = p->ChatMacro(chat_text[0] - '0');
                send_chat = true;
            }
        }
    }

    if (KeyDown(KEY_EXIT)) {
        SetChatMode(0);
        time_til_change = 0.5;
    }

    else if (send_chat || Keyboard::KeyDown(VK_RETURN)) {
        switch (chat_mode) {
        default:
        case CHAT_BROADCAST:
            {
                NetUtil::SendChat(0, name, chat_text);
            }
            break;

        case CHAT_TEAM:
            if (player_ship) {
                NetUtil::SendChat(player_ship->GetIFF()+1, name, chat_text);
            }
            break;
            
        case CHAT_WING:
            if (player_ship) {
                Element* elem = player_ship->GetElement();
                if (elem) {
                    for (int i = 1; i <= elem->NumShips(); i++) {
                        Ship* s = elem->GetShip(i);
                        if (s && s != player_ship)
                        NetUtil::SendChat(s->GetObjID(), name, chat_text);
                    }
                }
            }
            break;

        case CHAT_UNIT:
            {
                Sim*           sim  = Sim::GetSim();
                ListIter<Ship> seln = sim->GetSelection();

                while (++seln) {
                    Ship* s = seln.value();
                    if (s != player_ship)
                    NetUtil::SendChat(s->GetObjID(), name, chat_text);
                }
            }
            break;
        }

        HUDView::Message("%s> %s", name.data(), chat_text.data());

        SetChatMode(0);
        time_til_change = 0.5;
    }

    else {
        int key = 0;
        int shift = 0;

        while (GetKeyPlus(key, shift)) {
            if (key >= 'A' && key <= 'Z') {
                if (shift & 1)
                chat_text += (char) key;
                else
                chat_text += (char) tolower(key);
            }
            else {
                switch (key) {
                case VK_BACK:
                    chat_text = chat_text.substring(0, chat_text.length()-1);
                    break;

                case VK_SPACE: chat_text += ' '; break;
                case '0': if (shift & 1) chat_text += ')'; else chat_text += '0'; break;
                case '1': if (shift & 1) chat_text += '!'; else chat_text += '1'; break;
                case '2': if (shift & 1) chat_text += '@'; else chat_text += '2'; break;
                case '3': if (shift & 1) chat_text += '#'; else chat_text += '3'; break;
                case '4': if (shift & 1) chat_text += '$'; else chat_text += '4'; break;
                case '5': if (shift & 1) chat_text += '%'; else chat_text += '5'; break;
                case '6': if (shift & 1) chat_text += '^'; else chat_text += '6'; break;
                case '7': if (shift & 1) chat_text += '&'; else chat_text += '7'; break;
                case '8': if (shift & 1) chat_text += '*'; else chat_text += '8'; break;
                case '9': if (shift & 1) chat_text += '('; else chat_text += '9'; break;
                case 186: if (shift & 1) chat_text += ':'; else chat_text += ';'; break;
                case 187: if (shift & 1) chat_text += '+'; else chat_text += '='; break;
                case 188: if (shift & 1) chat_text += '<'; else chat_text += ','; break;
                case 189: if (shift & 1) chat_text += '_'; else chat_text += '-'; break;
                case 190: if (shift & 1) chat_text += '>'; else chat_text += '.'; break;
                case 191: if (shift & 1) chat_text += '?'; else chat_text += '/'; break;
                case 192: if (shift & 1) chat_text += '~'; else chat_text += '`'; break;
                case 219: if (shift & 1) chat_text += '{'; else chat_text += '['; break;
                case 221: if (shift & 1) chat_text += '}'; else chat_text += ']'; break;
                case 220: if (shift & 1) chat_text += '|'; else chat_text += '\\'; break;
                case 222: if (shift & 1) chat_text += '"'; else chat_text += '\''; break;
                }
            }
        }
    }
}

void
Starshatter::SetChatMode(int mode)
{
    if (mode >= 0 && mode <= 4 && mode != chat_mode) {
        chat_text = "";

        if (!NetGame::IsNetGame()) {
            chat_mode = 0;
        }
        else {
            chat_mode = mode;

            // flush input before reading chat message:
            if (chat_mode) {
                FlushKeys();
            }

            // flush input before sampling flight controls:
            else {
                for (int i = 1; i < 255; i++) {
                    Keyboard::KeyDown(i);
                }
            }
        }
    }
}

// +--------------------------------------------------------------------+

void
Starshatter::DoMouseFrame()
{
    EventDispatch* event_dispatch = EventDispatch::GetInstance();
    if (event_dispatch && event_dispatch->GetCapture())
    return;

    mouse_dx = 0;
    mouse_dy = 0;

    static int old_mouse_x = 0;
    static int old_mouse_y = 0;

    if (!spinning && Mouse::RButton()) {
        spinning = true;
        old_mouse_x = Mouse::X();
        old_mouse_y = Mouse::Y();
        mouse_x     = Mouse::X();
        mouse_y     = Mouse::Y();
        Mouse::Show(false);
    }

    else if (spinning) {
        if (!Mouse::RButton()) {
            spinning = false;

            if (tactical) {
                Mouse::Show(true);
                Mouse::SetCursor(Mouse::ARROW);
            }

            Mouse::SetCursorPos(old_mouse_x, old_mouse_y);
        }

        else {
            mouse_dx = Mouse::X() - mouse_x;
            mouse_dy = Mouse::Y() - mouse_y;

            Mouse::SetCursorPos(mouse_x, mouse_y);
        }
    }

    else if (cutscene || !player_ship) {
        Mouse::Show(false);
        return;
    }

    // hide mouse cursor when mouse controller is actively steering:
    else if (mouse_input && mouse_input->Active()) {
        if (mouse_input->Selector() == 1) {
            Mouse::Show(false);
        }

        else if (mouse_input->Selector() == 2) {
            Mouse::Show(true);
            Mouse::SetCursor(Mouse::CROSS);
        }
    }

    else {
        HUDView* hud_view = HUDView::GetInstance();

        if (hud_view && hud_view->GetHUDMode() != HUDView::HUD_MODE_OFF) {
            Mouse::Show(true);
            Mouse::SetCursor(Mouse::ARROW);
        }
    }

    if (gamescreen && gamescreen->IsFormShown())
    return;

    TacticalView*  tac_view = TacticalView::GetInstance();

    if (tac_view)
    tac_view->DoMouseFrame();
}

// +--------------------------------------------------------------------+

void
Starshatter::SetupSplash()
{
    Color::SetFade(0);

    if (gamewin) {
        screen->DelWindow(gamewin);
        delete gamewin;
        gamewin = 0;
    }

    switch (splash_index) {
    case 0:
        splash_image.ClearImage();
        loader->SetDataPath(0);
        loader->LoadBitmap("matrix.pcx",   splash_image);
        break;

    case 1:
    default:
        splash_image.ClearImage();
        loader->SetDataPath(0);
        loader->LoadBitmap("studio.pcx",   splash_image);
        break;
    }

    int screen_width  = GetScreenWidth();
    int screen_height = GetScreenHeight();

    gamewin = new(__FILE__,__LINE__) Window(screen, 0, 0, screen_width, screen_height);
    splash  = new(__FILE__,__LINE__) FadeView(gamewin, 2, 2, 2);

    gamewin->AddView(splash);
    gamewin->AddView(new(__FILE__,__LINE__) ImgView(gamewin, &splash_image));
    screen->AddWindow(gamewin);
}

// +--------------------------------------------------------------------+

void
Starshatter::SetupMenuScreen()
{
    if (menuscreen) {
        delete menuscreen;
        menuscreen = 0;
    }

    menuscreen = new(__FILE__,__LINE__) MenuScreen();
    menuscreen->Setup(screen);
}

// +--------------------------------------------------------------------+

void
Starshatter::SetupCmpnScreen()
{
    if (cmpnscreen) {
        delete cmpnscreen;
        cmpnscreen = 0;
    }

    cmpnscreen = new(__FILE__,__LINE__) CmpnScreen();
    cmpnscreen->Setup(screen);
}

// +--------------------------------------------------------------------+

void
Starshatter::SetupPlanScreen()
{
    if (planscreen) {
        delete planscreen;
        planscreen = 0;
    }

    planscreen = new(__FILE__,__LINE__) PlanScreen();
    planscreen->Setup(screen);
}

// +--------------------------------------------------------------------+

void
Starshatter::SetupLoadScreen()
{
    if (loadscreen) {
        delete loadscreen;
        loadscreen = 0;
    }

    loadscreen = new(__FILE__,__LINE__) LoadScreen();
    loadscreen->Setup(screen);
}

// +--------------------------------------------------------------------+

void
Starshatter::SetupGameScreen()
{
    if (gamescreen) {
        delete gamescreen;
        gamescreen = 0;
    }

    gamescreen = new(__FILE__,__LINE__) GameScreen();
    gamescreen->Setup(screen);
    gamescreen->SetFieldOfView(field_of_view);

    // initialize player_ship's MFD choices:
    Player::SelectPlayer(Player::GetCurrentPlayer());
}

// +--------------------------------------------------------------------+

void
Starshatter::LoadVideoConfig(const char* filename)
{
    // set up defaults:
    int   screen_width         = 1024;
    int   screen_height        =  768;
    int   screen_depth         =   32;
    int   terrain_detail_level =    3;
    bool  shadows_enabled      = true;
    bool  spec_maps_enabled    = true;
    bool  bump_maps_enabled    = true;
    bool  vertex_shader        = true;
    bool  pixel_shader         = true;
    float depth_bias           = video_settings->depth_bias;

    max_tex_size   = 2048;

    if (MachineInfo::GetCpuSpeed() >= 1000 && MachineInfo::GetTotalRam() > 128)
    terrain_detail_level = 4;

    Terrain::SetDetailLevel(terrain_detail_level);

    // read the config file:
    BYTE*       block    = 0;
    int         blocklen = 0;

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

    if (blocklen == 0)
    return;

    Parser parser(new(__FILE__,__LINE__) BlockReader((const char*) block, blocklen));
    Term*  term = parser.ParseTerm();

    if (!term) {
        Print("ERROR: could not parse '%s'.\n", filename);
        exit(-3);
    }
    else {
        TermText* file_type = term->isText();
        if (!file_type || file_type->value() != "VIDEO") {
            Print("WARNING: invalid %s file.  Using defaults\n", filename);
            return;
        }
    }

    do {
        delete term;

        term = parser.ParseTerm();
        
        if (term) {
            TermDef* def = term->isDef();
            if (def) {
                if (def->name()->value() == "width") {
                    int w;
                    GetDefNumber(w, def, filename);

                    switch (w) {
                    case 800:
                        screen_width  =  800;
                        screen_height =  600;
                        break;

                    case 1024:
                        screen_width  = 1024;
                        screen_height =  768;
                        break;

                    case 1152:
                        screen_width  = 1152;
                        screen_height =  864;
                        break;

                    case 1280:
                        screen_width  = 1280;
                        screen_height =  960;
                        break;

                    case 1440:
                        screen_width  = 1440;
                        screen_height =  900;
                        break;

                    default:
                        screen_width  = w;
                        screen_height = (w*3)/4;
                        break;
                    }
                }

                else if (def->name()->value() == "height") {
                    int h;
                    GetDefNumber(h, def, filename);

                    if (screen_width == 1280 && (h == 800 || h == 1024))
                    screen_height = h;
                    else if (screen_width == 1600 && (h == 900 || h == 1200))
                    screen_height = h;
                }

                else if (def->name()->value() == "depth" ||
                        def->name()->value() == "bpp") {

                    int bpp;
                    GetDefNumber(bpp, def, filename);

                    switch (bpp) {
                    case  8:
                    case 16:
                    case 24:
                    case 32:
                        screen_depth = bpp;
                        break;

                    default:
                        Print("WARNING: Invalid screen bpp (%d) in '%s'\n", bpp, filename);
                        screen_depth = 8;
                        break;
                    }
                }

                else if (def->name()->value() == "max_tex") {
                    int n;
                    GetDefNumber(n, def, filename);
                    if (n >= 64 && n <= 4096)
                    max_tex_size = n;
                }

                else if (def->name()->value() == "gamma") {
                    int level;
                    GetDefNumber(level, def, filename);
                    if (level >= 0 && level <= 255)
                    gamma = level;
                }

                else if (def->name()->value() == "terrain_detail_level") {
                    GetDefNumber(terrain_detail_level, def, filename);
                    Terrain::SetDetailLevel(terrain_detail_level);
                }

                else if (def->name()->value() == "terrain_texture_enable") {
                    bool enable;
                    GetDefBool(enable, def, filename);

                    // no longer supported!
                }

                else if (def->name()->value() == "shadows") {
                    GetDefBool(shadows_enabled, def, filename);
                }

                else if (def->name()->value() == "spec_maps") {
                    GetDefBool(spec_maps_enabled, def, filename);
                }

                else if (def->name()->value() == "bump_maps") {
                    GetDefBool(bump_maps_enabled, def, filename);
                }

                else if (def->name()->value() == "vertex_shader") {
                    GetDefBool(vertex_shader, def, filename);
                }

                else if (def->name()->value() == "pixel_shader") {
                    GetDefBool(pixel_shader, def, filename);
                }

                else if (def->name()->value().contains("bias")) {
                    GetDefNumber(depth_bias, def, filename);
                }

                else if (def->name()->value() == "flare") {
                    bool b;
                    GetDefBool(b, def, filename);
                    lens_flare = b;
                }

                else if (def->name()->value() == "corona") {
                    bool b;
                    GetDefBool(b, def, filename);
                    corona = b;
                }

                else if (def->name()->value() == "nebula") {
                    bool b;
                    GetDefBool(b, def, filename);
                    nebula = b;
                }

                else if (def->name()->value() == "dust") {
                    GetDefNumber(dust, def, filename);
                }

                else if (def->name()->value().indexOf("cam_range") == 0) {
                    double range_max = 0;
                    GetDefNumber(range_max, def, filename);
                    CameraDirector::SetRangeLimit(range_max);
                }
            }
            else {
                Print("WARNING: term ignored in '%s'\n", filename);
                term->print();
            }
        }
    }
    while (term);

    loader->ReleaseBuffer(block);

    if (video_settings) {
        video_settings->fullscreen_mode.width  = screen_width;
        video_settings->fullscreen_mode.height = screen_height;

        if (screen_depth == 16)
        video_settings->fullscreen_mode.format = VideoMode::FMT_R5G6B5;
        else
        video_settings->fullscreen_mode.format = VideoMode::FMT_X8R8G8B8;

        video_settings->shadows    = shadows_enabled;
        video_settings->specmaps   = spec_maps_enabled;
        video_settings->bumpmaps   = bump_maps_enabled;
        video_settings->enable_vs  = vertex_shader;
        video_settings->enable_ps  = pixel_shader;
        video_settings->depth_bias = depth_bias;
    }

    if (video) {
        video->SetShadowEnabled(shadows_enabled);
        video->SetSpecMapEnabled(spec_maps_enabled);
        video->SetBumpMapEnabled(bump_maps_enabled);
    }
}

// +--------------------------------------------------------------------+

void
Starshatter::SaveVideoConfig(const char* filename)
{
    if (!video_settings)
    return;

    FILE* f = fopen(filename, "w");
    if (f) {
        fprintf(f, "VIDEO\n\n");
        fprintf(f, "width:     %4d\n", video_settings->fullscreen_mode.width);
        fprintf(f, "height:    %4d\n", video_settings->fullscreen_mode.height);
        fprintf(f, "depth:     %4d\n", video_settings->fullscreen_mode.format == VideoMode::FMT_R5G6B5 ? 16 : 32);
        fprintf(f, "\n");
        fprintf(f, "max_tex:   %4d\n", max_tex_size);
        fprintf(f, "primary3D: %s\n",  "true");
        fprintf(f, "gamma:     %4d\n", gamma);
        fprintf(f, "\n");
        fprintf(f, "terrain_detail_level:   %d\n", Terrain::DetailLevel());
        fprintf(f, "terrain_texture_enable: %true\n");
        fprintf(f, "\n");

        fprintf(f, "shadows:   %s\n",  video_settings->shadows  ? "true" : "false");
        fprintf(f, "spec_maps: %s\n",  video_settings->specmaps ? "true" : "false");
        fprintf(f, "bump_maps: %s\n",  video_settings->bumpmaps ? "true" : "false");
        fprintf(f, "bias:      %f\n",  video_settings->depth_bias);
        fprintf(f, "\n");

        fprintf(f, "flare:     %s\n",  lens_flare ? "true" : "false");
        fprintf(f, "corona:    %s\n",  corona     ? "true" : "false");
        fprintf(f, "nebula:    %s\n",  nebula     ? "true" : "false");
        fprintf(f, "dust:      %d\n",  dust);

        if (CameraDirector::GetRangeLimit() != 300e3)
        fprintf(f, "   cam_range_max: %f,\n", CameraDirector::GetRangeLimit());

        fclose(f);
    }

    video_changed = false;
}

// +--------------------------------------------------------------------+

void
Starshatter::InvalidateTextureCache()
{
    if (video)
    video->InvalidateCache();
}

// +--------------------------------------------------------------------+

void
Starshatter::ExecCutscene(const char* msn_file, const char* path)
{
    if (InCutscene() || !msn_file || !*msn_file) return;

    if (!world)
    CreateWorld();

    cutscene_mission  = new(__FILE__,__LINE__) Mission(0);
    cutscene_basetime = StarSystem::GetBaseTime();

    if (cutscene_mission->Load(msn_file, path)) {
        Sim* sim = (Sim*) world;

        if (sim) {
            bool        dynamic  = false;
            Campaign*   campaign = Campaign::GetCampaign();

            if (campaign && campaign->IsDynamic())
            dynamic = true;

            sim->UnloadMission();
            sim->LoadMission(cutscene_mission, true); // attempt to preload the tex cache
            sim->ExecMission();
            sim->ShowGrid(false);
            player_ship = sim->GetPlayerShip();

            Print("  Cutscene Instantiated.\n");

            UpdateWorld();
        }
    }
    else {
        delete cutscene_mission;
        cutscene_mission  = 0;
        cutscene_basetime = 0;
    }
}

void
Starshatter::BeginCutscene()
{
    Sim* sim = Sim::GetSim();

    if (cutscene == 0 && !sim->IsNetGame()) {
        HUDView* hud_view = HUDView::GetInstance();
        if (hud_view)
        hud_view->SetHUDMode(HUDView::HUD_MODE_OFF);

        if (sim->GetPlayerShip())
        sim->GetPlayerShip()->SetControls(0);

        AudioConfig* audio_cfg = AudioConfig::GetInstance();

        if (audio_cfg) {
            cut_efx_volume = audio_cfg->GetEfxVolume();
            cut_wrn_volume = audio_cfg->GetWrnVolume();
        }

        Ship::SetFlightModel(Ship::FM_ARCADE);
    }

    cutscene++;
}

void
Starshatter::EndCutscene()
{
    cutscene--;

    if (cutscene == 0) {
        DisplayView* disp_view = DisplayView::GetInstance();
        if (disp_view)
        disp_view->ClearDisplay();

        HUDView* hud_view = HUDView::GetInstance();
        if (hud_view)
        hud_view->SetHUDMode(HUDView::HUD_MODE_TAC);

        Sim* sim = Sim::GetSim();
        if (sim->GetPlayerShip())
        sim->GetPlayerShip()->SetControls(sim->GetControls());

        if (cam_dir) {
            cam_dir->SetViewOrbital(0);
            CameraDirector::SetRangeLimits(10, CameraDirector::GetRangeLimit());
            cam_dir->SetOrbitPoint(PI/4,PI/4,1);
            cam_dir->SetOrbitRates(0,0,0);
        }

        AudioConfig* audio_cfg = AudioConfig::GetInstance();

        if (audio_cfg) {
            audio_cfg->SetEfxVolume(cut_efx_volume);
            audio_cfg->SetWrnVolume(cut_wrn_volume);
        }

        Player* p = Player::GetCurrentPlayer();
        if (p)
        Ship::SetFlightModel(p->FlightModel());
    }
}

void
Starshatter::EndMission()
{
    if (cutscene_mission) {
        Sim* sim = Sim::GetSim();

        if (sim && sim->GetMission() == cutscene_mission) {
            ShipStats::Initialize();
            sim->UnloadMission();

            // restore world clock (true => absolute time reference)
            if (cutscene_basetime != 0)
            StarSystem::SetBaseTime(cutscene_basetime, true);

            delete cutscene_mission;
            cutscene_mission  = 0;
            cutscene_basetime = 0;

            return;
        }
    }

    SetGameMode(Starshatter::PLAN_MODE);
}

Mission*
Starshatter::GetCutsceneMission() const
{
    return cutscene_mission;
}

const char*
Starshatter::GetSubtitles() const
{
    if (cutscene_mission)
    return cutscene_mission->Subtitles();

    return "";
}

// +--------------------------------------------------------------------+

int
Starshatter::GetLobbyMode()
{
    return lobby_mode;
}

void
Starshatter::SetLobbyMode(int mode)
{
    lobby_mode = mode;
}

void
Starshatter::StartLobby()
{
    if (!net_lobby) {
        if (lobby_mode == NET_LOBBY_SERVER) {
            NetServerConfig::Initialize();
            NetServerConfig* server_config = NetServerConfig::GetInstance();

            if (server_config)
            net_lobby = new(__FILE__,__LINE__) NetLobbyServer;
        }

        else {
            NetClientConfig* client_config = NetClientConfig::GetInstance();
            if (client_config)
            client_config->Login();

            net_lobby = NetLobby::GetInstance();
        }
    }

    lobby_mode = NET_LOBBY_CLIENT;
}

void
Starshatter::StopLobby()
{
    if (net_lobby) {
        if (net_lobby->IsServer()) {
            delete net_lobby;
            NetServerConfig::Close();
        }

        else {
            NetClientConfig* client_config = NetClientConfig::GetInstance();
            if (client_config)
            client_config->Logout();
        }

        net_lobby = 0;
    }

    lobby_mode = NET_LOBBY_CLIENT;
}

void
Starshatter::StopNetGame()
{
    // local server:
    NetLobby* lobby = NetLobby::GetInstance();

    if (lobby && lobby->IsServer()) {
        lobby->GameStop();
    }

    // client connected to remote server:
    else {
        NetClientConfig* config = NetClientConfig::GetInstance();
        if (config && config->GetHostRequest()) {
            config->Login();

            NetLobbyClient* conn = config->GetConnection();

            if (conn) {
                conn->GameStop();
                conn->SelectMission(0);
            }
        }
    }
}
