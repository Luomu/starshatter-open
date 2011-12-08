/*  Project Starshatter 5.0
    Destroyer Studios LLC
    Copyright © 1997-2007. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         QuitView.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    View class for End Mission menu
*/

#include "MemDebug.h"
#include "QuitView.h"
#include "HUDView.h"
#include "RadioView.h"
#include "Ship.h"
#include "Contact.h"
#include "Sim.h"
#include "SimEvent.h"
#include "Campaign.h"
#include "Starshatter.h"
#include "GameScreen.h"

#include "CameraView.h"
#include "Color.h"
#include "Window.h"
#include "Video.h"
#include "Screen.h"
#include "DataLoader.h"
#include "Scene.h"
#include "FontMgr.h"
#include "Button.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "MouseController.h"
#include "Game.h"
#include "Menu.h"

// +====================================================================+

static bool             show_menu      = false;
static Bitmap*          menu_bmp       = 0;
static MouseController* mouse_con      = 0;
static bool             mouse_active   = false;
static const int        w2             = 200;
static const int        h2             = 128;

void
QuitView::Initialize()
{
   if (!menu_bmp) {
      menu_bmp = new(__FILE__,__LINE__) Bitmap;

      DataLoader* loader = DataLoader::GetLoader();
      loader->SetDataPath("Screens/");
      loader->LoadBitmap("QuitWin.pcx", *menu_bmp, Bitmap::BMP_TRANSPARENT);
      loader->SetDataPath(0);
   }
}

void
QuitView::Close()
{
   delete menu_bmp;
}

// +====================================================================+

QuitView* QuitView::quit_view = 0;

QuitView::QuitView(Window* c)
   : View(c), mouse_latch(false)
{
   quit_view   = this;
   sim         = Sim::GetSim();

   width       = window->Width();
   height      = window->Height();
   xcenter     = width  / 2;
   ycenter     = height / 2;

   mouse_con   = MouseController::GetInstance();
}

QuitView::~QuitView()
{
   quit_view = 0;
}

void
QuitView::OnWindowMove()
{
   width       = window->Width();
   height      = window->Height();
   xcenter     = width  / 2;
   ycenter     = height / 2;
}

// +--------------------------------------------------------------------+

void
QuitView::Refresh()
{
   if (show_menu && menu_bmp) {
      Rect clip_rect;

      clip_rect.x = xcenter - w2;
      clip_rect.y = ycenter - h2;
      clip_rect.w = w2 * 2;
      clip_rect.h = h2 * 2;

      window->ClipBitmap(xcenter - w2, 
                         ycenter - h2, 
                         xcenter - w2 + menu_bmp->Width(), 
                         ycenter - h2 + menu_bmp->Height(),
                         menu_bmp, 
                         Color::White, 
                         Video::BLEND_SOLID, 
                         clip_rect);
   }
}

// +--------------------------------------------------------------------+

void
QuitView::ExecFrame()
{
   sim = Sim::GetSim();

   if (show_menu) {
      Color::SetFade(1, Color::Black, 0);
      int action = 0;

      if (Mouse::LButton()) {
         mouse_latch = true;
      }
      else if (mouse_latch) {
         mouse_latch = false;

         if (Mouse::X() > xcenter - w2 && Mouse::X() < xcenter + w2) {
            int y0 = ycenter - h2;

            for (int i = 0; i < 4; i++)
               if (Mouse::Y() >= y0 + 75 + i * 30 && Mouse::Y() <= y0 + 105 + i * 30)
                  action = i+1;
         }
      }

      for (int i = 1; i <= 4; i++) {
         if (Keyboard::KeyDown('0' + i))
            action = i;
      }

      // was mission long enough to accept?
      if (action == 1 && !CanAccept()) {
         Button::PlaySound(Button::SND_REJECT);
         action = 3;
      }

      // exit and accept:
      if (action == 1) {
         CloseMenu();
         Game::SetTimeCompression(1);

         Starshatter* stars = Starshatter::GetInstance();
         stars->SetGameMode(Starshatter::PLAN_MODE);
      }

      // quit and discard results:
      else if (action == 2) {
         CloseMenu();
         Game::SetTimeCompression(1);

         Starshatter*   stars    = Starshatter::GetInstance();
         Campaign*      campaign = Campaign::GetCampaign();

         // discard mission and events:
         if (sim) sim->UnloadMission();
         else ShipStats::Initialize();

         if (campaign && campaign->GetCampaignId() < Campaign::SINGLE_MISSIONS) {
            campaign->RollbackMission();
            stars->SetGameMode(Starshatter::CMPN_MODE);
         }

         else {
            stars->SetGameMode(Starshatter::MENU_MODE);
         }
      }

      // resume:
      else if (action == 3) {
         CloseMenu();
      }

      // controls:
      else if (action == 4) {
         GameScreen* game_screen = GameScreen::GetInstance();

         if (game_screen)
            game_screen->ShowCtlDlg();
         else
            CloseMenu();
      }
   }
}

// +--------------------------------------------------------------------+

bool
QuitView::IsMenuShown()
{
   return show_menu;
}

// +--------------------------------------------------------------------+

bool
QuitView::CanAccept()
{
   sim = Sim::GetSim();

   if (!sim || sim->IsNetGame())
      return true;

   Ship* player_ship = sim->GetPlayerShip();

   if (player_ship->MissionClock() < 60000) {
      RadioView::Message(Game::GetText("QuitView.too-soon"));
      RadioView::Message(Game::GetText("QuitView.abort"));
      return false;
   }

   ListIter<Contact> iter = player_ship->ContactList();
   while (++iter) {
      Contact* c = iter.value();
      Ship*    cship = c->GetShip();
      int      ciff  = c->GetIFF(player_ship);

      if (c->Threat(player_ship)) {
         RadioView::Message(Game::GetText("QuitView.threats-present"));
         RadioView::Message(Game::GetText("QuitView.abort"));
         return false;
      }

      else if (cship && ciff > 0 && ciff != player_ship->GetIFF()) {
         Point  delta = c->Location() - player_ship->Location();
         double dist  = delta.length();

         if (cship->IsDropship() && dist < 50e3) {
            RadioView::Message(Game::GetText("QuitView.threats-present"));
            RadioView::Message(Game::GetText("QuitView.abort"));
            return false;
         }

         else if (cship->IsStarship() && dist < 100e3) {
            RadioView::Message(Game::GetText("QuitView.threats-present"));
            RadioView::Message(Game::GetText("QuitView.abort"));
            return false;
         }
      }
   }

   return true;
}

void
QuitView::ShowMenu()
{
   if (!show_menu) {
      show_menu = true;

      for (int i = 0; i < 10; i++) {
         if (Keyboard::KeyDown('1' + i)) {
            // just need to clear the key down flag
            // so we don't process old keystrokes
            // as valid menu picks...
         }
      }

      Button::PlaySound(Button::SND_CONFIRM);
      Starshatter::GetInstance()->Pause(true);

      if (mouse_con) {
         mouse_active = mouse_con->Active();
         mouse_con->SetActive(false);
      }
   }
}

void
QuitView::CloseMenu()
{
   show_menu = false;
   Starshatter::GetInstance()->Pause(false);

   if (mouse_con)
      mouse_con->SetActive(mouse_active);
}
