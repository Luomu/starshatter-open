/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         MsnEventDlg.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Mission Editor Element Dialog Active Window class
*/

#ifndef MsnEventDlg_h
#define MsnEventDlg_h

#include "Types.h"
#include "FormWindow.h"
#include "Bitmap.h"
#include "Button.h"
#include "ComboBox.h"
#include "ListBox.h"
#include "Font.h"
#include "Text.h"

// +--------------------------------------------------------------------+

class MenuScreen;
class MsnEditDlg;
class Mission;
class MissionElement;
class MissionEvent;

// +--------------------------------------------------------------------+

class MsnEventDlg : public FormWindow
{
public:
   MsnEventDlg(Screen* s, FormDef& def, MenuScreen* mgr);
   virtual ~MsnEventDlg();

   virtual void      RegisterControls();
   virtual void      Show();
   virtual void      ExecFrame();

   // Operations:
   virtual void      SetMission(Mission* msn);
   virtual void      SetMissionEvent(MissionEvent* event);
   virtual void      OnEventSelect(AWEvent* e);
   virtual void      OnAccept(AWEvent* e);
   virtual void      OnCancel(AWEvent* e);

protected:
   virtual void      FillShipList(ComboBox* cmb, const char* seln);
   virtual void      FillRgnList(ComboBox* cmb, const char* seln);

   MenuScreen*       manager;

   ActiveWindow*     lbl_id;
   EditBox*          edt_time;
   EditBox*          edt_delay;

   ComboBox*         cmb_event;
   ComboBox*         cmb_event_ship;
   ComboBox*         cmb_event_source;
   ComboBox*         cmb_event_target;
   EditBox*          edt_event_param;
   EditBox*          edt_event_chance;
   EditBox*          edt_event_sound;
   EditBox*          edt_event_message;

   ComboBox*         cmb_trigger;
   ComboBox*         cmb_trigger_ship;
   ComboBox*         cmb_trigger_target;
   EditBox*          edt_trigger_param;

   Button*           btn_accept;
   Button*           btn_cancel;

   Mission*          mission;
   MissionEvent*     event;
};

#endif MsnEventDlg_h

