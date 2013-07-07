/*  Starshatter OpenSource Distribution
    Copyright (c) 1997-2004, Destroyer Studios LLC.
    All Rights Reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.
    * Neither the name "Destroyer Studios" nor the names of its contributors
      may be used to endorse or promote products derived from this software
      without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

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

