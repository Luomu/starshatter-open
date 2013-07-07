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
    FILE:         ScrollWindow.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    ScrollWindow ActiveWindow class
*/

#include "MemDebug.h"
#include "ScrollWindow.h"
#include "Button.h"
#include "Bitmap.h"
#include "FormWindow.h"
#include "Video.h"
#include "Font.h"
#include "Keyboard.h"
#include "Mouse.h"


DWORD GetRealTime();

// +--------------------------------------------------------------------+

static int old_cursor;

// +--------------------------------------------------------------------+

ScrollWindow::ScrollWindow(ActiveWindow* p, int ax, int ay, int aw, int ah, DWORD aid, DWORD s, ActiveWindow* paw)
: ActiveWindow(p->GetScreen(), ax, ay, aw, ah, aid, s, paw)
{
    captured          = false;
    dragging          = false;
    selecting         = false;
    scrolling         = SCROLL_NONE;

    leading           = 0;
    scroll_bar        = SCROLL_AUTO;
    dragdrop          = false;
    scroll_count      = 0;
    line_count        = 0;
    page_count        = 0;
    page_size         = 1;
    top_index         = 0;
    line_height       = 0;
    mouse_x           = 0;
    mouse_y           = 0;

    smooth_scroll     = false;
    smooth_offset     = 0;

    track_length      = ah - 2*TRACK_START - THUMB_HEIGHT;
    thumb_pos         = TRACK_START;

    char buf[32];
    sprintf_s(buf, "ScrollWindow %d", id);
    desc = buf;
}

ScrollWindow::ScrollWindow(Screen* s, int ax, int ay, int aw, int ah, DWORD aid, DWORD s1, ActiveWindow* paw)
: ActiveWindow(s, ax, ay, aw, ah, aid, s1, paw)
{
    captured          = false;
    dragging          = false;
    selecting         = false;
    scrolling         = SCROLL_NONE;

    leading           = 0;
    scroll_bar        = SCROLL_AUTO;
    dragdrop          = false;
    scroll_count      = 0;
    line_count        = 0;
    page_count        = 0;
    page_size         = 1;
    top_index         = 0;
    line_height       = 0;
    mouse_x           = 0;
    mouse_y           = 0;

    smooth_scroll     = false;
    smooth_offset     = 0;

    track_length      = ah - 2*TRACK_START - THUMB_HEIGHT;
    thumb_pos         = TRACK_START;

    char buf[32];
    sprintf_s(buf, "ScrollWindow %d", id);
    desc = buf;
}

ScrollWindow::~ScrollWindow()
{ }

// +--------------------------------------------------------------------+

void
ScrollWindow::MoveTo(const Rect& r)
{
    ActiveWindow::MoveTo(r);

    track_length      = rect.h - 2*TRACK_START - THUMB_HEIGHT;
    thumb_pos         = TRACK_START;
}

// +--------------------------------------------------------------------+

void
ScrollWindow::Paint()
{
    if (transparent) {
        DrawTransparent();
    }

    else {
        ActiveWindow::Paint();
    }
}

// +--------------------------------------------------------------------+

void
ScrollWindow::Draw()
{
    int x = 0;
    int y = 0;
    int w = rect.w;
    int h = rect.h;

    if (w < 1 || h < 1 || !shown)
    return;

    ActiveWindow::Draw();

    if (line_height < 1)
    line_height = GetFont()->Height();
    page_size = h / (line_height + leading);

    Rect ctrl_rect(x,y,w,h);
    ctrl_rect.Deflate(BORDER_WIDTH, BORDER_WIDTH);

    if (IsScrollVisible()) {
        ctrl_rect.w -= SCROLL_TRACK;
    }

    DrawContent(ctrl_rect);
    DrawScrollBar();
}

// +--------------------------------------------------------------------+

void
ScrollWindow::DrawTransparent()
{
    int x = 0;
    int y = 0;
    int w = rect.w;
    int h = rect.h;

    if (w < 1 || h < 1 || !shown)
    return;

    if (line_height < 1)
    line_height = GetFont()->Height();
    page_size = h / (line_height + leading);

    Rect ctrl_rect(x,y,w,h);
    ctrl_rect.Deflate(BORDER_WIDTH, BORDER_WIDTH);

    if (IsScrollVisible()) {
        ctrl_rect.w -= SCROLL_TRACK;
    }

    DrawTransparentContent(ctrl_rect);
    DrawScrollBar();
}

// +--------------------------------------------------------------------+

void
ScrollWindow::DrawContent(const Rect& ctrl_rect)
{
    // override to do control-specific drawing
}

void
ScrollWindow::DrawTransparentContent(const Rect& ctrl_rect)
{
    // override (if necessary) to do control-specific drawing
    DrawContent(ctrl_rect);
}

void
ScrollWindow::DrawScrollBar()
{
    // draw scroll bar if necessary:
    if (IsScrollVisible()) {
        Color save_color = back_color;
        back_color = ShadeColor(back_color, 1.3);

        // draw scroll track border:
        DrawLine(rect.w-SCROLL_TRACK, BORDER_WIDTH, rect.w-SCROLL_TRACK, rect.h-BORDER_WIDTH, back_color);

        // draw top button
        Rect btn_rect(rect.w-SCROLL_WIDTH, BORDER_WIDTH, SCROLL_WIDTH-BORDER_WIDTH, SCROLL_HEIGHT);
        FillRect(btn_rect, back_color);
        DrawStyleRect(btn_rect, WIN_RAISED_FRAME);

        // draw bottom button:
        btn_rect.y = rect.h - (SCROLL_HEIGHT+BORDER_WIDTH);
        FillRect(btn_rect, back_color);
        DrawStyleRect(btn_rect, WIN_RAISED_FRAME);

        // draw thumb:
        btn_rect.y = thumb_pos;
        btn_rect.h = btn_rect.w;
        FillRect(btn_rect, back_color);
        DrawStyleRect(btn_rect, WIN_RAISED_FRAME);

        back_color = save_color;
    }

    if (scrolling && scroll_count)
    Scroll(scrolling, scroll_count);
}

// +--------------------------------------------------------------------+

bool
ScrollWindow::IsScrollVisible()
{
    bool vis = false;

    if (scroll_bar == SCROLL_ALWAYS ||
            (scroll_bar == SCROLL_AUTO   &&
                line_count > page_size)) {
        vis = true;
    }

    return vis;
}

int   ScrollWindow::GetLineHeight()
{
    return line_height;
}

void  ScrollWindow::SetLineHeight(int h)
{
    if (h >= 0)
    line_height = h;
}

int   ScrollWindow::GetLeading()
{
    return leading;
}

void  ScrollWindow::SetLeading(int nNewValue)
{
    if (leading != nNewValue && nNewValue >= 0) {
        leading = nNewValue;
    }
}

int   ScrollWindow::GetDragDrop()
{
    return dragdrop;
}

void  ScrollWindow::SetDragDrop(int nNewValue)
{
    if (dragdrop != nNewValue && (nNewValue == 0 || nNewValue == 1)) {
        dragdrop = nNewValue;
    }
}

int   ScrollWindow::GetScrollBarVisible()
{
    return scroll_bar;
}

void  ScrollWindow::SetScrollBarVisible(int nNewValue)
{
    if (scroll_bar != nNewValue) {
        scroll_bar = nNewValue;
    }
}

bool  ScrollWindow::GetSmoothScroll()
{
    return smooth_scroll;
}

void  ScrollWindow::SetSmoothScroll(bool bNewValue)
{
    if (smooth_scroll != bNewValue) {
        smooth_scroll = bNewValue;
        smooth_offset = top_index;
    }
}

bool  ScrollWindow::CanScroll(int direction, int nlines)
{
    return false;
}

void  ScrollWindow::EnsureVisible(int index)
{
    if (index < top_index)
    ScrollTo(index);

    else if (index > top_index+page_size)
    ScrollTo(index-page_size);
}

void  ScrollWindow::Scroll(int direction, int nlines)
{
    if (nlines) {
        scrolling = direction;

        if (direction == SCROLL_UP || direction == SCROLL_PAGE_UP) {
            top_index--;

            if (top_index < 0)
            top_index = 0;

            else
            scroll_count = nlines-1;
        }

        else if (direction == SCROLL_DOWN || direction == SCROLL_PAGE_DOWN) {
            top_index++;

            if (top_index >= line_count)
            top_index = line_count-1;

            else
            scroll_count = nlines-1;
        }

        smooth_offset = top_index;
        thumb_pos = TRACK_START + (int) (track_length * (double) top_index/(line_count-1));

        if (scroll_count < 1)
        scrolling = SCROLL_NONE;
    }
}

void  ScrollWindow::SmoothScroll(int direction, double nlines)
{
    if (!smooth_scroll) {
        Scroll(direction, (int) nlines);
        return;
    }

    if (direction == SCROLL_UP || direction == SCROLL_PAGE_UP) {
        smooth_offset -= nlines;

        if (smooth_offset < 0)
        smooth_offset = 0;
    }

    else if (direction == SCROLL_DOWN || direction == SCROLL_PAGE_DOWN) {
        smooth_offset += nlines;

        if (smooth_offset >= line_count)
        smooth_offset = line_count-1;
    }

    top_index = (int) smooth_offset;
    thumb_pos = TRACK_START + (int) (track_length * smooth_offset/(line_count-1));
    scrolling = SCROLL_NONE;
}

void  ScrollWindow::ScrollTo(int index)
{
    if (index >= 0 && index < line_count) {
        top_index     = index;
        smooth_offset = index;

        thumb_pos = TRACK_START + (int) (track_length * smooth_offset/(line_count-1));
    }
}

int   ScrollWindow::GetTopIndex()
{
    return top_index;
}

int   ScrollWindow::GetPageCount()
{
    return line_count / GetPageSize();
}

int   ScrollWindow::GetPageSize()
{
    return page_size;
}

int   ScrollWindow::GetScrollTrack()
{
    return rect.w-SCROLL_TRACK;
}

int   ScrollWindow::GetLineCount()
{
    return line_count;
}

// +--------------------------------------------------------------------+

int ScrollWindow::OnMouseMove(int x, int y)
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

                int dest = (int) ((double) mouse_y/track_length * (line_count-1));
                ScrollTo(dest);
                dirty = true;
            }
        }
    }

    return ActiveWindow::OnMouseMove(x,y);
}

// +--------------------------------------------------------------------+

int ScrollWindow::OnLButtonDown(int x, int y)
{
    if (!captured)
    captured = SetCapture();

    mouse_x = x - rect.x;
    mouse_y = y - rect.y;

    int x_scroll_bar = rect.w;

    if (scroll_bar == SCROLL_ALWAYS ||
            (scroll_bar == SCROLL_AUTO   &&
                line_count > page_size))
    x_scroll_bar -= SCROLL_WIDTH;

    if (mouse_x < x_scroll_bar) {
        scrolling = SCROLL_NONE;
        selecting = true;
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
            if (top_index < line_count-1)
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

    return ActiveWindow::OnLButtonDown(x,y);
}

// +--------------------------------------------------------------------+

int ScrollWindow::OnLButtonUp(int x, int y)
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

        ReleaseCapture();
        captured = false;

        Mouse::SetCursor((Mouse::CURSOR) old_cursor);
    }

    dragging    = false;
    selecting   = false;

    return ActiveWindow::OnLButtonUp(x,y);
}

// +--------------------------------------------------------------------+

int ScrollWindow::OnMouseWheel(int wheel)
{
    if (wheel > 0)
    Scroll(SCROLL_UP, 1);

    else if (wheel < 0)
    Scroll(SCROLL_DOWN, 1);

    if (GetLineCount() > 0) {
        static double scroll_time = 0;

        if (GetRealTime() - scroll_time > 0.5) {
            scroll_time = GetRealTime();
            Button::PlaySound(Button::SND_LIST_SCROLL);
        }
    }

    return ActiveWindow::OnMouseWheel(wheel);
}

// +--------------------------------------------------------------------+

int ScrollWindow::OnClick()
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

int ScrollWindow::OnKeyDown(int vk, int flags)
{
    switch (vk) {
    case VK_UP:    Scroll(SCROLL_UP,    1);
        Button::PlaySound(Button::SND_LIST_SCROLL);
        break;

    case VK_DOWN:  Scroll(SCROLL_DOWN,  1);
        Button::PlaySound(Button::SND_LIST_SCROLL);
        break;

    case VK_PRIOR: Scroll(SCROLL_UP,    page_count);
        Button::PlaySound(Button::SND_LIST_SCROLL);
        break;

    case VK_NEXT:  Scroll(SCROLL_DOWN,  page_count);
        Button::PlaySound(Button::SND_LIST_SCROLL);
        break;

    case VK_HOME:  EnsureVisible(0);
        Button::PlaySound(Button::SND_LIST_SCROLL);
        break;

    case VK_END:   EnsureVisible(line_count-1);
        Button::PlaySound(Button::SND_LIST_SCROLL);
        break;

    default: break;
    }

    return ActiveWindow::OnKeyDown(vk, flags);
}

// +--------------------------------------------------------------------+

int ScrollWindow::OnDragStart(int x, int y)
{
    old_cursor = Mouse::SetCursor(Mouse::DRAG);
    return ActiveWindow::OnDragStart(x,y);
}

// +--------------------------------------------------------------------+

int ScrollWindow::OnDragDrop(int x, int y, ActiveWindow* source)
{
    return ActiveWindow::OnDragDrop(x,y,source);
}
