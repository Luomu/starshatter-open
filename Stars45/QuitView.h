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
    FILE:         QuitView.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    View class for End Mission menu
*/

#ifndef QuitView_h
#define QuitView_h

#include "Types.h"
#include "View.h"
#include "Bitmap.h"
#include "Font.h"
#include "SimObject.h"
#include "Text.h"

// +--------------------------------------------------------------------+

class HUDView;
class Menu;

// +--------------------------------------------------------------------+

class QuitView : public View
{
public:
    QuitView(Window* c);
    virtual ~QuitView();

    // Operations:
    virtual void      Refresh();
    virtual void      OnWindowMove();
    virtual void      ExecFrame();

    virtual bool      CanAccept();
    virtual bool      IsMenuShown();
    virtual void      ShowMenu();
    virtual void      CloseMenu();

    static void       Initialize();
    static void       Close();

    static QuitView*  GetInstance() { return quit_view; }

protected:
    int         width, height;
    int         xcenter, ycenter;
    bool        mouse_latch;

    Sim*        sim;

    static QuitView* quit_view;
};

#endif QuitView_h

