/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         CmpLoadDlg.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Campaign title card and load progress dialog
*/

#ifndef CmpLoadDlg_h
#define CmpLoadDlg_h

#include "Types.h"
#include "FormWindow.h"

// +--------------------------------------------------------------------+

class CmpLoadDlg : public FormWindow
{
public:
   CmpLoadDlg(Screen* s, FormDef& def);
   virtual ~CmpLoadDlg();

   // Operations:
   virtual void      ExecFrame();
   virtual void      MoveTo(const Rect& r);
   virtual void      RegisterControls();
   virtual void      Show();

   virtual bool      IsDone();

protected:
   ActiveWindow*     lbl_activity;
   Slider*           lbl_progress;
   ActiveWindow*     lbl_title;
   ImageBox*         img_title;
   DWORD             show_time;
};

#endif CmpLoadDlg_h

