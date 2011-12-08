/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    nGenEx.lib
    FILE:         Menu.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Simple menu hierarchy class
*/

#ifndef Menu_h
#define Menu_h

#include "Types.h"
#include "List.h"
#include "Text.h"

// +-------------------------------------------------------------------+

class Menu;
class MenuItem;
class MenuHistory;

// +-------------------------------------------------------------------+

class Menu
{
public:
   static const char* TYPENAME() { return "Menu"; }

   Menu()            { }
   Menu(Text t)      : title(t) { }
   virtual ~Menu()   { items.destroy(); }

   virtual Text      GetTitle()     const { return title;  }
   virtual void      SetTitle(Text t)     { title = t;     }
   virtual Menu*     GetParent()    const { return parent; }
   virtual void      SetParent(Menu* p)   { parent = p;    }
   
   virtual void      AddItem(Text label, DWORD value=0, bool enabled=true);
   virtual void      AddItem(MenuItem* item);
   virtual void      AddMenu(Text label, Menu* menu, DWORD value=0);
   virtual MenuItem* GetItem(int index);
   virtual void      SetItem(int index, MenuItem* item);
   virtual int       NumItems() const;
   virtual void      ClearItems();

   ListIter<MenuItem> GetItems() { return items; }

protected:
   Text           title;
   List<MenuItem> items;
   Menu*          parent;

   friend class MenuItem;
};

// +-------------------------------------------------------------------+

class MenuItem
{
public:
   static const char* TYPENAME() { return "MenuItem"; }

   MenuItem(Text label, DWORD value=0, bool enabled=true);
   virtual ~MenuItem();

   virtual Text      GetText()      const { return text;     }
   virtual void      SetText(Text t)      { text = t;        }

   virtual DWORD     GetData()      const { return data;     }
   virtual void      SetData(DWORD d)     { data = d;        }

   virtual int       GetEnabled()   const { return enabled;  }
   virtual void      SetEnabled(int  e)   { enabled = e;     }

   virtual int       GetSelected()  const { return selected; }
   virtual void      SetSelected(int  s)  { selected = s;    }

   virtual Menu*     GetMenu()      const { return menu;     }
   virtual void      SetMenu(Menu* m)     { menu = m;        }

   virtual Menu*     GetSubmenu()   const { return submenu;  }
   virtual void      SetSubmenu(Menu* s)  { submenu = s;     }

protected:
   Text           text;
   DWORD          data;
   int            enabled;
   int            selected;

   Menu*          menu;
   Menu*          submenu;

   friend class Menu;
};

// +-------------------------------------------------------------------+

class MenuHistory
{
public:
   static const char* TYPENAME() { return "MenuHistory"; }

   MenuHistory() { }
   virtual ~MenuHistory() { history.clear(); }

   virtual Menu*     GetCurrent();
   virtual Menu*     GetLevel(int n);
   virtual Menu*     Find(const char* title);
   virtual void      Pop();
   virtual void      Push(Menu* menu);
   virtual void      Clear();

private:
   List<Menu>  history;
};

#endif Menu_h

