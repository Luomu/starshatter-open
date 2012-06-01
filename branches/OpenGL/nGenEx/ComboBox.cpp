/*  Project nGenEx
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    nGenEx.lib
	FILE:         ComboBox.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Drop-down list of Buttons class
*/

#include "MemDebug.h"
#include "ComboBox.h"
#include "ComboList.h"
#include "Button.h"
#include "Video.h"
#include "Screen.h"
#include "Bitmap.h"
#include "Font.h"
#include "EventDispatch.h"

// +--------------------------------------------------------------------+
// DECLARE MAPPING FUNCTIONS:
DEF_MAP_CLIENT(ComboBox, OnListSelect);
DEF_MAP_CLIENT(ComboBox, OnListExit);

// +--------------------------------------------------------------------+

ComboBox::ComboBox(ActiveWindow* p, int ax, int ay, int aw, int ah, DWORD aid)
: ActiveWindow(p->GetScreen(), ax, ay, aw, ah, aid, 0, p)
{
	button_state   = 0;
	captured       = 0;
	pre_state      = 0;
	seln           = 0;
	list           = 0;
	list_showing   = false;
	border         = true;
	animated       = true;
	bevel_width    = 5;

	char buf[32];
	sprintf_s(buf, "ComboBox %d", id);
	desc = buf;
}

ComboBox::ComboBox(Screen* s, int ax, int ay, int aw, int ah, DWORD aid)
: ActiveWindow(s, ax, ay, aw, ah, aid)
{
	button_state   = 0;
	captured       = 0;
	pre_state      = 0;
	seln           = 0;
	list           = 0;
	list_showing   = false;
	border         = true;
	animated       = true;
	bevel_width    = 5;

	char buf[32];
	sprintf_s(buf, "ComboBox %d", id);
	desc = buf;
}

// +--------------------------------------------------------------------+

ComboBox::~ComboBox()
{
	items.destroy();

	if (list && !list_showing)
	delete list;
}

// +--------------------------------------------------------------------+

void ComboBox::SetBevelWidth(short nNewValue)
{
	if (nNewValue < 0) nNewValue = 0;
	bevel_width = nNewValue;
}

void  ComboBox::SetBorder(bool bNewValue)
{
	border = bNewValue;
}

void  ComboBox::SetBorderColor(Color newValue)
{
	border_color = newValue;
}

void  ComboBox::SetActiveColor(Color newValue)
{
	active_color = newValue;
}

void  ComboBox::SetAnimated(bool bNewValue)
{
	animated = bNewValue;
}

// +--------------------------------------------------------------------+

void
ComboBox::Draw()
{
	int x = 0;
	int y = 0;
	int w = rect.w;
	int h = rect.h;

	if (w < 1 || h < 1 || !shown)
	return;

	Rect btn_rect(x,y,w,h);

	// draw the bevel:
	DrawRectSimple(btn_rect, button_state);

	// draw the border:
	if (border)
	DrawRect(0,0,w-1,h-1,border_color);

	// draw the arrow:
	POINT arrow[3];
	double h3 = (double)h/3.0;

	arrow[0].x = (int) (w-2*h3);
	arrow[0].y = (int) (h3);
	arrow[1].x = (int) (w-h3);
	arrow[1].y = (int) (h3);
	arrow[2].x = (int) ((arrow[1].x + arrow[0].x)/2);
	arrow[2].y = (int) (2*h3);

	FillPoly(3, arrow, border_color);

	// draw text here:
	Text caption = text;
	if (GetSelectedIndex() >= 0)
	caption = GetSelectedItem();

	if (font && caption.length()) {
		int border_size = 4;

		if (style & WIN_RAISED_FRAME && style & WIN_SUNK_FRAME)
		border_size = 8;

		Rect label_rect = CalcLabelRect();
		int  vert_space = label_rect.h;
		int  horz_space = label_rect.w;

		DrawText(caption.data(), 0, label_rect, DT_CALCRECT | DT_WORDBREAK | text_align);
		vert_space = (vert_space - label_rect.h)/2;
		
		label_rect.w = horz_space;

		if (vert_space > 0)
		label_rect.y += vert_space;

		if (animated && button_state > 0) {
			label_rect.x += button_state;
			label_rect.y += button_state;
		}

		font->SetColor(fore_color);
		DrawText(caption.data(), 0, label_rect, DT_WORDBREAK | text_align);
	}
}

Rect ComboBox::CalcLabelRect()
{
	// fit the text in the bevel:
	Rect label_rect;
	label_rect.x = 0;
	label_rect.y = 0;
	label_rect.w = rect.w;
	label_rect.h = rect.h;

	label_rect.Deflate(bevel_width, bevel_width);

	return label_rect;
}

// +--------------------------------------------------------------------+

void
ComboBox::DrawRectSimple(Rect& rect, int state)
{
	if (state && active_color != Color::Black)
	FillRect(rect, active_color);
	else
	FillRect(rect, back_color);
}

// +--------------------------------------------------------------------+

int ComboBox::OnMouseMove(int x, int y)
{
	bool dirty = false;

	if (captured)
	{
		ActiveWindow* test = GetCapture();

		if (test != this)
		{
			captured = false;
			button_state = 0;
			dirty = true;
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

int ComboBox::OnLButtonDown(int x, int y)
{
	if (!captured)
	captured = SetCapture();

	button_state = 1;

	return ActiveWindow::OnLButtonDown(x,y);
}

int ComboBox::OnLButtonUp(int x, int y)
{
	if (captured) {
		ReleaseCapture();
		captured = 0;
	}

	button_state = -1;
	ShowList();
	Button::PlaySound(Button::SND_COMBO_OPEN);
	return ActiveWindow::OnLButtonUp(x,y);
}

int ComboBox::OnClick()
{
	return ActiveWindow::OnClick();
}

int ComboBox::OnMouseEnter(int mx, int my)
{ 
	if (button_state == 0)
	button_state = -1;

	return ActiveWindow::OnMouseEnter(mx, my);
}

int ComboBox::OnMouseExit(int mx, int my)
{
	if (button_state == -1)
	button_state = 0;

	return ActiveWindow::OnMouseExit(mx, my);
}

// +--------------------------------------------------------------------+

void ComboBox::MoveTo(const Rect& r)
{
	ActiveWindow::MoveTo(r);

	if (list) {
		delete list;
		list = 0;
	}
}

void ComboBox::ShowList()
{
	if (!list) {
		list = new(__FILE__,__LINE__) ComboList(this, screen, 
		rect.x, rect.y,
		rect.w, rect.h,
		items.size());

	}

	if (list) {
		list->SetTextAlign(text_align);
		list->SetFont(font);
		list->SetText(text);
		list->SetSelection(seln);
		list->SetItems(items);

		list->Show();
		list_showing = true;

		EventDispatch* dispatch = EventDispatch::GetInstance();
		if (dispatch) {
			dispatch->MouseEnter(list);
			dispatch->SetFocus(list);
		}

		REGISTER_CLIENT(EID_CLICK,      list, ComboBox, OnListSelect);
		REGISTER_CLIENT(EID_MOUSE_EXIT, list, ComboBox, OnListExit);
	}
}

void ComboBox::HideList()
{
	if (list) {
		// These will be handled by the list window itself (i hope)
		UNREGISTER_CLIENT(EID_CLICK,      list, ComboBox);
		UNREGISTER_CLIENT(EID_MOUSE_EXIT, list, ComboBox);

		list->Hide();
		list_showing = false;
	}
}

// +--------------------------------------------------------------------+

void ComboBox::OnListSelect(AWEvent* event)
{
	if (list) {
		int new_seln = list->GetSelectedIndex();
		
		if (new_seln >= 0 && new_seln < items.size())
		seln = new_seln;

		HideList();
		OnSelect();
		Button::PlaySound(Button::SND_COMBO_SELECT);
	}
}

void ComboBox::OnListExit(AWEvent* event)
{
	//HideList();
	//Button::PlaySound(Button::SND_COMBO_CLOSE);
}

// +--------------------------------------------------------------------+

int ComboBox::NumItems()
{
	return items.size();
}

void ComboBox::ClearItems()
{
	items.destroy();
	seln = 0;
}

void ComboBox::AddItem(const char* item)
{
	Text* t = new(__FILE__,__LINE__) Text(item);

	if (t) items.append(t);
}

const char*  ComboBox::GetItem(int index)
{
	if (index >= 0 && index < items.size())
	return items[index]->data();
	else
	return 0;
}

void ComboBox::SetItem(int index, const char* item)
{
	if (index >= 0 && index < items.size()) {
		*items[index] = item;
	}

	else {
		Text* t = new(__FILE__,__LINE__) Text(item);
		if (t)
		items.append(t);
	}
}

void ComboBox::SetLabel(const char* label)
{
	SetText(label);
}

int  ComboBox::GetCount()
{
	return items.size();
}

const char*  ComboBox::GetSelectedItem()
{
	if (seln >= 0 && seln < items.size())
	return items[seln]->data();
	else
	return 0;
}

int  ComboBox::GetSelectedIndex()
{
	if (seln >= 0 && seln < items.size())
	return seln;
	else
	return -1;
}

void ComboBox::SetSelection(int index)
{
	if (seln != index && index >= 0 && index < items.size()) {
		seln = index;
	}
}

