/*  Project nGenEx
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    nGenEx.lib
	FILE:         ActiveWindow.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Window class
*/

#include "MemDebug.h"
#include "ActiveWindow.h"
#include "EventDispatch.h"
#include "Color.h"
#include "Bitmap.h"
#include "Font.h"
#include "FontMgr.h"
#include "Layout.h"
#include "Polygon.h"
#include "Screen.h"
#include "View.h"
#include "Video.h"

// +--------------------------------------------------------------------+

Font* ActiveWindow::sys_font = 0;
Color ActiveWindow::sys_back_color = Color(128,128,128);
Color ActiveWindow::sys_fore_color = Color(  0,  0,  0);

void  ActiveWindow::SetSystemFont(Font* f)      { sys_font = f; }
void  ActiveWindow::SetSystemBackColor(Color c) { sys_back_color = c; }
void  ActiveWindow::SetSystemForeColor(Color c) { sys_fore_color = c; }

// +--------------------------------------------------------------------+

ActiveWindow::ActiveWindow(Screen* screen, int ax, int ay, int aw, int ah,
DWORD aid, DWORD s, ActiveWindow* pParent)
: Window(screen, ax, ay, aw, ah), id(aid), style(s), focus(false), enabled(true),
text_align(DT_CENTER), single_line(false), alpha(1),
texture(0), back_color(sys_back_color), fore_color(sys_fore_color),
parent(pParent), form(0), transparent(false), topmost(true),
layout(0), rows(1), cols(1), polys(0), vset(0), mtl(0),
fixed_width(0), fixed_height(0), hide_partial(true)
{
	ZeroMemory(tab, sizeof(tab));

	font = sys_font;

	if (parent) {
		parent->AddChild(this);
	}
	else {
		screen->AddWindow(this);
	}

	shown = false;
	Show();

	char buf[32];
	sprintf_s(buf, "ActiveWindow %d", id);
	desc = buf;
}

// +--------------------------------------------------------------------+

ActiveWindow::~ActiveWindow()
{
	if (layout) delete layout;

	screen->DelWindow(this);
	Hide();
	clients.destroy();
	children.destroy();

	if (polys)  delete [] polys;
	if (vset)   delete    vset;
	if (mtl)    delete    mtl;
}

// +--------------------------------------------------------------------+

void
ActiveWindow::Show()
{
	EventDispatch* dispatch = EventDispatch::GetInstance();
	if (dispatch)
	dispatch->Register(this);

	ListIter<View> v_iter = view_list;
	while (++v_iter) {
		View* view = v_iter.value();
		view->OnShow();
	}

	ListIter<ActiveWindow> c_iter = children;
	while (++c_iter) {
		ActiveWindow* child = c_iter.value();
		child->Show();
	}

	shown = true;
}

// +--------------------------------------------------------------------+

void
ActiveWindow::Hide()
{
	EventDispatch* dispatch = EventDispatch::GetInstance();
	if (dispatch) {
		dispatch->Unregister(this);
		focus = false;
	}

	ListIter<View> v_iter = view_list;
	while (++v_iter) {
		View* view = v_iter.value();
		view->OnHide();
	}

	ListIter<ActiveWindow> c_iter = children;
	while (++c_iter) {
		ActiveWindow* child = c_iter.value();
		child->Hide();
	}

	shown = false;
}

// +--------------------------------------------------------------------+

void
ActiveWindow::MoveTo(const Rect& r)
{
	if (rect.x == r.x &&
			rect.y == r.y &&
			rect.w == r.w &&
			rect.h == r.h)
	return;

	rect = r;
	CalcGrid();

	ListIter<View> v = view_list;
	while (++v)
	v->OnWindowMove();

	if (layout)
	layout->DoLayout(this);
}

// +--------------------------------------------------------------------+

void
ActiveWindow::AddChild(ActiveWindow* child)
{
	if (child)
	children.append(child);
}

// +--------------------------------------------------------------------+

void
ActiveWindow::DoLayout()
{
	if (layout)
	layout->DoLayout(this);
}

// +--------------------------------------------------------------------+

void
ActiveWindow::UseLayout(const ArrayList& min_x,
const ArrayList& min_y,
const FloatList& weight_x,
const FloatList& weight_y)
{
	if (!layout)
	layout = new(__FILE__,__LINE__) Layout;

	if (layout)
	layout->SetConstraints(min_x, min_y, weight_x, weight_y);
}

void
ActiveWindow::UseLayout(const FloatList& min_x,
const FloatList& min_y,
const FloatList& weight_x,
const FloatList& weight_y)
{
	if (!layout)
	layout = new(__FILE__,__LINE__) Layout;

	if (layout)
	layout->SetConstraints(min_x, min_y, weight_x, weight_y);
}

void
ActiveWindow::UseLayout(int      nrows,
int      ncols,
int*     min_x,
int*     min_y,
float*   weight_x,
float*   weight_y)
{
	if (!layout)
	layout = new(__FILE__,__LINE__) Layout;

	if (layout)
	layout->SetConstraints(nrows, ncols, min_x, min_y, weight_x, weight_y);
}

// +--------------------------------------------------------------------+

void
ActiveWindow::Paint()
{
	Draw();
}

// +--------------------------------------------------------------------+

Color
ActiveWindow::ShadeColor(Color c, double shade)
{
	int ishade = (int) (shade * Color::SHADE_LEVELS);
	return c.ShadeColor(ishade);
}

// +--------------------------------------------------------------------+

void
ActiveWindow::Draw()
{
	int w = rect.w;
	int h = rect.h;

	if (w < 1 || h < 1 || !shown)
	return;

	float old_alpha = alpha;

	if (!enabled)
	SetAlpha(0.5);

	if (!transparent) {
		if (texture && texture->Width()) {
			DrawTextureGrid();
		}
		else {
			FillRect(0, 0, w, h, ShadeColor(back_color, 1.0));
		}
	}

	if (enabled && view_list.size()) {
		ListIter<View> v = view_list;
		while (++v)
		v->Refresh();
	}

	if (!transparent) {
		DrawStyleRect(0, 0, w, h, style);
	}

	// draw text here:
	DrawTabbedText();

	if (!enabled)
	SetAlpha(old_alpha);

	// update children windows:
	ListIter<ActiveWindow> iter = children;
	while (++iter) {
		ActiveWindow* child = iter.value();
		child->Draw();
	}
}

void
ActiveWindow::CalcGrid()
{
	if (polys)  delete [] polys;
	if (vset)   delete    vset;
	if (mtl)    delete    mtl;

	rows = 1;
	cols = 1;

	if (!texture || texture->Width() < 1)
	return;

	if (margins.left   > 0)  cols++;
	if (margins.right  > 0)  cols++;
	if (margins.top    > 0)  rows++;
	if (margins.bottom > 0)  rows++;

	int npolys = rows*cols;
	int nverts = (rows+1) * (cols+1);

	if (style & WIN_FRAME_ONLY && npolys == 9)
	npolys = 8; // skip the center poly

	if (npolys > 0) {
		int   i, j;
		int   x_offsets[4];
		int   y_offsets[4];
		float u_offsets[4];
		float v_offsets[4];

		x_offsets[0] = 0;
		x_offsets[1] = margins.left ? margins.left : rect.w - margins.right;
		x_offsets[2] = cols==2     ? rect.w      : rect.w - margins.right;
		x_offsets[3] = rect.w;

		y_offsets[0] = 0;
		y_offsets[1] = margins.top  ? margins.top  : rect.h - margins.bottom;
		y_offsets[2] = rows==2     ? rect.h      : rect.h - margins.bottom;
		y_offsets[3] = rect.h;

		float tex_w = (float) texture->Width();
		float tex_h = (float) texture->Height();

		if (tex_w > rect.w) tex_w = (float) rect.w;
		if (tex_h > rect.h) tex_h = (float) rect.h;

		u_offsets[0] = 0.0f;
		u_offsets[1] = margins.left ? (float) margins.left : tex_w - (float) margins.right;
		u_offsets[2] = cols==2     ? tex_w               : tex_w - (float) margins.right;
		u_offsets[3] = tex_w;

		v_offsets[0] = 0.0f;
		v_offsets[1] = margins.top  ? (float) margins.top  : tex_h - (float) margins.bottom;
		v_offsets[2] = rows==2     ? tex_h               : tex_h - (float) margins.bottom;
		v_offsets[3] = tex_h;

		tex_w = (float) texture->Width();
		tex_h = (float) texture->Height();

		vset = new(__FILE__,__LINE__) VertexSet(nverts);

		int v = 0;

		Color c = Color::White;
		c.SetAlpha((BYTE) (alpha*255));

		vset->space = VertexSet::SCREEN_SPACE;

		for (i = 0; i <= rows; i++) {
			for (j = 0; j <= cols; j++) {
				vset->diffuse[v]   = c.Value();

				vset->s_loc[v].x   = (float) (rect.x + x_offsets[j]) - 0.5f;
				vset->s_loc[v].y   = (float) (rect.y + y_offsets[i]) - 0.5f;
				vset->s_loc[v].z   = 0.0f;
				vset->rw[v]        = 1.0f;

				vset->tu[v]        = u_offsets[j] / tex_w;
				vset->tv[v]        = v_offsets[i] / tex_h;

				v++;
			}
		}

		mtl = new(__FILE__,__LINE__) Material;
		mtl->tex_diffuse = texture;

		polys = new(__FILE__,__LINE__) Poly[npolys];
		
		Poly* p     = polys;

		ZeroMemory(polys, npolys*sizeof(Poly));

		for (i = 0; i < rows; i++) {
			for (j = 0; j < cols; j++) {
				if (style & WIN_FRAME_ONLY) {
					if (i == 1 && j == 1)
					continue;
				}

				p->nverts     = 4;
				p->vertex_set = vset;
				p->material   = mtl;

				p->verts[0]   = (i+0)*(cols+1) + j;
				p->verts[1]   = (i+0)*(cols+1) + j + 1;
				p->verts[2]   = (i+1)*(cols+1) + j + 1;
				p->verts[3]   = (i+1)*(cols+1) + j;

				p++;
			}
		}
	}
}

void
ActiveWindow::DrawTextureGrid()
{
	int npolys = rows*cols;

	if (style & WIN_FRAME_ONLY && npolys == 9)
	npolys = 8; // skip the center poly

	if (mtl) {
		mtl->tex_diffuse = texture;
	}

	int blend = Video::BLEND_SOLID;

	if (alpha < 1)
	blend = Video::BLEND_ALPHA;

	Video* video = screen->GetVideo();
	video->SetRenderState(Video::TEXTURE_WRAP, 0);
	video->DrawScreenPolys(npolys, polys, blend);
	video->SetRenderState(Video::TEXTURE_WRAP, 1);
}

void
ActiveWindow::DrawStyleRect(const Rect& r, int style)
{
	DrawStyleRect(r.x, r.y, r.x+r.w, r.y+r.h, style);
}

void
ActiveWindow::DrawStyleRect(int x1, int y1, int x2, int y2, int style)
{
	if (style & WIN_THIN_FRAME) {
		DrawRect(x1,y1,x2-1,y2-1, ShadeColor(fore_color, 1.0));
	}
	else if (style & WIN_THICK_FRAME) {
		DrawRect(x1+0,y1+0,x2-1,y2-1, ShadeColor(fore_color, 1.0));
		DrawRect(x1+1,y1+1,x2-2,y2-2, ShadeColor(fore_color, 1.0));
		DrawRect(x1+2,y1+2,x2-3,y2-3, ShadeColor(fore_color, 1.0));
	}
	else {
		// draw bevel:
		if ((style & WIN_RAISED_FRAME) && (style & WIN_SUNK_FRAME)) {
			Color c = ShadeColor(back_color, 1.6);    // full highlight
			DrawLine(x1,  y1,   x2-1,  y1,   c);
			DrawLine(x1,  y1,   x1,  y2-1,   c);

			c = ShadeColor(back_color, 1.3);          // soft highlight
			DrawLine(x1+1,y1+1, x2-2, y1+1, c);
			DrawLine(x1+1,y1+1, x1+1, y2-2, c);

			c = ShadeColor(back_color, 0.6);          // soft shadow
			DrawLine(x2-2,y1+1, x2-2,y2-1, c);
			DrawLine(x1+1,y2-2, x2-1,y2-2, c);

			c = ShadeColor(back_color, 0.3);          // full shadow
			DrawLine(x2-1,y1,   x2-1,y2, c);
			DrawLine(x1  ,y2-1, x2,y2-1, c);

			DrawRect(x1+4,y1+4, x2-5,y2-5, ShadeColor(back_color, 0.6));   // soft shadow
			DrawRect(x1+5,y1+5, x2-6,y2-6, ShadeColor(back_color, 0.3));   // full shadow
			DrawLine(x1+5,y2-6, x2-5,y2-6, ShadeColor(back_color, 1.3));   // soft highlight (bottom)
			DrawLine(x2-6,y1+5, x2-6,y2-6, ShadeColor(back_color, 1.3));   // soft highlight (side)
			DrawLine(x1+4,y2-5, x2-4,y2-5, ShadeColor(back_color, 1.6));   // soft highlight (bottom)
			DrawLine(x2-5,y1+4, x2-5,y2-5, ShadeColor(back_color, 1.6));   // soft highlight (side)
		}

		else if (style & WIN_RAISED_FRAME) {
			Color c = ShadeColor(back_color, 1.6);    // full highlight
			DrawLine(x1,  y1,   x2-1,  y1,   c);
			DrawLine(x1,  y1,   x1,  y2-1,   c);

			c = ShadeColor(back_color, 1.3);          // soft highlight
			DrawLine(x1+1,y1+1, x2-2, y1+1, c);
			DrawLine(x1+1,y1+1, x1+1, y2-2, c);

			c = ShadeColor(back_color, 0.6);          // soft shadow
			DrawLine(x2-2,y1+1, x2-2,y2-1, c);
			DrawLine(x1+1,y2-2, x2-1,y2-2, c);

			c = ShadeColor(back_color, 0.3);          // full shadow
			DrawLine(x2-1,y1, x2-1,y2, c);
			DrawLine(x1  ,y2-1, x2,y2-1, c);
		}

		else if (style & WIN_SUNK_FRAME) {
			Color c = ShadeColor(back_color, 0.3);    // full shadow
			DrawLine(x1+1,y1+1, x1+1,  y2,   c);
			DrawLine(x1+1,y1+1, x2,  y1+1,   c);
			
			c = ShadeColor(back_color, 0.6);          // soft shadow
			DrawLine(x1,  y1,   x1,  y2,   c);
			DrawLine(x1,  y1,   x2,  y1,   c);

			c = ShadeColor(back_color, 1.3);          // soft highlight
			DrawLine(x2-2,y1+1, x2-2,y2-1, c);
			DrawLine(x1+1,y2-2, x2-1,y2-2, c);

			c = ShadeColor(back_color, 1.6);          // full highlight
			DrawLine(x2-1,y1+1, x2-1,y2, c);
			DrawLine(x1  ,y2-1, x2,y2-1, c);
		}

		// draw frame:
		if (style & WIN_BLACK_FRAME)
		DrawRect(x1,y1,x2-1,y2-1, ShadeColor(Color::Black, 1.0));
		else if (style & WIN_WHITE_FRAME)
		DrawRect(x1,y1,x2-1,y2-1, ShadeColor(Color::White, 1.0));
	}
}

void
ActiveWindow::DrawTabbedText()
{
	if (shown && font && text.length()) {
		Rect label_rect;

		if (text_insets.left) {
			label_rect.w = rect.w;
			label_rect.h = rect.h;

			label_rect.Inset(text_insets.left,
			text_insets.right,
			text_insets.top,
			text_insets.bottom);
		}
		else {
			int border_size = 4;

			if (style & WIN_RAISED_FRAME && style & WIN_SUNK_FRAME)
			border_size = 8;

			label_rect.x = border_size;
			label_rect.y = border_size;
			label_rect.w = rect.w - border_size * 2;
			label_rect.h = rect.h - border_size * 2;
		}

		font->SetAlpha(alpha);

		// no tabs set:
		if (tab[0] == 0) {
			DWORD text_flags = DT_WORDBREAK | text_align;

			if (single_line)
			text_flags = text_flags | DT_SINGLELINE;

			if (style & WIN_TEXT_SHADOW) {
				label_rect.x++;
				label_rect.y++;

				if (transparent) {
					font->SetColor(back_color);
					DrawText(text.data(), 0, label_rect, text_flags);
				}

				else {
					Color shadow = ShadeColor(back_color, 1.6);
					font->SetColor(shadow);
					DrawText(text.data(), 0, label_rect, text_flags);
				}
				
				label_rect.x--;
				label_rect.y--;
			}
			
			Color fore = ShadeColor(fore_color, 1);
			font->SetColor(fore);
			DrawText(text.data(), 0, label_rect, text_flags);
		}

		// use tabs:
		else {
		}

		font->SetAlpha(1);
	}
}

// +--------------------------------------------------------------------+

void
ActiveWindow::SetTabStop(int n, int x)
{
	if (n >= 0 && n < 10)
	tab[n] = x;
}

int 
ActiveWindow::GetTabStop(int n) const
{
	if (n >= 0 && n < 10)
	return tab[n];

	return 0;
}

// +--------------------------------------------------------------------+

void
ActiveWindow::SetText(const char* t)
{
	if (t && text != t) {
		int len = strlen(t);

		if (len > 0) {
			char* buf = new(__FILE__,__LINE__) char[2*len];

			if (buf != 0) {
				const char* src = t;
				char* dst = buf;

				while (*src) {
					if (*src != '\\') {
						*dst++ = *src++;
					}
					else {
						src++;

						switch (*src) {
						case 'n': *dst++ = '\n';   break;
						case 't': *dst++ = '\t';   break;
						default:  *dst++ = *src;   break;
						}

						src++;
					}
				}

				*dst = 0;

				if (text != buf) {
					text = buf;
				}

				delete [] buf;
			}
		}
		else {
			text = t;
		}
	}
}

void
ActiveWindow::SetText(const Text& t)
{
	if (t && text != t) {
		int len = t.length();

		if (len > 0 && t.contains('\\')) {
			char* buf = new(__FILE__,__LINE__) char[2*len];

			if (buf != 0) {
				const char* src = t;
				char* dst = buf;

				while (*src) {
					if (*src != '\\') {
						*dst++ = *src++;
					}
					else {
						src++;

						switch (*src) {
						case 'n': *dst++ = '\n';   break;
						case 't': *dst++ = '\t';   break;
						default:  *dst++ = *src;   break;
						}

						src++;
					}
				}

				*dst = 0;

				if (text != buf) {
					text = buf;
				}

				delete [] buf;
			}
		}
		else {
			text = t;
		}
	}
}

// +--------------------------------------------------------------------+

void
ActiveWindow::AddText(const char* t)
{
	if (t) {
		text += t;
	}
}

void
ActiveWindow::AddText(const Text& t)
{
	if (t) {
		text += t;
	}
}

void
ActiveWindow::SetTextAlign(DWORD a)
{
	if (a == DT_LEFT || a == DT_RIGHT || a == DT_CENTER)
	text_align = a;
}

void
ActiveWindow::SetMargins(const Insets& m)
{
	margins = m;
	CalcGrid();
}

void
ActiveWindow::SetTextInsets(const Insets& t)
{
	text_insets = t;
}

void
ActiveWindow::SetCellInsets(const Insets& c)
{
	cell_insets = c;
}

void
ActiveWindow::SetCells(int cx, int cy, int cw, int ch)
{
	cells.x  = cx;
	cells.y  = cy;
	cells.w  = cw;
	cells.h  = ch;

	if (cells.w < 1)
	cells.w = 1;

	if (cells.h < 1)
	cells.h = 1;
}

void
ActiveWindow::SetAlpha(double a)
{
	if (alpha != a) {
		alpha = (float) a;

		Color c = Color::White;
		c.SetAlpha((BYTE) (alpha*255));

		if (vset && vset->nverts) {
			for (int i = 0; i < vset->nverts; i++) {
				vset->diffuse[i] = c.Value();
			}
		}
	}
}

// +--------------------------------------------------------------------+

void
ActiveWindow::DrawText(const char* txt, int count, Rect& txt_rect, DWORD flags)
{
	if (font) {
		font->SetAlpha(alpha);
		Window::DrawText(txt, count, txt_rect, flags);
	}
}

// +--------------------------------------------------------------------+

void
ActiveWindow::RegisterClient(int eid, ActiveWindow* client, PFVAWE callback)
{
	AWMap* map = new(__FILE__,__LINE__) AWMap(eid, client, callback);

	if (map != 0)
	clients.append(map);
}

void
ActiveWindow::UnregisterClient(int eid, ActiveWindow* client)
{
	AWMap test(eid, client, 0);
	int   index = clients.index(&test);

	if (index >= 0)
	delete clients.removeIndex(index);
}

void
ActiveWindow::ClientEvent(int eid, int x, int y)
{
	event.window = this;
	event.eid    = eid;
	event.x      = x;
	event.y      = y;

	ListIter<AWMap> map = clients;
	while (++map) {
		if (map->eid == eid)
		map->func(map->client, &event);
	}
}   

// +--------------------------------------------------------------------+

int ActiveWindow::OnMouseEnter(int x, int y)
{
	ClientEvent(EID_MOUSE_ENTER, x, y);
	return 0;
}

int ActiveWindow::OnMouseExit(int x, int y)
{
	ClientEvent(EID_MOUSE_EXIT, x, y);
	return 0;
}

int ActiveWindow::OnMouseMove(int x, int y)
{
	ClientEvent(EID_MOUSE_MOVE, x, y);
	return 0;
}

int ActiveWindow::OnMouseWheel(int wheel)
{
	ClientEvent(EID_MOUSE_WHEEL, wheel, 0);
	return 0;
}

int ActiveWindow::OnLButtonDown(int x, int y)
{
	ClientEvent(EID_LBUTTON_DOWN, x, y);
	return 0;
}

int ActiveWindow::OnLButtonUp(int x, int y)
{
	ClientEvent(EID_LBUTTON_UP, x, y);
	return 0;
}

int ActiveWindow::OnClick()
{
	ClientEvent(EID_CLICK);
	return 0;
}

int ActiveWindow::OnSelect()
{
	ClientEvent(EID_SELECT);
	return 0;
}

int ActiveWindow::OnRButtonDown(int x, int y)
{
	ClientEvent(EID_RBUTTON_DOWN, x, y);
	return 0;
}

int ActiveWindow::OnRButtonUp(int x, int y)
{
	ClientEvent(EID_RBUTTON_UP, x, y);
	return 0;
}

int ActiveWindow::OnKeyDown(int vk, int flags)
{
	ClientEvent(EID_KEY_DOWN, vk, flags);
	return 0;
}

int ActiveWindow::OnDragStart(int x, int y)
{
	ClientEvent(EID_DRAG_START, x, y);
	return 0;
}

int ActiveWindow::OnDragDrop(int x, int y, ActiveWindow* source)
{
	ClientEvent(EID_DRAG_DROP, x, y);
	return 0;
}

Rect ActiveWindow::TargetRect() const
{
	return rect;
}

// +--------------------------------------------------------------------+

void ActiveWindow::SetFocus()
{
	EventDispatch* dispatch = EventDispatch::GetInstance();
	if (dispatch)
	dispatch->SetFocus(this);

	focus = true;
	ClientEvent(EID_SET_FOCUS);
}

void ActiveWindow::KillFocus()
{
	EventDispatch* dispatch = EventDispatch::GetInstance();
	if (dispatch)
	dispatch->KillFocus(this);

	focus = false;
	ClientEvent(EID_KILL_FOCUS);
}

// +--------------------------------------------------------------------+

ActiveWindow*
ActiveWindow::GetCapture()
{
	EventDispatch* dispatch = EventDispatch::GetInstance();
	if (dispatch)
	return (ActiveWindow*) dispatch->GetCapture();

	return 0;
}

// +--------------------------------------------------------------------+

int
ActiveWindow::SetCapture()
{
	EventDispatch* dispatch = EventDispatch::GetInstance();
	if (dispatch)
	return dispatch->CaptureMouse(this);

	return 0;
}

// +--------------------------------------------------------------------+

int
ActiveWindow::ReleaseCapture()
{
	EventDispatch* dispatch = EventDispatch::GetInstance();
	if (dispatch)
	return dispatch->ReleaseMouse(this);

	return 0;
}

// +--------------------------------------------------------------------+

bool
ActiveWindow::IsFormActive() const
{
	if (form)
	return form->IsTopMost();

	return true;
}

// +--------------------------------------------------------------------+

ActiveWindow*
ActiveWindow::FindChild(DWORD id)
{
	ListIter<ActiveWindow> iter = children;
	while (++iter) {
		ActiveWindow* w = iter.value();
		if (w->GetID() == id)
		return w;

		ActiveWindow* w2 = w->FindChild(id);

		if (w2)
		return w2;
	}

	return 0;
}


// +--------------------------------------------------------------------+

ActiveWindow*
ActiveWindow::FindChild(int x, int y)
{
	ActiveWindow* mouse_tgt = 0;

	ListIter<ActiveWindow> iter = children;
	while (++iter) {
		ActiveWindow* test = iter.value();
		if (test->TargetRect().Contains(x,y))
		mouse_tgt = test;
	}

	return mouse_tgt;
}
