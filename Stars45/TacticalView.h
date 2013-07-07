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
    FILE:         TacticalView.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    View class for Radio Communications HUD Overlay
*/

#ifndef TacticalView_h
#define TacticalView_h

#include "Types.h"
#include "View.h"
#include "Color.h"
#include "SimObject.h"
#include "Text.h"

// +--------------------------------------------------------------------+

class Font;
class Ship;
class RadioMessage;
class CameraView;
class Projector;
class HUDView;
class Menu;
class MenuItem;
class MenuView;
class GameScreen;

// +--------------------------------------------------------------------+

class TacticalView : public View, 
public SimObserver
{
public:
    TacticalView(Window* c, GameScreen* parent);
    virtual ~TacticalView();

    // Operations:
    virtual void      Refresh();
    virtual void      OnWindowMove();
    virtual void      ExecFrame();
    virtual void      UseProjector(Projector* p);

    virtual void      DoMouseFrame();

    virtual bool         Update(SimObject* obj);
    virtual const char*  GetObserverName() const;

    static void       SetColor(Color c);

    static void       Initialize();
    static void       Close();

    static TacticalView* GetInstance() { return tac_view; }

protected:
    virtual bool      SelectAt(int x, int y);
    virtual bool      SelectRect(const Rect& r);
    virtual Ship*     WillSelectAt(int x, int y);
    virtual void      SetHelm(bool approach);

    virtual void      DrawMouseRect();
    virtual void      DrawSelection(Ship* seln);
    virtual void      DrawSelectionInfo(Ship* seln);
    virtual void      DrawSelectionList(ListIter<Ship> seln);

    virtual void      BuildMenu();
    virtual void      DrawMenu();
    virtual void      ProcessMenuItem(int action);

    virtual void      DrawMove();
    virtual void      SendMove();
    virtual bool      GetMouseLoc3D();

    virtual void      DrawAction();
    virtual void      SendAction();

    GameScreen* gamescreen;
    CameraView* camview;
    Projector*  projector;

    int         width, height;
    double      xcenter, ycenter;

    int         shift_down;
    int         mouse_down;
    int         right_down;
    int         show_move;
    int         show_action;

    Point       move_loc;
    double      base_alt;
    double      move_alt;

    POINT       mouse_action;
    POINT       mouse_start;
    Rect        mouse_rect;

    Font*       font;
    Sim*        sim;
    Ship*       ship;
    Ship*       msg_ship;
    Text        current_sector;

    Menu*       active_menu;
    MenuView*   menu_view;

    static TacticalView* tac_view;
};

#endif TacticalView_h

