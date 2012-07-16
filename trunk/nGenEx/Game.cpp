/*  Project nGenEx
	Destroyer Studios LLC
	Copyright (C) 1997-2004. All Rights Reserved.

	SUBSYSTEM:    nGenEx.lib
	FILE:         Game.cpp
	AUTHOR:       John DiCamillo

*/

#include "MemDebug.h"
#include "Game.h"
#include "Mouse.h"
#include "Universe.h"
#include "Screen.h"
#include "Window.h"
#include "EventDispatch.h"
#include "Color.h"
#include "DataLoader.h"
#include "Keyboard.h"
#include "Pcx.h"
#include "Resource.h"
#include "Bitmap.h"
#include "MachineInfo.h"
#include "Video.h"
#include "VideoFactory.h"
#include "VideoSettings.h"
#include "AviFile.h"
#include "ContentBundle.h"

// +--------------------------------------------------------------------+

FILE*    ErrLog         = 0;
int      ErrLine        = 0;
char     ErrBuf[1024];

Game*    game           = 0;

bool     Game::active         = false;
bool     Game::paused         = false;
bool     Game::server         = false;
bool     Game::show_mouse     = false;
DWORD    Game::base_game_time = 0;
DWORD    Game::real_time      = 0;
DWORD    Game::game_time      = 0;
DWORD    Game::time_comp      = 1;
DWORD    Game::frame_number   = 0;

const int    VIDEO_FPS              = 30;
const double MAX_FRAME_TIME_VIDEO   = 1.0 / (double) VIDEO_FPS;
const double MAX_FRAME_TIME_NORMAL  = 1.0 / 5.0;
const double MIN_FRAME_TIME_NORMAL  = 1.0 / 60.0;

double   Game::max_frame_length = MAX_FRAME_TIME_NORMAL;
double   Game::min_frame_length = MIN_FRAME_TIME_NORMAL;

char     Game::panicbuf[256];

static LARGE_INTEGER  perf_freq;
static LARGE_INTEGER  perf_cnt1;
static LARGE_INTEGER  perf_cnt2;

// +--------------------------------------------------------------------+

Game::Game()
: world(0), video_factory(0), video(0), video_settings(0), soundcard(0),
gamma(128), max_tex_size(2048), screen(0), totaltime(0), 
hInst(0), hwnd(0), frame_rate(0), frame_count(0), frame_count0(0),
frame_time(0), frame_time0(0), gui_seconds(0), content(0),
status(Game::OK), exit_code(0), window_style(0), avi_file(0)
{
	if (!game) {
		panicbuf[0] = 0;
		game = this;
		ZeroMemory(ErrBuf, 1024);

		video_settings = new(__FILE__,__LINE__) VideoSettings;

		is_windowed             = false;
		is_active               = false;
		is_device_lost          = false;
		is_minimized            = false;
		is_maximized            = false;
		ignore_size_change      = false;
		is_device_initialized   = false;
		is_device_restored      = false;
	}
	else
	status = TOO_MANY;
}

Game::~Game()
{
	if (game == this)
	game = 0;

	delete content;
	delete world;
	delete screen;
	delete video_factory;
	delete video;
	delete soundcard;
	delete video_settings;
	delete avi_file;

	if (status == EXIT)
	ShowStats();
}

// +--------------------------------------------------------------------+

HINSTANCE Game::GetHINST()
{
	if (game)
	return game->hInst;

	return 0;
}

HWND Game::GetHWND()
{
	if (game)
	return game->hwnd;

	return 0;
}

bool Game::IsWindowed()
{
	if (game)
	return game->is_windowed;

	return false;
}

// +--------------------------------------------------------------------+

Text
Game::GetText(const char* key)
{
	if (game && game->content && game->content->IsLoaded())
	return game->content->GetText(key);

	return key;
}

// +--------------------------------------------------------------------+

int
Game::GammaLevel()
{
	if (game)
	return game->gamma;

	return 0;
}

void
Game::SetGammaLevel(int g)
{
	if (game) {
		game->gamma = g;

		if (game->video)
		game->video->SetGammaLevel(g);
	}
}

int
Game::MaxTexSize()
{
	if (game && game->video) {
		int max_vid_size = game->video->MaxTexSize();
		return max_vid_size < game->max_tex_size ? 
		max_vid_size : game->max_tex_size;
	}
	else if (Video::GetInstance()) {
		return Video::GetInstance()->MaxTexSize();
	}

	return 256;
}

int
Game::MaxTexAspect()
{
	if (game && game->video) {
		return game->video->MaxTexAspect();
	}
	else if (Video::GetInstance()) {
		return Video::GetInstance()->MaxTexAspect();
	}

	return 1;
}

void
Game::SetMaxTexSize(int n)
{
	if (game && n >= 64 && n <= 4096)
	game->max_tex_size = n;
}

bool
Game::DisplayModeSupported(int w, int h, int bpp)
{
	return game && game->video && game->video->IsModeSupported(w,h,bpp);
}

double
Game::FrameRate()
{
	if (game)
	return game->frame_rate;

	return 0;
}

double
Game::FrameTime()
{
	if (game)
	return game->seconds;

	return 0;
}

double
Game::GUITime()
{
	if (game)
	return game->gui_seconds;

	return 0;
}

// +--------------------------------------------------------------------+

bool
Game::Init(HINSTANCE hi, HINSTANCE hpi, LPSTR cmdline, int nCmdShow)
{
	status = OK;
	hInst  = hi;

	Print("  Initializing Game\n");

	stats.Clear();

	if (!InitApplication(hInst)) {  // Initialize shared things
		Panic("Could not initialize the application.");
		status = INIT_FAILED;
	}

	if (status == OK && !video_settings) {
		Panic("No video settings specified");
		status = INIT_FAILED;
	}

	if (status == OK) {
		static int os_version = MachineInfo::GetPlatform();

		if (os_version == MachineInfo::OS_WIN95 || os_version == MachineInfo::OS_WIN98) {
			Panic("  Windows 95 and 98 are no longer supported. Please update to Windows XP or higher.");
			status = INIT_FAILED;
		} else if (os_version == MachineInfo::OS_WINNT) {
			Panic("  D3D not available under WinNT 4");
			status = INIT_FAILED;
		} else if (MachineInfo::GetDirectXVersion() < MachineInfo::DX_9) {
			Panic("  Insufficient DirectX detected (Dx9 IS REQUIRED)");
			status = INIT_FAILED;
		}

		Print("  Gamma Level = %d\n", gamma);
	}

	if (status == OK) {
		Print("\n  Initializing instance...\n");
		// Perform initializations that apply to a specific instance
		if (!InitInstance(hInst, nCmdShow)) {
			Panic("Could not initialize the instance.");
			status = INIT_FAILED;
		}
	}

	if (status == OK) {
		Print("  Initializing content...\n");
		InitContent();

		Print("  Initializing game...\n");
		if (!InitGame()) {
			if (!panicbuf[0])
			Panic("Could not initialize the game.");
			status = INIT_FAILED;
		}
	}

	return status == OK;
}

// +--------------------------------------------------------------------+

bool
Game::InitApplication(HINSTANCE hInstance)
{
	WNDCLASS  wc;
	LOGBRUSH  brush = { BS_SOLID, RGB(0,0,0), 0 };

	if (server)
	brush.lbColor = RGB(255,255,255);

	// Fill in window class structure with parameters that
	// describe the main window.
	wc.style         = CS_HREDRAW | CS_VREDRAW;
	wc.lpfnWndProc   = (WNDPROC) WndProc;
	wc.cbClsExtra    = 0;
	wc.cbWndExtra    = 0;
	wc.hInstance     = hInstance;
	wc.hIcon         = LoadIcon(hInstance, MAKEINTRESOURCE(100));
	wc.hCursor       = LoadCursor(NULL, IDC_ARROW);

	wc.hbrBackground = CreateBrushIndirect(&brush);
	wc.lpszMenuName  = app_name;
	wc.lpszClassName = app_name;

	// Register the window class and return success/failure code.
	if (RegisterClass(&wc) == 0) {
		DWORD err = GetLastError();

		if (err == 1410) // class already exists, this is OK
		return true;

		else
		Print("WARNING: Register Window Class: %08x\n", err);
	}

	return true;
}

// +--------------------------------------------------------------------+

bool
Game::InitInstance(HINSTANCE hInstance, int nCmdShow)
{
	hInst = hInstance;

	// initialize the game timer:
	base_game_time = 0;
	QueryPerformanceFrequency(&perf_freq);
	QueryPerformanceCounter(&perf_cnt1);

	// center window on display:
	int   screenx  = GetSystemMetrics(SM_CXSCREEN);
	int   screeny  = GetSystemMetrics(SM_CYSCREEN);
	int   x_offset = 0;
	int   y_offset = 0;
	int   s_width  = 800;
	int   s_height = 600;

	if (server) {
		s_width  = 320;
		s_height = 200;
	}

	else if (video_settings) {
		s_width  = video_settings->window_width;
		s_height = video_settings->window_height;
	}

	if (s_width < screenx)
	x_offset = (screenx - s_width) / 2;

	if (s_height < screeny)
	y_offset = (screeny - s_height) / 2;

	// Create a main window for this application instance
	RECT  rctmp;
	rctmp.left   = x_offset;
	rctmp.top    = y_offset;
	rctmp.right  = x_offset + s_width;
	rctmp.bottom = y_offset + s_height;

	window_style = WS_OVERLAPPED  | WS_CAPTION     | WS_SYSMENU | WS_THICKFRAME | 
	WS_MINIMIZEBOX | WS_MAXIMIZEBOX | WS_VISIBLE;

	AdjustWindowRect(&rctmp, window_style, 1);

	hwnd = CreateWindow(
	app_name,                  // Class name
	app_name,                  // Caption

	window_style,

	x_offset,                  // Position
	y_offset,

	rctmp.right - rctmp.left,  // Size
	rctmp.bottom - rctmp.top,

	0,                         // Parent window (no parent)
	0,                         // use class menu
	hInst,                     // handle to window instance
	0);                        // no params to pass on

	// If window could not be created, return "failure"
	if (!hwnd) {
		Panic("Could not create window\n");
		return false;
	}

	Print("  Window created.\n");

	// Make the window visible and draw it
	ShowWindow(hwnd, nCmdShow); // Show the window
	UpdateWindow(hwnd);         // Sends WM_PAINT message

	// Save window properties
	window_style = GetWindowLong(hwnd, GWL_STYLE);
	GetWindowRect(hwnd, &bounds_rect);
	GetClientRect(hwnd, &client_rect);

	// Use client area to set video window size
	video_settings->window_width  = client_rect.right - client_rect.left;
	video_settings->window_height = client_rect.bottom - client_rect.top;

	Print("  Instance initialized.\n");
	return true;
}

// +--------------------------------------------------------------------+

bool
Game::InitVideo()
{
	if (server) return true;

	// create a video factory, and video object:
	video_factory = new(__FILE__,__LINE__) VideoFactory(hwnd);

	if (video_factory) {
		Print("  Init Video...\n");
		Print("  Request %s mode\n", video_settings->GetModeDescription());

		video = video_factory->CreateVideo(video_settings);

		if (video) {
			if (!video->IsHardware()) {
				video_factory->DestroyVideo(video);
				video = 0;

				Panic("3D Hardware Not Found");
			}

			// save a copy of the device-specific video settings:
			else if (video->GetVideoSettings()) {
				*video_settings = *video->GetVideoSettings();
				is_windowed = video_settings->IsWindowed();
			}
		}

		soundcard = video_factory->CreateSoundCard();
	}

	return (video && video->Status() == Video::VIDEO_OK);
}

// +--------------------------------------------------------------------+

bool
Game::ResetVideo()
{
	if (server)          return true;
	if (!video_factory)  return InitVideo();

	Print("  Reset Video...\n");
	Print("  Request %s mode\n", video_settings->GetModeDescription());

	delete screen;

	if (video && !video->Reset(video_settings)) {
		video_factory->DestroyVideo(video);
		video = video_factory->CreateVideo(video_settings);
	}

	if (!video || video->Status() != Video::VIDEO_OK) {
		Panic("Could not re-create Video Interface.");
		return false;
	}

	Print("  Re-created video object.\n");

	// save a copy of the device-specific video settings:
	if (video->GetVideoSettings()) {
		*video_settings = *video->GetVideoSettings();
		is_windowed = video_settings->IsWindowed();
	}

	Color::UseVideo(video);

	screen = new(__FILE__,__LINE__) Screen(video);
	if (!screen) {
		Panic("Could not re-create Screen object.");
		return false;
	}

	Print("  Re-created screen object.\n");

	if (!screen->SetBackgroundColor(Color::Black))
	Print("  WARNING: could not set video background color to Black\n");

	screen->ClearAllFrames(true);
	video->SetGammaLevel(gamma);

	Print("  Re-established requested video parameters.\n");

	Bitmap::CacheUpdate();
	Print("  Refreshed texture bitmaps.\n\n");
	return true;
}

// +--------------------------------------------------------------------+

bool
Game::ResizeVideo()
{
	if (!video || !video_settings)   return false;
	if (!is_windowed)                return false;
	if (ignore_size_change)          return true;

	HRESULT  hr = S_OK;
	RECT     client_old;

	client_old = client_rect;

	// Update window properties
	GetWindowRect(hwnd, &bounds_rect);
	GetClientRect(hwnd, &client_rect);

	if (client_old.right   - client_old.left  !=
			client_rect.right  - client_rect.left ||
			client_old.bottom  - client_old.top   !=
			client_rect.bottom - client_rect.top) {

		// A new window size will require a new backbuffer
		// size, so the 3D structures must be changed accordingly.
		Pause(true);

		video_settings->is_windowed   = true;
		video_settings->window_width  = client_rect.right - client_rect.left;
		video_settings->window_height = client_rect.bottom - client_rect.top;

		::Print("ResizeVideo() %d x %d\n", video_settings->window_width, video_settings->window_height);

		if (video) {
			video->Reset(video_settings);
		}

		Pause(false);
	}

	// save a copy of the device-specific video settings:
	if (video->GetVideoSettings()) {
		*video_settings = *video->GetVideoSettings();
		is_windowed = video_settings->IsWindowed();
	}

	screen->Resize(video_settings->window_width, video_settings->window_height);

	return hr == S_OK;
}

bool
Game::ToggleFullscreen()
{
	bool result = false;

	if (video && video_settings) {
		Pause(true);
		ignore_size_change = true;

		// Toggle the windowed state
		is_windowed = !is_windowed;
		video_settings->is_windowed = is_windowed;

		// Prepare window for windowed/fullscreen change
		AdjustWindowForChange();

		// Reset the 3D device
		if (!video->Reset(video_settings)) {
			// reset failed, try to restore...
			ignore_size_change = false;

			if (!is_windowed) {
				// Restore window type to windowed mode
				is_windowed = !is_windowed;
				video_settings->is_windowed = is_windowed;

				AdjustWindowForChange();

				SetWindowPos(hwnd,
				HWND_NOTOPMOST,
				bounds_rect.left, 
				bounds_rect.top,
				bounds_rect.right  - bounds_rect.left,
				bounds_rect.bottom - bounds_rect.top,
				SWP_SHOWWINDOW);
			}

			::Print("Unable to toggle %s fullscreen mode.\n", is_windowed ? "into" : "out of");
		}

		else {
			ignore_size_change = false;

			// When moving from fullscreen to windowed mode, it is important to
			// adjust the window size after resetting the device rather than
			// beforehand to ensure that you get the window size you want.  For
			// example, when switching from 640x480 fullscreen to windowed with
			// a 1000x600 window on a 1024x768 desktop, it is impossible to set
			// the window size to 1000x600 until after the display mode has
			// changed to 1024x768, because windows cannot be larger than the
			// desktop.

			if (is_windowed) {
				SetWindowPos(hwnd,
				HWND_NOTOPMOST,
				bounds_rect.left, 
				bounds_rect.top,
				bounds_rect.right  - bounds_rect.left,
				bounds_rect.bottom - bounds_rect.top,
				SWP_SHOWWINDOW);
			}

			GetClientRect(hwnd, &client_rect);  // Update our copy
			Pause(false);

			if (is_windowed)
			screen->Resize(video_settings->window_width,
			video_settings->window_height);

			else
			screen->Resize(video_settings->fullscreen_mode.width,
			video_settings->fullscreen_mode.height);

			result = true;
		}
	}

	return result;
}

bool
Game::AdjustWindowForChange()
{
	if (is_windowed) {
		// Set windowed-mode style
		SetWindowLong(hwnd, GWL_STYLE, window_style);
		if (hmenu != NULL) {
			SetMenu(hwnd, hmenu);
			hmenu = NULL;
		}
	}
	else {
		// Set fullscreen-mode style
		SetWindowLong(hwnd, GWL_STYLE, WS_POPUP|WS_SYSMENU|WS_VISIBLE);
		if (hmenu == NULL) {
			hmenu = GetMenu(hwnd);
			SetMenu(hwnd, NULL);
		}
	}

	return true;
}


// +--------------------------------------------------------------------+

bool
Game::InitGame()
{
	if (server) {
		Print("  InitGame() - server mode.\n");
	}

	else {
		if (!SetupPalette()) {
			Panic("Could not set up the palette.");
			return false;
		}

		Print("  Palette loaded.\n");

		if (!InitVideo() || !video || video->Status() != Video::VIDEO_OK) {
			if (!panicbuf[0])
			Panic("Could not create the Video Interface.");
			return false;
		}

		Print("  Created video object.\n");

		Color::UseVideo(video);

		screen = new(__FILE__,__LINE__) Screen(video);
		if (!screen) {
			if (!panicbuf[0])
			Panic("Could not create the Screen object.");
			return false;
		}

		Print("  Created screen object.\n");

		if (!screen->SetBackgroundColor(Color::Black))
		Print("  WARNING: could not set video background color to Black\n");
		screen->ClearAllFrames(true);

		video->SetGammaLevel(gamma);

		Print("  Established requested video parameters.\n\n");
	}

	return true;
}


// +--------------------------------------------------------------------+

bool
Game::InitContent()
{
	DataLoader* loader = DataLoader::GetLoader();
	List<Text>  bundles;

	loader->SetDataPath("Content/");
	loader->ListFiles("content*", bundles);

	ListIter<Text> iter = bundles;
	while (++iter) {
		Text* filename = iter.value();
		int   n        = filename->indexOf('_');

		if (n > 0) {
			Locale::ParseLocale(filename->data() + n);
		}
		else {
			delete content;
			content = new(__FILE__,__LINE__) ContentBundle("content", 0);
		}
	}

	loader->SetDataPath(0);
	return true;
}

void
Game::UseLocale(Locale* locale)
{
	if (game) {
		DataLoader* loader = DataLoader::GetLoader();
		loader->SetDataPath("Content/");
		delete game->content;

		game->content = new(__FILE__,__LINE__) ContentBundle("content", locale);

		loader->SetDataPath(0);
	}
}

// +--------------------------------------------------------------------+

bool
Game::SetupPalette()
{
	if (LoadPalette(standard_palette, inverse_palette)) {
		Color::SetPalette(standard_palette, 256, inverse_palette);
		return true;
	}

	return false;
}   

// +--------------------------------------------------------------------+

bool
Game::LoadPalette(PALETTEENTRY* pal, BYTE* inv)
{
	char palheader[32];
	struct {
		WORD Version;
		WORD NumberOfEntries;
		PALETTEENTRY Entries[256];

	} Palette = { 0x300, 256 };

	DataLoader* loader = DataLoader::GetLoader();
	BYTE* block;
	char fname[256];
	sprintf_s(fname, "%s.pal", palette_name);

	if (!loader->LoadBuffer(fname, block)) {
		Print("  Could not open file '%s'\n", fname);
		return false;
	}

	memcpy(&palheader, block, 24);
	memcpy((char*) Palette.Entries, (block+24), 256*4);

	for (int i = 0; i < 256; i++) {
		*pal++ = Palette.Entries[i];
	}

	loader->ReleaseBuffer(block);

	sprintf_s(fname, "%s.ipl", palette_name);
	int size = loader->LoadBuffer(fname, block);
	if (size < 32768) {
		Print("  Could not open file '%s'\n", fname);
		return false;
	}

	memcpy(inv, block, 32768);
	loader->ReleaseBuffer(block);

	return true;
}

// +--------------------------------------------------------------------+

int 
Game::Run()
{
	MSG msg;

	status = RUN;
	Print("\n");
	Print("+====================================================================+\n");
	Print("|                              RUN                                   |\n");
	Print("+====================================================================+\n");

	// Polling messages from event queue until quit
	while (status < EXIT) {
		if (PeekMessage(&msg, hwnd, 0, 0, PM_REMOVE)) {
			if (msg.message == WM_QUIT)
			break;

			TranslateMessage(&msg);
			DispatchMessage(&msg);
		}
		else {
			if (ProfileGameLoop())
			WaitMessage();
		}
	}

	return exit_code ? exit_code : msg.wParam;
}

// +--------------------------------------------------------------------+

void
Game::Exit()
{
	Print("\n\n*** Game::Exit()\n");
	status = EXIT;
}

void
Game::Panic(const char* msg)
{
	if (msg) Print("*** PANIC: %s\n", msg);
	else     Print("*** PANIC! ***\n");

	if (!msg) msg = "Unspecified fatal error.";
	sprintf_s(panicbuf, "%s\nThis game will now terminate.", msg);

	if (game) {
		game->status = PANIC;
	}
}

// +--------------------------------------------------------------------+

void
Game::Activate(bool f)
{
	active = f;

	if (active && video)
	video->InvalidateCache();
}

// +--------------------------------------------------------------------+

void
Game::Pause(bool f)
{
	if (f) {
		if (soundcard)
		soundcard->Pause();
		paused = true;
	}
	else {
		if (soundcard)
		soundcard->Resume();
		paused = false;
	}
}

// +--------------------------------------------------------------------+

bool ProfileGameLoop(void)
{
	return game->GameLoop();
}

bool
Game::GameLoop()
{
	bool wait_for_windows_events = true;

	if (active && !paused) {
		if (!server) {
			// Route Events to EventTargets
			EventDispatch* ed = EventDispatch::GetInstance();
			if (ed)
			ed->Dispatch();
		}

		UpdateWorld();
		GameState();

		if (!server) {
			UpdateScreen();
			CollectStats();
		}

		wait_for_windows_events = false;
	}
	else if (active && paused) {
		if (GetKey()=='P')
		Pause(false);
	}

	QueryPerformanceCounter(&perf_cnt2);

	double freq = (double) (perf_freq.QuadPart);
	double msec = (double) (perf_cnt2.QuadPart - perf_cnt1.QuadPart);

	msec /= freq;
	msec *= 1000.0;

	if (msec < 1)
	msec = 1;

	real_time += (DWORD) msec;

	frame_number++;
	Mouse::w = 0;

	perf_cnt1 = perf_cnt2;

	return wait_for_windows_events;
}

// +--------------------------------------------------------------------+

void
Game::UpdateWorld()
{
	long   new_time      = real_time;
	double delta         = new_time - frame_time;
	gui_seconds   = delta * 0.001;
	seconds       = max_frame_length;

	if (time_comp == 1)
	{
		if (delta < max_frame_length * 1000)
		seconds = delta * 0.001;
	}
	else
	{
		seconds = time_comp * delta * 0.001;
	}

	frame_time = new_time;
	game_time += (DWORD) (seconds * 1000);

	if (world)
	world->ExecFrame(seconds);
}

// +--------------------------------------------------------------------+

void
Game::GameState()
{
}

// +--------------------------------------------------------------------+

void
Game::UpdateScreen()
{
	if (!screen || !video) return;

	if (screen->Refresh()) {
		if (Keyboard::KeyDown(VK_F12)) {
			if (Keyboard::KeyDown(VK_SHIFT)) {
				if (!avi_file) {
					AVICapture();                    // begin capturing
					SetMaxFrameLength(MAX_FRAME_TIME_VIDEO);
				}
				else {
					delete avi_file;                 // end capture;
					avi_file = 0;
					SetMaxFrameLength(MAX_FRAME_TIME_NORMAL);
				}
			}
			else {
				if (!avi_file) {
					ScreenCapture();
				}
				else {
					delete avi_file;                 // end capture;
					avi_file = 0;
					SetMaxFrameLength(MAX_FRAME_TIME_NORMAL);
				}
			}
		}
		else if (avi_file) {
			AVICapture();                          // continue capturing...
		}

		video->Present();
	}
	else {
		Panic("Screen refresh failed.");
	}
}

// +--------------------------------------------------------------------+

Game*
Game::GetInstance()
{
	return game;
}

Video*
Game::GetVideo()
{
	if (game)
	return game->video;

	return 0;
}

Color
Game::GetScreenColor()
{
	if (game)
	return game->screen_color;

	return Color::Black;
}

void
Game::SetScreenColor(Color c)
{
	if (game) {
		game->screen_color = c;

		if (game->screen)
		game->screen->SetBackgroundColor(c);
	}
}

int
Game::GetScreenWidth()
{
	if (game && game->video)
	return game->video->Width();

	return 0;
}

int
Game::GetScreenHeight()
{
	if (game && game->video)
	return game->video->Height();

	return 0;
}

// +--------------------------------------------------------------------+

void
Game::ScreenCapture(const char* name)
{
	if (server || !video || !screen) return;

	static DWORD last_shot = 0;
	static DWORD shot_num  = 1;
	DataLoader*  loader    = DataLoader::GetLoader();
	char         filename[256];

	if (!name && (real_time - last_shot) < 1000)
	return;

	// try not to overwrite existing screen shots...
	if (loader) {
		bool use_file_sys = loader->IsFileSystemEnabled();
		loader->UseFileSystem(true);
		loader->SetDataPath(0);
		List<Text> shot_list;
		loader->ListFiles("*.PCX", shot_list);
		loader->UseFileSystem(use_file_sys);

		for (int i = 0; i < shot_list.size(); i++) {
			Text* s = shot_list[i];
			int   n = 0;

			sscanf_s(s->data()+1, "%d", &n);
			if (shot_num <= (DWORD) n)
			shot_num = n+1;
		}

		shot_list.destroy();
	}

	if (name)
	strcpy_s(filename, name);
	else
	sprintf_s(filename, "A%d.PCX", shot_num++); //-V576

	Bitmap bmp;

	if (video && video->Capture(bmp)) {
		PcxImage pcx(bmp.Width(), bmp.Height(), (LPDWORD) bmp.HiPixels());
		pcx.Save((char*) filename);
	}

	last_shot = real_time;
}

// +--------------------------------------------------------------------+

void
Game::AVICapture(const char* name)
{
	if (server || !video || !screen) return;

	if (!avi_file) {
		char        filename[256];
		Bitmap      bmp;
		DataLoader* loader    = DataLoader::GetLoader();
		DWORD       avi_num   = 1;

		// try not to overwrite existing screen shots...
		if (loader) {
			bool use_file_sys = loader->IsFileSystemEnabled();
			loader->UseFileSystem(true);
			loader->SetDataPath(0);
			List<Text> avi_list;
			loader->ListFiles("*.avi", avi_list);
			loader->UseFileSystem(use_file_sys);

			for (int i = 0; i < avi_list.size(); i++) {
				Text* s = avi_list[i];
				int   n = 0;

				sscanf_s(s->data()+1, "%d", &n);
				if (avi_num <= (DWORD) n)
				avi_num = n+1;
			}

			avi_list.destroy();
		}

		if (name)
		strcpy_s(filename, name);
		else
		sprintf_s(filename, "A%d.avi", avi_num); //-V576

		if (video && video->Capture(bmp)) {
			//bmp.ScaleTo(bmp.Width()/2, bmp.Height()/2);
			avi_file = new(__FILE__,__LINE__) AviFile(filename, Rect(0,0,bmp.Width(),bmp.Height()), VIDEO_FPS);
		}

	}

	else {
		Bitmap bmp;

		if (video && video->Capture(bmp)) {
			//bmp.ScaleTo(bmp.Width()/2, bmp.Height()/2);
			avi_file->AddFrame(bmp);
		}
	}
}



// +--------------------------------------------------------------------+

void
Game::CollectStats()
{
	frame_count++;

	if (!totaltime) totaltime = real_time;

	if (frame_time - frame_time0 > 200) {
		frame_rate   = (frame_count - frame_count0) * 1000.0 / (frame_time - frame_time0);
		frame_time0  = frame_time;
		frame_count0 = frame_count;
	}

	if (video) {
		stats.nframe  = video->GetStats().nframe;
		stats.nverts  = video->GetStats().nverts;
		stats.npolys  = video->GetStats().npolys;
		stats.nlines  = video->GetStats().nlines;
		stats.ncalls += video->GetStats().ncalls;

		stats.total_verts += stats.nverts;
		stats.total_polys += stats.npolys;
		stats.total_lines += stats.nlines;
	}
}

// +--------------------------------------------------------------------+

void
Game::ShowStats()
{
	if (server) return;

	totaltime = real_time - totaltime;

	Print("\n");
	Print("Performance Data:\n");
	Print("-----------------\n");

	Print("   Time:            %d msec\n", totaltime);
	Print("   Frames:          %d\n", stats.nframe);
	Print("   Polys Rendered:  %d\n", stats.total_polys);
	Print("   Lines Rendered:  %d\n", stats.total_lines);
	Print("   Verts Rendered:  %d\n", stats.total_verts);
	Print("   Render Calls:    %d\n", stats.ncalls);
	Print("\n");

	Print("Performance Statistics:\n");
	Print("-----------------------\n");

	Print("   Frames/Second:   %.2f\n", (stats.nframe * 1000.0)      / totaltime);
	Print("   Polys/Frame:     %.2f\n", (double) stats.total_polys   / (double) stats.nframe);
	Print("   Polys/Call:      %.2f\n", (double) stats.total_polys   / (double) stats.ncalls);
	Print("   Polys/Second:    %.2f\n", (stats.total_polys * 1000.0) / totaltime);
	Print("   Lines/Second:    %.2f\n", (stats.total_lines * 1000.0) / totaltime);
	Print("   Verts/Second:    %.2f\n", (stats.total_verts * 1000.0) / totaltime);

	Print("\n");
}

// +====================================================================+
// WndProc
// +====================================================================+

#ifndef WM_MOUSEWHEEL
#define WM_MOUSEWHEEL 0x20A
#endif

LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM uParam, LPARAM lParam)
{
	switch (message) {
	case WM_SYSKEYDOWN:
		if (uParam == VK_TAB || uParam == VK_F4)
		return DefWindowProc(hwnd, message, uParam, lParam);

		return 0;

	case WM_MENUCHAR:
		return MNC_CLOSE << 16;

	case WM_ACTIVATEAPP:
		// Keep track of whether or not the app is in the foreground
		if (game)
		game->Activate(uParam?true:false);
		break;

	case WM_PAINT:
		if (!game || !game->OnPaint())
		return DefWindowProc(hwnd, message, uParam, lParam);
		break;

	case WM_SETCURSOR:
		if (Game::ShowMouse()) {
			return DefWindowProc(hwnd, message, uParam, lParam);
		}
		else {
			// hide the windows mouse cursor
			SetCursor(NULL);
			return 1;
		}
		break;

	case WM_ENTERSIZEMOVE:
		// Halt frame movement while the app is sizing or moving
		if (game)
		game->Pause(true);
		break;

	case WM_SIZE:
		// Pick up possible changes to window style due to maximize, etc.
		if (game && game->hwnd != NULL ) {
			game->window_style = GetWindowLong(game->hwnd, GWL_STYLE );

			if (uParam == SIZE_MINIMIZED) {
				game->Pause(true); // Pause while we're minimized
				game->is_minimized = true;
				game->is_maximized = false;
			}

			else if (uParam == SIZE_MAXIMIZED) {
				if (game->is_minimized)
				game->Pause(false); // Unpause since we're no longer minimized

				game->is_minimized = false;
				game->is_maximized = true;
				game->ResizeVideo();
			}

			else if (uParam == SIZE_RESTORED) {
				if (game->is_maximized) {
					game->is_maximized = false;
					game->ResizeVideo();
				}

				else if (game->is_minimized) {
					game->Pause(false); // Unpause since we're no longer minimized
					game->is_minimized = false;
					game->ResizeVideo();
				}
				else {
					// If we're neither maximized nor minimized, the window size 
					// is changing by the user dragging the window edges.  In this 
					// case, we don't reset the device yet -- we wait until the 
					// user stops dragging, and a WM_EXITSIZEMOVE message comes.
				}
			}
		}
		break;

	case WM_EXITSIZEMOVE:
		if (game) {
			game->Pause(false);
			game->ResizeVideo();
		}
		break;


	case WM_ENTERMENULOOP:
		if (game)
		game->Pause(true);
		break;

	case WM_EXITMENULOOP:
		if (game)
		game->Pause(false);
		break;

		/*
case WM_HELP:
	if (game)
		return game->OnHelp();
	break;
*/

	case WM_KEYDOWN:
		BufferKey(uParam);
		return 0;

	case WM_DESTROY:
		PostQuitMessage(0);
		break;

	case WM_MOUSEMOVE:
		Mouse::x = LOWORD(lParam);
		Mouse::y = HIWORD(lParam);
		break;

	case WM_LBUTTONDOWN:
		Mouse::l = 1;
		break;

	case WM_LBUTTONDBLCLK:
		Mouse::l = 2;
		break;

	case WM_LBUTTONUP:
		Mouse::l = 0;
		break;

	case WM_MBUTTONDOWN:
		Mouse::m = 1;
		break;

	case WM_MBUTTONDBLCLK:
		Mouse::m = 2;
		break;

	case WM_MBUTTONUP:
		Mouse::m = 0;
		break;

	case WM_RBUTTONDOWN:
		Mouse::r = 1;
		break;

	case WM_RBUTTONDBLCLK:
		Mouse::r = 2;
		break;

	case WM_RBUTTONUP:
		Mouse::r = 0;
		break;

	case WM_MOUSEWHEEL:
		{
			int w = (int) (uParam >> 16);
			if (w > 32000) w -= 65536;
			Mouse::w += w;
		}
		break;

	case WM_CLOSE:
		if (game) // && game->Server())
		game->Exit();
		break;

	default:
		return DefWindowProc(hwnd, message, uParam, lParam);
	}

	return 0;
}

// +====================================================================+

const int MAX_KEY_BUF = 512;
static int vkbuf[MAX_KEY_BUF];
static int vkshiftbuf[MAX_KEY_BUF];
static int vkins = 0;
static int vkext = 0;

void
FlushKeys()
{
	Keyboard::FlushKeys();
	vkins = vkext = 0;
}

void
BufferKey(int vkey)
{
	if (vkey < 1) return;

	int shift = 0;

	if (GetAsyncKeyState(VK_SHIFT))
	shift |= 1;

	if (GetAsyncKeyState(VK_CONTROL))
	shift |= 2;

	if (GetAsyncKeyState(VK_MENU))
	shift |= 4;

	vkbuf[vkins] = vkey;
	vkshiftbuf[vkins++] = shift;

	if (vkins >= MAX_KEY_BUF)
	vkins = 0;

	if (vkins == vkext) {
		vkext++;
		if (vkext >= MAX_KEY_BUF)
		vkext = 0;
	}   
}

int
GetKey()
{
	if (vkins == vkext) return 0;

	int result = vkbuf[vkext++];
	if (vkext >= MAX_KEY_BUF)
	vkext = 0;

	return result;
}

int
GetKeyPlus(int& key, int& shift)
{
	if (vkins == vkext) return 0;

	key = vkbuf[vkext];
	shift = vkshiftbuf[vkext++];

	if (vkext >= MAX_KEY_BUF)
	vkext = 0;

	return key;
}

// +====================================================================+

void Print(const char* fmt, ...)
{
	if (ErrLog) {
		vsprintf_s(ErrBuf, fmt, (char *)(&fmt+1));

		fprintf(ErrLog, ErrBuf);
		fflush(ErrLog);
	}
}

// +====================================================================+

DWORD GetRealTime()
{
	if (game)
	return Game::RealTime();

	return timeGetTime();
}

DWORD Game::RealTime()
{
	return real_time;
}

DWORD Game::GameTime()
{
	return game_time;
}

DWORD Game::TimeCompression()
{
	return time_comp;
}

void Game::SetTimeCompression(DWORD comp)
{
	if (comp > 0 && comp <= 100)
	time_comp = comp;
}

DWORD Game::Frame()
{
	return frame_number;
}

void Game::ResetGameTime()
{
	game_time = 0;
}

void Game::SkipGameTime(double seconds)
{
	if (seconds > 0)
	game_time += (DWORD) (seconds * 1000);
}
