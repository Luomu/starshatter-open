/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

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

