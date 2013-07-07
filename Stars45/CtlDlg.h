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
    FILE:         CtlDlg.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Control Options (keyboard/joystick) Dialog Active Window class
*/

#ifndef CtlDlg_h
#define CtlDlg_h

#include "Types.h"
#include "FormWindow.h"
#include "Bitmap.h"
#include "Button.h"
#include "ComboBox.h"
#include "ListBox.h"
#include "Font.h"

// +--------------------------------------------------------------------+

class BaseScreen;
class MenuScreen;
class GameScreen;

// +--------------------------------------------------------------------+

class CtlDlg : public FormWindow
{
public:
    CtlDlg(Screen* s, FormDef& def, BaseScreen* mgr);
    virtual ~CtlDlg();

    virtual void      RegisterControls();
    virtual void      Show();
    virtual void      ExecFrame();

    // Operations:
    virtual void      OnCommand(AWEvent* event);
    virtual void      OnCategory(AWEvent* event);

    virtual void      OnControlModel(AWEvent* event);

    virtual void      OnJoySelect(AWEvent* event);
    virtual void      OnJoyThrottle(AWEvent* event);
    virtual void      OnJoyRudder(AWEvent* event);
    virtual void      OnJoySensitivity(AWEvent* event);
    virtual void      OnJoyAxis(AWEvent* event);

    virtual void      OnMouseSelect(AWEvent* event);
    virtual void      OnMouseSensitivity(AWEvent* event);
    virtual void      OnMouseInvert(AWEvent* event);

    virtual void      Apply();
    virtual void      Cancel();

    virtual void      OnApply(AWEvent* event);
    virtual void      OnCancel(AWEvent* event);

    virtual void      OnAudio(AWEvent* event);
    virtual void      OnVideo(AWEvent* event);
    virtual void      OnOptions(AWEvent* event);
    virtual void      OnControls(AWEvent* event);
    virtual void      OnMod(AWEvent* event);

protected:
    void              ShowCategory();
    void              UpdateCategory();

    BaseScreen*       manager;

    Button*           category[4];
    ListBox*          commands;
    int               command_index;

    ComboBox*         control_model_combo;

    ComboBox*         joy_select_combo;
    ComboBox*         joy_throttle_combo;
    ComboBox*         joy_rudder_combo;
    Slider*           joy_sensitivity_slider;
    Button*           joy_axis_button;

    ComboBox*         mouse_select_combo;
    Slider*           mouse_sensitivity_slider;
    Button*           mouse_invert_checkbox;

    Button*           aud_btn;
    Button*           vid_btn;
    Button*           opt_btn;
    Button*           ctl_btn;
    Button*           mod_btn;

    Button*           apply;
    Button*           cancel;

    int               selected_category;
    int               control_model;

    int               joy_select;
    int               joy_throttle;
    int               joy_rudder;
    int               joy_sensitivity;

    int               mouse_select;
    int               mouse_sensitivity;
    int               mouse_invert;

    bool              closed;
};

#endif CtlDlg_h

