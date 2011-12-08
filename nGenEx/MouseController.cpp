/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    nGenEx.lib
    FILE:         MouseController.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    MouseController Input class
*/

#include "MemDebug.h"
#include "MouseController.h"
#include "Keyboard.h"
#include "Mouse.h"
#include "Game.h"
#include "Video.h"

// +--------------------------------------------------------------------+

static MouseController* instance = 0;
static DWORD rbutton_latch       = 0;
static DWORD mbutton_latch       = 0;
static DWORD active_latch        = 0;

// +--------------------------------------------------------------------+

MouseController::MouseController()
   : p(0), r(0), w(0), dx(0), dy(0), t(0)
{
   instance    = this;
   select      =   0;
   sensitivity =  10;
   swapped     =   0;
   active      = false;

   active_key  = 20; // caps lock

   rbutton_latch = 0;

   for (int i = 0; i < MotionController::MaxActions; i++)
      action[i] = 0;
}

MouseController::~MouseController()
{
   instance = 0;
}

MouseController*
MouseController::GetInstance()
{
   return instance;
}

// +--------------------------------------------------------------------+

void
MouseController::MapKeys(KeyMapEntry* mapping, int nkeys)
{
   for (int i = 0; i < nkeys; i++) {
      KeyMapEntry k = mapping[i];

      if (k.act >= KEY_MAP_FIRST && k.act <= KEY_MAP_LAST) {

         if (k.act == KEY_MOUSE_SENSE)
            sensitivity = k.key;

         else if (k.act == KEY_MOUSE_SWAP)
            swapped = k.key;

         else if (k.act == KEY_MOUSE_INVERT)
            inverted = k.key;

         else if (k.act == KEY_MOUSE_SELECT)
            select = k.key;

         else if (k.act == KEY_MOUSE_ACTIVE)
            active_key = k.key;
      }
   }
}

// +--------------------------------------------------------------------+

static inline double sqr(double a) { return a*a; }

void
MouseController::Acquire()
{
   p = r = w = 0;
   action[0] = 0;
   action[1] = 0;
   action[2] = 0;
   action[3] = 0;

   if (active_key && Keyboard::KeyDown(active_key)) {
      active_latch = 1;
   }
   else {
      if (active_latch) {
         active_latch = 0;
         active = !active;
      }
   }
   
   if (!select || !active)
      return;

   action[0] = Mouse::LButton();

   int roll_enable = 0;

   if (Mouse::RButton()) {
      roll_enable = 1;

      if (!rbutton_latch)
         rbutton_latch = Game::RealTime();
   }
   else {
      if (rbutton_latch) {
         rbutton_latch = Game::RealTime() - rbutton_latch;
         if (rbutton_latch < 250)
            action[1] = 1;
      }

      rbutton_latch = 0;
   }

   if (Mouse::MButton()) {
      if (!mbutton_latch)
         mbutton_latch = Game::RealTime();
   }
   else {
      if (mbutton_latch) {
         action[3] = 1;
      }

      mbutton_latch = 0;
   }

   double   step = 0;
   int      cx   = Video::GetInstance()->Width()/2;
   int      cy   = Video::GetInstance()->Height()/2;

   dx += Mouse::X() - cx;
   dy += Mouse::Y() - cy;

   step = fabs(dx)/cx;

   if (roll_enable || select == 1)
      step *= 3 * sensitivity;
   else
      step *= step * sensitivity/4;

   if (roll_enable) {
      if (dx > 0)
         r = -step;
      else if (dx < 0)
         r =  step;
   }
   else {
      if (dx > 0)
         w =  step;
      else if (dx < 0)
         w = -step;
   }

   step = fabs(dy)/cy;

   if (select == 1)
      step *= 2 * sensitivity;
   else
      step *= step * sensitivity/4;

   if (inverted) {
      step *= -1;
   }

   if (dy > 0)
         p  = step;
   else if (dy < 0)
         p = -step;

   if (select == 1) {
      ::SetCursorPos(cx, cy);

      double drain = cx * 4 * Game::FrameTime();

      if (dx > drain) {
         dx -= drain;
      }
      else if (dx < -drain) {
         dx += drain;
      }
      else {
         dx = 0;
      }

      if (dy > drain) {
         dy -= drain;
      }
      else if (dy < -drain) {
         dy += drain;
      }
      else {
         dy = 0;
      }
   }
   else {
      dx = 0;
      dy = 0;
   }

   if (Mouse::Wheel() > 0) {
      if (t < 0.25)
         t += 0.025;
      else
         t += 0.1;

      if (t > 1) t = 1;
   }

   else if (Mouse::Wheel() < 0) {
      if (t < 0.25)
         t -= 0.025;
      else
         t -= 0.1;

      if (t < 0) t = 0;
   }
}

// +--------------------------------------------------------------------+

int
MouseController::ActionMap(int n)
{
   if (n >= KEY_ACTION_0 && n <= KEY_ACTION_3)
      return action[n - KEY_ACTION_0];

   return 0;
}

