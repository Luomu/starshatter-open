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
    FILE:         VidDlg.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Main Menu Dialog Active Window class
*/

#ifndef VidDlg_h
#define VidDlg_h

#include "Types.h"
#include "FormWindow.h"
#include "Bitmap.h"
#include "Button.h"
#include "ComboBox.h"
#include "ListBox.h"
#include "Slider.h"
#include "Font.h"

// +--------------------------------------------------------------------+

class BaseScreen;
class Starshatter;

// +--------------------------------------------------------------------+

class VidDlg : public FormWindow
{
public:
    VidDlg(Screen* s, FormDef& def, BaseScreen* mgr);
    virtual ~VidDlg();

    virtual void      RegisterControls();
    virtual void      Show();
    virtual void      ExecFrame();

    // Operations:
    virtual void      OnTexSize(AWEvent* event);
    virtual void      OnMode(AWEvent* event);
    virtual void      OnDetail(AWEvent* event);
    virtual void      OnTexture(AWEvent* event);
    virtual void      OnGamma(AWEvent* event);

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
    virtual void      BuildModeList();

    BaseScreen*       manager;
    Starshatter*      stars;

    ComboBox*         mode;
    ComboBox*         tex_size;
    ComboBox*         detail;
    ComboBox*         texture;

    ComboBox*         shadows;
    ComboBox*         bump_maps;
    ComboBox*         spec_maps;

    ComboBox*         lens_flare;
    ComboBox*         corona;
    ComboBox*         nebula;
    ComboBox*         dust;

    Slider*           gamma;

    Button*           aud_btn;
    Button*           vid_btn;
    Button*           opt_btn;
    Button*           ctl_btn;
    Button*           mod_btn;

    Button*           apply;
    Button*           cancel;

    int               selected_render;
    int               selected_card;
    int               selected_tex_size;
    int               selected_mode;
    int               selected_detail;
    int               selected_texture;
    int               orig_gamma;

    bool              closed;
};

#endif VidDlg_h

