/*  Project nGenEx
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    nGenEx.lib
	FILE:         ComboList.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	ComboList class
*/

#ifndef ComboList_h
#define ComboList_h

#include "Types.h"
#include "ScrollWindow.h"
#include "Bitmap.h"

#include "List.h"
#include "Text.h"

// +--------------------------------------------------------------------+

class ComboBox;

// +--------------------------------------------------------------------+

class ComboList : public ScrollWindow
{
public:
	static const char* TYPENAME() { return "ComboList"; }

	ComboList(ComboBox* ctrl, ActiveWindow* p, int ax, int ay, int aw, int ah, int maxentries);
	ComboList(ComboBox* ctrl, Screen* s,       int ax, int ay, int aw, int ah, int maxentries);
	virtual ~ComboList();

	// Operations:
	virtual void         Draw();
	virtual void         Show();
	virtual void         Hide();

	// Event Target Interface:
	virtual int          OnMouseMove(int x, int y);
	virtual int          OnLButtonDown(int x, int y);
	virtual int          OnLButtonUp(int x, int y);
	virtual int          OnClick();
	virtual int          OnMouseEnter(int x, int y);
	virtual int          OnMouseExit(int x, int y);
	virtual void         KillFocus();

	// Property accessors:
	virtual void         ClearItems();
	virtual void         AddItem(const char* item);
	virtual void         AddItems(ListIter<Text> item_list);
	virtual void         SetItems(ListIter<Text> item_list);
	virtual const char*  GetItem(int index);
	virtual void         SetItem(int index, const char* item);

	virtual int          GetCount();
	virtual const char*  GetSelectedItem();
	virtual int          GetSelectedIndex();
	virtual void         SetSelection(int index);

protected:
	void  DrawRectSimple(Rect& rect, int stat);
	void  DrawItem(Text label, Rect& btn_rect, int state);
	Rect  CalcLabelRect(const Rect& btn_rect);
	int   CalcSeln(int x, int y);
	void  CopyStyle(const ComboBox& ctrl);

	ComboBox*      combo_box;
	List<Text>     items;
	bool           animated;
	bool           border;
	int            seln;
	int            captured;
	int            bevel_width;
	int            button_state;
	int            button_height;
	int            max_entries;
	int            scroll;
	int            scrolling;

	Color          active_color;
	Color          border_color;
};

#endif ComboList_h

