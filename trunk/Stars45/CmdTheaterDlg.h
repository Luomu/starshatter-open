/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         CmdTheaterDlg.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Operational Command Dialog (Order of Battle Tab)
*/

#ifndef CmdTheaterDlg_h
#define CmdTheaterDlg_h

#include "Types.h"
#include "FormWindow.h"
#include "CmdDlg.h"
#include "Bitmap.h"
#include "Button.h"
#include "ComboBox.h"
#include "ListBox.h"
#include "MapView.h"
#include "Font.h"
#include "Text.h"

// +--------------------------------------------------------------------+

class CmdTheaterDlg : public FormWindow,
                      public CmdDlg
{
public:
   CmdTheaterDlg(Screen* s, FormDef& def, CmpnScreen* mgr);
   virtual ~CmdTheaterDlg();

   virtual void      RegisterControls();
   virtual void      Show();
   virtual void      ExecFrame();

   // Operations:
   virtual void      OnMode(AWEvent* event);
   virtual void      OnSave(AWEvent* event);
   virtual void      OnExit(AWEvent* event);
   virtual void      OnView(AWEvent* event);

protected:
   CmpnScreen*       manager;

   ActiveWindow*     map_theater;
   MapView*          map_view;
   Button*           view_btn[3];
   Button*           zoom_in_btn;
   Button*           zoom_out_btn;

   Starshatter*      stars;
   Campaign*         campaign;
};

#endif CmdTheaterDlg_h

