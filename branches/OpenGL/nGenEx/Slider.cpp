/*  Project nGenEx
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    nGenEx.lib
	FILE:         Slider.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Slider/Gauge ActiveWindow class
*/

#include "MemDebug.h"
#include "Slider.h"
#include "Video.h"
#include "Font.h"

// +--------------------------------------------------------------------+

Slider::Slider(ActiveWindow* p, int ax, int ay, int aw, int ah, DWORD aid)
: ActiveWindow(p->GetScreen(), ax, ay, aw, ah, aid, 0, p)
{
	captured       = false;
	dragging       = false;

	border_color   = Color::Black;
	fill_color     = Color::DarkBlue;

	active         = true;
	border         = true;
	num_leds       = 1;
	orientation    = 0;

	range_min      = 0;
	range_max      = 100;
	step_size      = 10;
	show_thumb     = 0;
	thumb_size     = -1;

	nvalues        = 1;
	ZeroMemory(value, sizeof(value));

	marker[0]      = -1;
	marker[1]      = -1;

	char buf[32];
	sprintf_s(buf, "Slider %d", id);
	desc = buf;
}

Slider::Slider(Screen* s, int ax, int ay, int aw, int ah, DWORD aid)
: ActiveWindow(s, ax, ay, aw, ah, aid)
{
	captured       = false;
	dragging       = false;

	border_color   = Color::Black;
	fill_color     = Color::DarkBlue;

	active         = true;
	border         = true;
	num_leds       = 1;
	orientation    = 0;

	range_min      = 0;
	range_max      = 100;
	step_size      = 10;
	show_thumb     = 0;
	thumb_size     = -1;

	nvalues        = 1;
	ZeroMemory(value, sizeof(value));

	marker[0]      = -1;
	marker[1]      = -1;

	char buf[32];
	sprintf_s(buf, "Slider %d", id);
	desc = buf;
}

Slider::~Slider()
{
}

// +--------------------------------------------------------------------+

void
Slider::Draw()
{
	int x = 0;
	int y = 0;
	int w = rect.w;
	int h = rect.h;

	if (w < 1 || h < 1 || !shown)
	return;

	Rect  ctrl_rect(x,y,w,h);

	// draw the border:
	if (border) {
		Color oc = ShadeColor(border_color, 1);
		DrawRect(0,0,w-1,h-1,oc);
		DrawRect(1,1,w-2,h-2,oc);

		ctrl_rect.Deflate(2,2);
	}

	// draw the bevel:
	FillRect(ctrl_rect, back_color);
	DrawStyleRect(ctrl_rect, WIN_SUNK_FRAME);

	// HORIZONTAL
	if (orientation == 0) {
		// draw the leds:
		int led_width  = ((w - 6) / (num_leds)) - 1;
		int led_height = ((h - 5) / (nvalues))  - 1;

		if (nvalues < 2) {
			int fill_width  = (int) ((double)(w-4) * FractionalValue());
			int num_lit     = fill_width / (led_width+1);

			Color fc = ShadeColor(fill_color, 1);

			if (num_leds == 1) {
				FillRect(2,2,2+fill_width,h-3,fc);
			}
			else {
				int x0 = 2;

				for (int i = 0; i < num_lit; i++) {
					FillRect(x0,2,x0+led_width,h-3,fc);
					x0 += led_width + 1;
				}
			}

			// draw the thumb:
			if (thumb_size) {
				if (thumb_size < 0) thumb_size = h;

				thumb_pos = 2+fill_width;

				Rect thumb_rect(thumb_pos-thumb_size/2, 0, thumb_size, h);

				if (thumb_rect.x < 0)
				thumb_rect.x = 0;

				else if (thumb_rect.x > w-thumb_size)
				thumb_rect.x = w-thumb_size;

				if (show_thumb) {
					FillRect(thumb_rect, back_color);
					DrawStyleRect(thumb_rect, WIN_RAISED_FRAME);
				}
			}
		}

		else {
			Color fc = ShadeColor(fill_color, 1);

			int y0 = 3;

			for (int i = 0; i < nvalues; i++) {
				int fill_width  = (int) ((double)(w-6) * FractionalValue(i));
				FillRect(3,y0,3+fill_width,y0+led_height,fc);

				y0 += led_height+1;
			}
		}

		// draw the markers:
		if (marker[0] >= 0) {
			int   m = marker[0];
			Color c = ShadeColor(base_color, 1.6);    // full highlight
			DrawLine(m-3, 1, m+4,  1, c);
			c = ShadeColor(base_color, 1.3);          // soft highlight
			DrawLine(m-3, 2, m-3,  4, c);
			DrawLine(m-2, 4, m-2,  6, c);
			DrawLine(m-1, 6, m-1,  8, c);
			DrawLine(m  , 8, m  , 10, c);
			c = base_color;                           // body
			DrawLine(m-2, 2, m-2,  4, c);
			DrawLine(m-1, 2, m-1,  6, c);
			DrawLine(m  , 2, m  ,  8, c);
			DrawLine(m+1, 2, m+1,  6, c);
			DrawLine(m+2, 2, m+2,  4, c);
			c = ShadeColor(base_color, 0.5);          // shadow
			DrawLine(m+1, 6, m+1,  8, c);
			DrawLine(m+2, 4, m+2,  6, c);
			DrawLine(m+3, 2, m+3,  4, c);
		}

		if (marker[1] >= 0) {
			int   m = marker[0];
			Color c = ShadeColor(base_color, 0.5);    // shadow
			DrawLine(m-3, h-2, m+4, h-2, c);
			DrawLine(m+1, h-6, m+1, h-8, c);
			DrawLine(m+2, h-4, m+2, h-6, c);
			DrawLine(m+3, h-2, m+3, h-4, c);
			c = ShadeColor(base_color, 1.3);          // soft highlight
			DrawLine(m-3, h-2, m-3, h-4, c);
			DrawLine(m-2, h-4, m-2, h-6, c);
			DrawLine(m-1, h-6, m-1, h-8, c);
			DrawLine(m  , h-8, m  , h-10, c);
			c = base_color;                           // body
			DrawLine(m-2, h-2, m-2, h-4, c);
			DrawLine(m-1, h-2, m-1, h-6, c);
			DrawLine(m  , h-2, m  , h-8, c);
			DrawLine(m+1, h-2, m+1, h-6, c);
			DrawLine(m+2, h-2, m+2, h-4, c);
		}
	}

	// VERTICAL
	else {
		// draw the leds:
		int led_width  = ((w - 5) / (nvalues))  - 1;

		if (num_leds > 1) {
		}
		else {
			if (nvalues < 2) {
				led_width  = w - 4;
				int led_height = (int) ((double)(h-4) * FractionalValue());

				Color fc = ShadeColor(fill_color, 1);
				FillRect(2, h-2-led_height, 2+led_width, h-2, fc);

				// draw the thumb:
				if (thumb_size) {
					if (thumb_size < 0) thumb_size = w;

					thumb_pos = h-2-led_height;

					Rect thumb_rect(0, thumb_pos-(thumb_size/2), w, thumb_size);

					if (thumb_rect.y < 0)
					thumb_rect.y = 0;

					else if (thumb_rect.y > h-thumb_size)
					thumb_rect.y = h-thumb_size;

					if (show_thumb) {
						FillRect(thumb_rect, back_color);
						DrawStyleRect(thumb_rect, WIN_RAISED_FRAME);
					}
				}
			}

			else {
				Color fc = ShadeColor(fill_color, 1);

				int x0 = 3;

				for (int i = 0; i < nvalues; i++) {
					int led_height = (int) ((double)(h-6) * FractionalValue(i));
					FillRect(x0,h-3-led_height,x0+led_width,h-3,fc);

					x0 += led_width+1;
				}
			}
		}
	}
}

// +--------------------------------------------------------------------+

bool Slider::GetActive()
{
	return active;
}

void  Slider::SetActive(bool bNewValue)
{
	active = bNewValue;
}

bool  Slider::GetBorder()
{
	return border;
}

void  Slider::SetBorder(bool bNewValue)
{
	border = bNewValue;
}

Color  Slider::GetBorderColor()
{
	return border_color;
}

void  Slider::SetBorderColor(Color newValue)
{
	border_color = newValue;
}

Color Slider::GetFillColor()
{
	return fill_color;
}

void  Slider::SetFillColor(Color cNewValue)
{
	fill_color = cNewValue;
}

int   Slider::GetNumLeds()
{
	return num_leds;
}

void  Slider::SetNumLeds(int nNewValue)
{
	if (nNewValue >= 0) {
		num_leds = nNewValue;
	}
}

int   Slider::GetOrientation()
{
	return orientation;
}

void  Slider::SetOrientation(int nNewValue)
{
	if (nNewValue == 0 || nNewValue == 1) {
		orientation = nNewValue;
	}
}

int   Slider::GetRangeMin()
{
	return range_min;
}

void  Slider::SetRangeMin(int nNewValue)
{
	range_min = nNewValue;
}

int   Slider::GetRangeMax()
{
	return range_max;
}

void  Slider::SetRangeMax(int nNewValue)
{
	range_max = nNewValue;
}

int   Slider::GetStepSize()
{
	return step_size;
}

void  Slider::SetStepSize(int nNewValue)
{
	if (nNewValue < range_max - range_min)
	step_size = nNewValue;
}

bool  Slider::GetShowThumb()
{
	return show_thumb?true:false;
}

void  Slider::SetShowThumb(bool bNewValue)
{
	show_thumb = bNewValue;
}

int   Slider::GetThumbSize()
{
	return thumb_size;
}

void  Slider::SetThumbSize(int nNewValue)
{
	if (nNewValue < range_max - range_min)
	thumb_size = nNewValue;
}

int   Slider::NumValues()
{
	return nvalues;
}

int   Slider::GetValue(int index)
{
	if (index >= 0 && index < nvalues)
	return value[index];

	return 0;
}

void  Slider::SetValue(int nNewValue, int index)
{
	if (index >= 0 && index < MAX_VAL) {
		value[index] = nNewValue;
		
		if (index >= nvalues)
		nvalues = index+1;
	}
}

void  Slider::SetMarker(int nNewValue, int index)
{
	if (index >= 0 && index < 2) {
		marker[index] = nNewValue;
	}
}

double Slider::FractionalValue(int index)
{
	if (index >= 0 && index < nvalues)
	return ((double) (value[index]-range_min)) / ((double) (range_max-range_min));

	return 0;
}

// +--------------------------------------------------------------------+

void  Slider::StepUp(int index)
{
	if (index >= 0 && index < nvalues) {
		value[index] += step_size;

		if (value[index] > range_max)
		value[index] = range_max;
	}
}

void  Slider::StepDown(int index)
{
	if (index >= 0 && index < nvalues) {
		value[index] -= step_size;

		if (value[index] < range_min)
		value[index] = range_min;
	}
}

// +--------------------------------------------------------------------+

int Slider::OnMouseMove(int x, int y)
{
	bool dirty = false;

	if (captured)
	{
		ActiveWindow* test = GetCapture();

		if (test != this)
		{
			captured = false;
			dirty = true;
		}

		else if (dragging)
		{
			mouse_x = x - rect.x;
			if (mouse_x < 0) mouse_x = 0;
			else if (mouse_x > rect.w) mouse_x = rect.w;

			mouse_y = rect.h - (y - rect.y);
			if (mouse_y < 0) mouse_y = 0;
			else if (mouse_y > rect.h) mouse_y = rect.h;

			// HORIZONTAL
			if (orientation == 0) {
				SetValue((int) ((double) mouse_x/rect.w * (range_max-range_min) + range_min));
			}

			// VERTICAL
			else {
				SetValue((int) ((double) mouse_y/rect.h * (range_max-range_min) + range_min));
			}

			dirty = true;
		}
	}

	if (dirty)
	OnClick();

	return ActiveWindow::OnMouseMove(x,y);
}

int Slider::OnLButtonDown(int x, int y)
{
	if (!active)
	return 0;

	if (!captured)
	captured = SetCapture();

	mouse_x = x - rect.x;
	mouse_y = y - rect.y;

	// HORIZONTAL
	if (orientation == 0) {
		if (mouse_x < thumb_pos-thumb_size/2) {
			StepDown();
		}

		else if (mouse_x > thumb_pos+thumb_size/2) {
			StepUp();
		}

		else {
			dragging = true;
		}
	}

	// VERTICAL
	else {
		if (mouse_y < thumb_pos-thumb_size/2) {
			StepUp();
		}

		else if (mouse_y > thumb_pos+thumb_size/2) {
			StepDown();
		}

		else {
			dragging = true;
		}
	}

	if (!dragging)
	OnClick();

	return ActiveWindow::OnLButtonDown(x,y);
}

int Slider::OnLButtonUp(int x, int y)
{
	if (!active)
	return 0;

	if (captured) {
		ReleaseCapture();
		captured = 0;
		dragging = false;
	}

	return ActiveWindow::OnLButtonUp(x,y);
}

int Slider::OnClick()
{
	return ActiveWindow::OnClick();
}
