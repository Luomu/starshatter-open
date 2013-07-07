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
    FILE:         MsnWepDlg.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Mission Briefing Dialog Active Window class
*/

#ifndef MsnWepDlg_h
#define MsnWepDlg_h

#include "Types.h"
#include "FormWindow.h"
#include "MsnDlg.h"
#include "Bitmap.h"
#include "Button.h"
#include "ImageBox.h"
#include "ListBox.h"
#include "Font.h"
#include "Text.h"

// +--------------------------------------------------------------------+

class  PlanScreen;
class  Campaign;
class  Mission;
class  MissionElement;
class  HardPoint;
class  WeaponDesign;

// +--------------------------------------------------------------------+

class MsnWepDlg : public FormWindow,
public MsnDlg
{
public:
    MsnWepDlg(Screen* s, FormDef& def, PlanScreen* mgr);
    virtual ~MsnWepDlg();

    virtual void      RegisterControls();
    virtual void      ExecFrame();
    virtual void      Show();

    // Operations:
    virtual void      OnCommit(AWEvent* event);
    virtual void      OnCancel(AWEvent* event);
    virtual void      OnTabButton(AWEvent* event);
    virtual void      OnMount(AWEvent* event);
    virtual void      OnLoadout(AWEvent* event);

protected:
    virtual void      SetupControls();
    virtual void      BuildLists();
    virtual int       LoadToPointIndex(int n);
    virtual int       PointIndexToLoad(int n, int index);

    ActiveWindow*     lbl_element;
    ActiveWindow*     lbl_type;
    ActiveWindow*     lbl_weight;
    ActiveWindow*     player_desc;
    ImageBox*         beauty;

    ActiveWindow*     lbl_station[8];
    ActiveWindow*     lbl_desc[8];
    Button*           btn_load[8][8];

    ListBox*          loadout_list;

    MissionElement*   elem;
    WeaponDesign*     designs[8];
    bool              mounts[8][8];
    int               loads[8];
    int               first_station;

    Bitmap            led_off;
    Bitmap            led_on;
};

#endif MsnWepDlg_h

