/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    nGenEx.lib
    FILE:         EditBox.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    EditBox ActiveWindow class
*/

#include "MemDebug.h"
#include "EditBox.h"
#include "FormWindow.h"
#include "Video.h"
#include "Font.h"
#include "Keyboard.h"
#include "Mouse.h"

DWORD GetRealTime();

// +--------------------------------------------------------------------+

static int old_cursor;

// +--------------------------------------------------------------------+

EditBox::EditBox(ActiveWindow* p, int ax, int ay, int aw, int ah, DWORD aid)
   : ScrollWindow(p->GetScreen(), ax, ay, aw, ah, aid, 0, p), caret_x(0), caret_y(0)
{
   sel_start         = 0;
   sel_length        = 0;
   h_offset          = 0;
   pass_char         = 0;

   selected_color    = Color::Yellow;

   char buf[32];
   sprintf_s(buf, "EditBox %d", id);
   desc = buf;
}

EditBox::EditBox(Screen* s, int ax, int ay, int aw, int ah, DWORD aid)
   : ScrollWindow(s, ax, ay, aw, ah, aid), caret_x(0), caret_y(0)
{
   sel_start         = 0;
   sel_length        = 0;
   h_offset          = 0;
   pass_char         = 0;

   selected_color    = Color::Yellow;

   char buf[32];
   sprintf_s(buf, "EditBox %d", id);
   desc = buf;
}

EditBox::~EditBox()
{
}

// +--------------------------------------------------------------------+

void
EditBox::DrawContent(const Rect& ctrl_rect)
{
   int h = rect.h;

   if (line_height < 1)
      line_height = GetFont()->Height();
   page_size = h / (line_height + leading);
}

// +--------------------------------------------------------------------+

void
EditBox::DrawTabbedText()
{
   if (font && text.length()) {
      int border_size = 4;

      if (style & WIN_RAISED_FRAME && style & WIN_SUNK_FRAME)
         border_size = 8;

      Rect label_rect = rect;

      label_rect.x  = border_size;
      label_rect.y  = border_size;
      label_rect.w -= border_size * 2;
      label_rect.h -= border_size * 2;

      if (focus)
         font->SetCaretIndex(sel_start);

      // if displaying in password mode, create a display string
      // containing the proper number of password chars:
      Text s = text;
      if (pass_char)
         s = Text(pass_char, text.length());

      // no tabs set:
      if (tab[0] == 0) {
         DWORD text_flags = DT_WORDBREAK | text_align;

         if (single_line)
            text_flags = text_flags | DT_SINGLELINE;

         if (style & WIN_TEXT_SHADOW) {
            label_rect.x++;
            label_rect.y++;
         
            font->SetColor(back_color);
            DrawText(s.data() + h_offset, 0, label_rect, text_flags);
         
            label_rect.x--;
            label_rect.y--;
         }
      
         font->SetColor(fore_color);
         DrawText(s.data() + h_offset, 0, label_rect, text_flags);
      }
   
      // use tabs:
      else {
      }

      font->SetCaretIndex(-1);
  }
   else {
      caret_x = 2;
      caret_y = 3;
   }
}

// +--------------------------------------------------------------------+

Color EditBox::GetSelectedColor()
{
   return selected_color;
}

void  EditBox::SetSelectedColor(Color c)
{
   if (selected_color != c) {
      selected_color = c;
   }
}

Text  EditBox::GetSelText()
{
   if (sel_start == 0 && sel_length == text.length())
      return text;

   Text selection;
   char* buf = new(__FILE__,__LINE__) char[sel_length+1];

   if (buf) {
      for (int i = 0; i < sel_length; i++)
         buf[i] = text[(int) (sel_start + i)];

      buf[sel_length] = 0;

      selection = buf;
      delete [] buf;
   }

   return selection;
}

void  EditBox::SetSelStart(int n)
{
   if (n >= 0 && n <= text.length())
      sel_start = n;
}

void  EditBox::SetSelLength(int n)
{
   if (n <= text.length() - sel_start)
      sel_length = n;
}

void  EditBox::EnsureCaretVisible()
{
   if (!single_line) {
      h_offset = 0;
      return;
   }

   if (sel_start < 0) {
      sel_start = 0;
      h_offset  = 0;
   }

   else if (sel_start > h_offset) {
      int   x_caret;
      bool  moved;

      do {
         x_caret = 0;
         moved   = false;

         if (pass_char) {
            Text pass = Text(pass_char, sel_start-h_offset);
            x_caret += font->StringWidth(pass.data(), pass.length());
         }
         else {
            Text sub = text.substring(h_offset, sel_start-h_offset);
            x_caret += font->StringWidth(sub.data(), sub.length());
         }

         if (x_caret >= Width()-4) {
            if (h_offset < text.length()) {
               h_offset++;
               moved = true;
            }
         }
      }
      while (moved);
   }

   else {
      h_offset = sel_start;
   }
}

bool  EditBox::CanScroll(int dir, int nlines)
{
   return false;
}

void  EditBox::Scroll(int direction, int nlines)
{
   scrolling = SCROLL_NONE;
}

void  EditBox::ScrollTo(int index)
{
}

int   EditBox::GetPageCount()
{
   return 1;
}

int   EditBox::GetPageSize()
{
   return page_size;
}

// +--------------------------------------------------------------------+

void EditBox::SetFocus()
{
   ActiveWindow::SetFocus();

   sel_start  = text.length();
   sel_length = 0;
}

void EditBox::KillFocus()
{
   ActiveWindow::KillFocus();
}

// +--------------------------------------------------------------------+

int EditBox::CaretFromPoint(int x, int y) const
{
   return 0;
}

// +--------------------------------------------------------------------+

int EditBox::OnMouseMove(int x, int y)
{
   return ActiveWindow::OnMouseMove(x,y);
}

// +--------------------------------------------------------------------+

int EditBox::OnLButtonDown(int x, int y)
{
   return ActiveWindow::OnLButtonDown(x,y);
}

// +--------------------------------------------------------------------+

int EditBox::OnLButtonUp(int x, int y)
{
   return ActiveWindow::OnLButtonUp(x,y);
}

// +--------------------------------------------------------------------+

int EditBox::OnClick()
{
   int fire_click = !scrolling;

   if (scrolling == SCROLL_THUMB)
      scrolling = SCROLL_NONE;

   if (fire_click)
      return ActiveWindow::OnClick();

   return 0;
}

// +--------------------------------------------------------------------+

void EditBox::Insert(char c)
{
   if (single_line && c == '\n')
      return;

   if (sel_start < text.length()) {
      if (sel_start == 0) {
         text = Text(c) + text;
         sel_start = 1;
      }
      else {
         Text t1 = text.substring(0, sel_start);
         Text t2 = text.substring(sel_start, text.length()-sel_start);
         text = t1 + Text(c) + t2;
         sel_start++;
      }
   }
   else {
      text += c;
      sel_start = text.length();
   }

   EnsureCaretVisible();
}

void EditBox::Insert(const char* s)
{
}

void EditBox::Delete()
{
   if (sel_start < text.length()) {
      if (sel_start == 0) {
         text = text.substring(1, text.length()-1);
      }
      else {
         Text t1 = text.substring(0, sel_start);
         Text t2 = text.substring(sel_start+1, text.length()-sel_start-1);
         text = t1 + t2;
      }
   }

   EnsureCaretVisible();
}

void EditBox::Backspace()
{
   if (sel_start > 0) {
      if (sel_start == text.length()) {
         text = text.substring(0, sel_start-1);
      }
      else {
         Text t1 = text.substring(0, sel_start-1);
         Text t2 = text.substring(sel_start, text.length()-sel_start);
         text = t1 + t2;
      }

      sel_start--;
      EnsureCaretVisible();
   }
}

int EditBox::OnKeyDown(int vk, int flags)
{
   if (vk >= 'A' && vk <= 'Z') {
      if (flags & 1)
         Insert((char) vk);
      else
         Insert((char) tolower(vk));
   }
   else {
      switch (vk) {
      case VK_LEFT:
         if (sel_start > 0) sel_start--;
         EnsureCaretVisible();
         break;

      case VK_RIGHT:
         if (sel_start < text.length()) sel_start++;
         EnsureCaretVisible();
         break;

      case VK_HOME:
         sel_start = 0;
         EnsureCaretVisible();
         break;

      case VK_END:
         sel_start = text.length();
         EnsureCaretVisible();
         break;

      case VK_DELETE:   Delete();      break;
      case VK_BACK:     Backspace();   break;

      case VK_SPACE:    Insert(' ');   break;
      case VK_RETURN:   Insert('\n');  break;

      case VK_NUMPAD0:  Insert('0');   break;
      case VK_NUMPAD1:  Insert('1');   break;
      case VK_NUMPAD2:  Insert('2');   break;
      case VK_NUMPAD3:  Insert('3');   break;
      case VK_NUMPAD4:  Insert('4');   break;
      case VK_NUMPAD5:  Insert('5');   break;
      case VK_NUMPAD6:  Insert('6');   break;
      case VK_NUMPAD7:  Insert('7');   break;
      case VK_NUMPAD8:  Insert('8');   break;
      case VK_NUMPAD9:  Insert('9');   break;
      case VK_DECIMAL:  Insert('.');   break;
      case VK_ADD:      Insert('+');   break;
      case VK_SUBTRACT: Insert('-');   break;
      case VK_MULTIPLY: Insert('*');   break;
      case VK_DIVIDE:   Insert('/');   break;

      case '0': if (flags & 1) Insert(')'); else Insert('0'); break;
      case '1': if (flags & 1) Insert('!'); else Insert('1'); break;
      case '2': if (flags & 1) Insert('@'); else Insert('2'); break;
      case '3': if (flags & 1) Insert('#'); else Insert('3'); break;
      case '4': if (flags & 1) Insert('$'); else Insert('4'); break;
      case '5': if (flags & 1) Insert('%'); else Insert('5'); break;
      case '6': if (flags & 1) Insert('^'); else Insert('6'); break;
      case '7': if (flags & 1) Insert('&'); else Insert('7'); break;
      case '8': if (flags & 1) Insert('*'); else Insert('8'); break;
      case '9': if (flags & 1) Insert('('); else Insert('9'); break;
      case 186: if (flags & 1) Insert(':'); else Insert(';'); break;
      case 187: if (flags & 1) Insert('+'); else Insert('='); break;
      case 188: if (flags & 1) Insert('<'); else Insert(','); break;
      case 189: if (flags & 1) Insert('_'); else Insert('-'); break;
      case 190: if (flags & 1) Insert('>'); else Insert('.'); break;
      case 191: if (flags & 1) Insert('?'); else Insert('/'); break;
      case 192: if (flags & 1) Insert('~'); else Insert('`'); break;
      case 219: if (flags & 1) Insert('{'); else Insert('['); break;
      case 221: if (flags & 1) Insert('}'); else Insert(']'); break;
      case 220: if (flags & 1) Insert('|'); else Insert('\\'); break;
      case 222: if (flags & 1) Insert('"'); else Insert('\''); break;
      }
   }

   return ActiveWindow::OnKeyDown(vk, flags);
}
