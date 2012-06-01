/*  Project nGenEx
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    nGenEx.lib
	FILE:         Mouse.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Mouse class
*/

#ifndef Mouse_h
#define Mouse_h

#include "Types.h"

// +--------------------------------------------------------------------+

class Bitmap;
class Screen;
class Window;

// +--------------------------------------------------------------------+

class Mouse
{
	friend LRESULT CALLBACK WndProc(HWND, UINT, WPARAM, LPARAM);
	friend class Game;

public:
	static const char* TYPENAME() { return "Mouse"; }

	enum CURSOR    { ARROW, CROSS, WAIT, NOT, DRAG, USER1, USER2, USER3 };
	enum HOTSPOT   { HOTSPOT_CTR, HOTSPOT_NW };

	static int     X()               { return x; }
	static int     Y()               { return y; }
	static int     LButton()         { return l; }
	static int     MButton()         { return m; }
	static int     RButton()         { return r; }
	static int     Wheel()           { return w; }

	static void    Paint();

	static void    SetCursorPos(int x, int y);
	static void    Show(int s=1);
	static int     SetCursor(CURSOR c);
	static int     LoadCursor(CURSOR c, const char* name, HOTSPOT hs = HOTSPOT_CTR);

	static void    Create(Screen* screen);
	static void    Resize(Screen* screen);
	static void    Close();

private:
	static int     show;
	static int     cursor;

	static int     x;
	static int     y;
	static int     l;
	static int     m;
	static int     r;
	static int     w;

	static Bitmap* image[8];
	static int     hotspot[8];

	static Window* window;
};

#endif Mouse_h

