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
    FILE:         RadioView.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    View class for Radio Communications HUD Overlay
*/

#ifndef RadioView_h
#define RadioView_h

#include "Types.h"
#include "View.h"
#include "Color.h"
#include "SimObject.h"
#include "Text.h"

// +--------------------------------------------------------------------+

class Font;
class Element;
class Ship;
class RadioMessage;
class CameraView;
class HUDView;
class Menu;
class MenuItem;

// +--------------------------------------------------------------------+

class RadioView : public View, 
public SimObserver
{
public:
    RadioView(Window* c);
    virtual ~RadioView();

    // Operations:
    virtual void      Refresh();
    virtual void      OnWindowMove();
    virtual void      ExecFrame();

    virtual Menu*     GetRadioMenu(Ship* ship);
    virtual bool      IsMenuShown();
    virtual void      ShowMenu();
    virtual void      CloseMenu();

    static void       Message(const char* msg);
    static void       ClearMessages();

    virtual bool         Update(SimObject* obj);
    virtual const char*  GetObserverName() const;

    static void       SetColor(Color c);

    static void       Initialize();
    static void       Close();

    static RadioView* GetInstance() { return radio_view; }

protected:
    void              SendRadioMessage(Ship* ship, MenuItem* item);

    int         width, height;
    double      xcenter, ycenter;

    Font*       font;
    Sim*        sim;
    Ship*       ship;
    Element*    dst_elem;

    enum { MAX_MSG=6 };
    Text        msg_text[MAX_MSG];
    double      msg_time[MAX_MSG];

    static RadioView* radio_view;
    static ThreadSync sync;
};

#endif RadioView_h

