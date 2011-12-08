/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    nGenEx.lib
    FILE:         Screen.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    General Screen class - maintains and displays a list of windows
*/

#ifndef Screen_h
#define Screen_h

#include "Types.h"
#include "Color.h"
#include "List.h"

// +--------------------------------------------------------------------+

class  Bitmap;
class  Window;
struct Rect;

// +--------------------------------------------------------------------+

class Screen
{
public:
   static const char* TYPENAME() { return "Screen"; }

   Screen(Video* v);
   virtual ~Screen();

   virtual bool      SetBackgroundColor(Color c);

   virtual bool      Resize(int w, int h);
   virtual bool      Refresh();
   virtual bool      AddWindow(Window* c);
   virtual bool      DelWindow(Window* c);

   int               Width()     const { return width;   }
   int               Height()    const { return height;  }

   virtual void      ClearAllFrames(bool clear_all);
   virtual void      ClearNextFrames(int num_frames);

   virtual Video*    GetVideo()  const { return video;   }

protected:
   int               width;
   int               height;
   int               clear;
   int               closed;

   Video*            video;

   List<Window>      window_list;
};

#endif Screen_h

