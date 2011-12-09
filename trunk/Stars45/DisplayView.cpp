/*  Project Starshatter 4.5
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    Stars.exe
	FILE:         DisplayView.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	View class for Quantum Destination HUD Overlay
*/

#include "MemDebug.h"
#include "DisplayView.h"
#include "QuantumDrive.h"
#include "HUDView.h"
#include "Ship.h"
#include "Element.h"
#include "Sim.h"
#include "StarSystem.h"
#include "FormatUtil.h"

#include "Color.h"
#include "Window.h"
#include "Video.h"
#include "Screen.h"
#include "DataLoader.h"
#include "Scene.h"
#include "Font.h"
#include "FontMgr.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "Game.h"
#include "Menu.h"

// +====================================================================+

class DisplayElement
{
public:
	static const char* TYPENAME() { return "DisplayElement"; }

	DisplayElement() : image(0), font(0), blend(0), hold(0), fade_in(0), fade_out(0) { }

	Text           text;
	Bitmap*        image;
	Font*          font;
	Color          color;
	Rect           rect;
	int            blend;
	double         hold;
	double         fade_in;
	double         fade_out;
};

// +====================================================================+

DisplayView* display_view = 0;

DisplayView::DisplayView(Window* c)
: View(c), width(0), height(0), xcenter(0), ycenter(0)
{
	display_view   = this;
	DisplayView::OnWindowMove();
}

DisplayView::~DisplayView()
{
	if (display_view == this)
	display_view = 0;

	elements.destroy();
}

DisplayView*
DisplayView::GetInstance()
{
	if (display_view == 0)
	display_view = new DisplayView(0);

	return display_view;
}

void
DisplayView::OnWindowMove()
{
	if (window) {
		width       = window->Width();
		height      = window->Height();
		xcenter     = (width  / 2.0) - 0.5;
		ycenter     = (height / 2.0) + 0.5;
	}
}

// +--------------------------------------------------------------------+

void
DisplayView::Refresh()
{
	ListIter<DisplayElement> iter = elements;
	while (++iter) {
		DisplayElement* elem = iter.value();

		// convert relative rect to window rect:
		Rect elem_rect = elem->rect;
		if (elem_rect.x == 0 && elem_rect.y == 0 && elem_rect.w == 0 && elem_rect.h == 0) {
			// stretch to fit
			elem_rect.w = width;
			elem_rect.h = height;
		}
		else if (elem_rect.w < 0 && elem_rect.h < 0) {
			// center image in window
			elem_rect.w *= -1;
			elem_rect.h *= -1;

			elem_rect.x = (width  - elem_rect.w)/2;
			elem_rect.y = (height - elem_rect.h)/2;
		}
		else {
			// offset from right or bottom
			if (elem_rect.x < 0) elem_rect.x += width;
			if (elem_rect.y < 0) elem_rect.y += height;
		}

		// compute current fade,
		// assumes fades are 1 second or less:
		double fade = 0;
		if (elem->fade_in > 0)        fade = 1 - elem->fade_in;
		else if (elem->hold > 0)      fade = 1;
		else if (elem->fade_out > 0)  fade = elem->fade_out;

		// draw text:
		if (elem->text.length() && elem->font) {
			elem->font->SetColor(elem->color);
			elem->font->SetAlpha(fade);
			window->SetFont(elem->font);
			window->DrawText(elem->text, elem->text.length(), elem_rect, DT_WORDBREAK);
		}

		// draw image:
		else if (elem->image) {
			window->FadeBitmap(  elem_rect.x, 
			elem_rect.y,
			elem_rect.x + elem_rect.w,
			elem_rect.y + elem_rect.h,
			elem->image,
			elem->color * fade,
			elem->blend );

		}
	}
}

// +--------------------------------------------------------------------+

void
DisplayView::ExecFrame()
{
	double seconds = Game::GUITime();

	ListIter<DisplayElement> iter = elements;
	while (++iter) {
		DisplayElement* elem = iter.value();

		if (elem->fade_in > 0)
		elem->fade_in -= seconds;

		else if (elem->hold > 0)
		elem->hold -= seconds;

		else if (elem->fade_out > 0)
		elem->fade_out -= seconds;

		else
		delete iter.removeItem();
	}
}

// +--------------------------------------------------------------------+

void
DisplayView::ClearDisplay()
{
	elements.destroy();
}

// +--------------------------------------------------------------------+

void
DisplayView::AddText( const char*   text,
Font*         font,
Color         color,
const Rect&   rect, 
double        hold, 
double        fade_in, 
double        fade_out)
{
	DisplayElement* elem = new(__FILE__,__LINE__) DisplayElement;

	if (fade_in == 0 && fade_out == 0 && hold == 0)
	hold = 300;

	elem->text     = text;
	elem->font     = font;
	elem->color    = color;
	elem->rect     = rect;
	elem->hold     = hold;
	elem->fade_in  = fade_in;
	elem->fade_out = fade_out;

	elements.append(elem);
}

void
DisplayView::AddImage(Bitmap*       bmp,
Color         color,
int           blend,
const Rect&   rect,
double        hold, 
double        fade_in, 
double        fade_out)
{
	DisplayElement* elem = new(__FILE__,__LINE__) DisplayElement;

	if (fade_in == 0 && fade_out == 0 && hold == 0)
	hold = 300;

	elem->image    = bmp;
	elem->rect     = rect;
	elem->color    = color;
	elem->blend    = blend;
	elem->hold     = hold;
	elem->fade_in  = fade_in;
	elem->fade_out = fade_out;

	elements.append(elem);
}
