/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         QuitView.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    View class for End Mission menu
*/

#ifndef QuitView_h
#define QuitView_h

#include "Types.h"
#include "View.h"
#include "Bitmap.h"
#include "Font.h"
#include "SimObject.h"
#include "Text.h"

// +--------------------------------------------------------------------+

class HUDView;
class Menu;

// +--------------------------------------------------------------------+

class QuitView : public View
{
public:
   QuitView(Window* c);
   virtual ~QuitView();

   // Operations:
   virtual void      Refresh();
   virtual void      OnWindowMove();
   virtual void      ExecFrame();

   virtual bool      CanAccept();
   virtual bool      IsMenuShown();
   virtual void      ShowMenu();
   virtual void      CloseMenu();

   static void       Initialize();
   static void       Close();

   static QuitView*  GetInstance() { return quit_view; }

protected:
   int         width, height;
   int         xcenter, ycenter;
   bool        mouse_latch;

   Sim*        sim;

   static QuitView* quit_view;
};

#endif QuitView_h

