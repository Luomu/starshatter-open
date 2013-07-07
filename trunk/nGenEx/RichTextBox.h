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
    FILE:         RichTextBox.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Rich Text Window - an HTML-like control
*/

#ifndef RichTextBox_h
#define RichTextBox_h

#include "Types.h"
#include "Color.h"
#include "Bitmap.h"
#include "ScrollWindow.h"
#include "EventTarget.h"
#include "List.h"
#include "Text.h"

// +--------------------------------------------------------------------+

class RichTextBox : public ScrollWindow
{
public:
    static const char* TYPENAME() { return "RichTextBox"; }

    RichTextBox(ActiveWindow* p, int ax, int ay, int aw, int ah, DWORD aid=0, DWORD astyle=0);
    RichTextBox(Screen*       s, int ax, int ay, int aw, int ah, DWORD aid=0, DWORD astyle=0);
    virtual ~RichTextBox();

    int operator == (const RichTextBox& w) const { return id == w.id; }

    // Operations:
    virtual void   DrawContent(const Rect& ctrl_rect);
    virtual void   SetText(const char* t);

    // Event Target Interface:
    virtual int    OnMouseMove(int x, int y);
    virtual int    OnLButtonDown(int x, int y);
    virtual int    OnLButtonUp(int x, int y);
    virtual int    OnMouseWheel(int wheel);
    virtual int    OnClick();

    virtual int    OnKeyDown(int vk, int flags);

protected:
    virtual void   DrawTabbedText();
    virtual void   DrawRichText(Rect& text_rect);
    int            GetNextTab(int xpos);

    virtual int    find_next_word_start(const char* text, int index);
    virtual int    find_next_word_end(const char* text, int index);
    virtual int    parse_hex_digit(char c);
    virtual int    process_tag(const char* text, int index, Font*& font);

};

#endif RichTextBox_h

