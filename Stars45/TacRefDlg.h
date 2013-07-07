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
    FILE:         TacRefDlg.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Mission Briefing Dialog Active Window class
*/

#ifndef TacRefDlg_h
#define TacRefDlg_h

#include "Types.h"
#include "FormWindow.h"
#include "MsnDlg.h"
#include "Bitmap.h"
#include "Button.h"
#include "CameraView.h"
#include "ImgView.h"
#include "Scene.h"
#include "Font.h"
#include "Text.h"
#include "ListBox.h"
#include "RichTextBox.h"

// +--------------------------------------------------------------------+

class MenuScreen;
class ShipDesign;
class WeaponDesign;

// +--------------------------------------------------------------------+

class TacRefDlg : public FormWindow
{
public:
    enum MODES { MODE_NONE, MODE_SHIPS, MODE_WEAPONS };

    TacRefDlg(Screen* s, FormDef& def, MenuScreen* mgr);
    virtual ~TacRefDlg();

    virtual void      RegisterControls();
    virtual void      ExecFrame();
    virtual void      Show();

    // Operations:
    virtual void      OnClose(AWEvent* event);
    virtual void      OnMode(AWEvent* event);
    virtual void      OnSelect(AWEvent* event);
    virtual void      OnCamRButtonDown(AWEvent* event);
    virtual void      OnCamRButtonUp(AWEvent* event);
    virtual void      OnCamMove(AWEvent* event);
    virtual void      OnCamZoom(AWEvent* event);

protected:
    virtual void      SelectShip(const ShipDesign* dsn);
    virtual void      SelectWeapon(const WeaponDesign* dsn);

    virtual void      UpdateZoom(double r);
    virtual void      UpdateAzimuth(double a);
    virtual void      UpdateElevation(double e);
    virtual void      UpdateCamera();
    virtual bool      SetCaptureBeauty();
    virtual bool      ReleaseCaptureBeauty();

    MenuScreen*       manager;
    ActiveWindow*     beauty;
    ListBox*          lst_designs;
    ActiveWindow*     txt_caption;
    RichTextBox*      txt_stats;
    RichTextBox*      txt_description;
    Button*           btn_ships;
    Button*           btn_weaps;
    Button*           btn_close;

    ImgView*          imgview;
    CameraView*       camview;
    Scene             scene;
    Camera            cam;

    int               mode;
    double            radius;
    double            cam_zoom;
    double            cam_az;
    double            cam_el;
    int               mouse_x;
    int               mouse_y;
    bool              update_scene;
    bool              captured;

    int               ship_index;
    int               weap_index;
};

#endif TacRefDlg_h

