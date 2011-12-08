/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         NavDlg.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Navigation Active Window class
*/

#ifndef NavDlg_h
#define NavDlg_h

#include "Types.h"
#include "FormWindow.h"
#include "Bitmap.h"
#include "Button.h"
#include "Font.h"

class BaseScreen;
class MapView;
class StarSystem;
class Ship;
class SimRegion;
class Orbital;
class OrbitalRegion;
class Mission;

// +--------------------------------------------------------------------+

class NavDlg : public FormWindow
{
public:
   NavDlg(Screen* s, FormDef& def, BaseScreen* mgr);
   virtual ~NavDlg();

   virtual void      RegisterControls();

   // Operations:
   virtual void      OnView(AWEvent* event);
   virtual void      OnFilter(AWEvent* event);
   virtual void      OnSelect(AWEvent* event);
   virtual void      OnCommit(AWEvent* event);
   virtual void      OnCancel(AWEvent* event);
   virtual void      OnEngage(AWEvent* event);
   virtual void      OnMapDown(AWEvent* event);
   virtual void      OnMapMove(AWEvent* event);
   virtual void      OnMapClick(AWEvent* event);
   virtual void      OnClose(AWEvent* event);

   virtual void      ExecFrame();
   StarSystem*       GetSystem()                const { return star_system; }
   void              SetSystem(StarSystem* s);
   Mission*          GetMission()               const { return mission;     }
   void              SetMission(Mission* m);
   Ship*             GetShip()                  const { return ship;        }
   void              SetShip(Ship* s);

   bool              GetEditorMode()            const { return editor;      }
   void              SetEditorMode(bool b);

   void              UseViewMode(int mode);
   void              UseFilter(int index);
   void              SelectObject(int index);
   void              UpdateSelection();
   void              UpdateLists();
   void              CoordinateSelection();

   void              SelectStar(Orbital* star);
   void              SelectPlanet(Orbital* planet);
   void              SelectRegion(OrbitalRegion* rgn);

   enum NAV_EDIT_MODE { NAV_EDIT_NONE = 0,
                        NAV_EDIT_ADD  = 1,
                        NAV_EDIT_DEL  = 2,
                        NAV_EDIT_MOVE = 3 };

   void              SetNavEditMode(int mode);
   int               GetNavEditMode();

protected:
   Button*           view_btn[3];
   Button*           filter_btn[6];
   Button*           commit_btn;
   Button*           zoom_in_btn;
   Button*           zoom_out_btn;
   Button*           close_btn;

   MapView*          star_map;
   ActiveWindow*     map_win;
   ActiveWindow*     loc_labels;
   ActiveWindow*     dst_labels;
   ActiveWindow*     loc_data;
   ActiveWindow*     dst_data;

   ListBox*          seln_list;
   ListBox*          info_list;
   
   BaseScreen*       manager;
   int               seln_mode;
   int               nav_edit_mode;
   
   StarSystem*          star_system;
   List<Orbital>        stars;
   List<Orbital>        planets;
   List<OrbitalRegion>  regions;
   List<Ship>           contacts;

   Ship*             ship;
   Mission*          mission;
   bool              editor;
};

#endif NavDlg_h

