/*  Project nGenEx
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    nGenEx.lib
	FILE:         Slider.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Slider/Gauge ActiveWindow class
*/

#ifndef Slider_h
#define Slider_h

#include "Types.h"
#include "ActiveWindow.h"

// +--------------------------------------------------------------------+

class Slider : public ActiveWindow
{
public:
	static const char* TYPENAME() { return "Slider"; }

	enum { MAX_VAL=8, ORIENT_HORIZONTAL=0, ORIENT_VERTICAL=1 };

	Slider(ActiveWindow* p, int ax, int ay, int aw, int ah, DWORD aid);
	Slider(Screen* s,       int ax, int ay, int aw, int ah, DWORD aid);
	virtual ~Slider();

	// Operations:
	virtual void      Draw();

	// Event Target Interface:
	virtual int       OnMouseMove(int x, int y);
	virtual int       OnLButtonDown(int x, int y);
	virtual int       OnLButtonUp(int x, int y);
	virtual int       OnClick();

	// Property accessors:
	bool  GetActive();
	void  SetActive(bool bNewValue);
	Color GetFillColor();
	void  SetFillColor(Color c);
	bool  GetBorder();
	void  SetBorder(bool bNewValue);
	Color GetBorderColor();
	void  SetBorderColor(Color c);

	int   GetNumLeds();
	void  SetNumLeds(int nNewValue);
	int   GetOrientation();
	void  SetOrientation(int nNewValue);

	int   GetRangeMin();
	void  SetRangeMin(int nNewValue);
	int   GetRangeMax();
	void  SetRangeMax(int nNewValue);
	int   GetStepSize();
	void  SetStepSize(int nNewValue);
	int   GetThumbSize();
	void  SetThumbSize(int nNewValue);
	bool  GetShowThumb();
	void  SetShowThumb(bool bNewValue);

	int   NumValues();
	int   GetValue(int index=0);
	void  SetValue(int nNewValue, int index=0);
	double FractionalValue(int index=0);

	void  SetMarker(int nNewValue, int index=0);

	// Methods:
	void  StepUp(int index=0);
	void  StepDown(int index=0);

protected:
	int            captured;
	int            dragging;
	int            mouse_x;
	int            mouse_y;

	bool           active;        // true => slider; false => gauge
	bool           border;
	Color          border_color;
	Color          fill_color;    // default: dark blue

	int            num_leds;      // default: 1
	int            orientation;   // 0 => horizontal; !0 => vertical

	int            range_min;
	int            range_max;
	int            step_size;
	int            show_thumb;
	int            thumb_size;
	int            thumb_pos;

	int            nvalues;
	int            value[MAX_VAL];
	int            marker[2];
};

#endif Slider_h

