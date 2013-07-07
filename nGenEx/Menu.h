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

