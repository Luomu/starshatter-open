/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    nGenEx.lib
    FILE:         RichTextBox.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Window class
*/

#include "MemDebug.h"
#include "RichTextBox.h"
#include "EventDispatch.h"
#include "Color.h"
#include "Bitmap.h"
#include "Font.h"
#include "FontMgr.h"
#include "Mouse.h"
#include "Screen.h"
#include "View.h"

// +--------------------------------------------------------------------+

RichTextBox::RichTextBox(ActiveWindow* p, int ax, int ay, int aw, int ah, DWORD aid, DWORD astyle)
   : ScrollWindow(p->GetScreen(), ax, ay, aw, ah, aid, astyle, p)
{
   leading           = 2;

   char buf[32];
   sprintf(buf, "RichTextBox %d", id);
   desc = buf;
}

RichTextBox::RichTextBox(Screen* screen, int ax, int ay, int aw, int ah, DWORD aid, DWORD astyle)
   : ScrollWindow(screen, ax, ay, aw, ah, aid, astyle)
{
   leading           = 2;

   char buf[32];
   sprintf(buf, "RichTextBox %d", id);
   desc = buf;
}

RichTextBox::~RichTextBox()
{
}

// +--------------------------------------------------------------------+

void
RichTextBox::SetText(const char* t)
{
   ActiveWindow::SetText(t);
   ScrollTo(0);
}

// +--------------------------------------------------------------------+

void
RichTextBox::DrawContent(const Rect& ctrl_rect)
{
   DrawTabbedText();
}

// +--------------------------------------------------------------------+

void
RichTextBox::DrawTabbedText()
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

      if (scroll_bar)
         label_rect.w -= SCROLL_TRACK;

      if (line_height < font->Height())
         line_height = font->Height();

      font->SetColor(fore_color);
      DrawRichText(label_rect);
   }
}

// +--------------------------------------------------------------------+

int RichTextBox::find_next_word_start(const char* text, int index)
{
   // step through intra-word space:
   while (text[index] && isspace(text[index]) && 
         (text[index] != '\t')                && 
         (text[index] != '\n')                && 
         (text[index] != '<'))
      index++;

   return index;
}

int RichTextBox::find_next_word_end(const char* text, int index)
{
   // check for leading newline or tag:
   if (text[index] == '\n' || text[index] == '\t' || text[index] == '<')
      return index;

   // step through intra-word space:
   while (text[index] && isspace(text[index]))
      index++;

   // step through word:
   while (text[index] && !isspace(text[index]) && 
         (text[index] != '-')                  && 
         (text[index] != '<'))
      index++;

   if (index) {
      if (text[index] != '-')
         return index-1;
      else
         return index;
   }

   return 0;
}

int RichTextBox::parse_hex_digit(char c)
{
   if (isalpha(c))
      return 10 + tolower(c) - 'a';

   else if (isdigit(c))
      return c - '0';

   return 0;
}

int RichTextBox::process_tag(const char* text, int index, Font*& font)
{
   if (text[index] == '<') {
      char tag[64];
      int  i = 0;

      while (text[index] && (text[index] != '>'))
         tag[i++] = text[index++];

      if (text[index] == '>')
         tag[i++] = text[index++];

      tag[i] = 0;

      switch (tag[1]) {
      case 'c':
      case 'C':   if (strnicmp(tag+1, "color", 5) == 0) {
                     int r = 0;
                     int g = 0;
                     int b = 0;

                     if (i > 12) {
                        r = 16 * parse_hex_digit(tag[ 7]) + parse_hex_digit(tag[ 8]);
                        g = 16 * parse_hex_digit(tag[ 9]) + parse_hex_digit(tag[10]);
                        b = 16 * parse_hex_digit(tag[11]) + parse_hex_digit(tag[12]);
                     }

                     font->SetColor(Color(r,g,b));
                  }
         break;

      case 'f':
      case 'F':   if (strnicmp(tag+1, "font", 4) == 0) {
                     Color current_color = Color::White;

                     if (font)
                        current_color = font->GetColor();

                     tag[i-1] = 0;
                     font = FontMgr::Find(tag+6);
                     font->SetColor(current_color);
                  }
         break;
      }
   }

   return index;
}

int
RichTextBox::GetNextTab(int xpos)
{
   for (int i = 0; i < 10; i++) {
      if (tab[i] > xpos)
         return tab[i];
   }

   return (xpos / 20) * 20 + 20;
}

void
RichTextBox::DrawRichText(Rect& text_rect)
{
   // clip the rect:
   Rect clip_rect = ClipRect(text_rect);
   clip_rect.h -= 8;

   if (clip_rect.w < 1 || clip_rect.h < 1)
      return;

   const char* t     = text.data();
   int count         = text.length();
   int nlines        = 0;

   int xpos          = 0;
   int block_start   = 0;
   int block_count   = 0;
   int curr_word_end = -1;
   int next_word_end = 0;
   int eol_index     = 0;

   int new_line      = 0;
   int x_offset      = 0;
   int y_offset      = 0;
   int length        = 0;

   Font* rich_font   = font;
   rich_font->SetColor(fore_color);

   if (smooth_scroll) {
      double fraction = smooth_offset - (int) smooth_offset;

      y_offset = (int) ((1-fraction) * (rich_font->Height() + leading));
   }

   // while there is still text:
   while (block_start < count) {
      bool found_tag = false;

      do {
         found_tag = false;

         if (t[block_start] == '<') {
            block_start = process_tag(t, block_start, rich_font);
            found_tag = true;
         }

         else if (t[block_start] == '\t') {
            block_start++;
            x_offset = GetNextTab(x_offset);

            if (x_offset > text_rect.w) {
               nlines++;
               if (nlines > top_index)
                  y_offset += rich_font->Height() + leading;
               x_offset = 0;
               new_line = false;
            }

            found_tag = true;
         }

         else if (t[block_start] == '\r') {
            block_start++;

            if (t[block_start] == '\n')
               block_start++;

            nlines++;
            if (nlines > top_index)
               y_offset += rich_font->Height() + leading;
            x_offset = 0;
            new_line = false;

            found_tag = true;
         }

         else if (t[block_start] == '\n') {
            block_start++;

            if (t[block_start] == '\r')
               block_start++;

            nlines++;
            if (nlines > top_index)
               y_offset += rich_font->Height() + leading;
            x_offset = 0;
            new_line = false;

            found_tag = true;
         }
      }
      while (found_tag);

      next_word_end = find_next_word_end(t, block_start);

      if (!next_word_end || next_word_end == curr_word_end) {
         new_line = true;
      }

      else if (t[next_word_end] == '\n') {
         eol_index = curr_word_end = next_word_end;
         new_line = true;
      }

      else {
         int word_len = next_word_end - block_start + 1;

         length = rich_font->StringWidth(t+block_start, word_len);

         // if this word was too long, wrap to next line:
         if (x_offset + length > text_rect.w) {
            nlines++;
            if (nlines > top_index)
               y_offset += rich_font->Height() + leading;
            x_offset = 0;
            new_line = false;
         }

         // is there a trailing newline?
         curr_word_end = next_word_end;

         // check for a newline in the next block of white space:
         eol_index = 0;
         const char* eol = &t[curr_word_end+1];
         while (*eol && isspace(*eol) && *eol != '\n')
            eol++;

         if (*eol == '\n') {
            eol_index = eol - t;
            new_line = true;
         }
      }

      block_count = curr_word_end - block_start + 1;

      if (block_count > 0) {
         length = rich_font->StringWidth(t+block_start, block_count);
      }

      // there was a single word longer than the entire line:
      else {
         block_count = next_word_end - block_start + 1;
         length = rich_font->StringWidth(t+block_start, block_count);
         curr_word_end = next_word_end;
      }

      if (length > 0 && nlines >= top_index && nlines < top_index+page_size) {
         int x1 = text_rect.x + x_offset + rect.x;
         int y1 = text_rect.y + y_offset + rect.y;

         rich_font->DrawString(t+block_start, block_count, x1, y1, clip_rect);
      }

      if (new_line) {
         nlines++;
         if (nlines > top_index)
            y_offset += rich_font->Height() + leading;
         x_offset = 0;
         new_line = false;
      }

      else if (length < 1 || text[next_word_end] == '-') {
         x_offset += length;
      }

      else {
         x_offset += length + rich_font->SpaceWidth();
      }

      if (eol_index > 0)
         curr_word_end = eol_index;

      block_start = find_next_word_start(t, curr_word_end+1);
   }

   line_count = nlines;
}

// +--------------------------------------------------------------------+

int RichTextBox::OnMouseMove(int x, int y)
{
   if (captured) {
      ActiveWindow* test = GetCapture();

      if (test != this) {
         captured = false;
      }

      else {
         if (scrolling == SCROLL_THUMB) {
            mouse_y = y - rect.y - TRACK_START;

            int dest = (int) ((double) mouse_y/track_length * (line_count-1));
            ScrollTo(dest);
         }
      }
   }

   return ActiveWindow::OnMouseMove(x,y);
}

// +--------------------------------------------------------------------+

int RichTextBox::OnLButtonDown(int x, int y)
{
   return ScrollWindow::OnLButtonDown(x,y);
}

// +--------------------------------------------------------------------+

int RichTextBox::OnLButtonUp(int x, int y)
{
   return ScrollWindow::OnLButtonUp(x,y);
}

// +--------------------------------------------------------------------+

int RichTextBox::OnMouseWheel(int wheel)
{
   return ScrollWindow::OnMouseWheel(wheel);
}

// +--------------------------------------------------------------------+

int RichTextBox::OnClick()
{
   int fire_click = !scrolling;

   if (scrolling == SCROLL_THUMB)
      scrolling = SCROLL_NONE;

   if (fire_click)
      return ActiveWindow::OnClick();

   return 0;
}

// +--------------------------------------------------------------------+

int RichTextBox::OnKeyDown(int vk, int flags)
{
   return ScrollWindow::OnKeyDown(vk, flags);
}

