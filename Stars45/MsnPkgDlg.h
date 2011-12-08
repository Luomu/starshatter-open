/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         MsnPkgDlg.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Mission Briefing Dialog Active Window class
*/

#ifndef MsnPkgDlg_h
#define MsnPkgDlg_h

#include "Types.h"
#include "FormWindow.h"
#include "MsnDlg.h"
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

// +--------------------------------------------------------------------+

class MsnPkgDlg : public FormWindow,
                  public MsnDlg
{
public:
   MsnPkgDlg(Screen* s, FormDef& def, PlanScreen* mgr);
   virtual ~MsnPkgDlg();

   virtual void      RegisterControls();
   virtual void      ExecFrame();
   virtual void      Show();

   // Operations:
   virtual void      OnCommit(AWEvent* event);
   virtual void      OnCancel(AWEvent* event);
   virtual void      OnTabButton(AWEvent* event);
   virtual void      OnPackage(AWEvent* event);
   
protected:
   virtual void      DrawPackages();
   virtual void      DrawNavPlan();
   virtual void      DrawThreats();

   ListBox*          pkg_list;
   ListBox*          nav_list;

   ActiveWindow*     threat[5];
};

#endif MsnPkgDlg_h

