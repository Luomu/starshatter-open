/*  Starshatter OpenSource Distribution
    Copyright (c) 1997-2004, Destroyer Studios LLC.
    All Rights Reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.
    * Neither the name "Destroyer Studios" nor the names of its contributors
      may be used to endorse or promote products derived from this software
      without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

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

