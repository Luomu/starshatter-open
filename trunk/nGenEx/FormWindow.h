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

    SUBSYSTEM:    nGenEx.lib
    FILE:         FormWindow.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Form Window class (a window that manages controls)
*/

#ifndef FormWindow_h
#define FormWindow_h

#include "Types.h"
#include "ActiveWindow.h"
#include "FormDef.h"
#include "List.h"

// +--------------------------------------------------------------------+

class Button;
class ComboBox;
class EditBox;
class ImageBox;
class ListBox;
class RichTextBox;
class Slider;

// +--------------------------------------------------------------------+

class FormWindow : public ActiveWindow
{
public:
    FormWindow(Screen* s, int ax, int ay, int aw, int ah,
    DWORD aid=0, DWORD style=0, ActiveWindow* parent=0);
    virtual ~FormWindow();

    // operations:
    virtual void            Init();
    virtual void            Init(const FormDef& def);
    virtual void            Destroy();
    virtual ActiveWindow*   FindControl(DWORD id);
    virtual ActiveWindow*   FindControl(int x, int y);
    virtual void            RegisterControls()      { }

    virtual void            AdoptFormDef(const FormDef& def);
    virtual void            AddControl(ActiveWindow* ctrl);

    virtual ActiveWindow*   CreateLabel(      const char* text, int x, int y, int w, int h, DWORD id=0, DWORD pid=0, DWORD style=0);
    virtual Button*         CreateButton(     const char* text, int x, int y, int w, int h, DWORD id=0, DWORD pid=0);
    virtual ImageBox*       CreateImageBox(   const char* text, int x, int y, int w, int h, DWORD id=0, DWORD pid=0);
    virtual ListBox*        CreateListBox(    const char* text, int x, int y, int w, int h, DWORD id=0, DWORD pid=0);
    virtual ComboBox*       CreateComboBox(   const char* text, int x, int y, int w, int h, DWORD id=0, DWORD pid=0);
    virtual EditBox*        CreateEditBox(    const char* text, int x, int y, int w, int h, DWORD id=0, DWORD pid=0);
    virtual RichTextBox*    CreateRichTextBox(const char* text, int x, int y, int w, int h, DWORD id=0, DWORD pid=0, DWORD style=0);
    virtual Slider*         CreateSlider(     const char* text, int x, int y, int w, int h, DWORD id=0, DWORD pid=0, DWORD style=0);

    // property accessors:
    ListIter<ActiveWindow>  Controls() { return children; }

protected:
    virtual void            CreateDefLabel(CtrlDef& def);
    virtual void            CreateDefButton(CtrlDef& def);
    virtual void            CreateDefImage(CtrlDef& def);
    virtual void            CreateDefList(CtrlDef& def);
    virtual void            CreateDefCombo(CtrlDef& def);
    virtual void            CreateDefEdit(CtrlDef& def);
    virtual void            CreateDefSlider(CtrlDef& def);
    virtual void            CreateDefRichText(CtrlDef& def);
};

#endif FormWindow_h

