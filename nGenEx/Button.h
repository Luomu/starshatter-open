/*  Project nGenEx
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    nGenEx.lib
	FILE:         Button.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Button class
*/

#ifndef Button_h
#define Button_h

#include "Types.h"
#include "ActiveWindow.h"
#include "Bitmap.h"

// +--------------------------------------------------------------------+

class Button : public ActiveWindow
{
public:
	enum SOUNDS {
		SND_BUTTON,
		SND_CLICK,
		SND_SWISH,
		SND_CHIRP,
		SND_ACCEPT,
		SND_REJECT,
		SND_CONFIRM,
		SND_LIST_SELECT,
		SND_LIST_SCROLL,
		SND_LIST_DROP,
		SND_COMBO_OPEN,
		SND_COMBO_CLOSE,
		SND_COMBO_HILITE,
		SND_COMBO_SELECT,
		SND_MENU_OPEN,
		SND_MENU_CLOSE,
		SND_MENU_SELECT,
		SND_MENU_HILITE
	};

	Button(Screen*       s, int ax, int ay, int aw, int ah, DWORD id=0);
	Button(ActiveWindow* p, int ax, int ay, int aw, int ah, DWORD id=0);
	virtual ~Button();

	static void       Initialize();
	static void       Close();
	static void       PlaySound(int n=0);
	static void       SetVolume(int vol);

	// Operations:
	virtual void      Draw();     // refresh backing store

	// Event Target Interface:
	virtual int       OnMouseMove(int x, int y);
	virtual int       OnLButtonDown(int x, int y);
	virtual int       OnLButtonUp(int x, int y);
	virtual int       OnClick();
	virtual int       OnMouseEnter(int x, int y);
	virtual int       OnMouseExit(int x, int y);

	// Property accessors:
	Color GetActiveColor();
	void  SetActiveColor(Color c);
	bool  GetAnimated();
	void  SetAnimated(bool bNewValue);
	short GetBevelWidth();
	void  SetBevelWidth(short nNewValue);
	bool  GetBorder();
	void  SetBorder(bool bNewValue);
	Color GetBorderColor();
	void  SetBorderColor(Color c);
	short GetButtonState();
	void  SetButtonState(short nNewValue);
	bool  GetDropShadow();
	void  SetDropShadow(bool bNewValue);
	void  GetPicture(Bitmap& img);
	void  SetPicture(const Bitmap& img);
	short GetPictureLocation();
	void  SetPictureLocation(short nNewValue);
	bool  GetSticky();
	void  SetSticky(bool bNewValue);

	void  SetStandardImage(Bitmap* img);
	void  SetActivatedImage(Bitmap* img);
	void  SetTransitionImage(Bitmap* img);

protected:
	Rect  CalcLabelRect(int img_w, int img_h);
	Rect  CalcPictureRect();
	void  DrawImage(Bitmap* bmp, const Rect& irect);

	bool           animated;
	bool           drop_shadow;
	bool           sticky;
	bool           border;

	Color          active_color;
	Color          border_color;

	bool           captured;
	int            pre_state;
	short          bevel_width;
	short          button_state;

	short          picture_loc;
	Bitmap         picture;

	Bitmap*        standard_image;   // state = 0
	Bitmap*        activated_image;  // state = 1 (if sticky)
	Bitmap*        transition_image; // state = 2 (if sticky)
};

#endif Button_h

