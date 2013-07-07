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
    FILE:         ScrollWindow.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    ScrollWindow base class for List, Edit, and Rich Text controls
*/

#ifndef ScrollWindow_h
#define ScrollWindow_h

#include "Types.h"
#include "ActiveWindow.h"
#include "List.h"

// +--------------------------------------------------------------------+

class ScrollWindow : public ActiveWindow
{
public:
    enum POLICY {  SCROLL_NEVER,
        SCROLL_AUTO,
        SCROLL_ALWAYS
    };

    enum SCROLL {  SCROLL_NONE,
        SCROLL_UP,
        SCROLL_PAGE_UP,
        SCROLL_DOWN,
        SCROLL_PAGE_DOWN,
        SCROLL_THUMB
    };

    enum MISC   {  BORDER_WIDTH  = 2,
        EXTRA_WIDTH   = 4,
        SCROLL_WIDTH  = 16,
        SCROLL_HEIGHT = 6,
        SCROLL_TRACK  = SCROLL_WIDTH + 1,
        TRACK_START   = BORDER_WIDTH + SCROLL_HEIGHT,
        THUMB_HEIGHT  = SCROLL_WIDTH,
        HEADING_EXTRA = BORDER_WIDTH + EXTRA_WIDTH
    };

    ScrollWindow(ActiveWindow* p, int ax, int ay, int aw, int ah, DWORD aid, DWORD style=0, ActiveWindow* parent=0);
    ScrollWindow(Screen* s,       int ax, int ay, int aw, int ah, DWORD aid, DWORD style=0, ActiveWindow* parent=0);
    virtual ~ScrollWindow();

    // Operations:
    virtual void   Paint();
    virtual void   Draw();
    virtual void   DrawTransparent();
    virtual void   DrawContent(const Rect& ctrl_rect);
    virtual void   DrawTransparentContent(const Rect& ctrl_rect);
    virtual void   DrawScrollBar();
    virtual void   MoveTo(const Rect& r);

    // Event Target Interface:
    virtual int    OnMouseMove(int x, int y);
    virtual int    OnLButtonDown(int x, int y);
    virtual int    OnLButtonUp(int x, int y);
    virtual int    OnMouseWheel(int wheel);
    virtual int    OnClick();

    virtual int    OnKeyDown(int vk, int flags);

    // pseudo-events:
    virtual int    OnDragStart(int x, int y);
    virtual int    OnDragDrop(int x, int y, ActiveWindow* source);

    // Property accessors:
    virtual int    GetLineHeight();
    virtual void   SetLineHeight(int h);

    virtual int    GetLeading();
    virtual void   SetLeading(int nNewValue);
    virtual int    GetScrollBarVisible();
    virtual void   SetScrollBarVisible(int nNewValue);
    virtual int    GetDragDrop();
    virtual void   SetDragDrop(int nNewValue);
    virtual bool   GetSmoothScroll();
    virtual void   SetSmoothScroll(bool s);

    virtual bool   IsScrollVisible();
    virtual bool   CanScroll(int direction, int nlines=1);
    virtual void   EnsureVisible(int index);
    virtual void   Scroll(int direction, int nlines=1);
    virtual void   SmoothScroll(int direction, double nlines);
    virtual void   ScrollTo(int index);

    // read-only:
    virtual int    GetTopIndex();
    virtual int    GetLineCount();
    virtual int    GetPageCount();
    virtual int    GetPageSize();
    virtual int    GetScrollTrack();

    int            IsDragging()   const { return dragging;   }
    int            IsSelecting()  const { return selecting;  }
    int            IsScrolling()  const { return scrolling;  }

protected:
    int            captured;
    int            dragging;
    int            selecting;
    int            scrolling;
    int            scroll_count;
    int            mouse_x;
    int            mouse_y;
    int            track_length;
    int            thumb_pos;

    int            leading;
    int            scroll_bar;
    int            dragdrop;
    int            line_count;
    int            page_count;
    int            page_size;
    int            top_index;
    int            line_height;

    bool           smooth_scroll;
    double         smooth_offset;
};

#endif ScrollWindow_h

