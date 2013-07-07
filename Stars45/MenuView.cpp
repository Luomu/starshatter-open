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

    SUBSYSTEM:    Stars.exe
    FILE:         MenuView.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    
*/

#include "MemDebug.h"
#include "MenuView.h"

#include "Color.h"
#include "Window.h"
#include "Video.h"
#include "DataLoader.h"
#include "Scene.h"
#include "FontMgr.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "MouseController.h"
#include "Menu.h"
#include "Button.h"
#include "Game.h"
#include "FormatUtil.h"

// +--------------------------------------------------------------------+

MenuView::MenuView(Window* c)
: View(c),
mouse_down(0), right_down(0), shift_down(0), show_menu(false),
action(0), menu(0), menu_item(0), selected(0),
text_color(Color::White), back_color(Color::Black)
{
    right_start.x  = 0;
    right_start.y  = 0;

    OnWindowMove();
}

MenuView::~MenuView()
{
}

// +--------------------------------------------------------------------+

void
MenuView::OnWindowMove()
{
    offset.x    = window->X();
    offset.y    = window->Y();
    width       = window->Width();
    height      = window->Height();
}

// +--------------------------------------------------------------------+

void
MenuView::Refresh()
{
    if (show_menu)
    DrawMenu();
}

// +--------------------------------------------------------------------+

void
MenuView::DoMouseFrame()
{
    static DWORD rbutton_latch = 0;

    action = 0;

    if (Mouse::RButton()) {
        MouseController* mouse_con = MouseController::GetInstance();
        if (!right_down && (!mouse_con || !mouse_con->Active())) {
            rbutton_latch = Game::RealTime();
            right_down    = true;
            show_menu     = false;
        }
    }
    else {
        if (right_down && (Game::RealTime() - rbutton_latch < 250)) {
            right_start.x = Mouse::X() - offset.x;
            right_start.y = Mouse::Y() - offset.y;
            show_menu   = true;
            Button::PlaySound(Button::SND_MENU_OPEN);
        }

        right_down = false;
    }

    MouseController* mouse_con = MouseController::GetInstance();

    if (!mouse_con || !mouse_con->Active()) {
        if (Mouse::LButton()) {
            if (!mouse_down)
            shift_down = Keyboard::KeyDown(VK_SHIFT);

            mouse_down = true;
        }

        else if (mouse_down) {
            int mouse_x  = Mouse::X() - offset.x;
            int mouse_y  = Mouse::Y() - offset.y;
            int keep_menu = false;

            if (show_menu) {
                keep_menu = ProcessMenuItem();
                Mouse::Show(true);
            }

            mouse_down = false;

            if (!keep_menu) {
                ClearMenuSelection(menu);
                show_menu = false;
            }
        }
    }
}

// +--------------------------------------------------------------------+

int
MenuView::ProcessMenuItem()
{
    if (!menu_item || !menu_item->GetEnabled())
    return false;

    if (menu_item->GetSubmenu()) {
        ListIter<MenuItem> item = menu_item->GetMenu()->GetItems();
        while (++item)
        if (item.value() == menu_item)
        item->SetSelected(2);
        else
        item->SetSelected(0);

        Button::PlaySound(Button::SND_MENU_OPEN);
        return true;   // keep menu showing
    }

    action = menu_item->GetData();
    Button::PlaySound(Button::SND_MENU_SELECT);
    return false;
}

// +--------------------------------------------------------------------+

void
MenuView::DrawMenu()
{
    menu_item = 0;

    if (menu) {
        int  mx = right_start.x - 2;
        int  my = right_start.y - 2;

        DrawMenu(mx, my, menu);
    }
}

void
MenuView::DrawMenu(int mx, int my, Menu* m)
{
    if (m) {
        MenuItem*   locked_item = 0;
        Menu*       submenu     = 0;
        int         subx        = 0;
        int         suby        = 0;
        Font*       font        = FontMgr::Find("HUD");

        Rect menu_rect(mx, my, 100, m->NumItems() * 10 + 6);

        int max_width     = 0;
        int max_height    = 0;
        int extra_width   = 16;

        ListIter<MenuItem> item = m->GetItems();
        while (++item) {
            menu_rect.w = width/2;

            if (item->GetText().length()) {
                window->SetFont(font);
                window->DrawText(item->GetText(), 0, menu_rect, DT_LEFT|DT_SINGLELINE|DT_CALCRECT);
                if (menu_rect.w > max_width)
                max_width = menu_rect.w;
                max_height += 11;

                if (item->GetSubmenu())
                extra_width = 28;

                if (item->GetSelected() > 1)
                locked_item = item.value();
            }
            else {
                max_height += 4;
            }
        }

        menu_rect.h = max_height + 6;
        menu_rect.w = max_width  + extra_width;

        if (menu_rect.x + menu_rect.w >= width)
        menu_rect.x = width - menu_rect.w - 2;

        if (menu_rect.y + menu_rect.h >= height)
        menu_rect.y = height - menu_rect.h - 2;

        window->FillRect(menu_rect, back_color * 0.2);
        window->DrawRect(menu_rect, back_color);

        Rect item_rect = menu_rect;

        item_rect.x += 4;
        item_rect.y += 3;
        item_rect.w -= 8;
        item_rect.h = 12;

        item.reset();
        while (++item) {
            int line_height = 0;

            if (item->GetText().length()) {
                Rect fill_rect = item_rect;
                fill_rect.Inflate(2,-1);
                fill_rect.y -= 1;

                int mx = Mouse::X() - offset.x;
                int my = Mouse::Y() - offset.y;

                // is this item picked?
                if (menu_rect.Contains(mx, my)) {
                    if (my >= fill_rect.y && my <= fill_rect.y+fill_rect.h) {
                        if (Mouse::LButton()) {
                            menu_item = item.value();
                            item->SetSelected(2);
                            if (locked_item && locked_item->GetMenu() == m)
                            locked_item->SetSelected(0);
                            locked_item = menu_item;
                        }
                        else if (!locked_item || locked_item->GetMenu() != m) {
                            item->SetSelected(true);
                            menu_item = item.value();
                        }

                        if (menu_item && menu_item != selected) {
                            selected = menu_item;
                            Button::PlaySound(Button::SND_MENU_HILITE);
                        }
                    }
                    else if (item.value() != locked_item) {
                        item->SetSelected(false);
                    }
                }

                if (item->GetSelected()) {
                    window->FillRect(fill_rect, back_color * 0.35);
                    window->DrawRect(fill_rect, back_color * 0.75);

                    if (item->GetSubmenu()) {
                        submenu  = item->GetSubmenu();
                        subx     = menu_rect.x + max_width + extra_width;
                        suby     = fill_rect.y - 3;
                    }
                }

                if (item->GetEnabled())
                font->SetColor(text_color);
                else
                font->SetColor(text_color * 0.33);

                window->SetFont(font);
                window->DrawText(item->GetText(), 0, item_rect, DT_LEFT|DT_SINGLELINE);
                line_height = 11;
            }
            else {
                window->DrawLine(item_rect.x,
                item_rect.y + 2,
                item_rect.x + max_width + extra_width - 8,
                item_rect.y + 2,
                back_color);
                line_height = 4;
            }

            if (item->GetSubmenu()) {
                int left = item_rect.x + max_width + 10;
                int top  = item_rect.y + 1;

                // draw the arrow:
                POINT arrow[3];

                arrow[0].x = left;
                arrow[0].y = top;
                arrow[1].x = left + 8;
                arrow[1].y = top  + 4;
                arrow[2].x = left;
                arrow[2].y = top  + 8;

                window->FillPoly(3, arrow, back_color);
            }

            item_rect.y += line_height;
        }

        if (submenu) {
            if (subx + 60 > width)
            subx = menu_rect.x - 60;

            DrawMenu(subx, suby, submenu);
        }
    }
}

// +--------------------------------------------------------------------+

void
MenuView::ClearMenuSelection(Menu* menu)
{
    if (menu) {
        ListIter<MenuItem> item = menu->GetItems();
        while (++item) {
            item->SetSelected(0);
            if (item->GetSubmenu())
            ClearMenuSelection(item->GetSubmenu());
        }
    }
}
