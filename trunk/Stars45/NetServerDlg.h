/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         NetServerDlg.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Main Menu Dialog Active Window class
*/

#ifndef NetServerDlg_h
#define NetServerDlg_h

#include "Types.h"
#include "FormWindow.h"
#include "Bitmap.h"
#include "Button.h"
#include "ComboBox.h"
#include "ListBox.h"
#include "Font.h"

// +--------------------------------------------------------------------+

class MenuScreen;
class NetServerConfig;

// +--------------------------------------------------------------------+

class NetServerDlg : public FormWindow
{
public:
   NetServerDlg(Screen* s, FormDef& def, MenuScreen* mgr);
   virtual ~NetServerDlg();

   virtual void      RegisterControls();
   virtual void      Show();
   virtual void      ExecFrame();

   // Operations:
   virtual void      OnApply(AWEvent* event);
   virtual void      OnCancel(AWEvent* event);
   
protected:
   MenuScreen*       manager;
   NetServerConfig*  config;

   EditBox*          edt_name;
   ComboBox*         cmb_type;
   EditBox*          edt_game_port;
   EditBox*          edt_admin_port;
   EditBox*          edt_game_pass;
   EditBox*          edt_admin_name;
   EditBox*          edt_admin_pass;

   Button*           btn_apply;
   Button*           btn_cancel;
};

#endif NetServerDlg_h

