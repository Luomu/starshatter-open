/*  Project Starshatter 5.0
    Destroyer Studios LLC
    Copyright © 1997-2007. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         TacticalView.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    View class for Tactical Data Readout HUD Overlay
*/

#include "MemDebug.h"
#include "TacticalView.h"
#include "QuantumView.h"
#include "RadioView.h"
#include "RadioMessage.h"
#include "RadioTraffic.h"
#include "HUDSounds.h"
#include "HUDView.h"
#include "WepView.h"
#include "CameraDirector.h"
#include "Ship.h"
#include "ShipCtrl.h"
#include "ShipDesign.h"
#include "QuantumDrive.h"
#include "Farcaster.h"
#include "Instruction.h"
#include "Element.h"
#include "Contact.h"
#include "Sim.h"
#include "Starshatter.h"
#include "GameScreen.h"
#include "MenuView.h"

#include "Projector.h"
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
#include "Game.h"
#include "FormatUtil.h"

static Color  hud_color = Color::Black;
static Color  txt_color = Color::Black;

// +--------------------------------------------------------------------+

TacticalView* TacticalView::tac_view = 0;

// +--------------------------------------------------------------------+

TacticalView::TacticalView(Window* c, GameScreen* parent)
   : View(c), gamescreen(parent), ship(0), camview(0), projector(0),
     mouse_down(0), right_down(0), shift_down(0),
     show_move(0), show_action(0), active_menu(0), menu_view(0),
     msg_ship(0), base_alt(0), move_alt(0)
{
   tac_view = this;
   sim      = Sim::GetSim();

   width    = window->Width();
   height   = window->Height();
   xcenter  = (width  / 2.0) - 0.5;
   ycenter  = (height / 2.0) + 0.5;
   font     = FontMgr::Find("HUD");

   SetColor(Color::White);

   mouse_start.x  = 0;
   mouse_start.y  = 0;
   mouse_action.x = 0;
   mouse_action.y = 0;

   menu_view = new(__FILE__,__LINE__) MenuView(window);
}

TacticalView::~TacticalView()
{
   delete menu_view;
   tac_view = 0;
}

void
TacticalView::OnWindowMove()
{
   width       = window->Width();
   height      = window->Height();
   xcenter     = (width  / 2.0) - 0.5;
   ycenter     = (height / 2.0) + 0.5;

   if (menu_view)
      menu_view->OnWindowMove();
}

// +--------------------------------------------------------------------+

bool
TacticalView::Update(SimObject* obj)
{
   if (obj == ship) {
      ship = 0;
   }

   if (obj == msg_ship) {
      msg_ship = 0;
   }

   return SimObserver::Update(obj);
}

const char*
TacticalView::GetObserverName() const
{
   return "TacticalView";
}

void
TacticalView::UseProjector(Projector* p)
{
   projector = p;
}

// +--------------------------------------------------------------------+

void
TacticalView::Refresh()
{
   sim = Sim::GetSim();

   if (sim) {
      bool rebuild = false;

      if (ship != sim->GetPlayerShip()) {
         ship = sim->GetPlayerShip();

         if (ship) {
            if (ship->Life() == 0 || ship->IsDying() || ship->IsDead()) {
               ship = 0;
            }
            else {
               Observe(ship);
            }
         }

         rebuild = true;
      }

      if (ship) {
         if (current_sector != ship->GetRegion()->Name())
            rebuild = true;

         if (rebuild) {
            BuildMenu();
            current_sector = ship->GetRegion()->Name();
         }
      }
   }

   if (!ship || ship->InTransition())
      return;

   DrawMouseRect();

   if (sim) {
      ListIter<Ship> sel = sim->GetSelection();

      if (sel.size()) {
         while (++sel) {
            Ship* selection = sel.value();

            // draw selection rect on selected ship:
            if (selection && selection->Rep())
               DrawSelection(selection);
         }

         RadioView*   rv = RadioView::GetInstance();
         QuantumView* qv = QuantumView::GetInstance();

         if ((!rv || !rv->IsMenuShown()) && (!qv || !qv->IsMenuShown())) {
            sel.reset();

            if (sel.size() == 1) {
               DrawSelectionInfo(sel.next());
            }
            else {
               DrawSelectionList(sel);
            }
         }
      }
   }

   DrawMenu();

   if (show_move) {
      Mouse::Show(false);
      DrawMove();
   }
   else if (show_action) {
      Mouse::Show(false);
      DrawAction();
   }
}

// +--------------------------------------------------------------------+

void
TacticalView::ExecFrame()
{
   HUDView* hud = HUDView::GetInstance();
   if (hud) {
      if (hud_color != hud->GetTextColor()) {
         hud_color = hud->GetTextColor();
         SetColor(hud_color);
      }
   }
}

// +--------------------------------------------------------------------+

void
TacticalView::SetColor(Color c)
{
   HUDView* hud = HUDView::GetInstance();

   if (hud) {
      hud_color = hud->GetHUDColor();
      txt_color = hud->GetTextColor();
   }
   else {
      hud_color = c;
      txt_color = c;
   }
}

// +--------------------------------------------------------------------+

void
TacticalView::DrawMouseRect()
{
   if (mouse_rect.w > 0 && mouse_rect.h > 0) {
      Color c = hud_color * 0.66;

      if (shift_down)
         c = Color::Orange;

      window->DrawRect(mouse_rect, c);
   }
}

// +--------------------------------------------------------------------+

void
TacticalView::DrawSelection(Ship* seln)
{
   Graphic* g = seln->Rep();
   Rect r = g->ScreenRect();

   Point mark_pt;

   if (seln)
      mark_pt = seln->Location();

   projector->Transform(mark_pt);

   // clip:
   if (mark_pt.z > 1.0) {
      projector->Project(mark_pt);

      int x = (int) mark_pt.x;
      int y = r.y;

      if (y >= 2000)
         y = (int) mark_pt.y;

      if (x > 4 && x < width-4 &&
          y > 4 && y < height-4) {

         const int BAR_LENGTH = 40;

         // life bars:
         int sx = x - BAR_LENGTH/2;
         int sy = y - 8;

         double hull_strength = seln->HullStrength() / 100.0;

         int hw = (int) (BAR_LENGTH * hull_strength);
         int sw = (int) (BAR_LENGTH * (seln->ShieldStrength() / 100.0));

         if (hw < 0) hw = 0;
         if (sw < 0) sw = 0;

         System::STATUS s = System::NOMINAL;

         if (hull_strength < 0.30)        s = System::CRITICAL;
         else if (hull_strength < 0.60)   s = System::DEGRADED;

         Color hc = HUDView::GetStatusColor(s);
         Color sc = hud_color;

         window->FillRect(sx, sy,   sx+hw, sy+1, hc);
         window->FillRect(sx, sy+3, sx+sw, sy+4, sc);
      }
   }
}

// +--------------------------------------------------------------------+

void
TacticalView::DrawSelectionInfo(Ship* seln)
{
   if (!ship || !seln) return;

   Rect  label_rect(width-140, 10, 90, 12);
   Rect  info_rect(width-100, 10, 90, 12);

   if (width >= 800) {
      label_rect.x -= 20;
      info_rect.x  -= 20;
      info_rect.w  += 20;
   }

   static char name[64];
   static char design[64];
   static char shield[32];
   static char hull[32];
   static char range[32];
   static char heading[32];
   static char speed[32];
   static char orders[64];
   static char psv[32];
   static char act[32];

   int   show_labels = width > 640;
   int   full_info   = true;
   int   shield_val  = seln->ShieldStrength();
   int   hull_val    = seln->HullStrength();

   if (shield_val < 0) shield_val = 0;
   if (hull_val   < 0) hull_val   = 0;

   sprintf(name,     "%s",     seln->Name());

   if (show_labels) {
      sprintf(shield,   "%s %03d", Game::GetText("HUDView.symbol.shield").data(), shield_val);
      sprintf(hull,     "%s %03d", Game::GetText("HUDView.symbol.hull").data(),   hull_val);
   }
   else {
      sprintf(shield,   "%03d", shield_val);
      sprintf(hull,     "%03d", hull_val);
   }

   FormatNumberExp(range, Point(seln->Location()-ship->Location()).length()/1000);
   strcat(range,     " km");
   sprintf(heading,  "%03d %s", (int) (seln->CompassHeading() / DEGREES), Game::GetText("HUDView.symbol.degrees").data());

   double ss = seln->Velocity().length();
   if (seln->Velocity() * seln->Heading() < 0)
      ss = -ss;

   FormatNumberExp(speed, ss);
   strcat(speed,     " m/s");

   Contact* contact = 0;

   // always recognize ownside:
   if (seln->GetIFF() != ship->GetIFF()) {
      ListIter<Contact> c = ship->ContactList();
      while (++c) {
         if (c->GetShip() == seln) {
            contact = c.value();
            if (c->GetIFF(ship) > seln->GetIFF()) {
               sprintf(name, "%s %04d", Game::GetText("TacView.contact").data(), seln->GetContactID());
               full_info = false;
            }

            break;
         }
      }
   }

   if (show_labels) {
      font->SetColor(txt_color);
      font->SetAlpha(1);

      font->DrawText(Game::GetText("TacView.name"), 5, label_rect, DT_LEFT);
      label_rect.y += 10;
      font->DrawText(Game::GetText("TacView.type"), 5, label_rect, DT_LEFT);
      label_rect.y += 10;

      if (full_info) {
         font->DrawText(Game::GetText("TacView.shield"), 5, label_rect, DT_LEFT);
         label_rect.y += 10;
         font->DrawText(Game::GetText("TacView.hull"), 5, label_rect, DT_LEFT);
         label_rect.y += 10;
      }

      font->DrawText(Game::GetText("TacView.range"),  4, label_rect, DT_LEFT);
      label_rect.y += 10;

      if (full_info) {
         font->DrawText(Game::GetText("TacView.speed"),  4, label_rect, DT_LEFT);
         label_rect.y += 10;
         font->DrawText(Game::GetText("TacView.heading"),  4, label_rect, DT_LEFT);
         label_rect.y += 10;
      }
      else {
         font->DrawText(Game::GetText("TacView.passive"),  4, label_rect, DT_LEFT);
         label_rect.y += 10;
         font->DrawText(Game::GetText("TacView.active"),  4, label_rect, DT_LEFT);
         label_rect.y += 10;
      }
   }

   font->DrawText(name, 0, info_rect, DT_LEFT);
   info_rect.y += 10;

   if (full_info) {
      sprintf(design, "%s %s", seln->Abbreviation(), seln->Design()->display_name);
      font->DrawText(design, 0, info_rect, DT_LEFT);
      info_rect.y += 10;
   }
   else {
      if (seln->IsStarship())
         font->DrawText(Game::GetText("TacView.starship"), 8, info_rect, DT_LEFT);
      else
         font->DrawText(Game::GetText("TacView.fighter"),  7, info_rect, DT_LEFT);

      info_rect.y += 10;
   }

   if (full_info) {
      font->DrawText(shield, 0, info_rect, DT_LEFT);
      info_rect.y += 10;

      font->DrawText(hull, 0, info_rect, DT_LEFT);
      info_rect.y += 10;
   }

   font->DrawText(range, 0, info_rect, DT_LEFT);
   info_rect.y += 10;

   if (full_info) {
      font->DrawText(speed, 0, info_rect, DT_LEFT);
      info_rect.y += 10;

      font->DrawText(heading, 0, info_rect, DT_LEFT);
      info_rect.y += 10;

      if (seln->GetIFF() == ship->GetIFF()) {
         Instruction* instr = seln->GetRadioOrders();
         if (instr && instr->Action()) {
            strcpy(orders, RadioMessage::ActionName(instr->Action()));

            if (instr->Action() == RadioMessage::QUANTUM_TO) {
               strcat(orders, " ");
               strcat(orders, instr->RegionName());
            }
         }
         else {
            *orders = 0;
         }

         if (*orders) {
            if (show_labels) {
               font->DrawText(Game::GetText("TacView.orders"), 5, label_rect, DT_LEFT);
               label_rect.y += 10;
            }

            font->DrawText(orders, 0, info_rect, DT_LEFT);
            info_rect.y += 10;
         }
      }
   }
   else {
      sprintf(psv, "%03d", (int) (contact->PasReturn() * 100.0));
      sprintf(act, "%03d", (int) (contact->ActReturn() * 100.0));

      if (contact->Threat(ship))
         strcat(psv, " !");

      font->DrawText(psv, 0, info_rect, DT_LEFT);
      info_rect.y += 10;
      font->DrawText(act, 0, info_rect, DT_LEFT);
      info_rect.y += 10;
   }

   /*** XXX DEBUG
   font->DrawText(seln->GetDirectorInfo(), 0, info_rect, DT_LEFT);
   info_rect.y += 10;
   /***/
}

// +--------------------------------------------------------------------+

void
TacticalView::DrawSelectionList(ListIter<Ship> seln)
{
   int   index = 0;
   Rect  info_rect(width-100, 10, 90, 12);

   while (++seln) {
      char name[64];
      sprintf(name, "%s", seln->Name());

      // always recognize ownside:
      if (seln->GetIFF() != ship->GetIFF()) {
         ListIter<Contact> c = ship->ContactList();
         while (++c) {
            if (c->GetShip() == seln.value()) {
               if (c->GetIFF(ship) > seln->GetIFF()) {
                  sprintf(name, "%s %04d", Game::GetText("TacView.contact").data(), seln->GetContactID());
               }

               break;
            }
         }
      }

      font->DrawText(name, 0, info_rect, DT_LEFT);
      info_rect.y += 10;
      index++;

      if (index >= 10)
         break;
   }
}

// +--------------------------------------------------------------------+

void
TacticalView::DoMouseFrame()
{
   static DWORD rbutton_latch = 0;

   Starshatter* stars = Starshatter::GetInstance();

   if (stars->InCutscene())
      return;

   if (Mouse::RButton()) {
      MouseController* mouse_con = MouseController::GetInstance();
      if (!right_down && (!mouse_con || !mouse_con->Active())) {
         rbutton_latch = Game::RealTime();
         right_down    = true;
      }
   }
   else {
      if (sim && right_down && (Game::RealTime() - rbutton_latch < 250)) {
         Ship* seln = WillSelectAt(Mouse::X(), Mouse::Y());

         if (seln && sim->IsSelected(seln)    && 
             seln->GetIFF() == ship->GetIFF() &&
             ship->GetElement()->CanCommand(seln->GetElement())) {

            msg_ship = seln;
            Observe(msg_ship);
         }

         else if (ship && seln == ship && 
                     (!ship->GetDirector() || 
                       ship->GetDirector()->Type() != ShipCtrl::DIR_TYPE)) {

            msg_ship = seln;
         }

         else {
            msg_ship = 0;
         }
      }

      right_down = false;
   }

   if (menu_view)
      menu_view->DoMouseFrame();

   MouseController* mouse_con = MouseController::GetInstance();

   if (!mouse_con || !mouse_con->Active()) {
      if (Mouse::LButton()) {
         if (!mouse_down) {
            mouse_start.x = Mouse::X();
            mouse_start.y = Mouse::Y();

            shift_down = Keyboard::KeyDown(VK_SHIFT);
         }

         else {
            if (Mouse::X() < mouse_start.x) {
               mouse_rect.x = Mouse::X();
               mouse_rect.w = mouse_start.x - Mouse::X();
            }
            else {
               mouse_rect.x = mouse_start.x;
               mouse_rect.w = Mouse::X() - mouse_start.x;
            }

            if (Mouse::Y() < mouse_start.y) {
               mouse_rect.y = Mouse::Y();
               mouse_rect.h = mouse_start.y - Mouse::Y();
            }
            else {
               mouse_rect.y = mouse_start.y;
               mouse_rect.h = Mouse::Y() - mouse_start.y;
            }

            // don't draw seln rectangle while zooming:
            if (Mouse::RButton() || show_move || show_action) {
               mouse_rect.w = 0;
               mouse_rect.h = 0;
            }

            else {
               SelectRect(mouse_rect);
            }
         }

         mouse_down = true;
      }

      else {
         if (mouse_down) {
            int mouse_x  = Mouse::X();
            int mouse_y  = Mouse::Y();

            if (menu_view && menu_view->GetAction()) {
               ProcessMenuItem(menu_view->GetAction());
               Mouse::Show(true);
            }
            else if (show_move) {
               SendMove();
               show_move = false;
               Mouse::Show(true);
            }
            else if (show_action) {
               SendAction();
               show_action = false;
               Mouse::Show(true);
            }
            else {
               if (!HUDView::IsMouseLatched() && !WepView::IsMouseLatched()) {
                  int dx = (int) fabs((double) (mouse_x - mouse_start.x));
                  int dy = (int) fabs((double) (mouse_y - mouse_start.y));

                  static DWORD click_time = 0;

                  if (dx < 3 && dy < 3) {
                     bool hit = SelectAt(mouse_x, mouse_y);

                     if (ship->IsStarship() && Game::RealTime() - click_time < 350)
                        SetHelm(hit);

                     click_time = Game::RealTime();
                  }
               }
            }

            mouse_rect = Rect();
            mouse_down = false;
         }
      }
   }

   if (show_action && !mouse_down && !right_down) {
      mouse_action.x = Mouse::X();
      mouse_action.y = Mouse::Y();
   }
}

// +--------------------------------------------------------------------+

bool
TacticalView::SelectAt(int x, int y)
{
   if (!ship) return false;

   Ship* selection = WillSelectAt(x,y);

   if (selection && shift_down)
      ship->SetTarget(selection);

   else if (sim && selection)
      sim->SetSelection(selection);

   return selection != 0;
}

// +--------------------------------------------------------------------+

bool
TacticalView::SelectRect(const Rect& rect)
{
   bool result = false;

   if (!ship || !sim) return result;

   if (rect.w > 8 || rect.h > 8)
      sim->ClearSelection();

   // check distance to each contact:
   List<Contact>& contact_list = ship->ContactList();

   for (int i = 0; i < ship->NumContacts(); i++) {
      Ship* test = contact_list[i]->GetShip();

      if (test && test != ship) {

         Point test_loc = test->Location();
         projector->Transform(test_loc);

         if (test_loc.z > 1) {
            projector->Project(test_loc);

            if (rect.Contains((int) test_loc.x, (int) test_loc.y)) {
               // shift-select targets:
               if (shift_down) {
                  if (test->GetIFF() == 0 || test->GetIFF() == ship->GetIFF())
                     continue;

                  ship->SetTarget(test);
                  result = true;
               }
               else {
                  sim->AddSelection(test);
                  result = true;
               }
            }
         }
      }
   }

   // select self only in orbit cam
   if (!shift_down && CameraDirector::GetCameraMode() == CameraDirector::MODE_ORBIT) {
      Point test_loc = ship->Location();
      projector->Transform(test_loc);

      if (test_loc.z > 1) {
         projector->Project(test_loc);

         if (rect.Contains((int) test_loc.x, (int) test_loc.y)) {
            sim->AddSelection(ship);
            result = true;
         }
      }
   }

   return result;
}

// +--------------------------------------------------------------------+

Ship*
TacticalView::WillSelectAt(int x, int y)
{
   Ship* selection = 0;

   if (ship) {
      // check distance to each contact:
      List<Contact>& contact_list = ship->ContactList();

      for (int i = 0; i < ship->NumContacts(); i++) {
         Ship* test = contact_list[i]->GetShip();

         if (test) {
            // shift-select targets:
            if (shift_down) {
               if (test->GetIFF() == 0 || test->GetIFF() == ship->GetIFF())
                  continue;
            }

            Graphic* g = test->Rep();
            if (g) {
               Rect r = g->ScreenRect();

               if (r.x == 2000 && r.y == 2000 && r.w == 0 && r.h == 0) {
                  if (projector) {
                     Point loc = test->Location();
                     projector->Transform(loc);
                     projector->Project(loc);

                     r.x = (int) loc.x;
                     r.y = (int) loc.y;
                  }
               }

               if (r.w < 20 || r.h < 20)
                  r.Inflate(20,20);
               else
                  r.Inflate(10,10);

               if (r.Contains(x,y)) {
                  selection = test;
                  break;
               }
            }
         }
      }

      if (!selection && !shift_down) {
         Graphic* g = ship->Rep();
         if (g) {
            Rect r = g->ScreenRect();

            if (r.Contains(x,y)) {
               selection = ship;
            }
         }
      }
   }

   if (selection == ship && CameraDirector::GetCameraMode() != CameraDirector::MODE_ORBIT)
      selection = 0;

   return selection;
}

// +--------------------------------------------------------------------+

void
TacticalView::SetHelm(bool approach)
{
   Point delta;

   // double-click on ship: set helm to approach
   if (sim && approach) {
      ListIter<Ship> iter = sim->GetSelection();
      ++iter;
      Ship* selection = iter.value();

      if (selection != ship) {
         delta = selection->Location() - ship->Location();
         delta.Normalize();
      }
   }

   // double-click on space: set helm in direction
   if (delta.length() < 1) {
      int mx = Mouse::X();
      int my = Mouse::Y();

      if (projector) {
         double focal_dist  = width / tan(projector->XAngle());

         delta = projector->vpn() * focal_dist +
                 projector->vup() * -1 * (my-height/2) +
                 projector->vrt() * (mx-width/2);

         delta.Normalize();
      }

      else {
         return;
      }
   }

   double az = atan2(fabs(delta.x), delta.z);
   double el = asin(delta.y);

   if (delta.x < 0)
      az *= -1;

   az += PI;

   if (az >= 2*PI)
      az -= 2*PI;

   ship->SetHelmHeading(az);
   ship->SetHelmPitch(el);
}

// +====================================================================+
//
// TACTICAL COMMUNICATIONS MENU:
//

static Menu*         main_menu      = 0;
static Menu*         view_menu      = 0;
static Menu*         emcon_menu     = 0;

static Menu*         fighter_menu   = 0;
static Menu*         starship_menu  = 0;
static Menu*         action_menu    = 0;
static Menu*         formation_menu = 0;
static Menu*         sensors_menu   = 0;
static Menu*         quantum_menu   = 0;
static Menu*         farcast_menu   = 0;

static Element*      dst_elem       = 0;

enum   VIEW_MENU {
         VIEW_FORWARD = 1000,
         VIEW_CHASE,
         VIEW_PADLOCK,
         VIEW_ORBIT,
         VIEW_NAV,
         VIEW_WEP,
         VIEW_ENG,
         VIEW_FLT,
         VIEW_INS,
         VIEW_CMD
};

const int QUANTUM = 2000;
const int FARCAST = 2001;

void
TacticalView::Initialize()
{
   static int initialized = 0;
   if (initialized) return;

   view_menu   = new(__FILE__,__LINE__) Menu(Game::GetText("TacView.menu.view"));
   view_menu->AddItem(Game::GetText("TacView.item.forward"), VIEW_FORWARD);
   view_menu->AddItem(Game::GetText("TacView.item.chase"),   VIEW_CHASE);
   view_menu->AddItem(Game::GetText("TacView.item.orbit"),   VIEW_ORBIT);
   view_menu->AddItem(Game::GetText("TacView.item.padlock"), VIEW_PADLOCK);

   emcon_menu  = new(__FILE__,__LINE__) Menu(Game::GetText("TacView.menu.emcon"));

   quantum_menu = new(__FILE__,__LINE__) Menu(Game::GetText("TacView.menu.quantum"));
   farcast_menu = new(__FILE__,__LINE__) Menu(Game::GetText("TacView.menu.farcast"));

   main_menu    = new(__FILE__,__LINE__) Menu(Game::GetText("TacView.menu.main"));

   action_menu = new(__FILE__,__LINE__) Menu(Game::GetText("TacView.menu.action"));
   action_menu->AddItem(Game::GetText("TacView.item.engage"),     RadioMessage::ATTACK);
   action_menu->AddItem(Game::GetText("TacView.item.bracket"),    RadioMessage::BRACKET);
   action_menu->AddItem(Game::GetText("TacView.item.escort"),     RadioMessage::ESCORT);
   action_menu->AddItem(Game::GetText("TacView.item.identify"),   RadioMessage::IDENTIFY);
   action_menu->AddItem(Game::GetText("TacView.item.hold"),       RadioMessage::WEP_HOLD);

   formation_menu = new(__FILE__,__LINE__) Menu(Game::GetText("TacView.menu.formation"));
   formation_menu->AddItem(Game::GetText("TacView.item.diamond"),   RadioMessage::GO_DIAMOND);
   formation_menu->AddItem(Game::GetText("TacView.item.spread"),    RadioMessage::GO_SPREAD);
   formation_menu->AddItem(Game::GetText("TacView.item.box"),       RadioMessage::GO_BOX);
   formation_menu->AddItem(Game::GetText("TacView.item.trail"),     RadioMessage::GO_TRAIL);

   sensors_menu = new(__FILE__,__LINE__) Menu(Game::GetText("TacView.menu.emcon"));
   sensors_menu->AddItem(Game::GetText("TacView.item.emcon-1"),  RadioMessage::GO_EMCON1);
   sensors_menu->AddItem(Game::GetText("TacView.item.emcon-2"),  RadioMessage::GO_EMCON2);
   sensors_menu->AddItem(Game::GetText("TacView.item.emcon-3"),  RadioMessage::GO_EMCON3);
   sensors_menu->AddItem(Game::GetText("TacView.item.probe"),    RadioMessage::LAUNCH_PROBE);

   fighter_menu = new(__FILE__,__LINE__) Menu(Game::GetText("TacView.menu.context"));
   fighter_menu->AddMenu(Game::GetText("TacView.item.action"),    action_menu);
   fighter_menu->AddMenu(Game::GetText("TacView.item.formation"), formation_menu);
   fighter_menu->AddMenu(Game::GetText("TacView.item.sensors"),   sensors_menu);
   fighter_menu->AddItem(Game::GetText("TacView.item.patrol"),    RadioMessage::MOVE_PATROL);
   fighter_menu->AddItem(Game::GetText("TacView.item.cancel"),    RadioMessage::RESUME_MISSION);
   fighter_menu->AddItem("", 0);
   fighter_menu->AddItem(Game::GetText("TacView.item.rtb"),       RadioMessage::RTB);
   fighter_menu->AddItem(Game::GetText("TacView.item.dock"),      RadioMessage::DOCK_WITH);
   fighter_menu->AddMenu(Game::GetText("TacView.item.farcast"),   farcast_menu);

   starship_menu = new(__FILE__,__LINE__) Menu(Game::GetText("TacView.menu.context"));
   starship_menu->AddMenu(Game::GetText("TacView.item.action"),   action_menu);
   starship_menu->AddMenu(Game::GetText("TacView.item.sensors"),  sensors_menu);
   starship_menu->AddItem(Game::GetText("TacView.item.patrol"),   RadioMessage::MOVE_PATROL);
   starship_menu->AddItem(Game::GetText("TacView.item.cancel"),   RadioMessage::RESUME_MISSION);
   starship_menu->AddItem("", 0);
   starship_menu->AddMenu(Game::GetText("TacView.item.quantum"),  quantum_menu);
   starship_menu->AddMenu(Game::GetText("TacView.item.farcast"),  farcast_menu);

   initialized = 1;
}

void
TacticalView::Close()
{
   delete view_menu;
   delete emcon_menu;
   delete main_menu;
   delete fighter_menu;
   delete starship_menu;
   delete action_menu;
   delete formation_menu;
   delete sensors_menu;
   delete quantum_menu;
   delete farcast_menu;
}

// +--------------------------------------------------------------------+

void
TacticalView::ProcessMenuItem(int action)
{
   Starshatter* stars = Starshatter::GetInstance();

   switch (action) {
   case RadioMessage::MOVE_PATROL:
      show_move = true;
      base_alt  = 0;
      move_alt  = 0;

      if (msg_ship) base_alt = msg_ship->Location().y;
      break;

   case RadioMessage::ATTACK:
   case RadioMessage::BRACKET:
   case RadioMessage::ESCORT:
   case RadioMessage::IDENTIFY:
   case RadioMessage::DOCK_WITH:
      show_action = action;
      break;

   case RadioMessage::WEP_HOLD:
   case RadioMessage::RESUME_MISSION:
   case RadioMessage::RTB:
   case RadioMessage::GO_DIAMOND:
   case RadioMessage::GO_SPREAD:
   case RadioMessage::GO_BOX:
   case RadioMessage::GO_TRAIL:
   case RadioMessage::GO_EMCON1:
   case RadioMessage::GO_EMCON2:
   case RadioMessage::GO_EMCON3:
   case RadioMessage::LAUNCH_PROBE:
      if (msg_ship) {
         Element* elem = msg_ship->GetElement();
         RadioMessage* msg = new(__FILE__,__LINE__) RadioMessage(elem, ship, action);
         if (msg)
            RadioTraffic::Transmit(msg);
      }
      else if (ship) {
         if (action == RadioMessage::GO_EMCON1)
            ship->SetEMCON(1);
         else if (action == RadioMessage::GO_EMCON2)
            ship->SetEMCON(2);
         else if (action == RadioMessage::GO_EMCON3)
            ship->SetEMCON(3);
         else if (action == RadioMessage::LAUNCH_PROBE)
            ship->LaunchProbe();
      }
      break;

   case VIEW_FORWARD:   stars->PlayerCam(CameraDirector::MODE_COCKPIT); break;
   case VIEW_CHASE:     stars->PlayerCam(CameraDirector::MODE_CHASE);   break;
   case VIEW_PADLOCK:   stars->PlayerCam(CameraDirector::MODE_TARGET);  break;
   case VIEW_ORBIT:     stars->PlayerCam(CameraDirector::MODE_ORBIT);   break;

   case VIEW_NAV:    gamescreen->ShowNavDlg();           break;
   case VIEW_WEP:    gamescreen->ShowWeaponsOverlay();   break;
   case VIEW_ENG:    gamescreen->ShowEngDlg();           break;
   case VIEW_INS:    HUDView::GetInstance()->CycleHUDInst(); break;
   case VIEW_FLT:    gamescreen->ShowFltDlg();           break;

   case VIEW_CMD:    if (ship && ship->IsStarship()) {
                        ship->CommandMode();
                     }
                     break;

   case QUANTUM:     if (sim) {
                        Ship* s = msg_ship;

                        if (!s)
                           s = ship;
                        
                        if (s && s->GetQuantumDrive()) {
                           QuantumDrive* quantum = s->GetQuantumDrive();
                           if (quantum) {
                              MenuItem*   menu_item = menu_view->GetMenuItem();
                              Text        rgn_name  = menu_item->GetText();
                              SimRegion*  rgn       = sim->FindRegion(rgn_name);

                              if (rgn) {
                                 if (s == ship) {
                                    quantum->SetDestination(rgn, Point(0,0,0));
                                    quantum->Engage();
                                 }

                                 else {
                                    Element* elem = msg_ship->GetElement();
                                    RadioMessage* msg = new(__FILE__,__LINE__) RadioMessage(elem, ship, RadioMessage::QUANTUM_TO);
                                    if (msg) {
                                       msg->SetInfo(rgn_name);
                                       RadioTraffic::Transmit(msg);
                                    }
                                 }
                              }
                           }
                        }
                     }
                     break;

   case FARCAST:     if (sim && msg_ship) {
                        MenuItem*   menu_item = menu_view->GetMenuItem();
                        Text        rgn_name  = menu_item->GetText();
                        SimRegion*  rgn       = sim->FindRegion(rgn_name);

                        if (rgn) {
                           Element* elem = msg_ship->GetElement();
                           RadioMessage* msg = new(__FILE__,__LINE__) RadioMessage(elem, ship, RadioMessage::FARCAST_TO);
                           if (msg) {
                              msg->SetInfo(rgn_name);
                              RadioTraffic::Transmit(msg);
                           }
                        }
                     }
                     break;

   default:
      break;
   }
}

// +--------------------------------------------------------------------+

void
TacticalView::BuildMenu()
{
   main_menu->ClearItems();
   quantum_menu->ClearItems();
   farcast_menu->ClearItems();
   emcon_menu->ClearItems();

   if (!ship)
      return;

   // prepare quantum and farcast menus:
   ListIter<SimRegion> iter = sim->GetRegions();
   while (++iter) {
      SimRegion* rgn = iter.value();
      if (rgn != ship->GetRegion() && rgn->Type() != SimRegion::AIR_SPACE)
         quantum_menu->AddItem(rgn->Name(), QUANTUM);
   }

   if (ship->GetRegion()) {
      ListIter<Ship> iter = ship->GetRegion()->Ships();
      while (++iter) {
         Ship* s = iter.value();

         if (s && s->GetFarcaster()) {
            Farcaster*  farcaster = s->GetFarcaster();

            // ensure that the farcaster is connected:
            farcaster->ExecFrame(0);

            // now find the destination
            const Ship* dest      = farcaster->GetDest();

            if (dest && dest->GetRegion()) {
               SimRegion* rgn = dest->GetRegion();
               farcast_menu->AddItem(rgn->Name(), FARCAST);
            }
         }
      }
   }

   // build the main menu:
   main_menu->AddMenu(Game::GetText("TacView.item.camera"),         view_menu);
   main_menu->AddItem("", 0);
   main_menu->AddItem(Game::GetText("TacView.item.instructions"),   VIEW_INS);
   main_menu->AddItem(Game::GetText("TacView.item.navigation"),     VIEW_NAV);

   if (ship->Design()->repair_screen)
      main_menu->AddItem(Game::GetText("TacView.item.engineering"), VIEW_ENG);

   if (ship->Design()->wep_screen)
      main_menu->AddItem(Game::GetText("TacView.item.weapons"),     VIEW_WEP);

   if (ship->NumFlightDecks() > 0)
      main_menu->AddItem(Game::GetText("TacView.item.flight"),      VIEW_FLT);

   emcon_menu->AddItem(Game::GetText("TacView.item.emcon-1"),   RadioMessage::GO_EMCON1);
   emcon_menu->AddItem(Game::GetText("TacView.item.emcon-2"),   RadioMessage::GO_EMCON2);
   emcon_menu->AddItem(Game::GetText("TacView.item.emcon-3"),   RadioMessage::GO_EMCON3);

   if (ship->GetProbeLauncher())
   emcon_menu->AddItem(Game::GetText("TacView.item.probe"),   RadioMessage::LAUNCH_PROBE);

   main_menu->AddItem("", 0);
   main_menu->AddMenu(Game::GetText("TacView.item.sensors"), emcon_menu);

   if (sim && ship->GetQuantumDrive()) {
      main_menu->AddItem("", 0);
      main_menu->AddMenu(Game::GetText("TacView.item.quantum"), quantum_menu);
   }

   if (ship->IsStarship()) {
      main_menu->AddItem("", 0);
      main_menu->AddItem(Game::GetText("TacView.item.command"), VIEW_CMD);
   }
}

// +--------------------------------------------------------------------+

void
TacticalView::DrawMenu()
{
   active_menu = 0;

   if (ship)
      active_menu = main_menu;

   if (msg_ship) {
      if (msg_ship->IsStarship())
         active_menu = starship_menu;
      else if (msg_ship->IsDropship())
         active_menu = fighter_menu;
   }

   if (menu_view) {
      menu_view->SetBackColor(hud_color);
      menu_view->SetTextColor(txt_color);
      menu_view->SetMenu(active_menu);
      menu_view->Refresh();
   }
}

// +--------------------------------------------------------------------+

bool
TacticalView::GetMouseLoc3D()
{
   int mx = Mouse::X();
   int my = Mouse::Y();

   if (projector) {
      double focal_dist  = width / tan(projector->XAngle());
      Point  focal_vect  = projector->vpn() * focal_dist +
                           projector->vup() * -1 * (my-height/2) +
                           projector->vrt() * (mx-width/2);

      focal_vect.Normalize();

      if (Keyboard::KeyDown(VK_SHIFT)) {
         if (Mouse::RButton())
            return true;

         if (fabs(focal_vect.x) > fabs(focal_vect.z)) {
            double dx = move_loc.x - projector->Pos().x;
            double t  = -1 * ((projector->Pos().x - dx) / focal_vect.x);

            if (t > 0) {
               Point  p  = projector->Pos() + focal_vect * t;
               move_alt = p.y - base_alt;
            }
         }
         else {
            double dz = move_loc.z - projector->Pos().z;
            double t  = -1 * ((projector->Pos().z - dz) / focal_vect.z);
            Point  p  = projector->Pos() + focal_vect * t;

            if (t > 0) {
               Point  p  = projector->Pos() + focal_vect * t;
               move_alt = p.y - base_alt;
            }
         }

         if (move_alt > 25e3)
            move_alt = 25e3;
         else if (move_alt < -25e3)
            move_alt = -25e3;

         return true;
      }
      else {
         if (fabs(focal_vect.y) > 1e-5) {
            if (Mouse::RButton())
               return true;

            bool   clamp = false;
            double t = -1 * ((projector->Pos().y - base_alt) / focal_vect.y);

            while (t <= 0 && my < height-1) {
               my++;
               clamp = true;

               focal_vect  = projector->vpn() * focal_dist +
                             projector->vup() * -1 * (my-height/2) +
                             projector->vrt() * (mx-width/2);

               focal_vect.Normalize();
               t = -1 * ((projector->Pos().y - base_alt) / focal_vect.y);
            }

            if (t > 0) {
               if (clamp)
                  Mouse::SetCursorPos(mx, my);

               move_loc = projector->Pos() + focal_vect * t;
            }

            return true;
         }
      }
   }

   return false;
}

void
TacticalView::DrawMove()
{
   if (!projector || !show_move || !msg_ship) return;

   Point origin = msg_ship->Location();

   if (GetMouseLoc3D()) {
      Point dest = move_loc;

      double distance = (dest - origin).length();

      projector->Transform(origin);
      projector->Project(origin);

      int x0 = (int) origin.x;
      int y0 = (int) origin.y;

      projector->Transform(dest);
      projector->Project(dest);

      int x = (int) dest.x;
      int y = (int) dest.y;

      window->DrawEllipse(x-10, y-10, x+10, y+10, Color::White);
      window->DrawLine(x0, y0, x, y, Color::White);

      char range[32];
      Rect range_rect(x+12, y-8, 120, 20);

      if (fabs(move_alt) > 1) {
         dest = move_loc;
         dest.y += move_alt;
         distance = (dest - msg_ship->Location()).length();

         projector->Transform(dest);
         projector->Project(dest);

         int x1 = (int) dest.x;
         int y1 = (int) dest.y;

         window->DrawEllipse(x1-10, y1-10, x1+10, y1+10, Color::White);
         window->DrawLine(x0, y0, x1, y1, Color::White);
         window->DrawLine(x1, y1, x,  y,  Color::White);

         range_rect.x = x1+12;
         range_rect.y = y1-8;
      }

      FormatNumber(range, distance);
      font->SetColor(Color::White);
      font->DrawText(range, 0, range_rect, DT_LEFT | DT_SINGLELINE);
      font->SetColor(txt_color);
   }
}

void
TacticalView::SendMove()
{
   if (!projector || !show_move || !msg_ship) return;

   if (GetMouseLoc3D()) {
      Element* elem = msg_ship->GetElement();
      RadioMessage* msg = new(__FILE__,__LINE__) RadioMessage(elem, ship, RadioMessage::MOVE_PATROL);
      Point dest = move_loc;
      dest.y += move_alt;
      msg->SetLocation(dest);
      RadioTraffic::Transmit(msg);
      HUDSounds::PlaySound(HUDSounds::SND_TAC_ACCEPT);
   }
}

// +--------------------------------------------------------------------+

static int invalid_action = false;

void
TacticalView::DrawAction()
{
   if (!projector || !show_action || !msg_ship) return;

   Point origin = msg_ship->Location();
   projector->Transform(origin);
   projector->Project(origin);

   int x0 = (int) origin.x;
   int y0 = (int) origin.y;

   int mx = mouse_action.x;
   int my = mouse_action.y;
   int r  = 10;

   int enemy = 2;
   if (ship->GetIFF() > 1)
      enemy = 1;

   Ship* tgt = WillSelectAt(mx, my);
   int   tgt_iff = 0;

   if (tgt)
      tgt_iff = tgt->GetIFF();

   Color c = Color::White;

   switch (show_action) {
   case RadioMessage::ATTACK:
   case RadioMessage::BRACKET:
      c = Ship::IFFColor(enemy);
      if (tgt) {
         if (tgt_iff == ship->GetIFF() || tgt_iff == 0)
            r = 0;
      }
      break;

   case RadioMessage::ESCORT:
   case RadioMessage::DOCK_WITH:
      c = ship->MarkerColor();
      if (tgt) {
         if (tgt_iff == enemy)
            r = 0;

         // must have a hangar to dock with...
         if (show_action == RadioMessage::DOCK_WITH && tgt->GetHangar() == 0)
            r = 0;
      }
      break;

   default:                               
      if (tgt) {
         if (tgt_iff == ship->GetIFF())
            r = 0;
      }
      break;
   }

   if (tgt && r) {
      if ((Game::RealTime()/200) & 1)
         r = 20;
      else
         r = 15;
   }

   if (r) {
      invalid_action = false;
      window->DrawEllipse(mx-r, my-r, mx+r, my+r, c);
   }

   else {
      invalid_action = true;
      window->DrawLine(mx-10, my-10, mx+10, my+10, c);
      window->DrawLine(mx+10, my-10, mx-10, my+10, c);
   }

   window->DrawLine(x0, y0, mx, my, c);
}

void
TacticalView::SendAction()
{
   if (!show_action || !msg_ship || invalid_action) {
      HUDSounds::PlaySound(HUDSounds::SND_TAC_REJECT);
      return;
   }

   int mx = mouse_action.x;
   int my = mouse_action.y;

   Ship* tgt = WillSelectAt(mx, my);

   if (tgt) {
      Element*       elem     = msg_ship->GetElement();
      RadioMessage*  msg      = new(__FILE__,__LINE__) RadioMessage(elem, ship, show_action);

      /***
      Element*       tgt_elem = tgt->GetElement();

      if (tgt_elem) {
         for (int i = 1; i <= tgt_elem->NumShips(); i++)
            msg->AddTarget(tgt_elem->GetShip(i));
      }
      else {
         msg->AddTarget(tgt);
      }
      ***/

      msg->AddTarget(tgt);

      RadioTraffic::Transmit(msg);
      HUDSounds::PlaySound(HUDSounds::SND_TAC_ACCEPT);
   }
   else {
      HUDSounds::PlaySound(HUDSounds::SND_TAC_REJECT);
   }
}


