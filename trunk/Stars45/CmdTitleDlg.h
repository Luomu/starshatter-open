/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         CmdTitleDlg.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Campaign Title Card
*/

#ifndef CmdTitleDlg_h
#define CmdTitleDlg_h

#include "Types.h"
#include "FormWindow.h"
#include "Bitmap.h"
#include "Button.h"
#include "ImageBox.h"
#include "ListBox.h"
#include "Font.h"
#include "Text.h"

// +--------------------------------------------------------------------+

class CmpnScreen;
class Campaign;
class Starshatter;

// +--------------------------------------------------------------------+

class CmdTitleDlg : public FormWindow
{
public:
   CmdTitleDlg(Screen* s, FormDef& def, CmpnScreen* mgr);
   virtual ~CmdTitleDlg();

   virtual void      RegisterControls();
   virtual void      Show();
   virtual void      ExecFrame();
   
protected:
   CmpnScreen*       manager;

   ImageBox*         img_title;

   Starshatter*      stars;
   Campaign*         campaign;
   double            showTime;
};

#endif CmdTitleDlg_h

