/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    nGenEx.lib
    FILE:         View.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Abstract View class
*/

#ifndef View_h
#define View_h

#include "Types.h"

// +--------------------------------------------------------------------+

class Window;

// +--------------------------------------------------------------------+

class View
{
   friend class Window;

public:
   static const char* TYPENAME() { return "View"; }

   View(Window* c) : window(c)               { }
   virtual ~View()                           { }
   
   int operator == (const View& that) const { return this == &that; }

   // Operations:
   virtual void      Refresh()               { }
   virtual void      OnWindowMove()          { }
   virtual void      OnShow()                { }
   virtual void      OnHide()                { }

   virtual void      SetWindow(Window* w)    { window = w; OnWindowMove(); }
   virtual Window*   GetWindow()             { return window; }

protected:
   Window*  window;
};

#endif View_h

