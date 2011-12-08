/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         JoyDlg.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
*/

#include "MemDebug.h"
#include "JoyDlg.h"
#include "KeyMap.h"
#include "MenuScreen.h"
#include "Starshatter.h"
#include "FormatUtil.h"

#include "Game.h"
#include "ListBox.h"
#include "ComboBox.h"
#include "Button.h"
#include "Joystick.h"

// +--------------------------------------------------------------------+
// DECLARE MAPPING FUNCTIONS:

DEF_MAP_CLIENT(JoyDlg, OnApply);
DEF_MAP_CLIENT(JoyDlg, OnCancel);
DEF_MAP_CLIENT(JoyDlg, OnAxis);

static const char* joy_axis_names[] = {
   "JoyDlg.axis.0",
   "JoyDlg.axis.1",
   "JoyDlg.axis.2",
   "JoyDlg.axis.3",
   "JoyDlg.axis.4",
   "JoyDlg.axis.5",
   "JoyDlg.axis.6",
   "JoyDlg.axis.7"
};

static int selected_axis   = -1;
static int sample_axis     = -1;
static int samples[8];

static int map_axis[4];

// +--------------------------------------------------------------------+

JoyDlg::JoyDlg(Screen* s, FormDef& def, BaseScreen* mgr)
   : FormWindow(s,  0,  0, s->Width(), s->Height()), manager(mgr),
     apply(0), cancel(0), message(0)
{
   Init(def);
}

JoyDlg::~JoyDlg()
{
}

void
JoyDlg::RegisterControls()
{
   if (apply)
      return;

   for (int i = 0; i < 4; i++) {
      axis_button[i]       = (Button*) FindControl(201 + i);
      invert_checkbox[i]   = (Button*) FindControl(301 + i);

      if (axis_button[i])
         REGISTER_CLIENT(EID_CLICK, axis_button[i], JoyDlg, OnAxis);
   }

   message = FindControl(11);

   apply   = (Button*) FindControl(1);
   REGISTER_CLIENT(EID_CLICK, apply, JoyDlg, OnApply);

   cancel  = (Button*) FindControl(2);
   REGISTER_CLIENT(EID_CLICK, cancel, JoyDlg, OnCancel);
}

// +--------------------------------------------------------------------+

void
JoyDlg::ExecFrame()
{
   if (selected_axis >= 0 && selected_axis < 4) {
      Joystick* joystick = Joystick::GetInstance();
      if (joystick) {
         joystick->Acquire();

         int delta = 1000;

         for (int i = 0; i < 8; i++) {
            int a = Joystick::ReadRawAxis(i + KEY_JOY_AXIS_X);

            int d = a - samples[i];
            if (d < 0) d = -d;

            if (d > delta && samples[i] < 1e6) {
               delta       = d;
               sample_axis = i;
            }

            samples[i] = a;
         }

         Button* b = axis_button[selected_axis];

         if (sample_axis >= 0) {
            b->SetText(Game::GetText(joy_axis_names[sample_axis]));
            map_axis[selected_axis] = sample_axis;
         }

         else
            b->SetText(Game::GetText("JoyDlg.select"));
      }
   }
}

// +--------------------------------------------------------------------+

void
JoyDlg::Show()
{
   FormWindow::Show();

   for (int i = 0; i < 4; i++) {
      Button* b = axis_button[i];
      if (b) {
         int map = Joystick::GetAxisMap(i) - KEY_JOY_AXIS_X;
         int inv = Joystick::GetAxisInv(i);

         if (map >= 0 && map < 8) {
            b->SetText(Game::GetText(joy_axis_names[map]));
            map_axis[i] = map;
         }
         else {
            b->SetText(Game::GetText("JoyDlg.unmapped"));
         }

         b->SetButtonState(0);

         invert_checkbox[i]->SetButtonState(inv ? 1 : 0);
      }
   }

   SetFocus();
}

// +--------------------------------------------------------------------+

void
JoyDlg::OnAxis(AWEvent* event)
{
   for (int i = 0; i < 4; i++) {
      int      map   = map_axis[i];
      Text     name  = Game::GetText("JoyDlg.unmapped");
      Button*  b     = axis_button[i];

      if (map >= 0 && map < 8)
         name = Game::GetText(joy_axis_names[map]);

      if (b) {
         if (b == event->window) {
            if (selected_axis == i) {
               b->SetText(name);
               b->SetButtonState(0);
               selected_axis = -1;
            }
            else {
               b->SetText(Game::GetText("JoyDlg.select"));
               b->SetButtonState(1);
               selected_axis = i;
            }
         }
         else {
            b->SetText(name);
            b->SetButtonState(0);
         }
      }
   }

   for (int i = 0; i < 8; i++) {
      samples[i] = 10000000;
   }
}

// +--------------------------------------------------------------------+

void
JoyDlg::OnApply(AWEvent* event)
{
   Starshatter* stars = Starshatter::GetInstance();

   if (stars) {
      KeyMap&  keymap    = stars->GetKeyMap();

      keymap.Bind(KEY_AXIS_YAW,        map_axis[0]+KEY_JOY_AXIS_X, 0);
      keymap.Bind(KEY_AXIS_PITCH,      map_axis[1]+KEY_JOY_AXIS_X, 0);
      keymap.Bind(KEY_AXIS_ROLL,       map_axis[2]+KEY_JOY_AXIS_X, 0);
      keymap.Bind(KEY_AXIS_THROTTLE,   map_axis[3]+KEY_JOY_AXIS_X, 0);

      keymap.Bind(KEY_AXIS_YAW_INVERT,       invert_checkbox[0]->GetButtonState(), 0);
      keymap.Bind(KEY_AXIS_PITCH_INVERT,     invert_checkbox[1]->GetButtonState(), 0);
      keymap.Bind(KEY_AXIS_ROLL_INVERT,      invert_checkbox[2]->GetButtonState(), 0);
      keymap.Bind(KEY_AXIS_THROTTLE_INVERT,  invert_checkbox[3]->GetButtonState(), 0);

      keymap.SaveKeyMap("key.cfg", 256);

      stars->MapKeys();
   }

   if (manager)
      manager->ShowCtlDlg();
}

void
JoyDlg::OnCancel(AWEvent* event)
{
   if (manager)
      manager->ShowCtlDlg();
}

// +--------------------------------------------------------------------+
