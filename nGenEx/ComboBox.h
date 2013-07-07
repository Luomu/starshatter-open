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

