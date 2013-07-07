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
    FILE:         BaseScreen.h
    AUTHOR:       John DiCamillo

*/

#ifndef BaseScreen_h
#define BaseScreen_h

#include "Types.h"
#include "Bitmap.h"
#include "Screen.h"

// +--------------------------------------------------------------------+

class Screen;
class Sim;
class Window;
class Font;
class NavDlg;
class MsnElemDlg;
class AudDlg;
class VidDlg;
class ModDlg;
class ModInfoDlg;
class OptDlg;
class CtlDlg;
class KeyDlg;
class JoyDlg;
class MsgDlg;

// +--------------------------------------------------------------------+

class BaseScreen
{
public:
    BaseScreen()                           { }
    virtual ~BaseScreen()                  { }

    virtual void         ShowNavDlg()      { }
    virtual void         HideNavDlg()      { }
    virtual bool         IsNavShown()      { return false; }
    virtual NavDlg*      GetNavDlg()       { return 0; }

    virtual void         ShowMsnElemDlg()  { }
    virtual void         HideMsnElemDlg()  { }
    virtual MsnElemDlg*  GetMsnElemDlg()   { return 0; }

    virtual AudDlg*      GetAudDlg() const { return 0; }
    virtual VidDlg*      GetVidDlg() const { return 0; }
    virtual ModDlg*      GetModDlg() const { return 0; }
    virtual ModInfoDlg*  GetModInfoDlg() const { return 0; }
    virtual OptDlg*      GetOptDlg() const { return 0; }
    virtual CtlDlg*      GetCtlDlg() const { return 0; }
    virtual JoyDlg*      GetJoyDlg() const { return 0; }
    virtual KeyDlg*      GetKeyDlg() const { return 0; }

    virtual void         ShowAudDlg()      { }
    virtual void         ShowVidDlg()      { }
    virtual void         ShowModDlg()      { }
    virtual void         ShowModInfoDlg()  { }
    virtual void         ShowOptDlg()      { }
    virtual void         ShowCtlDlg()      { }
    virtual void         ShowJoyDlg()      { }
    virtual void         ShowKeyDlg()      { }

    virtual void         ShowMsgDlg()      { }
    virtual void         HideMsgDlg()      { }
    virtual bool         IsMsgShown()      { return false; }
    virtual MsgDlg*      GetMsgDlg()       { return 0; }

    virtual void         ApplyOptions()    { }
    virtual void         CancelOptions()   { }
};

// +--------------------------------------------------------------------+

#endif BaseScreen_h

