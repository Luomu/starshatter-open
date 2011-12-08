/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         JoyDlg.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Navigation Active Window class
*/

#ifndef JoyDlg_h
#define JoyDlg_h

#include "Types.h"
#include "FormWindow.h"

// +--------------------------------------------------------------------+

class BaseScreen;

// +--------------------------------------------------------------------+

class JoyDlg : public FormWindow
{
public:
   JoyDlg(Screen* s, FormDef& def, BaseScreen* mgr);
   virtual ~JoyDlg();

   virtual void      RegisterControls();
   virtual void      Show();

   // Operations:
   virtual void      ExecFrame();

   virtual void      OnApply(AWEvent* event);
   virtual void      OnCancel(AWEvent* event);

   virtual void      OnAxis(AWEvent* event);

protected:
   BaseScreen*       manager;

   ActiveWindow*     message;
   Button*           axis_button[4];
   Button*           invert_checkbox[4];

   Button*           apply;
   Button*           cancel;
};

#endif JoyDlg_h

