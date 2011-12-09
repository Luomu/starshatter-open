/*  Project nGenEx
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    nGenEx.lib
	FILE:         ComboBox.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	ComboBox class
*/

#ifndef ComboBox_h
#define ComboBox_h

#include "Types.h"
#include "ActiveWindow.h"
#include "Bitmap.h"

#include "List.h"
#include "Text.h"

// +--------------------------------------------------------------------+

class ComboList;

// +--------------------------------------------------------------------+

class ComboBox : public ActiveWindow
{
public:
	static const char* TYPENAME() { return "ComboBox"; }

	ComboBox(ActiveWindow* p, int ax, int ay, int aw, int ah, DWORD aid=0);
	ComboBox(Screen* s,       int ax, int ay, int aw, int ah, DWORD aid=0);
	virtual ~ComboBox();

	// Operations:
	virtual void         Draw();
	virtual void         ShowList();
	virtual void         HideList();
	virtual void         MoveTo(const Rect& r);

	// Event Target Interface:
	virtual int          OnMouseMove(int x, int y);
	virtual int          OnLButtonDown(int x, int y);
	virtual int          OnLButtonUp(int x, int y);
	virtual int          OnClick();
	virtual int          OnMouseEnter(int x, int y);
	virtual int          OnMouseExit(int x, int y);

	virtual void         OnListSelect(AWEvent* event);
	virtual void         OnListExit(AWEvent* event);

	// Property accessors:
	virtual int          NumItems();
	virtual void         ClearItems();
	virtual void         AddItem(const char* item);
	virtual const char*  GetItem(int index);
	virtual void         SetItem(int index, const char* item);
	virtual void         SetLabel(const char* label);

	virtual int          GetCount();
	virtual const char*  GetSelectedItem();
	virtual int          GetSelectedIndex();
	virtual void         SetSelection(int index);

	Color GetActiveColor()                 const { return active_color;  }
	void  SetActiveColor(Color c);
	bool  GetAnimated()                    const { return animated;      }
	void  SetAnimated(bool bNewValue);
	short GetBevelWidth()                  const { return bevel_width;   }
	void  SetBevelWidth(short nNewValue);
	bool  GetBorder()                      const { return border;        }
	void  SetBorder(bool bNewValue);
	Color GetBorderColor()                 const { return border_color;  }
	void  SetBorderColor(Color c);
	short GetButtonState()                 const { return button_state;  }
	void  SetButtonState(short nNewValue);

	bool  IsListShowing()                  const { return list_showing;  }

protected:
	Rect  CalcLabelRect();
	void  DrawRectSimple(Rect& rect, int stat);

	List<Text>     items;
	ComboList*     list;
	bool           list_showing;
	bool           animated;
	bool           border;
	int            seln;
	int            captured;
	int            bevel_width;
	int            button_state;
	int            pre_state;

	Color          active_color;
	Color          border_color;
};

#endif ComboBox_h

