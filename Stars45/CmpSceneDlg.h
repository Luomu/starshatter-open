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
    FILE:         CmpSceneDlg.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Campaign title card and load progress dialog
*/

#ifndef CmpSceneDlg_h
#define CmpSceneDlg_h

#include "Types.h"
#include "FormWindow.h"
#include "CameraView.h"
#include "DisplayView.h"

#include "ImageBox.h"
#include "Font.h"
#include "Text.h"

// +--------------------------------------------------------------------+

class CmpnScreen;

// +--------------------------------------------------------------------+

class CmpSceneDlg : public FormWindow
{
public:
    CmpSceneDlg(Screen* s, FormDef& def, CmpnScreen* mgr);
    virtual ~CmpSceneDlg();

    virtual void      RegisterControls();
    virtual void      Show();
    virtual void      Hide();

    // Operations:
    virtual void      ExecFrame();

    CameraView*       GetCameraView();
    DisplayView*      GetDisplayView();

protected:
    ActiveWindow*     mov_scene;
    RichTextBox*      subtitles_box;
    CameraView*       cam_view;
    DisplayView*      disp_view;
    Window*           old_disp_win;

    Bitmap*           flare1;
    Bitmap*           flare2;
    Bitmap*           flare3;
    Bitmap*           flare4;

    CmpnScreen*       manager;

    double            subtitles_delay;
    double            subtitles_time;
};

#endif CmpSceneDlg_h

