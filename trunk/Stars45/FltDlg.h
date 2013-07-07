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
    FILE:         FltDlg.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Flight Operations Active Window class
*/

#ifndef FltDlg_h
#define FltDlg_h

#include "Types.h"
#include "FormWindow.h"

// +--------------------------------------------------------------------+

class FlightPlanner;
class GameScreen;
class Ship;

// +--------------------------------------------------------------------+

class FltDlg : public FormWindow
{
public:
    FltDlg(Screen* s, FormDef& def, GameScreen* mgr);
    virtual ~FltDlg();

    virtual void      RegisterControls();

    // Operations:
    virtual void      Show();
    virtual void      Hide();

    virtual void      OnFilter(AWEvent* event);
    virtual void      OnPackage(AWEvent* event);
    virtual void      OnAlert(AWEvent* event);
    virtual void      OnLaunch(AWEvent* event);
    virtual void      OnStandDown(AWEvent* event);
    virtual void      OnRecall(AWEvent* event);
    virtual void      OnClose(AWEvent* event);
    virtual void      OnMissionType(AWEvent* event);

    virtual void      ExecFrame();
    void              SetShip(Ship* s);
    void              UpdateSelection();
    void              UpdateObjective();

protected:
    GameScreen*       manager;

    ComboBox*         filter_list;
    ListBox*          hangar_list;

    Button*           package_btn;
    Button*           alert_btn;
    Button*           launch_btn;
    Button*           stand_btn;
    Button*           recall_btn;
    Button*           close_btn;

    int               mission_type;
    Button*           mission_btn[6];

    ListBox*          objective_list;
    ListBox*          loadout_list;

    Ship*             ship;
    FlightPlanner*    flight_planner;

    int               patrol_pattern;
};

#endif FltDlg_h

