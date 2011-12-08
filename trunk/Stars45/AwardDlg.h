/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         AwardDlg.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Main Menu Dialog Active Window class
*/

#ifndef AwardDlg_h
#define AwardDlg_h

#include "Types.h"
#include "FormWindow.h"
#include "Bitmap.h"
#include "Button.h"
#include "ComboBox.h"
#include "ListBox.h"
#include "Font.h"

// +--------------------------------------------------------------------+

class PlanScreen;
class Player;

// +--------------------------------------------------------------------+

class AwardDlg : public FormWindow
{
public:
   AwardDlg(Screen* s, FormDef& def, PlanScreen* mgr);
   virtual ~AwardDlg();

   virtual void      RegisterControls();
   virtual void      Show();
   virtual void      ExecFrame();

   // Operations:
   virtual void      OnClose(AWEvent* event);
   virtual void      ShowPlayer();

protected:
   PlanScreen*       manager;

   ActiveWindow*     lbl_name;
   ActiveWindow*     lbl_info;
   ImageBox*         img_rank;
   Button*           btn_close;

   bool              exit_latch;
};

#endif AwardDlg_h

