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
    FILE:         Menu.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Simple menu hierarchy class
*/

#include "MemDebug.h"
#include "Menu.h"

// +--------------------------------------------------------------------+

void
Menu::AddItem(Text label, DWORD value, bool enabled)
{
    MenuItem* item = new(__FILE__,__LINE__) MenuItem(label, value, enabled);

    if (item) {
        item->menu = this;
        items.append(item);
    }
}

void
Menu::AddItem(MenuItem* item)
{
    if (item->submenu)
    item->submenu->SetParent(this);
    item->menu = this;
    items.append(item);
}

void
Menu::AddMenu(Text label, Menu* menu, DWORD value)
{
    MenuItem* item = new(__FILE__,__LINE__) MenuItem(label, value);

    if (item) {
        item->menu     = this;
        item->submenu  = menu;
        menu->parent   = this;

        items.append(item);
    }
}

MenuItem*
Menu::GetItem(int index)
{
    if (index >= 0 && index < items.size())
    return items[index];

    return 0;
}

void
Menu::SetItem(int index, MenuItem* item)
{
    if (item && index >= 0 && index < items.size())
    items[index] = item;
}

int
Menu::NumItems() const
{
    return items.size();
}

void
Menu::ClearItems()
{
    items.destroy();
}


// +--------------------------------------------------------------------+

MenuItem::MenuItem(Text label, DWORD value, bool e)
: text(label), data(value), enabled(e), submenu(0), selected(0)
{ }

MenuItem::~MenuItem()
{ }

// +--------------------------------------------------------------------+

Menu*
MenuHistory::GetCurrent()
{
    int n = history.size();

    if (n)
    return history[n-1];

    return 0;
}

Menu*
MenuHistory::GetLevel(int n)
{
    if (n >= 0 && n < history.size())
    return history[n];

    return 0;
}

Menu*
MenuHistory::Find(const char* title)
{
    for (int i = 0; i < history.size(); i++)
    if (history[i]->GetTitle() == title)
    return history[i];

    return 0;
}

void
MenuHistory::Pop()
{
    int n = history.size();

    if (n)
    history.removeIndex(n-1);
}

void
MenuHistory::Push(Menu* menu)
{
    history.append(menu);
}

void
MenuHistory::Clear()
{
    history.clear();
}
