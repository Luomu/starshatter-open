/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         ConfirmDlg.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    General-purpose confirmation dialog class
*/

#ifndef ConfirmDlg_h
#define ConfirmDlg_h

#include "Types.h"
#include "FormWindow.h"

// +--------------------------------------------------------------------+

class MenuScreen;

// +--------------------------------------------------------------------+

class ConfirmDlg : public FormWindow
{
public:
   ConfirmDlg(Screen* s, FormDef& def, MenuScreen* mgr);
   virtual ~ConfirmDlg();

   virtual void      RegisterControls();
   virtual void      Show();

   ActiveWindow*     GetParentControl();
   void              SetParentControl(ActiveWindow* p);

   Text              GetTitle();
   void              SetTitle(const char* t);

   Text              GetMessage();
   void              SetMessage(const char* m);

   // Operations:
   virtual void      ExecFrame();

   virtual void      OnApply(AWEvent* event);
   virtual void      OnCancel(AWEvent* event);

protected:
   MenuScreen*       manager;

   ActiveWindow*     lbl_title;
   ActiveWindow*     lbl_message;

   Button*           btn_apply;
   Button*           btn_cancel;

   ActiveWindow*     parent_control;
};

#endif ConfirmDlg_h

