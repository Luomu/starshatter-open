/*  Project nGenEx
	Destroyer Studios LLC
	Copyright (C) 1997-2004. All Rights Reserved.

	SUBSYSTEM:    nGenEx.lib
	FILE:         ActiveWindow.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	Active Window class (a window that knows how to draw itself)
*/

#ifndef ActiveWindow_h
#define ActiveWindow_h

#include <vector>
#include "Types.h"
#include "Color.h"
#include "Geometry.h"
#include "Bitmap.h"
#include "Window.h"
#include "EventTarget.h"
#include "List.h"
#include "Text.h"

// +--------------------------------------------------------------------+

struct Poly;
struct Material;
struct VertexSet;
class  Layout;

// +--------------------------------------------------------------------+

enum {
	WIN_NO_FRAME      = 0x0000,
	WIN_BLACK_FRAME   = 0x0001,
	WIN_WHITE_FRAME   = 0x0002,
	WIN_THIN_FRAME    = 0x0004,
	WIN_THICK_FRAME   = 0x0008,
	WIN_RAISED_FRAME  = 0x0010,
	WIN_SUNK_FRAME    = 0x0020,
	WIN_TEXT_SHADOW   = 0x0040,
	WIN_FRAME_ONLY    = 0x0080
};

enum {
	EID_CREATE,
	EID_DESTROY,
	EID_MOUSE_MOVE,
	EID_CLICK,
	EID_SELECT,
	EID_LBUTTON_DOWN,
	EID_LBUTTON_UP,
	EID_RBUTTON_DOWN,
	EID_RBUTTON_UP,
	EID_KEY_DOWN,
	EID_SET_FOCUS,
	EID_KILL_FOCUS,
	EID_MOUSE_ENTER,
	EID_MOUSE_EXIT,
	EID_MOUSE_WHEEL,
	EID_DRAG_START,
	EID_DRAG_DROP,

	EID_USER_1,
	EID_USER_2,
	EID_USER_3,
	EID_USER_4,

	EID_NUM_EVENTS
};

// +--------------------------------------------------------------------+

class ActiveWindow;

struct AWEvent
{
	static const char* TYPENAME() { return "AWEvent"; }

	AWEvent() : window(0), eid(0), x(0), y(0) { }
	AWEvent(ActiveWindow* w, int e, int ax=0, int ay=0) : window(w), eid(e), x(ax), y(ay) { }

	int operator == (const AWEvent& e) const { return (window == e.window) &&
		(eid    == e.eid)    &&
		(x      == e.x)      &&
		(y      == e.y); }

	ActiveWindow*  window;
	int            eid;
	int            x;
	int            y;
};

typedef void (*PFVAWE)(ActiveWindow*, AWEvent*);

struct AWMap
{
	static const char* TYPENAME() { return "AWMap"; }

	AWMap() : eid(0), client(0), func(0) { }
	AWMap(int e, ActiveWindow* w, PFVAWE f) : eid(e), client(w), func(f) { }

	int operator == (const AWMap& m) const { return (eid    == m.eid) &&
		(client == m.client); }

	int            eid;
	ActiveWindow*  client;
	PFVAWE         func;
};

// +--------------------------------------------------------------------+

class ActiveWindow : public Window,
public EventTarget
{
public:
	static const char* TYPENAME() { return "ActiveWindow"; }

	ActiveWindow(Screen* s, int ax, int ay, int aw, int ah,
	DWORD id=0, DWORD style=0, ActiveWindow* parent=0);
	virtual ~ActiveWindow();

	int operator == (const ActiveWindow& w) const { return id == w.id; }

	// Operations:
	virtual void      Paint();    // blt to screen
	virtual void      Draw();     // refresh backing store
	virtual void      Show();
	virtual void      Hide();
	virtual void      MoveTo(const Rect& r);
	virtual void      UseLayout(const std::vector<DWORD>& min_x,
		const std::vector<DWORD>& min_y,
		const std::vector<float>& weight_x,
		const std::vector<float>& weight_y);
	virtual void      UseLayout(const std::vector<float>& min_x,
		const std::vector<float>& min_y,
		const std::vector<float>& weight_x,
		const std::vector<float>& weight_y);
	virtual void      UseLayout(int     ncols,
		int     nrows,
		int*    min_x,
		int*    min_y,
		float*  weight_x,
		float*  weight_y);
	virtual void      DoLayout();

	// Event Target Interface:
	virtual int       OnMouseMove(int x, int y);
	virtual int       OnLButtonDown(int x, int y);
	virtual int       OnLButtonUp(int x, int y);
	virtual int       OnClick();
	virtual int       OnSelect();
	virtual int       OnRButtonDown(int x, int y);
	virtual int       OnRButtonUp(int x, int y);
	virtual int       OnMouseEnter(int x, int y);
	virtual int       OnMouseExit(int x, int y);
	virtual int       OnMouseWheel(int wheel);

	virtual int       OnKeyDown(int vk, int flags);

	virtual const char* GetDescription() const { return desc; }

	// pseudo-events:
	virtual int       OnDragStart(int x, int y);
	virtual int       OnDragDrop(int x, int y, ActiveWindow* source);

	virtual ActiveWindow*   FindControl(int x, int y) { return 0; }
	virtual Rect      TargetRect() const;

	virtual ActiveWindow* GetCapture();
	virtual bool       SetCapture();
	virtual int       ReleaseCapture();

	// Property accessors:
	virtual void      SetFocus();
	virtual void      KillFocus();
	virtual bool      HasFocus() const { return focus; }

	void              SetEnabled(bool e=true) { enabled = e;        }
	bool              IsEnabled() const       { return enabled;     }
	bool              IsVisible() const       { return shown;       }

	DWORD             GetID() const           { return id;          }
	void              SetStyle(DWORD s)       { style = s;          }
	DWORD             GetStyle() const        { return style;       }

	void              SetText(const char* t);
	void              SetText(const Text& t);
	void              AddText(const char* t);
	void              AddText(const Text& t);
	const Text&       GetText() const         { return text;        }

	void              SetAltText(const char* t) { alt_text = t;     }
	void              SetAltText(const Text& t) { alt_text = t;     }
	const Text&       GetAltText() const      { return alt_text;    }

	void              SetTexture(Bitmap* bmp) { texture = bmp;      }
	Bitmap*           GetTexture()            { return texture;     }
	void              SetMargins(const Insets& m);
	Insets&           GetMargins()            { return margins;     }
	void              SetTextInsets(const Insets& t);
	Insets&           GetTextInsets()         { return text_insets; }

	List<ActiveWindow>& GetChildren()         { return children;    }
	void              SetCellInsets(const Insets& c);
	Insets&           GetCellInsets()         { return cell_insets; }
	void              SetCells(int cx, int cy, int cw=1, int ch=1);
	void              SetCells(const Rect& r) { cells = r;          }
	Rect&             GetCells()              { return cells;       }
	void              SetFixedWidth(int w)    { fixed_width = w;    }
	int               GetFixedWidth()  const  { return fixed_width; }
	void              SetFixedHeight(int h)   { fixed_height = h;   }
	int               GetFixedHeight() const  { return fixed_height;}

	void              SetAlpha(double a);
	double            GetAlpha()        const { return alpha;       }
	void              SetBackColor(Color c)   { back_color = c;     }
	Color             GetBackColor()    const { return back_color;  }
	void              SetBaseColor(Color c)   { base_color = c;     }
	Color             GetBaseColor()    const { return base_color;  }
	void              SetForeColor(Color c)   { fore_color = c;     }
	Color             GetForeColor()    const { return fore_color;  }
	void              SetSingleLine(bool a)   { single_line = a;    }
	bool              GetSingleLine()   const { return single_line; }
	void              SetTextAlign(DWORD a);
	DWORD             GetTextAlign()    const { return text_align;  }
	void              SetTransparent(bool t)  { transparent = t;    }
	bool              GetTransparent()  const { return transparent; }
	void              SetHidePartial(bool a)  { hide_partial = a;   }
	bool              GetHidePartial()  const { return hide_partial;}

	void              SetTabStop(int n, int x);
	int               GetTabStop(int n) const;

	void              DrawText(const char* txt, int count, Rect& txt_rect, DWORD flags);

	// class properties:
	static void       SetSystemFont(Font* f);
	static void       SetSystemBackColor(Color c);
	static void       SetSystemForeColor(Color c);

	// callback function registration:
	virtual void      RegisterClient(int EID, ActiveWindow* client, PFVAWE callback);
	virtual void      UnregisterClient(int EID, ActiveWindow* client);
	virtual void      ClientEvent(int EID, int x=0, int y=0);

	// form context:
	virtual ActiveWindow*   GetForm()                  { return form; }
	virtual void            SetForm(ActiveWindow* f)   { form = f;    }
	virtual bool            IsFormActive()       const;
	virtual bool            IsTopMost()          const { return topmost; }
	virtual void            SetTopMost(bool t)         { topmost = t;    }

	virtual ActiveWindow*   FindChild(DWORD id);
	virtual ActiveWindow*   FindChild(int x, int y);

protected:
	virtual Color     ShadeColor(Color c, double shade);
	virtual void      AddChild(ActiveWindow* child);
	virtual void      DrawStyleRect(const Rect& r, int style);
	virtual void      DrawStyleRect(int x1, int y1, int x2, int y2, int style);
	virtual void      DrawTabbedText();
	virtual void      DrawTextureGrid();
	virtual void      CalcGrid();

	DWORD             id;
	DWORD             style;
	DWORD             text_align;
	bool              single_line;
	bool              focus;
	bool              enabled;
	bool              hide_partial;
	float             alpha;
	Color             back_color;
	Color             base_color;
	Color             fore_color;
	Text              text;
	Text              alt_text;
	Text              desc;
	Bitmap*           texture;
	Insets            margins;
	Insets            text_insets;
	Insets            cell_insets;
	Rect              cells;
	int               fixed_width;
	int               fixed_height;
	int               tab[10];

	ActiveWindow*     parent;
	ActiveWindow*     form;
	bool              transparent;
	bool              topmost;

	Layout*              layout;
	List<ActiveWindow>   children;
	List<AWMap>          clients;
	AWEvent              event;

	int               rows;
	int               cols;
	Poly*             polys;
	VertexSet*        vset;
	Material*         mtl;

	static Font*      sys_font;
	static Color      sys_back_color;
	static Color      sys_fore_color;
};

#define DEF_MAP_CLIENT(cname, fname)\
	void Map##cname##fname(ActiveWindow* client, AWEvent* event) \
	{ cname* c = (cname*) client; c->fname(event); }

#define REGISTER_CLIENT(eid, ctrl, cname, fname)\
	if (ctrl) ctrl->RegisterClient(eid, this, Map##cname##fname);

#define UNREGISTER_CLIENT(eid, ctrl, cname)\
	if (ctrl) ctrl->UnregisterClient(eid, this);

#endif ActiveWindow_h

