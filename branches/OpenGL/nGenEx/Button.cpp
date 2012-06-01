/*  Project nGenEx
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    nGenEx.lib
	FILE:         Button.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Button class
*/

#include "MemDebug.h"
#include "Button.h"
#include "Video.h"
#include "Bitmap.h"
#include "Font.h"
#include "Sound.h"
#include "DataLoader.h"

// +--------------------------------------------------------------------+

static Sound* button_sound       = 0;
static Sound* click_sound        = 0;
static Sound* swish_sound        = 0;
static Sound* chirp_sound        = 0;
static Sound* accept_sound       = 0;
static Sound* reject_sound       = 0;
static Sound* confirm_sound      = 0;
static Sound* list_select_sound  = 0;
static Sound* list_scroll_sound  = 0;
static Sound* list_drop_sound    = 0;
static Sound* combo_open_sound   = 0;
static Sound* combo_close_sound  = 0;
static Sound* combo_hilite_sound = 0;
static Sound* combo_select_sound = 0;
static Sound* menu_open_sound    = 0;
static Sound* menu_close_sound   = 0;
static Sound* menu_select_sound  = 0;
static Sound* menu_hilite_sound  = 0;

static int    gui_volume   = 0;

// +--------------------------------------------------------------------+

Button::Button(Screen* s, int ax, int ay, int aw, int ah, DWORD aid)
: ActiveWindow(s, ax, ay, aw, ah, aid)
{
	animated       = true;
	bevel_width    = 5;
	border         = false;
	button_state   = 0;
	drop_shadow    = false;
	sticky         = false;
	picture_loc    = 1;
	captured       = 0;
	pre_state      = 0;
	text_align     = DT_CENTER;

	standard_image    = 0;
	activated_image   = 0;
	transition_image  = 0;

	char buf[32];
	sprintf_s(buf, "Button %d", id);
	desc = buf;
}

Button::Button(ActiveWindow* p, int ax, int ay, int aw, int ah, DWORD aid)
: ActiveWindow(p->GetScreen(), ax, ay, aw, ah, aid, 0, p)
{
	animated       = true;
	bevel_width    = 5;
	border         = false;
	button_state   = 0;
	drop_shadow    = false;
	sticky         = false;
	picture_loc    = 1;
	captured       = 0;
	pre_state      = 0;
	text_align     = DT_CENTER;

	standard_image    = 0;
	activated_image   = 0;
	transition_image  = 0;

	char buf[32];
	sprintf_s(buf, "Button %d", id);
	desc = buf;
}

// +--------------------------------------------------------------------+

Button::~Button()
{
}

// +--------------------------------------------------------------------+

static void LoadInterfaceSound(DataLoader* loader, const char* wave, Sound*& s)
{
	loader->LoadSound(wave, s, 0, true);   // optional sound effect

	if (s)
	s->SetFlags(s->GetFlags() | Sound::INTERFACE);
}

void
Button::Initialize()
{
	DataLoader* loader = DataLoader::GetLoader();
	loader->SetDataPath("Sounds/");

	LoadInterfaceSound(loader, "button.wav",        button_sound);
	LoadInterfaceSound(loader, "click.wav",         click_sound);
	LoadInterfaceSound(loader, "swish.wav",         swish_sound);
	LoadInterfaceSound(loader, "chirp.wav",         chirp_sound);
	LoadInterfaceSound(loader, "accept.wav",        accept_sound);
	LoadInterfaceSound(loader, "reject.wav",        reject_sound);
	LoadInterfaceSound(loader, "confirm.wav",       confirm_sound);
	LoadInterfaceSound(loader, "list_select.wav",   list_select_sound);
	LoadInterfaceSound(loader, "list_scroll.wav",   list_scroll_sound);
	LoadInterfaceSound(loader, "list_drop.wav",     list_drop_sound);
	LoadInterfaceSound(loader, "combo_open.wav",    combo_open_sound);
	LoadInterfaceSound(loader, "combo_close.wav",   combo_close_sound);
	LoadInterfaceSound(loader, "combo_hilite.wav",  combo_hilite_sound);
	LoadInterfaceSound(loader, "combo_select.wav",  combo_select_sound);
	LoadInterfaceSound(loader, "menu_open.wav",     menu_open_sound);
	LoadInterfaceSound(loader, "menu_close.wav",    menu_close_sound);
	LoadInterfaceSound(loader, "menu_select.wav",   menu_select_sound);
	LoadInterfaceSound(loader, "menu_hilite.wav",   menu_hilite_sound);

	loader->SetDataPath(0);
}

// +--------------------------------------------------------------------+

void
Button::Close()
{
	delete button_sound;
	delete click_sound;
	delete swish_sound;
	delete chirp_sound;
	delete accept_sound;
	delete reject_sound;
	delete confirm_sound;
	delete list_select_sound;
	delete list_scroll_sound;
	delete list_drop_sound;
	delete combo_open_sound;
	delete combo_close_sound;
	delete combo_hilite_sound;
	delete combo_select_sound;
	delete menu_open_sound;
	delete menu_close_sound;
	delete menu_select_sound;
	delete menu_hilite_sound;
}

// +--------------------------------------------------------------------+

void
Button::Draw()
{
	if (!IsShown()) return;

	int x = 0;
	int y = 0;
	int w = rect.w;
	int h = rect.h;
	int img_w = picture.Width();
	int img_h = picture.Height();

	float old_alpha = alpha;

	if (!enabled)
	SetAlpha(0.35);

	Rect btn_rect(x,y,w,h);

	if (!transparent) {
		if (standard_image) {
			if (!enabled) {
				texture = standard_image;
			}

			else {
				switch (button_state) {
				case -1:
					texture = activated_image;
					break;

				default:
				case 0:
					texture = standard_image;
					break;

				case 1:
					if (sticky)
					texture = activated_image;
					else
					texture = transition_image;
					break;

				case 2:
					texture = transition_image;
					break;
				}
			}

			if (!texture)
			texture = standard_image;

			DrawTextureGrid();
		}

		else {
			FillRect(0, 0, w, h, ShadeColor(back_color, 1.0));
			DrawStyleRect(0, 0, w, h, style);
		}
	}

	// draw the picture (if any)
	if (picture.Width()) {
		Rect irect = CalcPictureRect();
		DrawImage(&picture, irect);
	}

	// draw text here:
	if (font && text.length()) {
		Rect label_rect = CalcLabelRect(img_w,img_h);
		int  vert_space = label_rect.h;
		int  horz_space = label_rect.w;
		int  align      = DT_WORDBREAK | text_align;

		DrawText(text.data(), 0, label_rect, DT_CALCRECT | align);
		vert_space = (vert_space - label_rect.h)/2;
		
		label_rect.w = horz_space;

		if (vert_space > 0)
		label_rect.y += vert_space;

		if (animated && button_state > 0) {
			label_rect.x += button_state;
			label_rect.y += button_state;
		}

		if (drop_shadow) {
			label_rect.x++;
			label_rect.y++;
			
			font->SetColor(back_color);
			DrawText(text.data(), text.length(), label_rect, align);
			
			label_rect.x--;
			label_rect.y--;
		}
		
		font->SetColor(fore_color);
		DrawText(text.data(), text.length(), label_rect, align);
	}

	if (!enabled)
	SetAlpha(old_alpha);
}

Rect Button::CalcLabelRect(int img_w, int img_h)
{
	// fit the text in the bevel:
	Rect label_rect;
	label_rect.x = 0;
	label_rect.y = 0;
	label_rect.w = rect.w;
	label_rect.h = rect.h;

	if (text_align == DT_LEFT)
	label_rect.Deflate(bevel_width + 8, bevel_width + 1);
	else
	label_rect.Deflate(bevel_width + 1, bevel_width + 1);

	// and around the picture, if any:
	if (img_h != 0)
	{
		switch (picture_loc)
		{
		default:
		case 0:  // the four corner positions
		case 2:  // and the center position
		case 4:  // don't affect the text position
		case 6:
		case 8:
			break;

		case 1:  // north
			label_rect.y += img_h;
			label_rect.h -= img_h;
			break;

		case 3:  // west
			label_rect.x += img_w;
			label_rect.w -= img_w;
			break;

		case 5:  // east
			label_rect.w -= img_w;
			break;

		case 7:  // south
			label_rect.h -= img_h;
			break;
		}
	}

	return label_rect;
}

// +--------------------------------------------------------------------+

Rect
Button::CalcPictureRect()
{
	int w     = rect.w;
	int h     = rect.h;
	int img_w = picture.Width();
	int img_h = picture.Height();
	
	if (img_h > h) img_h = h-2;
	if (img_w > w) img_w = w-2;
	
	int img_x_offset = bevel_width;
	int img_y_offset = bevel_width;
	
	switch (picture_loc)
	{
	default:
		// TOP ROW:
	case  0: break;

	case  1: img_x_offset = (w/2-img_w/2);
		break;

	case  2: img_x_offset = w - img_w - bevel_width;
		break;

		// MIDDLE ROW:
	case  3: img_y_offset = (h/2-img_h/2);
		break;
	case  4: img_x_offset = (w/2-img_w/2);
		img_y_offset = (h/2-img_h/2);
		break;
	case  5: img_x_offset = w - img_w - bevel_width;
		img_y_offset = (h/2-img_h/2);
		break;

		// BOTTOM ROW:
	case  6:
		img_y_offset = h - img_h - bevel_width;
		break;
	case  7: img_x_offset = (w/2-img_w/2);
		img_y_offset = h - img_h - bevel_width;
		break;
	case  8: img_x_offset = w - img_w - bevel_width;
		img_y_offset = h - img_h - bevel_width;
		break;
	}
	
	Rect img_rect;
	img_rect.x = img_x_offset;
	img_rect.y = img_y_offset;

	if (animated && button_state > 0) {
		img_rect.x += button_state;
		img_rect.y += button_state;
	}

	img_rect.w = img_w;
	img_rect.h = img_h;

	return img_rect;
}

// +--------------------------------------------------------------------+

void
Button::DrawImage(Bitmap* bmp, const Rect& irect)
{
	if (bmp) {
		DrawBitmap(irect.x,
		irect.y,
		irect.x + irect.w,
		irect.y + irect.h,
		bmp,
		Video::BLEND_ALPHA);
	}
}

// +--------------------------------------------------------------------+

int Button::OnMouseMove(int x, int y)
{
	bool dirty = false;

	if (captured)
	{
		ActiveWindow* test = GetCapture();

		if (test != this)
		{
			captured = false;
			button_state = pre_state;
			dirty = true;
		}

		else if (sticky)
		{
			if (button_state == 2)
			{
				if (!rect.Contains(x,y))
				{
					button_state = pre_state;
					dirty = true;
				}
			}
			else
			{
				if (rect.Contains(x,y))
				{
					button_state = 2;
					dirty = true;
				}
			}
		}
		else
		{
			if (button_state == 1)
			{
				if (!rect.Contains(x,y))
				{
					button_state = 0;
					dirty = true;
				}
			}
			else
			{
				if (rect.Contains(x,y))
				{
					button_state = 1;
					dirty = true;
				}
			}
		}
	}

	return ActiveWindow::OnMouseMove(x,y);
}

int Button::OnLButtonDown(int x, int y)
{
	if (!captured)
	captured = SetCapture();

	if (sticky)
	button_state = 2;
	else
	button_state = 1;

	return ActiveWindow::OnLButtonDown(x,y);
}

int Button::OnLButtonUp(int x, int y)
{
	if (captured) {
		ReleaseCapture();
		captured = 0;
	}

	button_state = pre_state;
	return ActiveWindow::OnLButtonUp(x,y);
}

void Button::SetVolume(int vol)
{
	if (vol >= -10000 && vol <= 0)
	gui_volume = vol;
}

void Button::PlaySound(int n)
{
	Sound* sound = 0;

	switch (n) {
	default:
	case SND_BUTTON:        if (button_sound)       sound = button_sound->Duplicate();        break;
	case SND_CLICK:         if (click_sound)        sound = click_sound->Duplicate();         break;
	case SND_SWISH:         if (swish_sound)        sound = swish_sound->Duplicate();         break;
	case SND_CHIRP:         if (chirp_sound)        sound = chirp_sound->Duplicate();         break;
	case SND_ACCEPT:        if (accept_sound)       sound = accept_sound->Duplicate();        break;
	case SND_REJECT:        if (reject_sound)       sound = reject_sound->Duplicate();        break;
	case SND_CONFIRM:       if (confirm_sound)      sound = confirm_sound->Duplicate();       break;
	case SND_LIST_SELECT:   if (list_select_sound)  sound = list_select_sound->Duplicate();   break;
	case SND_LIST_SCROLL:   if (list_scroll_sound)  sound = list_scroll_sound->Duplicate();   break;
	case SND_LIST_DROP:     if (list_drop_sound)    sound = list_drop_sound->Duplicate();     break;
	case SND_COMBO_OPEN:    if (combo_open_sound)   sound = combo_open_sound->Duplicate();    break;
	case SND_COMBO_CLOSE:   if (combo_close_sound)  sound = combo_close_sound->Duplicate();   break;
	case SND_COMBO_HILITE:  if (combo_hilite_sound) sound = combo_hilite_sound->Duplicate();  break;
	case SND_COMBO_SELECT:  if (combo_select_sound) sound = combo_select_sound->Duplicate();  break;
	case SND_MENU_OPEN:     if (menu_open_sound)    sound = menu_open_sound->Duplicate();     break;
	case SND_MENU_CLOSE:    if (menu_close_sound)   sound = menu_close_sound->Duplicate();    break;
	case SND_MENU_SELECT:   if (menu_select_sound)  sound = menu_select_sound->Duplicate();   break;
	case SND_MENU_HILITE:   if (menu_hilite_sound)  sound = menu_hilite_sound->Duplicate();   break;
	}

	if (sound) {
		sound->SetVolume(gui_volume);
		sound->Play();
	}
}

int Button::OnClick()
{
	PlaySound(SND_BUTTON);

	if (sticky)
	button_state = !pre_state;

	pre_state = button_state;

	return ActiveWindow::OnClick();
}

int Button::OnMouseEnter(int mx, int my)
{ 
	if (button_state >= 0)
	pre_state = button_state;

	if (button_state == 0)   
	button_state = -1;

	if (IsEnabled() && IsShown())
	PlaySound(SND_SWISH);

	return ActiveWindow::OnMouseEnter(mx, my);
}

int Button::OnMouseExit(int mx, int my)
{ 
	if (button_state == -1)
	button_state = pre_state;

	return ActiveWindow::OnMouseExit(mx, my);
}

// +--------------------------------------------------------------------+

void Button::SetStandardImage(Bitmap* img)
{
	standard_image = img;
	texture        = standard_image;
}

void Button::SetActivatedImage(Bitmap* img)
{
	activated_image = img;
}

void Button::SetTransitionImage(Bitmap* img)
{
	transition_image = img;
}

// +--------------------------------------------------------------------+

short Button::GetBevelWidth()
{
	return bevel_width;
}

void Button::SetBevelWidth(short nNewValue)
{
	if (nNewValue < 0) nNewValue = 0;
	if (nNewValue > rect.w/2) nNewValue = rect.w/2;
	bevel_width = nNewValue;
}

bool  Button::GetBorder()
{
	return border;
}

void  Button::SetBorder(bool bNewValue)
{
	border = bNewValue;
}

Color  Button::GetBorderColor()
{
	return border_color;
}

void  Button::SetBorderColor(Color newValue)
{
	border_color = newValue;
}

Color  Button::GetActiveColor()
{
	return active_color;
}

void  Button::SetActiveColor(Color newValue)
{
	active_color = newValue;
}

bool  Button::GetAnimated()
{
	return animated;
}

void  Button::SetAnimated(bool bNewValue)
{
	animated = bNewValue;
}

bool  Button::GetDropShadow()
{
	return drop_shadow;
}

void  Button::SetDropShadow(bool bNewValue)
{
	drop_shadow = bNewValue;
}

// +--------------------------------------------------------------------+

short Button::GetButtonState()
{
	return button_state;
}

void  Button::SetButtonState(short n)
{
	if (button_state != n && n >= -2 && n <= 2) {
		button_state = n;
		pre_state    = n;
	}
}

void  Button::GetPicture(Bitmap& img)
{
	img.CopyBitmap(picture);
}

void  Button::SetPicture(const Bitmap& img)
{
	picture.CopyBitmap(img);
	picture.AutoMask();
}

short Button::GetPictureLocation()
{
	return picture_loc;
}

void  Button::SetPictureLocation(short n)
{
	if (picture_loc != n && n >= 0 && n <= 8) {
		picture_loc = n;
	}
}

bool  Button::GetSticky()
{
	return sticky;
}

void  Button::SetSticky(bool n)
{
	if (sticky != n)
	sticky = n;
}

