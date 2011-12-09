/*  Project nGenEx
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    nGenEx.lib
	FILE:         ListBox.h
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	ListBox ActiveWindow class
*/

#ifndef ListBox_h
#define ListBox_h

#include "Types.h"
#include "ScrollWindow.h"
#include "List.h"

// +--------------------------------------------------------------------+

class Bitmap;
class ListBox;
class ListBoxCell;
class ListBoxItem;
class ListBoxColumn;

// +--------------------------------------------------------------------+

class ListBox : public ScrollWindow
{
public:
	enum SORT   {  LIST_SORT_NUMERIC_DESCENDING = -2,
		LIST_SORT_ALPHA_DESCENDING,
		LIST_SORT_NONE,
		LIST_SORT_ALPHA_ASCENDING,
		LIST_SORT_NUMERIC_ASCENDING,
		LIST_SORT_NEVER
	};

	enum ALIGN  {  LIST_ALIGN_LEFT   = DT_LEFT,
		LIST_ALIGN_CENTER = DT_CENTER,
		LIST_ALIGN_RIGHT  = DT_RIGHT
	};

	enum STYLE  {  LIST_ITEM_STYLE_PLAIN,
		LIST_ITEM_STYLE_BOX,
		LIST_ITEM_STYLE_FILLED_BOX
	};

	ListBox(ActiveWindow* p, int ax, int ay, int aw, int ah, DWORD aid);
	ListBox(Screen* s,       int ax, int ay, int aw, int ah, DWORD aid);
	virtual ~ListBox();

	// Operations:
	virtual void      DrawContent(const Rect& ctrl_rect);

	// Event Target Interface:
	virtual int       OnMouseMove(int x, int y);
	virtual int       OnLButtonDown(int x, int y);
	virtual int       OnLButtonUp(int x, int y);
	virtual int       OnMouseWheel(int wheel);
	virtual int       OnClick();

	virtual int       OnKeyDown(int vk, int flags);

	// pseudo-events:
	virtual int       OnDragStart(int x, int y);
	virtual int       OnDragDrop(int x, int y, ActiveWindow* source);

	// Property accessors:
	int            NumItems();
	int            NumColumns();

	Text           GetItemText(int index);
	void           SetItemText(int index, const char* text);
	DWORD          GetItemData(int index);
	void           SetItemData(int index, DWORD data);
	Bitmap*        GetItemImage(int index);
	void           SetItemImage(int index, Bitmap* img);
	Color          GetItemColor(int index);
	void           SetItemColor(int index, Color c);

	Text           GetItemText(int index, int column);
	void           SetItemText(int index, int column, const char* text);
	DWORD          GetItemData(int index, int column);
	void           SetItemData(int index, int column, DWORD data);
	Bitmap*        GetItemImage(int index, int column);
	void           SetItemImage(int index, int column, Bitmap* img);

	int            AddItem(const char* text);
	int            AddImage(Bitmap* img);
	int            AddItemWithData(const char* text, int data);
	void           InsertItem(int index, const char* text);
	void           InsertItemWithData(int index, const char* text, int data);
	void           ClearItems();
	void           RemoveItem(int index);
	void           RemoveSelectedItems();

	void           AddColumn(const char* title,
	int width,
	int align = ListBox::LIST_ALIGN_LEFT,
	int sort  = ListBox::LIST_SORT_NONE);

	Text           GetColumnTitle(int index);
	void           SetColumnTitle(int index, const char* title);
	int            GetColumnWidth(int index);
	void           SetColumnWidth(int index, int width);
	int            GetColumnAlign(int index);
	void           SetColumnAlign(int index, int align);
	int            GetColumnSort(int index);
	void           SetColumnSort(int index, int sort);
	Color          GetColumnColor(int index);
	void           SetColumnColor(int index, Color c);

	Color          GetItemColor(int index, int column);

	int            GetMultiSelect();
	void           SetMultiSelect(int nNewValue);
	bool           GetShowHeadings();
	void           SetShowHeadings(bool nNewValue);
	Color          GetSelectedColor();
	void           SetSelectedColor(Color c);

	int            GetItemStyle() const;
	void           SetItemStyle(int style);
	int            GetSelectedStyle() const;
	void           SetSelectedStyle(int style);

	bool           IsSelected(int index);
	void           SetSelected(int index, bool bNewValue=true);
	void           ClearSelection();

	int            GetSortColumn();
	void           SetSortColumn(int col_index);
	int            GetSortCriteria();
	void           SetSortCriteria(SORT sort);
	void           SortItems();
	void           SizeColumns();

	// read-only:
	virtual int    GetListIndex();
	virtual int    GetLineCount();
	virtual int    GetSelCount();
	virtual int    GetSelection();
	virtual Text   GetSelectedItem();

protected:
	int            IndexFromPoint(int x, int y) const;

	// properties:
	List<ListBoxItem>    items;
	List<ListBoxColumn>  columns;

	bool           show_headings;
	int            multiselect;
	int            list_index;
	int            selcount;

	Color          selected_color;

	int            sort_column;
	int            item_style;
	int            seln_style;
};

#endif ListBox_h

