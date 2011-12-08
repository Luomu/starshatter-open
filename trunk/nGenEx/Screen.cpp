/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    nGenEx.lib
    FILE:         Screen.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    General Screen class - maintains and displays a list of windows
*/

#include "MemDebug.h"
#include "Screen.h"
#include "Bitmap.h"
#include "Color.h"
#include "Window.h"
#include "Mouse.h"
#include "Pcx.h"
#include "Video.h"

// +--------------------------------------------------------------------+

Screen::Screen(Video* v)
   : width(0), height(0), video(v), clear(0), closed(0)
{
   if (video) {
      width  = video->Width();
      height = video->Height();
   }

   Mouse::Create(this);
}

Screen::~Screen()
{
   Mouse::Close();

   closed = 1;
   window_list.destroy();
}

// +--------------------------------------------------------------------+

bool
Screen::AddWindow(Window* c)
{
   if (!c || closed) return false;
   
   if (c->X() < 0) return false;
   if (c->Y() < 0) return false;
   if (c->X() + c->Width()  > Width())  return false;
   if (c->Y() + c->Height() > Height()) return false;
   
   if (!window_list.contains(c))
      window_list.append(c);
   
   return true;
}

bool
Screen::DelWindow(Window* c)
{
   if (!c || closed) return false;

   return window_list.remove(c) == c;
}

// +--------------------------------------------------------------------+

void
Screen::ClearAllFrames(bool clear_all)
{
   if (clear_all)
      clear = -1;
   else
      clear = 0;
}   

void
Screen::ClearNextFrames(int num_frames)
{
   if (clear >= 0 && clear < num_frames)
      clear = num_frames;
}

// +--------------------------------------------------------------------+

bool
Screen::SetBackgroundColor(Color c)
{
   if (video)
      return video->SetBackgroundColor(c);
   else
      return false;
}

// +--------------------------------------------------------------------+

bool
Screen::Resize(int w, int h)
{
   // scale all root-level windows to new screen size:

   ListIter<Window> iter = window_list;
   while (++iter) {
      Window* win = iter.value();

      double  w_x = win->GetRect().x / (double) width;
      double  w_y = win->GetRect().y / (double) height;
      double  w_w = win->GetRect().w / (double) width;
      double  w_h = win->GetRect().h / (double) height;

      Rect    r;

      r.x = (int) (w_x * w);
      r.y = (int) (w_y * h);
      r.w = (int) (w_w * w);
      r.h = (int) (w_h * h);

      win->MoveTo(r);
   }

   width  = w;
   height = h;

   return true;
}

// +--------------------------------------------------------------------+

bool
Screen::Refresh()
{
   if (clear && !video->ClearAll())
      return false;

   video->StartFrame();

   ListIter<Window> iter = window_list;
   while (++iter) {
      Window* win = iter.value();
      
      if (win->IsShown()) {
         win->Paint();
      }
   }
   
   Mouse::Paint();

   video->EndFrame();

   if (clear > 0) clear--;
   return true;
}




