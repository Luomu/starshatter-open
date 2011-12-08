/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         MsnEditNavDlg.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Mission Briefing Dialog Active Window class
*/

#ifndef MsnEditNavDlg_h
#define MsnEditNavDlg_h

#include "Types.h"
#include "NavDlg.h"
#include "Bitmap.h"
#include "Button.h"
#include "ComboBox.h"
#include "EditBox.h"
#include "ListBox.h"
#include "Font.h"
#include "Text.h"

// +--------------------------------------------------------------------+

class MenuScreen;
class Campaign;
class Mission;
class MissionInfo;

// +--------------------------------------------------------------------+

class MsnEditNavDlg : public NavDlg
{
public:
   friend class MsnEditDlg;

   MsnEditNavDlg(Screen* s, FormDef& def, MenuScreen* mgr);
   virtual ~MsnEditNavDlg();

   virtual void      RegisterControls();
   virtual void      Show();
   virtual void      SetMissionInfo(MissionInfo* m);
   virtual void      ScrapeForm();

   // Operations:
   virtual void      OnCommit(AWEvent* event);
   virtual void      OnCancel(AWEvent* event);
   virtual void      OnTabButton(AWEvent* event);
   virtual void      OnSystemSelect(AWEvent* event);

protected:
   virtual void      ShowTab(int tab);

   MenuScreen*       menu_screen;

   Button*           btn_accept;
   Button*           btn_cancel;

   Button*           btn_sit;
   Button*           btn_pkg;
   Button*           btn_map;

   EditBox*          txt_name;
   ComboBox*         cmb_type;
   ComboBox*         cmb_system;
   ComboBox*         cmb_region;
   MissionInfo*      mission_info;

   bool              exit_latch;
};

#endif MsnEditNavDlg_h

