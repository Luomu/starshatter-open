/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         QuantumView.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    View class for Radio Communications HUD Overlay
*/

#ifndef QuantumView_h
#define QuantumView_h

#include "Types.h"
#include "View.h"
#include "SimObject.h"
#include "Color.h"
#include "Text.h"

// +--------------------------------------------------------------------+

class Ship;
class RadioMessage;
class HUDView;
class Menu;
class Font;

// +--------------------------------------------------------------------+

class QuantumView : public View, 
                    public SimObserver
{
public:
   QuantumView(Window* c);
   virtual ~QuantumView();

   // Operations:
   virtual void      Refresh();
   virtual void      OnWindowMove();
   virtual void      ExecFrame();

   virtual Menu*     GetQuantumMenu(Ship* ship);
   virtual bool      IsMenuShown();
   virtual void      ShowMenu();
   virtual void      CloseMenu();

   virtual bool         Update(SimObject* obj);
   virtual const char*  GetObserverName() const;

   static void       SetColor(Color c);

   static void       Initialize();
   static void       Close();

   static QuantumView* GetInstance() { return quantum_view; }

protected:
   int         width, height;
   double      xcenter, ycenter;

   Font*       font;
   Sim*        sim;
   Ship*       ship;

   static QuantumView* quantum_view;
};

#endif QuantumView_h

