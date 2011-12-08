/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         CmdDlg.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Operational Command Dialog Active Window class
*/

#ifndef CmdDlg_h
#define CmdDlg_h

#include "Types.h"
#include "FormWindow.h"
#include "Bitmap.h"
#include "Button.h"
#include "ComboBox.h"
#include "ListBox.h"
#include "Font.h"
#include "Text.h"

// +--------------------------------------------------------------------+

class CmpnScreen;
class Campaign;
class Combatant;
class CombatGroup;
class CombatUnit;
class Starshatter;

// +--------------------------------------------------------------------+

class CmdDlg
{
public:
   enum MODE { 
      MODE_ORDERS,
      MODE_THEATER,
      MODE_FORCES,
      MODE_INTEL,
      MODE_MISSIONS,
      NUM_MODES
   };

   CmdDlg(CmpnScreen* mgr);
   virtual ~CmdDlg();

   virtual void      RegisterCmdControls(FormWindow* win);
   virtual void      ShowCmdDlg();

   virtual void      ExecFrame();

   // Operations:
   virtual void      OnMode(AWEvent* event);
   virtual void      OnSave(AWEvent* event);
   virtual void      OnExit(AWEvent* event);
   
protected:
   virtual void      ShowMode();

   CmpnScreen*       cmpn_screen;

   ActiveWindow*     txt_group;
   ActiveWindow*     txt_score;
   ActiveWindow*     txt_name;
   ActiveWindow*     txt_time;
   Button*           btn_mode[NUM_MODES];
   Button*           btn_save;
   Button*           btn_exit;

   Starshatter*      stars;
   Campaign*         campaign;

   int               mode;
};

#endif CmdDlg_h

