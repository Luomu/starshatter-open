/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         MenuView.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    View class for displaying right-click context menus
*/

#ifndef MenuView_h
#define MenuView_h

#include "Types.h"
#include "View.h"
#include "Bitmap.h"
#include "Font.h"
#include "Text.h"

// +--------------------------------------------------------------------+

class Menu;
class MenuItem;

// +--------------------------------------------------------------------+

class MenuView : public View
{
public:
   MenuView(Window* c);
   virtual ~MenuView();

   // Operations:
   virtual void      Refresh();
   virtual void      OnWindowMove();
   virtual void      DoMouseFrame();
   virtual void      DrawMenu();
   virtual void      DrawMenu(int x, int y, Menu* menu);
   virtual int       ProcessMenuItem();
   virtual void      ClearMenuSelection(Menu* menu);

   virtual bool      IsShown()         { return show_menu != 0;   }
   virtual int       GetAction()       { return action;           }
   virtual Menu*     GetMenu()         { return menu;             }
   virtual void      SetMenu(Menu* m)  { menu = m;                }
   virtual MenuItem* GetMenuItem()     { return menu_item;        }

   virtual Color     GetBackColor()          { return back_color; }
   virtual void      SetBackColor(Color c)   { back_color = c;    }
   virtual Color     GetTextColor()          { return text_color; }
   virtual void      SetTextColor(Color c)   { text_color = c;    }

protected:
   int         width, height;

   int         shift_down;
   int         mouse_down;
   int         right_down;
   int         show_menu;
   POINT       right_start;
   POINT       offset;

   int         action;
   Menu*       menu;
   MenuItem*   menu_item;
   MenuItem*   selected;

   Color       back_color;
   Color       text_color;
};

#endif MenuView_h

