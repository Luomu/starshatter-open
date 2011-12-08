/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         CmdMsgDlg.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Navigation Active Window class
*/

#ifndef CmdMsgDlg_h
#define CmdMsgDlg_h

#include "Types.h"
#include "FormWindow.h"

// +--------------------------------------------------------------------+

class CmpnScreen;

// +--------------------------------------------------------------------+

class CmdMsgDlg : public FormWindow
{
public:
   CmdMsgDlg(Screen* s, FormDef& def, CmpnScreen* mgr);
   virtual ~CmdMsgDlg();

   virtual void      RegisterControls();
   virtual void      Show();

   // Operations:
   virtual void      ExecFrame();
   virtual void      OnApply(AWEvent* event);

   ActiveWindow*     Title()     { return title;   }
   ActiveWindow*     Message()   { return message; }

protected:
   CmpnScreen*       manager;

   ActiveWindow*     title;
   ActiveWindow*     message;

   Button*           apply;
   bool              exit_latch;
};

#endif CmdMsgDlg_h

