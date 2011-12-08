/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         MapView.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Star Map class
*/

#ifndef MapView_h
#define MapView_h

#include "Types.h"
#include "SimObject.h"
#include "View.h"
#include "EventTarget.h"
#include "Bitmap.h"
#include "List.h"
#include "Text.h"

// +--------------------------------------------------------------------+

class ActiveWindow;
class StarSystem;
class Orbital;
class OrbitalRegion;
class Ship;
class Instruction;
class Mission;
class MissionElement;
class Campaign;
class Combatant;
class CombatGroup;
class Menu;
class MenuItem;
class MenuView;
class Font;

const int EID_MAP_CLICK = 1000;

// +--------------------------------------------------------------------+

class MapView : public View, 
                public EventTarget, 
                public SimObserver
{
public:
   MapView(Window* win);
   virtual ~MapView();

   // Operations:
   virtual void      Refresh();
   virtual void      OnWindowMove();
   virtual void      OnShow();
   virtual void      OnHide();

   virtual void      DrawTitle();
   virtual void      DrawGalaxy();
   virtual void      DrawSystem();
   virtual void      DrawRegion();

   virtual void      DrawGrid();
   virtual void      DrawOrbital(Orbital& orbital, int index);
   virtual void      DrawShip(Ship& ship,           bool current=false, int rep=3);
   virtual void      DrawElem(MissionElement& elem, bool current=false, int rep=3);
   virtual void      DrawNavRoute(OrbitalRegion*      rgn,
                                  List<Instruction>&  route,
                                  Color               smarker,
                                  Ship*               ship=0,
                                  MissionElement*     elem=0);

   virtual void      DrawCombatantSystem(Combatant* c, Orbital* rgn, int x, int y, int r);
   virtual void      DrawCombatGroupSystem(CombatGroup* g, Orbital* rgn, int x1, int x2, int& y, int a);
   virtual void      DrawCombatGroup(CombatGroup* g, int rep=3);

   virtual int       GetViewMode() const { return view_mode; }
   virtual void      SetViewMode(int mode);
   virtual void      SetSelectionMode(int mode);
   virtual int       GetSelectionMode() const { return seln_mode; }
   virtual void      SetSelection(int index);
   virtual void      SetSelectedShip(Ship* ship);
   virtual void      SetSelectedElem(MissionElement* elem);
   virtual void      SetRegion(OrbitalRegion* rgn);
   virtual void      SetRegionByName(const char* rgn_name);
   virtual void      SelectAt(int x, int y);
   virtual Orbital*  GetSelection();
   virtual Ship*     GetSelectedShip();
   virtual MissionElement* GetSelectedElem();
   virtual int       GetSelectionIndex();
   virtual void      SetShipFilter(DWORD f) { ship_filter = f; }

   // Event Target Interface:
   virtual int       OnMouseMove(int x, int y);
   virtual int       OnLButtonDown(int x, int y);
   virtual int       OnLButtonUp(int x, int y);
   virtual int       OnClick();
   virtual int       OnRButtonDown(int x, int y);
   virtual int       OnRButtonUp(int x, int y);

   virtual bool      IsEnabled()    const;
   virtual bool      IsVisible()    const;
   virtual bool      IsFormActive() const;
   virtual Rect      TargetRect()   const;

   void              ZoomIn();
   void              ZoomOut();

   void              SetGalaxy(List<StarSystem>& systems);
   void              SetSystem(StarSystem* s);
   void              SetMission(Mission* m);
   void              SetShip(Ship* s);
   void              SetCampaign(Campaign* c);

   bool              IsVisible(const Point& loc);

   // accessors:
   virtual void      GetClickLoc(double& x, double& y) { x = click_x; y = click_y; }
   List<StarSystem>& GetGalaxy()       { return system_list; }
   StarSystem*       GetSystem() const { return system; }
   OrbitalRegion*    GetRegion() const;

   virtual bool         Update(SimObject* obj);
   virtual const char*  GetObserverName() const { return "MapWin"; }

   bool              GetEditorMode()            const { return editor;      }
   void              SetEditorMode(bool b)            { editor = b;         }

protected:
   virtual void      BuildMenu();
   virtual void      ClearMenu();
   virtual void      ProcessMenuItem(int action);
   virtual bool      SetCapture();
   virtual bool      ReleaseCapture();

   virtual void      DrawTabbedText(Font* font, const char* text);

   bool              IsClutter(Ship& s);
   bool              IsCrowded(Ship& s);
   bool              IsCrowded(MissionElement& elem);
   void              GetShipLoc(Ship& s, POINT& loc);
   void              GetElemLoc(MissionElement& s, POINT& loc);
   void              SelectShip(Ship* selship);
   void              SelectElem(MissionElement* selelem);
   void              SelectNavpt(Instruction* navpt);
   void              FindShips(bool friendly, bool station, bool starship, bool dropship,
                               List<Text>& result);
   void              SetupScroll(Orbital* s);

   double            GetMinRadius(int type);

   Text              title;
   Rect              rect;
   Campaign*         campaign;
   Mission*          mission;
   List<StarSystem>  system_list;
   StarSystem*       system;
   List<Orbital>     stars;
   List<Orbital>     planets;
   List<Orbital>     regions;
   Ship*             ship;
   Bitmap            galaxy_image;
   bool              editor;
   
   int               current_star;
   int               current_planet;
   int               current_region;
   Ship*             current_ship;
   MissionElement*   current_elem;
   Instruction*      current_navpt;
   int               current_status;

   int               view_mode;
   int               seln_mode;
   bool              captured;
   bool              dragging;
   bool              adding_navpt;
   bool              moving_navpt;
   bool              moving_elem;
   int               scrolling;
   int               mouse_x;
   int               mouse_y;
   DWORD             ship_filter;

   double            zoom;
   double            view_zoom[3];
   double            offset_x;
   double            offset_y;
   double            view_offset_x[3];
   double            view_offset_y[3];
   double            c, r;
   double            scroll_x;
   double            scroll_y;
   double            click_x;
   double            click_y;

   Font*             font;
   Font*             title_font;

   ActiveWindow*     active_window;
   Menu*             active_menu;

   Menu*       map_menu;
   Menu*       map_system_menu;
   Menu*       map_sector_menu;
   Menu*       ship_menu;
   Menu*       nav_menu;
   Menu*       action_menu;
   Menu*       objective_menu;
   Menu*       formation_menu;
   Menu*       speed_menu;
   Menu*       hold_menu;
   Menu*       farcast_menu;

   MenuView*   menu_view;
};

#endif MapView_h

