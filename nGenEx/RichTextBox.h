/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

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

