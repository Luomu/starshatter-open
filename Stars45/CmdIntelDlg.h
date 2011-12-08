/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         CmdIntelDlg.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Operational Command Dialog (Intel/Newsfeed Tab)
*/

#ifndef CmdIntelDlg_h
#define CmdIntelDlg_h

#include "Types.h"
#include "FormWindow.h"
#include "CmdDlg.h"
#include "CameraView.h"
#include "DisplayView.h"

#include "Bitmap.h"
#include "Button.h"
#include "ImageBox.h"
#include "ListBox.h"
#include "RichTextBox.h"
#include "Font.h"
#include "Text.h"

// +--------------------------------------------------------------------+

class CmdIntelDlg : public FormWindow,
                    public CmdDlg
{
public:
   CmdIntelDlg(Screen* s, FormDef& def, CmpnScreen* mgr);
   virtual ~CmdIntelDlg();

   virtual void      RegisterControls();
   virtual void      Show();
   virtual void      ExecFrame();

   // Operations:
   virtual void      OnMode(AWEvent* event);
   virtual void      OnSave(AWEvent* event);
   virtual void      OnExit(AWEvent* event);
   virtual void      OnNews(AWEvent* event);
   virtual void      OnPlay(AWEvent* event);
   
protected:
   virtual void      ShowMovie();
   virtual void      HideMovie();

   CmpnScreen*       manager;

   ListBox*          lst_news;
   RichTextBox*      txt_news;
   ImageBox*         img_news;
   Button*           btn_play;
   ActiveWindow*     mov_news;
   CameraView*       cam_view;
   DisplayView*      dsp_view;

   Bitmap            bmp_default;

   Starshatter*      stars;
   Campaign*         campaign;
   double            update_time;
   int               start_scene;
   Text              event_scene;
};

#endif CmdIntelDlg_h

