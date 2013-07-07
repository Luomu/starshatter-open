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
    FILE:         EditBox.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    EditBox ActiveWindow class
*/

#ifndef EditBox_h
#define EditBox_h

#include "Types.h"
#include "ScrollWindow.h"

// +--------------------------------------------------------------------+

class EditBox : public ScrollWindow
{
public:
    static const char* TYPENAME() { return "EditBox"; }

    enum ALIGN  {  EDIT_ALIGN_LEFT   = DT_LEFT,
        EDIT_ALIGN_CENTER = DT_CENTER,
        EDIT_ALIGN_RIGHT  = DT_RIGHT
    };

    EditBox(ActiveWindow* p, int ax, int ay, int aw, int ah, DWORD aid);
    EditBox(Screen* s,       int ax, int ay, int aw, int ah, DWORD aid);
    virtual ~EditBox();

    // Operations:
    virtual void   DrawContent(const Rect& ctrl_rect);
    virtual void   DrawTabbedText();

    // Event Target Interface:
    virtual void   SetFocus();
    virtual void   KillFocus();

    virtual int    OnMouseMove(int x, int y);
    virtual int    OnLButtonDown(int x, int y);
    virtual int    OnLButtonUp(int x, int y);
    virtual int    OnClick();

    virtual int    OnKeyDown(int vk, int flags);

    // ScrollWindow Interface:
    virtual bool   CanScroll(int direction, int nlines=1);
    virtual void   Scroll(int direction, int nlines=1);
    virtual void   ScrollTo(int index);
    virtual int    GetPageCount();
    virtual int    GetPageSize();

    Color          GetSelectedColor();
    void           SetSelectedColor(Color c);

    int            GetSelStart()        { return sel_start;  }
    int            GetSelLength()       { return sel_length; }
    Text           GetSelText();

    void           SetSelStart(int n);
    void           SetSelLength(int n);

    char           GetPasswordChar()       { return pass_char;  }
    void           SetPasswordChar(char c) { pass_char = c;     }

protected:
    void           Insert(char c);
    void           Insert(const char* s);
    void           Delete();
    void           Backspace();
    int            CaretFromPoint(int x, int y) const;
    void           EnsureCaretVisible();

    int            sel_start;
    int            sel_length;
    int            h_offset;
    int            caret_x;
    int            caret_y;

    char           pass_char;

    Color          selected_color;
};

#endif EditBox_h

