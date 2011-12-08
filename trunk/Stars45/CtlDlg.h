/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         CtlDlg.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Control Options (keyboard/joystick) Dialog Active Window class
*/

#ifndef CtlDlg_h
#define CtlDlg_h

#include "Types.h"
#include "FormWindow.h"
#include "Bitmap.h"
#include "Button.h"
#include "ComboBox.h"
#include "ListBox.h"
#include "Font.h"

// +--------------------------------------------------------------------+

class BaseScreen;
class MenuScreen;
class GameScreen;

// +--------------------------------------------------------------------+

class CtlDlg : public FormWindow
{
public:
   CtlDlg(Screen* s, FormDef& def, BaseScreen* mgr);
   virtual ~CtlDlg();

   virtual void      RegisterControls();
   virtual void      Show();
   virtual void      ExecFrame();

   // Operations:
   virtual void      OnCommand(AWEvent* event);
   virtual void      OnCategory(AWEvent* event);

   virtual void      OnControlModel(AWEvent* event);

   virtual void      OnJoySelect(AWEvent* event);
   virtual void      OnJoyThrottle(AWEvent* event);
   virtual void      OnJoyRudder(AWEvent* event);
   virtual void      OnJoySensitivity(AWEvent* event);
   virtual void      OnJoyAxis(AWEvent* event);

   virtual void      OnMouseSelect(AWEvent* event);
   virtual void      OnMouseSensitivity(AWEvent* event);
   virtual void      OnMouseInvert(AWEvent* event);

   virtual void      Apply();
   virtual void      Cancel();

   virtual void      OnApply(AWEvent* event);
   virtual void      OnCancel(AWEvent* event);

   virtual void      OnAudio(AWEvent* event);
   virtual void      OnVideo(AWEvent* event);
   virtual void      OnOptions(AWEvent* event);
   virtual void      OnControls(AWEvent* event);
   virtual void      OnMod(AWEvent* event);

protected:
   void              ShowCategory();
   void              UpdateCategory();

   BaseScreen*       manager;

   Button*           category[4];
   ListBox*          commands;
   int               command_index;

   ComboBox*         control_model_combo;

   ComboBox*         joy_select_combo;
   ComboBox*         joy_throttle_combo;
   ComboBox*         joy_rudder_combo;
   Slider*           joy_sensitivity_slider;
   Button*           joy_axis_button;

   ComboBox*         mouse_select_combo;
   Slider*           mouse_sensitivity_slider;
   Button*           mouse_invert_checkbox;

   Button*           aud_btn;
   Button*           vid_btn;
   Button*           opt_btn;
   Button*           ctl_btn;
   Button*           mod_btn;

   Button*           apply;
   Button*           cancel;

   int               selected_category;
   int               control_model;

   int               joy_select;
   int               joy_throttle;
   int               joy_rudder;
   int               joy_sensitivity;

   int               mouse_select;
   int               mouse_sensitivity;
   int               mouse_invert;

   bool              closed;
};

#endif CtlDlg_h

