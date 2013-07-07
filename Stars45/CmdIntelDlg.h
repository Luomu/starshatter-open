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
    FILE:         CmdIntelDlg.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Operational Command Dialog (Intel/Newsfeed Tab)
*/

#ifndef CmdIntelDlg_h
#define CmdIntelDlg_h

#include "Types.h"
#include "FormWindow.h"
#include "CmdDlg.h"
#include "CameraView.h"
#include "DisplayView.h"

#include "Bitmap.h"
#include "Button.h"
#include "ImageBox.h"
#include "ListBox.h"
#include "RichTextBox.h"
#include "Font.h"
#include "Text.h"

// +--------------------------------------------------------------------+

class CmdIntelDlg : public FormWindow,
public CmdDlg
{
public:
    CmdIntelDlg(Screen* s, FormDef& def, CmpnScreen* mgr);
    virtual ~CmdIntelDlg();

    virtual void      RegisterControls();
    virtual void      Show();
    virtual void      ExecFrame();

    // Operations:
    virtual void      OnMode(AWEvent* event);
    virtual void      OnSave(AWEvent* event);
    virtual void      OnExit(AWEvent* event);
    virtual void      OnNews(AWEvent* event);
    virtual void      OnPlay(AWEvent* event);

protected:
    virtual void      ShowMovie();
    virtual void      HideMovie();

    CmpnScreen*       manager;

    ListBox*          lst_news;
    RichTextBox*      txt_news;
    ImageBox*         img_news;
    Button*           btn_play;
    ActiveWindow*     mov_news;
    CameraView*       cam_view;
    DisplayView*      dsp_view;

    Bitmap            bmp_default;

    Starshatter*      stars;
    Campaign*         campaign;
    double            update_time;
    int               start_scene;
    Text              event_scene;
};

#endif CmdIntelDlg_h

