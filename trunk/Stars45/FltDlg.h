/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         FltDlg.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Flight Operations Active Window class
*/

#ifndef FltDlg_h
#define FltDlg_h

#include "Types.h"
#include "FormWindow.h"

// +--------------------------------------------------------------------+

class FlightPlanner;
class GameScreen;
class Ship;

// +--------------------------------------------------------------------+

class FltDlg : public FormWindow
{
public:
   FltDlg(Screen* s, FormDef& def, GameScreen* mgr);
   virtual ~FltDlg();

   virtual void      RegisterControls();

   // Operations:
   virtual void      Show();
   virtual void      Hide();

   virtual void      OnFilter(AWEvent* event);
   virtual void      OnPackage(AWEvent* event);
   virtual void      OnAlert(AWEvent* event);
   virtual void      OnLaunch(AWEvent* event);
   virtual void      OnStandDown(AWEvent* event);
   virtual void      OnRecall(AWEvent* event);
   virtual void      OnClose(AWEvent* event);
   virtual void      OnMissionType(AWEvent* event);

   virtual void      ExecFrame();
   void              SetShip(Ship* s);
   void              UpdateSelection();
   void              UpdateObjective();
   
protected:
   GameScreen*       manager;

   ComboBox*         filter_list;
   ListBox*          hangar_list;

   Button*           package_btn;
   Button*           alert_btn;
   Button*           launch_btn;
   Button*           stand_btn;
   Button*           recall_btn;
   Button*           close_btn;

   int               mission_type;
   Button*           mission_btn[6];

   ListBox*          objective_list;
   ListBox*          loadout_list;

   Ship*             ship;
   FlightPlanner*    flight_planner;

   int               patrol_pattern;
};

#endif FltDlg_h

