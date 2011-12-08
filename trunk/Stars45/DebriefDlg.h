/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         DebriefDlg.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Mission Debriefing Dialog Active Window class
*/

#ifndef DebriefDlg_h
#define DebriefDlg_h

#include "Types.h"
#include "FormWindow.h"
#include "Bitmap.h"
#include "Button.h"
#include "ComboBox.h"
#include "ListBox.h"
#include "Font.h"
#include "Text.h"

// +--------------------------------------------------------------------+

class PlanScreen;
class Campaign;
class Mission;
class MissionInfo;
class Sim;
class Ship;

// +--------------------------------------------------------------------+

class DebriefDlg : public FormWindow
{
public:
   DebriefDlg(Screen* s, FormDef& def, PlanScreen* mgr);
   virtual ~DebriefDlg();

   virtual void      RegisterControls();
   virtual void      ExecFrame();
   virtual void      Show();

   // Operations:
   virtual void      OnClose(AWEvent* event);
   virtual void      OnUnit(AWEvent* event);
   
protected:
   virtual void      DrawUnits();

   PlanScreen*       manager;
   Button*           close_btn;

   ActiveWindow*     mission_name;
   ActiveWindow*     mission_system;
   ActiveWindow*     mission_sector;
   ActiveWindow*     mission_time_start;

   ActiveWindow*     objectives;
   ActiveWindow*     situation;
   ActiveWindow*     mission_score;

   ListBox*          unit_list;
   ListBox*          summary_list;
   ListBox*          event_list;

   Campaign*         campaign;
   Mission*          mission;
   MissionInfo*      info;
   int               unit_index;

   Sim*              sim;
   Ship*             ship;
};

#endif DebriefDlg_h

