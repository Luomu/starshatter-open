/*  Project nGenEx
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    nGenEx.lib
	FILE:         ComboList.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Drop-down list of Buttons class
*/

#include "MemDebug.h"
#include "ComboList.h"
#include "ComboBox.h"
#include "Button.h"
#include "Video.h"
#include "Bitmap.h"
#include "Font.h"
#include "Screen.h"

// +--------------------------------------------------------------------+

ComboList::ComboList(ComboBox* ctrl, ActiveWindow* p, int ax, int ay, int aw, int ah, int maxentries)
: ScrollWindow(p, ax, ay, aw, ah + (ah-2)*maxentries + 2, 0, 0, p), combo_box(ctrl)
{
	button_state   = 0;
	button_height  = ah;
	captured       = 0;
	seln           = -1;
	max_entries    = maxentries;
	scroll         = 0;
	scrolling      = false;
	border         = true;
	animated       = true;
	bevel_width    = 5;

	if (ctrl)
	CopyStyle(*ctrl);

	rect.h = (ah-2)*maxentries + 2;

	int screen_height = Video::GetInstance()->Height();

	if (rect.h > screen_height) {
		rect.y = 0;
		rect.h = screen_height;
	}

	else if (rect.y + rect.h > screen_height) {
		rect.y = screen_height - rect.h;
	}
}

ComboList::ComboList(ComboBox* ctrl, Screen* s, int ax, int ay, int aw, int ah, int maxentries)
: ScrollWindow(s, ax, ay, aw, ah + (ah-2)*maxentries + 2, 0), combo_box(ctrl)
{
	button_state   = 0;
	button_height  = ah;
	captured       = 0;
	seln           = -1;
	max_entries    = maxentries;
	scroll         = 0;
	scrolling      = false;
	border         = true;
	animated       = true;
	bevel_width    = 5;

	if (ctrl)
	CopyStyle(*ctrl);

	rect.h = (ah-2)*maxentries + 2;

	int screen_height = Video::GetInstance()->Height();

	if (rect.h > screen_height) {
		rect.y = 0;
		rect.h = screen_height;
	}

	else if (rect.y + rect.h > screen_height) {
		rect.y = screen_height - rect.h;
	}

	screen->AddWindow(this);   // just in case the base ctor couldn't do this
}

// +--------------------------------------------------------------------+

ComboList::~ComboList()
{
	items.destroy();
}

// +--------------------------------------------------------------------+

void
ComboList::CopyStyle(const ComboBox& ctrl)
{
	active_color   = ctrl.GetActiveColor();
	border_color   = ctrl.GetBorderColor();
	fore_color     = ctrl.GetForeColor();
	back_color     = ctrl.GetBackColor();

	animated       = ctrl.GetAnimated();
	bevel_width    = ctrl.GetBevelWidth();
	border         = ctrl.GetBorder();
}

// +--------------------------------------------------------------------+

void
ComboList::Show()
{
	if (!scrolling) {
		scrolling = 1;
		scroll    = 0;
	}

	ScrollWindow::Show();
}

void
ComboList::Hide()
{
	scrolling = 0;
	scroll    = 0;
	ScrollWindow::Hide();
}

// +--------------------------------------------------------------------+

void
ComboList::Draw()
{
	int x = 0;
	int y = 0;
	int w = rect.w;
	int h = button_height;

	int index   = 0;

	// opening:
	if (scrolling > 0) {
		int limit = min(items.size(), max_entries);
		
		if (scroll < limit) {
			if (limit > 6)
			scroll += 4;

			else if (limit > 4)
			scroll += 2;

			else
			scroll += 1;
		}

		if (scroll >= limit) {
			scroll    = limit;
			scrolling = 0;
		}
	}

	// closing:
	else if (scrolling < 0) {
		if (scroll > 0)
		scroll--;
		else
		scrolling = 0;
	}

	if (items.size() < 1) {
		Rect btn_rect(x,y,w,h);
		DrawItem(" ", btn_rect, 0);
	}
	else {
		ListIter<Text> item = items;
		while (++item && index < max_entries) {
			Rect btn_rect(x,y,w,h);
			int sub_state = (index == seln) ? button_state : 0;

			DrawItem(*item, btn_rect, sub_state);

			y += button_height-2;
			index++;
		}
	}

	DrawRect(0,0,rect.w-1,rect.h-1,fore_color);
}

// +--------------------------------------------------------------------+

void
ComboList::DrawRectSimple(Rect& rect, int state)
{
	if (state && active_color != Color::Black)
	FillRect(rect, active_color);
	else
	FillRect(rect, back_color);
}

void
ComboList::DrawItem(Text label, Rect& btn_rect, int state)
{
	int x = btn_rect.x;
	int y = btn_rect.y;
	int w = btn_rect.w;
	int h = btn_rect.h;

	// draw the bevel:
	DrawRectSimple(btn_rect, state);

	// draw text here:
	if (font && label.length()) {
		int border_size = 4;

		if (style & WIN_RAISED_FRAME && style & WIN_SUNK_FRAME)
		border_size = 8;

		Rect label_rect = CalcLabelRect(btn_rect);
		int  vert_space = label_rect.h;
		int  horz_space = label_rect.w;

		DrawText(label.data(), 0, label_rect, DT_CALCRECT | DT_WORDBREAK | text_align);
		vert_space = (vert_space - label_rect.h)/2;
		
		label_rect.w = horz_space;

		if (vert_space > 0)
		label_rect.y += vert_space;

		if (animated && state > 0) {
			label_rect.x += state;
			label_rect.y += state;
		}

		font->SetColor(fore_color);
		DrawText(label.data(), 0, label_rect, DT_WORDBREAK | text_align);
	}
}

Rect ComboList::CalcLabelRect(const Rect& btn_rect)
{
	// fit the text in the bevel:
	Rect label_rect = btn_rect;
	label_rect.Deflate(bevel_width, bevel_width);

	return label_rect;
}

int ComboList::CalcSeln(int x, int y)
{
	y -= rect.y;

	if (button_height < 1)
	return 0;

	return (int) (y / (button_height-2));
}

// +--------------------------------------------------------------------+

int ComboList::OnMouseMove(int x, int y)
{
	if (captured)
	{
		ActiveWindow* test = GetCapture();

		if (test != this)
		{
			captured = false;
			button_state = -1;
		}
	}

	int new_seln = CalcSeln(x,y);

	if (new_seln != seln) {
		seln = new_seln;   
		Button::PlaySound(Button::SND_COMBO_HILITE);
	}

	return ActiveWindow::OnMouseMove(x,y);
}

int ComboList::OnLButtonDown(int x, int y)
{
	if (!captured)
	captured = SetCapture();

	seln = CalcSeln(x,y);
	button_state = 1;

	return ActiveWindow::OnLButtonDown(x,y);
}

int ComboList::OnLButtonUp(int x, int y)
{
	if (captured) {
		ReleaseCapture();
		captured = 0;
	}

	seln = CalcSeln(x,y);
	button_state = -1;
	return ActiveWindow::OnLButtonUp(x,y);
}

int ComboList::OnClick()
{
	return ActiveWindow::OnClick();
}

int ComboList::OnMouseEnter(int mx, int my)
{ 
	if (button_state == 0)
	button_state = -1;

	return ActiveWindow::OnMouseEnter(mx, my);
}

int ComboList::OnMouseExit(int mx, int my)
{ 
	if (button_state == -1)
	button_state = 0;

	return ActiveWindow::OnMouseExit(mx, my);
}

void ComboList::KillFocus()
{
	if (combo_box)
	combo_box->HideList();
}

// +--------------------------------------------------------------------+

void ComboList::ClearItems()
{
	items.destroy();
}

// +--------------------------------------------------------------------+

void ComboList::AddItem(const char* item)
{
	items.append(new(__FILE__,__LINE__) Text(item));
}

void ComboList::AddItems(ListIter<Text> item)
{
	while (++item)
	items.append(new(__FILE__,__LINE__) Text(*item));
}

void ComboList::SetItems(ListIter<Text> item)
{
	items.destroy();
	while (++item)
	items.append(new(__FILE__,__LINE__) Text(*item));

	// Resize window:

	int ah      = button_height;
	max_entries = items.size();
	Rect r      = rect;
	r.y    = combo_box->Y();
	int length  = max_entries;

	if (length < 1)
	length = 1;

	r.h = (ah-2)*length + 2;

	int screen_height = Video::GetInstance()->Height();

	if (r.h > screen_height) {
		r.y = 0;
		r.h = screen_height;
	}

	else if (r.y + r.h > screen_height) {
		r.y = screen_height - r.h;
	}

	MoveTo(r);
}

const char*  ComboList::GetItem(int index)
{
	if (index >= 0 && index < items.size())
	return items[index]->data();
	else
	return 0;
}

void ComboList::SetItem(int index, const char* item)
{
	if (index >= 0 && index < items.size())
	*items[index] = item;
	else
	items.append(new(__FILE__,__LINE__) Text(item));
}

int  ComboList::GetCount()
{
	return items.size();
}

const char*  ComboList::GetSelectedItem()
{
	if (seln >= 0 && seln < items.size())
	return items[seln]->data();
	else
	return 0;
}

int  ComboList::GetSelectedIndex()
{
	if (seln >= 0 && seln < items.size())
	return seln;
	else
	return -1;
}

void ComboList::SetSelection(int index)
{
	if (index >= 0 && index < items.size())
	seln = index;
}

