/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

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

