/*  Project nGenEx
	Destroyer Studios LLC
	Copyright � 1997-2004. All Rights Reserved.

	SUBSYSTEM:    nGenEx.lib
	FILE:         ListBox.cpp
	AUTHOR:       John DiCamillo


	OVERVIEW
	========
	ListBox ActiveWindow class
*/

#include "MemDebug.h"
#include "ListBox.h"
#include "Button.h"
#include "Bitmap.h"
#include "FormWindow.h"
#include "Video.h"
#include "Font.h"
#include "Keyboard.h"
#include "Mouse.h"

// +--------------------------------------------------------------------+

class ListBoxCell
{
public:
	static const char* TYPENAME() { return "ListBoxCell"; }

	ListBoxCell() : data(0), image(0) { }

	Text     text;
	DWORD    data;
	Bitmap*  image;
};

// +--------------------------------------------------------------------+

class ListBoxItem
{
public:
	static const char* TYPENAME() { return "ListBoxItem"; }

	ListBoxItem() : data(0), image(0), selected(false), listbox(0), color(Color::White) { }
	~ListBoxItem() { subitems.destroy(); }

	int operator < (const ListBoxItem& item) const;
	int operator <=(const ListBoxItem& item) const;
	int operator ==(const ListBoxItem& item) const;

	Text                 text;
	DWORD                data;
	Bitmap*              image;
	bool                 selected;
	Color                color;
	List<ListBoxCell>    subitems;

	ListBox*             listbox;
};

// +--------------------------------------------------------------------+

class ListBoxColumn
{
public:
	static const char* TYPENAME() { return "ListBoxColumn"; }

	ListBoxColumn() : width(0), align(0), sort(0), color(Color::White), use_color(0), percent(0) { }

	Text  title;
	int   width;
	int   align;
	int   sort;
	Color color;
	int   use_color;

	double percent;
};

// +--------------------------------------------------------------------+

int ListBoxItem::operator < (const ListBoxItem& item) const
{
    if (listbox && listbox == item.listbox) {
        int sort_column   = listbox->GetSortColumn() - 1;
        int sort_criteria = listbox->GetSortCriteria();

		if (sort_column == -1) {
			switch (sort_criteria) {
			case ListBox::LIST_SORT_NUMERIC_DESCENDING:
				return data > item.data;

			case ListBox::LIST_SORT_ALPHA_DESCENDING:
				return text > item.text;

			case ListBox::LIST_SORT_ALPHA_ASCENDING:
				return text < item.text;

			case ListBox::LIST_SORT_NUMERIC_ASCENDING:
				return data < item.data;
			}
		}

		else if (sort_column >= 0 &&
				sort_column <= subitems.size() &&
				sort_column <= item.subitems.size()) {

			switch (sort_criteria) {
			case ListBox::LIST_SORT_NUMERIC_DESCENDING:
				return subitems[sort_column]->data > item.subitems[sort_column]->data;

			case ListBox::LIST_SORT_ALPHA_DESCENDING:
				return subitems[sort_column]->text > item.subitems[sort_column]->text;

			case ListBox::LIST_SORT_ALPHA_ASCENDING:
				return subitems[sort_column]->text < item.subitems[sort_column]->text;

			case ListBox::LIST_SORT_NUMERIC_ASCENDING:
				return subitems[sort_column]->data < item.subitems[sort_column]->data;
			}
		}
	}

	return 0;
}

// +--------------------------------------------------------------------+

int ListBoxItem::operator <=(const ListBoxItem& item) const
{
	if (listbox && listbox == item.listbox) {
    	int sort_column   = listbox->GetSortColumn() - 1;
        int sort_criteria = listbox->GetSortCriteria();
        
		if (sort_column == -1) {
			switch (sort_criteria) {
			case ListBox::LIST_SORT_NUMERIC_DESCENDING:
				return data >= item.data;

			case ListBox::LIST_SORT_ALPHA_DESCENDING:
				return text >= item.text;

			case ListBox::LIST_SORT_ALPHA_ASCENDING:
				return text <= item.text;

			case ListBox::LIST_SORT_NUMERIC_ASCENDING:
				return data <= item.data;
			}
		}

		else if (sort_column >= 0 &&
				sort_column <= subitems.size() &&
				sort_column <= item.subitems.size()) {

			switch (sort_criteria) {
			case ListBox::LIST_SORT_NUMERIC_DESCENDING:
				return subitems[sort_column]->data >= item.subitems[sort_column]->data;

			case ListBox::LIST_SORT_ALPHA_DESCENDING:
				return subitems[sort_column]->text >= item.subitems[sort_column]->text;

			case ListBox::LIST_SORT_ALPHA_ASCENDING:
				return subitems[sort_column]->text <= item.subitems[sort_column]->text;

			case ListBox::LIST_SORT_NUMERIC_ASCENDING:
				return subitems[sort_column]->data <= item.subitems[sort_column]->data;
			}
		}
	}

	return 0;
}

// +--------------------------------------------------------------------+

int ListBoxItem::operator == (const ListBoxItem& item) const
{
	if (listbox && listbox == item.listbox) {
        int sort_column   = listbox->GetSortColumn() - 1;
        int sort_criteria = listbox->GetSortCriteria();
        
		if (sort_column == -1) {
			switch (sort_criteria) {
			case ListBox::LIST_SORT_NUMERIC_DESCENDING:
				return data == item.data;

			case ListBox::LIST_SORT_ALPHA_DESCENDING:
				return text == item.text;

			case ListBox::LIST_SORT_ALPHA_ASCENDING:
				return text == item.text;

			case ListBox::LIST_SORT_NUMERIC_ASCENDING:
				return data == item.data;
			}
		}

		else if (sort_column >= 0 &&
				sort_column <= subitems.size() &&
				sort_column <= item.subitems.size()) {

			switch (sort_criteria) {
			case ListBox::LIST_SORT_NUMERIC_DESCENDING:
				return subitems[sort_column]->data == item.subitems[sort_column]->data;

			case ListBox::LIST_SORT_ALPHA_DESCENDING:
				return subitems[sort_column]->text == item.subitems[sort_column]->text;

			case ListBox::LIST_SORT_ALPHA_ASCENDING:
				return subitems[sort_column]->text == item.subitems[sort_column]->text;

			case ListBox::LIST_SORT_NUMERIC_ASCENDING:
				return subitems[sort_column]->data == item.subitems[sort_column]->data;
			}
		}
	}

	return 0;
}

// +--------------------------------------------------------------------+

static int old_cursor;

// +--------------------------------------------------------------------+

ListBox::ListBox(ActiveWindow* p, int ax, int ay, int aw, int ah, DWORD aid)
: ScrollWindow(p->GetScreen(), ax, ay, aw, ah, aid, 0, p)
{
	show_headings     = false;
	multiselect       = false;
	list_index        = 0;
	selcount          = 0;

	selected_color    = Color(255, 255, 128);

	sort_column       = 0;
	item_style        = LIST_ITEM_STYLE_PLAIN;
	seln_style        = LIST_ITEM_STYLE_PLAIN;

	char buf[32];
	sprintf_s(buf, "ListBox %d", id);
	desc = buf;
}

ListBox::ListBox(Screen* s, int ax, int ay, int aw, int ah, DWORD aid)
: ScrollWindow(s, ax, ay, aw, ah, aid)
{
	show_headings     = false;
	multiselect       = false;
	list_index        = 0;
	selcount          = 0;

	selected_color    = Color(255, 255, 128);

	sort_column       = 0;
	item_style        = LIST_ITEM_STYLE_PLAIN;
	seln_style        = LIST_ITEM_STYLE_PLAIN;

	char buf[32];
	sprintf_s(buf, "ListBox %d", id);
	desc = buf;
}

ListBox::~ListBox()
{
	items.destroy();
	columns.destroy();
}

// +--------------------------------------------------------------------+

void
ListBox::DrawContent(const Rect& ctrl_rect)
{
	SizeColumns();

	Rect item_rect = ctrl_rect;
	item_rect.h = line_height;

	int h = rect.h;

	// draw headings at top, if needed:
	if (show_headings) {
		Color save_color = back_color;
		back_color = ShadeColor(back_color, 1.3);
		font->SetColor(fore_color);

		int max_column = columns.size()-1;
		item_rect.h += HEADING_EXTRA;

		page_size = (h-item_rect.h) / (line_height + leading);

		for (int column = 0; column <= max_column; column++) {
			item_rect.w = GetColumnWidth(column);

			// draw heading button
			FillRect(item_rect, back_color);
			DrawStyleRect(item_rect, WIN_RAISED_FRAME);

			Rect title_rect = item_rect;
			title_rect.Deflate(3,3);

			DrawText(GetColumnTitle(column),
			0,
			title_rect,
			DT_CENTER|DT_SINGLELINE);

			item_rect.x += item_rect.w;
		}

		item_rect.y += item_rect.h;
		back_color = save_color;
		item_rect.h = line_height;
	}

	int index = 0;
	ListIter<ListBoxItem> iter = items;

	while (++iter && item_rect.y < h) {
		ListBoxItem* item = iter.value();

		if (index++ >= top_index) {
			// draw main item:
			int column = 0;
			item_rect.x = ctrl_rect.x;
			item_rect.w = GetColumnWidth(column) - 2;

			if (item_rect.y + item_rect.h > h) {
				item_rect.h = h - item_rect.y - 1;
			}

			Color item_color = GetItemColor(index-1, 0);

			if (item->selected) {
				font->SetColor(selected_color);

				if (seln_style == LIST_ITEM_STYLE_FILLED_BOX)
				FillRect(item_rect, selected_color * 0.25);

				if (seln_style >= LIST_ITEM_STYLE_BOX)
				DrawRect(item_rect, selected_color);
			}
			else {
				font->SetColor(item_color);

				if (item_style == LIST_ITEM_STYLE_FILLED_BOX)
				FillRect(item_rect, item_color * 0.25);

				if (item_style >= LIST_ITEM_STYLE_BOX)
				DrawRect(item_rect, item_color);
			}

			Rect text_rect = item_rect;

			if (item->image && item->image->Width() > 0 && item->image->Height() > 0) {
				DrawBitmap(text_rect.x, text_rect.y, text_rect.x + text_rect.w, text_rect.y + line_height, item->image);
			}
			else {
				text_rect.Deflate(2,0);
				DrawText(item->text.data(), 
				item->text.length(),
				text_rect,
				GetColumnAlign(column)|DT_SINGLELINE);
			}

			// draw subitems:
			ListIter<ListBoxCell> sub_iter = item->subitems;
			while (++sub_iter) {
				ListBoxCell* sub = sub_iter.value();

				column++;
				item_rect.x += item_rect.w + 2;
				item_rect.w = GetColumnWidth(column) - 2;

				if (item->selected) {
					if (seln_style == LIST_ITEM_STYLE_FILLED_BOX)
					FillRect(item_rect, selected_color * 0.25);

					if (seln_style >= LIST_ITEM_STYLE_BOX)
					DrawRect(item_rect, selected_color);
				}
				else {
					if (item_style == LIST_ITEM_STYLE_FILLED_BOX)
					FillRect(item_rect, item_color * 0.25);

					if (item_style >= LIST_ITEM_STYLE_BOX)
					DrawRect(item_rect, item_color);
				}

				if (item->selected)
				font->SetColor(selected_color);
				else
				font->SetColor(GetItemColor(index-1, column));

				Rect text_rect = item_rect;
				if (sub->image && sub->image->Width() > 0 && sub->image->Height() > 0) {
					DrawBitmap(text_rect.x, text_rect.y, text_rect.x + text_rect.w, text_rect.y + line_height, sub->image);
				}
				else {
					text_rect.Deflate(2,0);
					DrawText(sub->text.data(),
					sub->text.length(),
					text_rect,
					GetColumnAlign(column)|DT_SINGLELINE);
				}
			}

			item_rect.y += line_height + leading;
		}
	}
}

// +--------------------------------------------------------------------+

void
ListBox::SizeColumns()
{
	ListBoxColumn* c = columns.first();

	if (c->percent < 0.001) {
		double total = 0;

		ListIter<ListBoxColumn> iter = columns;
		while (++iter) {
			c = iter.value();
			total += c->width;
		}

		iter.reset();
		while (++iter) {
			c = iter.value();
			c->percent = c->width / total;
		}
	}

	int usable_width = rect.w;
	int used         = 0;

	if (IsScrollVisible()) {
		usable_width -= SCROLL_WIDTH + 2;
	}
	else {
		usable_width -= 3;
	}

	for (int i = 0; i < columns.size(); i++) {
		c = columns[i];

		if (i < columns.size() - 1)
		c->width = (int) (c->percent * usable_width);
		else
		c->width = usable_width - used;

		used += c->width;
	}
}

// +--------------------------------------------------------------------+

int   ListBox::NumItems()
{
	return items.size();
}

int   ListBox::NumColumns()
{
	return columns.size();
}

Text  ListBox::GetItemText(int index)
{
	if (index >= 0 && index < items.size())
	return items[index]->text;

	return Text();
}

void  ListBox::SetItemText(int index, const char* text)
{
	if (index >= 0 && index < items.size()) {
		items[index]->text = text;
	}
}

DWORD ListBox::GetItemData(int index)
{
	if (index >= 0 && index < items.size())
	return items[index]->data;

	return 0;
}

void  ListBox::SetItemData(int index, DWORD data)
{
	if (index >= 0 && index < items.size()) {
		items[index]->data = data;
	}
}

Bitmap* ListBox::GetItemImage(int index)
{
	if (index >= 0 && index < items.size())
	return items[index]->image;

	return 0;
}

void    ListBox::SetItemImage(int index, Bitmap* img)
{
	if (index >= 0 && index < items.size()) {
		items[index]->image = img;
	}
}

Color ListBox::GetItemColor(int index)
{
	if (index >= 0 && index < items.size())
	return items[index]->color;

	return Color::White;
}

void  ListBox::SetItemColor(int index, Color c)
{
	if (index >= 0 && index < items.size()) {
		items[index]->color = c;
	}
}

Text  ListBox::GetItemText(int index, int column)
{
	if (column == 0) {
		return GetItemText(index);
	}

	if (index >= 0 && index < items.size()) {
		ListBoxItem* item = items[index];

		column--;
		if (column >= 0 && column < item->subitems.size())
		return item->subitems[column]->text;
	}

	return Text();
}

void  ListBox::SetItemText(int index, int column, const char* text)
{
	if (column == 0) {
		SetItemText(index, text);
		return;
	}

	if (index >= 0 && index < items.size()) {
		ListBoxItem* item = items[index];
		
		column--;
		if (column >= 0 && column < columns.size()-1) {
			while (column >= item->subitems.size()) {
				ListBoxCell* cell = new(__FILE__,__LINE__) ListBoxCell;
				if (cell)
				item->subitems.append(cell);
			}

			item->subitems[column]->text = text;
		}
	}
}

DWORD ListBox::GetItemData(int index, int column)
{
	if (column == 0) {
		return GetItemData(index);
	}

	if (index >= 0 && index < items.size()) {
		ListBoxItem* item = items[index];
		
		column--;
		if (column >= 0 && column < item->subitems.size())
		return item->subitems[column]->data;
	}

	return 0;
}

void  ListBox::SetItemData(int index, int column, DWORD data)
{
	if (column == 0) {
		SetItemData(index, data);
		return;
	}

	if (index >= 0 && index < items.size()) {
		ListBoxItem* item = items[index];
		
		column--;
		if (column >= 0 && column < columns.size()-1) {
			while (column >= item->subitems.size()) {
				ListBoxCell* cell = new(__FILE__,__LINE__) ListBoxCell;
				if (cell)
				item->subitems.append(cell);
			}

			item->subitems[column]->data = data;
		}
	}
}

Bitmap* ListBox::GetItemImage(int index, int column)
{
	if (column == 0) {
		return GetItemImage(index);
	}

	if (index >= 0 && index < items.size()) {
		ListBoxItem* item = items[index];
		
		column--;
		if (column >= 0 && column < item->subitems.size())
		return item->subitems[column]->image;
	}

	return 0;
}

void  ListBox::SetItemImage(int index, int column, Bitmap* img)
{
	if (column == 0) {
		SetItemImage(index, img);
		return;
	}

	if (index >= 0 && index < items.size()) {
		ListBoxItem* item = items[index];
		
		column--;
		if (column >= 0 && column < columns.size()-1) {
			while (column >= item->subitems.size()) {
				ListBoxCell* cell = new(__FILE__,__LINE__) ListBoxCell;
				if (cell)
				item->subitems.append(cell);
			}

			item->subitems[column]->image = img;
		}
	}
}

int   ListBox::AddItem(const char* text)
{
	ListBoxItem* item = new(__FILE__,__LINE__) ListBoxItem;

	if (item) {
		item->text    = text;
		item->color   = fore_color;
		item->listbox = this;

		items.append(item);

		line_count = items.size();
		list_index = items.size()-1;
	}

	return list_index+1;
}

int   ListBox::AddItemWithData(const char* text, int data)
{
	ListBoxItem* item = new(__FILE__,__LINE__) ListBoxItem;

	if (item) {
		item->text    = text;
		item->data    = data;
		item->color   = fore_color;
		item->listbox = this;

		items.append(item);

		line_count = items.size();
		list_index = items.size()-1;
	}

	return list_index+1;
}

int   ListBox::AddImage(Bitmap* img)
{
	ListBoxItem* item = new(__FILE__,__LINE__) ListBoxItem;

	if (item) {
		item->image   = img;
		item->color   = fore_color;
		item->listbox = this;

		items.append(item);

		line_count = items.size();
		list_index = items.size()-1;
	}

	return list_index+1;
}

void  ListBox::InsertItem(int index, const char* text)
{
	if (index >=0 && index < items.size()) {
		ListBoxItem* item = new(__FILE__,__LINE__) ListBoxItem;

		if (item) {
			item->text    = text;
			item->color   = fore_color;
			item->listbox = this;

			list_index = index;
			items.insert(item, list_index);
			line_count = items.size();
		}
	}
}

void  ListBox::InsertItemWithData(int index, const char* text, int data)
{
	if (index >=0 && index < items.size()) {
		ListBoxItem* item = new(__FILE__,__LINE__) ListBoxItem;

		if (item) {
			item->text    = text;
			item->data    = data;
			item->color   = fore_color;
			item->listbox = this;

			list_index = index;
			items.insert(item, list_index);
			line_count = items.size();
		}
	}
}

void  ListBox::ClearItems()
{
	items.destroy();
	selcount   = 0;
	top_index  = 0;
	list_index = 0;
	line_count = 0;
}

void  ListBox::RemoveItem(int index)
{
	if (index >= 0 && index < items.size()) {
		if (items[index]->selected)
		selcount--;
		items.removeIndex(index);
		line_count = items.size();
	}
}

void  ListBox::RemoveSelectedItems()
{
	if (selcount) {
		ListIter<ListBoxItem> item = items;
		while (++item) {
			if (item->selected) {
				delete item.removeItem();
			}
		}

		line_count = items.size();
		selcount = 0;
	}
}

void  ListBox::AddColumn(const char* title, int width, int align, int sort)
{
	ListBoxColumn* column = new(__FILE__,__LINE__) ListBoxColumn;

	if (column) {
		column->title = title;
		column->width = width;
		column->align = align;
		column->sort  = sort;

		columns.append(column);
	}
}

Text  ListBox::GetColumnTitle(int index)
{
	if (index >= 0 && index < columns.size())
	return columns[index]->title;

	return Text();
}

void  ListBox::SetColumnTitle(int index, const char* title)
{
	if (index >= 0 && index < columns.size()) {
		columns[index]->title = title;
	}
}

int   ListBox::GetColumnWidth(int index)
{
	if (index >= 0 && index < columns.size())
	return columns[index]->width;

	return 0;
}

void  ListBox::SetColumnWidth(int index, int width)
{
	if (index >= 0 && index < columns.size()) {
		columns[index]->width = width;
	}
}

int   ListBox::GetColumnAlign(int index)
{
	if (index >= 0 && index < columns.size())
	return columns[index]->align;

	return 0;
}

void  ListBox::SetColumnAlign(int index, int align)
{
	if (index >= 0 && index < columns.size()) {
		columns[index]->align = align;
	}
}

int   ListBox::GetColumnSort(int index)
{
	if (index >= 0 && index < columns.size())
	return columns[index]->sort;

	return 0;
}

void  ListBox::SetColumnSort(int index, int sort)
{
	if (index >= 0 && index < columns.size()) {
		columns[index]->sort = sort;
	}
}

Color ListBox::GetColumnColor(int index)
{
	if (index >= 0 && index < columns.size())
	return columns[index]->color;

	return Color::White;
}

void  ListBox::SetColumnColor(int index, Color c)
{
	if (index >= 0 && index < columns.size()) {
		columns[index]->color = c;
		columns[index]->use_color = true;
	}
}

Color ListBox::GetItemColor(int index, int column)
{
	Color c = Color::White;

	if (index >= 0 && index < items.size())
	c = items[index]->color;

	if (column >= 0 && column < columns.size()) {
		if (columns[column]->use_color)
		c = columns[column]->color;
	}

	return c;
}

int   ListBox::GetMultiSelect()
{
	return multiselect;
}

void  ListBox::SetMultiSelect(int nNewValue)
{
	if (multiselect != nNewValue && (nNewValue == 0 || nNewValue == 1)) {
		multiselect = nNewValue;
		ClearSelection();
	}
}

bool  ListBox::GetShowHeadings()
{
	return show_headings;
}

void  ListBox::SetShowHeadings(bool nNewValue)
{
	if (show_headings != nNewValue) {
		show_headings = nNewValue;
	}
}

Color ListBox::GetSelectedColor()
{
	return selected_color;
}

void  ListBox::SetSelectedColor(Color c)
{
	if (selected_color != c) {
		selected_color = c;
	}
}

int   ListBox::GetItemStyle() const
{
	return item_style;
}

void  ListBox::SetItemStyle(int style)
{
	if (style >= LIST_ITEM_STYLE_PLAIN && style <= LIST_ITEM_STYLE_FILLED_BOX) {
		item_style = style;
	}
}

int   ListBox::GetSelectedStyle() const
{
	return seln_style;
}

void  ListBox::SetSelectedStyle(int style)
{
	if (style >= LIST_ITEM_STYLE_PLAIN && style <= LIST_ITEM_STYLE_FILLED_BOX) {
		seln_style = style;
	}
}

bool  ListBox::IsSelected(int index)
{
	if (index >= 0 && index < items.size())
	return items[index]->selected;

	return false;
}

void  ListBox::SetSelected(int index, bool bNewValue)
{
	if (index >= 0 && index < items.size()) {
		if (!multiselect)
		ClearSelection();

		if (items[index]->selected != bNewValue) {
			items[index]->selected = bNewValue;

			if (bNewValue) {
				list_index = index;
				selcount++;
			}
			else {
				selcount--;
			}
		}
	}
}

void  ListBox::ClearSelection()
{
	ListIter<ListBoxItem> item = items;
	while (++item)
	item->selected = false;

	selcount = 0;
}

int   ListBox::GetListIndex()
{
	return list_index;
}

int   ListBox::GetLineCount()
{
	line_count = items.size();
	return line_count;
}

int   ListBox::GetSelCount()
{
	return selcount;
}

int   ListBox::GetSelection()
{
	for (int i = 0; i < items.size(); i++)
	if (items[i]->selected)
	return i;

	return -1;
}

Text  ListBox::GetSelectedItem()
{
	int n = GetSelection();
	return GetItemText(n);
}

// +--------------------------------------------------------------------+

int   ListBox::GetSortColumn()
{
	return sort_column;
}

void  ListBox::SetSortColumn(int col_index)
{
	if (col_index >= 0 && col_index <= columns.size())
	sort_column = col_index;
}

int   ListBox::GetSortCriteria()
{
	return GetColumnSort(sort_column);
}

void  ListBox::SetSortCriteria(SORT sort)
{
	SetColumnSort(sort_column, sort);
}

// +--------------------------------------------------------------------+

void  ListBox::SortItems()
{
	if (sort_column >=0 && sort_column <= columns.size())
	items.sort();
}

// +--------------------------------------------------------------------+

int ListBox::IndexFromPoint(int x, int y) const
{
	int sel_index = -1;

	if (show_headings)
	sel_index = top_index + (y - (line_height+HEADING_EXTRA)) / (line_height + leading);

	else
	sel_index = top_index + y / (line_height + leading);

	return sel_index;
}

// +--------------------------------------------------------------------+

int ListBox::OnMouseMove(int x, int y)
{
	bool dirty = false;

	if (captured) {
		ActiveWindow* test = GetCapture();

		if (test != this) {
			captured = false;
			dirty = true;
		}

		else {
			if (selecting && !dragging) {
				if (dragdrop && (x < rect.x        ||
							x > rect.x+rect.w ||
							y < rect.y        ||
							y > rect.y+rect.h)) {

					dragging = true;
					OnDragStart(x,y);
				}
			}

			if (scrolling == SCROLL_THUMB) {
				mouse_y = y - rect.y - TRACK_START;

				int dest = (int) ((double) mouse_y/track_length * (items.size()-1));
				ScrollTo(dest);
				dirty = true;
			}
		}
	}

	return ActiveWindow::OnMouseMove(x,y);
}

// +--------------------------------------------------------------------+

static bool preselected = false;

int ListBox::OnLButtonDown(int x, int y)
{
	if (!captured)
	captured = SetCapture();

	mouse_x = x - rect.x;
	mouse_y = y - rect.y;

	int x_scroll_bar = rect.w;

	if (IsScrollVisible())
	x_scroll_bar -= SCROLL_WIDTH;

	if (mouse_x < x_scroll_bar) {
		scrolling = SCROLL_NONE;

		if (show_headings && mouse_y < line_height+BORDER_WIDTH+EXTRA_WIDTH) {
			int next_col_start = 0;
			int max_column = columns.size()-1;
			int column;
			
			for (column = 0; column < max_column; column++) {
				next_col_start += GetColumnWidth(column);

				if (mouse_x < next_col_start)
				break;
			}

			sort_column = column;

			int& sort_criteria = columns[sort_column]->sort;

			if (sort_criteria != LIST_SORT_NEVER) {
				if (!sort_criteria)
				sort_criteria = LIST_SORT_ALPHA_DESCENDING;
				else
				sort_criteria = -sort_criteria;

				SortItems();
			}
		}

		else {
			selecting = true;
		}
	}

	else {
		selecting = false;

		if (mouse_y < TRACK_START) {
			scrolling = SCROLL_UP;
			Scroll(scrolling, 1);
			Button::PlaySound(Button::SND_LIST_SCROLL);
		}

		else if (mouse_y > rect.h-TRACK_START) {
			scrolling = SCROLL_DOWN;
			if (top_index < items.size()-1)
			top_index++;
			Button::PlaySound(Button::SND_LIST_SCROLL);
		}

		else if (mouse_y < thumb_pos) {
			scrolling = SCROLL_PAGE_UP;
			Scroll(scrolling, page_size);
			Button::PlaySound(Button::SND_LIST_SCROLL);
		}

		else if (mouse_y > thumb_pos+THUMB_HEIGHT) {
			scrolling = SCROLL_PAGE_DOWN;
			Scroll(scrolling, page_size);
			Button::PlaySound(Button::SND_LIST_SCROLL);
		}

		else {
			scrolling = SCROLL_THUMB;
		}
	}

	if (selecting) {
		list_index  = IndexFromPoint(mouse_x, mouse_y);
		preselected = IsSelected(list_index);
		if (!multiselect || !Keyboard::KeyDown(VK_SHIFT))
		ClearSelection();
		SetSelected(list_index);
		EnsureVisible(list_index);
		Button::PlaySound(Button::SND_LIST_SELECT);
	}

	return ActiveWindow::OnLButtonDown(x,y);
}

// +--------------------------------------------------------------------+

int ListBox::OnLButtonUp(int x, int y)
{
	if (captured) {
		mouse_x = x-rect.x;
		mouse_y = y-rect.y;

		if (dragging) {
			if (mouse_x < 0 || mouse_x > rect.w || mouse_y < 0 || mouse_y > rect.h) {
				FormWindow* parent_form = (FormWindow*) form;

				if (parent_form) {
					ActiveWindow* drop_target = parent_form->FindControl(x,y);

					if (drop_target && drop_target->IsEnabled() && drop_target->IsShown())
					drop_target->OnDragDrop(x,y,this);
				}
			}
		}
		else if (preselected) {
			if (multiselect && Keyboard::KeyDown(VK_SHIFT)) {
				SetSelected(list_index, false);
				Button::PlaySound(Button::SND_LIST_SELECT);
			}
		}

		ReleaseCapture();
		captured = false;

		Mouse::SetCursor((Mouse::CURSOR) old_cursor);
	}

	dragging    = false;
	selecting   = false;

	return ActiveWindow::OnLButtonUp(x,y);
}

// +--------------------------------------------------------------------+

int ListBox::OnMouseWheel(int wheel)
{
	return ScrollWindow::OnMouseWheel(wheel);
}

// +--------------------------------------------------------------------+

int ListBox::OnClick()
{
	int fire_select = !scrolling;

	if (scrolling == SCROLL_THUMB)
	scrolling = SCROLL_NONE;

	if (fire_select)
	return ActiveWindow::OnSelect();
	else
	return ActiveWindow::OnClick();

	return 0;
}

// +--------------------------------------------------------------------+

int ListBox::OnKeyDown(int vk, int flags)
{
	if (selcount == 1 && list_index >= 0 && list_index < items.size()) {
		ListBoxItem* item = items[list_index];

		if (vk == VK_DOWN) {
			if (list_index < items.size() - 1) {
				item->selected = false;
				list_index++;
				item = items[list_index];
				item->selected = true;
				OnClick();
				return ActiveWindow::OnKeyDown(vk, flags);
			}
		}

		else if (vk == VK_UP) {
			if (list_index > 0) {
				item->selected = false;
				list_index--;
				item = items[list_index];
				item->selected = true;
				OnClick();
				return ActiveWindow::OnKeyDown(vk, flags);
			}
		}
	}

	return ScrollWindow::OnKeyDown(vk, flags);
}

// +--------------------------------------------------------------------+

int ListBox::OnDragStart(int x, int y)
{
	old_cursor = Mouse::SetCursor(Mouse::DRAG);
	return ActiveWindow::OnDragStart(x,y);
}

// +--------------------------------------------------------------------+

int ListBox::OnDragDrop(int x, int y, ActiveWindow* source)
{
	if (!dragdrop) return 0;

	ListBox* drag_source = (ListBox*) source;

	if (drag_source) {
		int max_col = NumColumns();

		if (max_col != drag_source->NumColumns())
		max_col = 0;

		for (int i = 0; i < drag_source->NumItems(); i++) {
			if (drag_source->IsSelected(i)) {
				AddItemWithData(drag_source->GetItemText(i),
				drag_source->GetItemData(i));

				for (int c = 1; c < max_col; c++) {
					SetItemText(list_index, c, drag_source->GetItemText(i,c));
					SetItemData(list_index, c, drag_source->GetItemData(i,c));
				}

				if (!multiselect)
				ClearSelection();

				items[list_index]->selected = true;
				selcount++;
			}
		}

		drag_source->RemoveSelectedItems();
		Button::PlaySound(Button::SND_LIST_DROP);
	}

	return ActiveWindow::OnDragDrop(x,y,source);
}
