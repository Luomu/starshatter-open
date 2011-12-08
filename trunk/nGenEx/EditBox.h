/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

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

