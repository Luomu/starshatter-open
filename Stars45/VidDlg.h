/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         VidDlg.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Main Menu Dialog Active Window class
*/

#ifndef VidDlg_h
#define VidDlg_h

#include "Types.h"
#include "FormWindow.h"
#include "Bitmap.h"
#include "Button.h"
#include "ComboBox.h"
#include "ListBox.h"
#include "Slider.h"
#include "Font.h"

// +--------------------------------------------------------------------+

class BaseScreen;
class Starshatter;

// +--------------------------------------------------------------------+

class VidDlg : public FormWindow
{
public:
   VidDlg(Screen* s, FormDef& def, BaseScreen* mgr);
   virtual ~VidDlg();

   virtual void      RegisterControls();
   virtual void      Show();
   virtual void      ExecFrame();

   // Operations:
   virtual void      OnTexSize(AWEvent* event);
   virtual void      OnMode(AWEvent* event);
   virtual void      OnDetail(AWEvent* event);
   virtual void      OnTexture(AWEvent* event);
   virtual void      OnGamma(AWEvent* event);

   virtual void      Apply();
   virtual void      Cancel();

   virtual void      OnApply(AWEvent* event);
   virtual void      OnCancel(AWEvent* event);

   virtual void      OnAudio(AWEvent* event);
   virtual void      OnVideo(AWEvent* event);
   virtual void      OnOptions(AWEvent* event);
   virtual void      OnControls(AWEvent* event);
   virtual void      OnMod(AWEvent* event);

protected:
   virtual void      BuildModeList();

   BaseScreen*       manager;
   Starshatter*      stars;

   ComboBox*         mode;
   ComboBox*         tex_size;
   ComboBox*         detail;
   ComboBox*         texture;

   ComboBox*         shadows;
   ComboBox*         bump_maps;
   ComboBox*         spec_maps;

   ComboBox*         lens_flare;
   ComboBox*         corona;
   ComboBox*         nebula;
   ComboBox*         dust;

   Slider*           gamma;

   Button*           aud_btn;
   Button*           vid_btn;
   Button*           opt_btn;
   Button*           ctl_btn;
   Button*           mod_btn;

   Button*           apply;
   Button*           cancel;

   int               selected_render;
   int               selected_card;
   int               selected_tex_size;
   int               selected_mode;
   int               selected_detail;
   int               selected_texture;
   int               orig_gamma;

   bool              closed;
};

#endif VidDlg_h

