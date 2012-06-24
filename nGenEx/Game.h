/*  Project nGenEx
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    nGenEx.lib
	FILE:         Game.h
	AUTHOR:       John DiCamillo

*/

#ifndef Game_h
#define Game_h

#include "Types.h"
#include "Screen.h"
#include "Video.h"

// +--------------------------------------------------------------------+

void              Print(const char* fmt, ...);

LRESULT CALLBACK  WndProc(HWND, UINT, WPARAM, LPARAM);

void              FlushKeys();
void              BufferKey(int vkey);
int               GetKey();
int               GetKeyPlus(int& key, int& shift);
void			  ProcessKeyMessage();

extern "C" bool   ProfileGameLoop(void);

// +--------------------------------------------------------------------+

class ContentBundle;
class Locale;
class Universe;
class Sound;
class SoundCard;
class Video;
class VideoFactory;
class VideoSettings;
class AviFile;
class Text;

// +--------------------------------------------------------------------+

class Game
{
public:
	static const char* TYPENAME() { return "Game"; }
	enum STATUS { OK, RUN, EXIT, PANIC, INIT_FAILED, TOO_MANY };

	Game();
	virtual ~Game();

	//
	// MAIN GAME FUNCTIONALITY:
	//

	virtual bool      Init(HINSTANCE hi, HINSTANCE hpi, LPSTR cmdline, int nCmdShow);
	virtual int       Run();
	virtual void      Exit();
	virtual bool      OnPaint()      { return false;  } 
	virtual bool      OnHelp()       { return false;  }

	virtual void      Activate(bool f);
	virtual void      Pause(bool f);
	int               Status() const { return status; }

	virtual void      ScreenCapture(const char* name = 0);
	virtual void      AVICapture(const char* fname = 0);

	const RenderStats& GetPolyStats() { return stats;  }

	//
	// GENERAL GAME CLASS UTILITY METHODS:
	//

	static void       Panic(const char* msg=0);
	static bool       DisplayModeSupported(int w, int h, int bpp);
	static int        MaxTexSize();
	static int        MaxTexAspect();
	static int        GammaLevel();
	static void       SetGammaLevel(int g);
	static void       SetMaxTexSize(int n);

	static DWORD      RealTime();
	static DWORD      GameTime();
	static DWORD      TimeCompression();
	static void       SetTimeCompression(DWORD comp);
	static DWORD      Frame();
	static void       ResetGameTime();
	static void       SkipGameTime(double seconds);

	static double     FrameRate();
	static double     FrameTime();
	static double     GUITime();
	static void       SetMaxFrameLength(double seconds) { max_frame_length = seconds; }
	static void       SetMinFrameLength(double seconds) { min_frame_length = seconds; }
	static double     GetMaxFrameLength()               { return max_frame_length;    }
	static double     GetMinFrameLength()               { return min_frame_length;    }

	static Game*      GetInstance();
	static Video*     GetVideo();
	static Color      GetScreenColor();
	static void       SetScreenColor(Color c);
	static int        GetScreenWidth();
	static int        GetScreenHeight();

	static bool       Active()       { return active;     }
	static bool       Paused()       { return paused;     }
	static bool       Server()       { return server;     }
	static bool       ShowMouse()    { return show_mouse; }
	static bool       IsWindowed();

	static HINSTANCE  GetHINST();
	static HWND       GetHWND();

	static void       UseLocale(Locale* locale);
	static Text       GetText(const char* key);

	static const char* GetPanicMessage() { return panicbuf; }

	virtual bool      GameLoop();
	virtual void      UpdateWorld();
	virtual void      GameState();
	virtual void      UpdateScreen();
	virtual void      CollectStats();

	virtual bool      InitApplication(HINSTANCE);
	virtual bool      InitInstance(HINSTANCE, int);
	virtual bool      InitContent();
	virtual bool      InitGame();
	virtual bool      InitVideo();
	virtual bool      ResizeVideo();
	virtual bool      ResetVideo();
	virtual bool      ToggleFullscreen();
	virtual bool      AdjustWindowForChange();

	virtual bool      SetupPalette();
	virtual bool      LoadPalette(PALETTEENTRY* pal, BYTE* inv);
	virtual void      ShowStats();

protected:
	friend  bool      ProfileGameLoop(void);
	friend  LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM uParam, LPARAM lParam);

	ContentBundle*    content;
	Universe*         world;
	VideoFactory*     video_factory;
	Video*            video;
	VideoSettings*    video_settings;
	SoundCard*        soundcard;
	Screen*           screen;
	int               gamma;
	int               max_tex_size;

	RenderStats       stats;
	DWORD             totaltime;

	PALETTEENTRY      standard_palette[256];
	BYTE              inverse_palette[32768];

	HINSTANCE         hInst;
	HWND              hwnd;
	HMENU             hmenu;
	DWORD             winstyle;

	char*             app_name;
	char*             title_text;
	char*             palette_name;

	// Internal variables for the state of the app
	bool              is_windowed;
	bool              is_active;
	bool              is_device_lost;
	bool              is_minimized;
	bool              is_maximized;
	bool              ignore_size_change;
	bool              is_device_initialized;
	bool              is_device_restored;
	DWORD             window_style;        // Saved window style for mode switches
	RECT              bounds_rect;         // Saved window bounds for mode switches
	RECT              client_rect;         // Saved client area size for mode switches


	double            gui_seconds;
	double            seconds;
	double            frame_rate;
	int               frame_count;
	int               frame_count0;
	int               frame_time;
	int               frame_time0;

	int               status;
	int               exit_code;
	Color             screen_color;

	AviFile*          avi_file;

	static bool       active;
	static bool       paused;
	static bool       server;
	static bool       show_mouse;
	static DWORD      base_game_time;
	static DWORD      real_time;
	static DWORD      game_time;
	static DWORD      time_comp;
	static DWORD      frame_number;

	static double     max_frame_length;
	static double     min_frame_length;

	static char       panicbuf[256];
};

// +--------------------------------------------------------------------+

#endif Game_h


