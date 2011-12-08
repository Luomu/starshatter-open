/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         ModDlg.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Mod Config Dialog Active Window class
*/

#ifndef ModDlg_h
#define ModDlg_h

#include "Types.h"
#include "FormWindow.h"
#include "Bitmap.h"
#include "Button.h"
#include "ComboBox.h"
#include "ListBox.h"
#include "Font.h"
#include "Text.h"

// +--------------------------------------------------------------------+

class  BaseScreen;
class  Campaign;
class  ModConfig;

// +--------------------------------------------------------------------+

class ModDlg : public FormWindow
{
public:
   ModDlg(Screen* s, FormDef& def, BaseScreen* mgr);
   virtual ~ModDlg();

   virtual void      RegisterControls();
   virtual void      Show();
   virtual void      ExecFrame();

   // Operations:

   virtual void      OnIncrease(AWEvent* event);
   virtual void      OnDecrease(AWEvent* event);

   virtual void      OnSelectEnabled(AWEvent* event);
   virtual void      OnSelectDisabled(AWEvent* event);

   virtual void      OnEnable(AWEvent* event);
   virtual void      OnDisable(AWEvent* event);

   virtual void      OnAccept(AWEvent* event);
   virtual void      OnCancel(AWEvent* event);

   virtual void      Apply();
   virtual void      Cancel();

   virtual void      OnAudio(AWEvent* event);
   virtual void      OnVideo(AWEvent* event);
   virtual void      OnOptions(AWEvent* event);
   virtual void      OnControls(AWEvent* event);
   virtual void      OnMod(AWEvent* event);
   
protected:
   void              UpdateLists();

   BaseScreen*       manager;

   ListBox*          lst_disabled;
   ListBox*          lst_enabled;

   Button*           btn_accept;
   Button*           btn_cancel;
   Button*           btn_enable;
   Button*           btn_disable;
   Button*           btn_increase;
   Button*           btn_decrease;

   Button*           aud_btn;
   Button*           vid_btn;
   Button*           opt_btn;
   Button*           ctl_btn;
   Button*           mod_btn;

   ModConfig*        config;
   bool              changed;
};

#endif ModDlg_h

