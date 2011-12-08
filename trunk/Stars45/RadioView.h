/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         RadioView.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    View class for Radio Communications HUD Overlay
*/

#ifndef RadioView_h
#define RadioView_h

#include "Types.h"
#include "View.h"
#include "Color.h"
#include "SimObject.h"
#include "Text.h"

// +--------------------------------------------------------------------+

class Font;
class Element;
class Ship;
class RadioMessage;
class CameraView;
class HUDView;
class Menu;
class MenuItem;

// +--------------------------------------------------------------------+

class RadioView : public View, 
                  public SimObserver
{
public:
   RadioView(Window* c);
   virtual ~RadioView();

   // Operations:
   virtual void      Refresh();
   virtual void      OnWindowMove();
   virtual void      ExecFrame();

   virtual Menu*     GetRadioMenu(Ship* ship);
   virtual bool      IsMenuShown();
   virtual void      ShowMenu();
   virtual void      CloseMenu();

   static void       Message(const char* msg);
   static void       ClearMessages();

   virtual bool         Update(SimObject* obj);
   virtual const char*  GetObserverName() const;

   static void       SetColor(Color c);

   static void       Initialize();
   static void       Close();

   static RadioView* GetInstance() { return radio_view; }

protected:
   void              SendRadioMessage(Ship* ship, MenuItem* item);

   int         width, height;
   double      xcenter, ycenter;

   Font*       font;
   Sim*        sim;
   Ship*       ship;
   Element*    dst_elem;

   enum { MAX_MSG=6 };
   Text        msg_text[MAX_MSG];
   double      msg_time[MAX_MSG];

   static RadioView* radio_view;
   static ThreadSync sync;
};

#endif RadioView_h

