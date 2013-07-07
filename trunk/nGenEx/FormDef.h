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
    FILE:         FormDef.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Form and Control Definition Resources
*/

#ifndef FormDef_h
#define FormDef_h

#include <vector>
#include "Types.h"
#include "Geometry.h"
#include "Color.h"
#include "Text.h"
#include "List.h"

// +--------------------------------------------------------------------+

class FormDef;    // values defining form style and control placement
class CtrlDef;    // values defining control style
class WinDef;     // base class for FormDef and CtrlDef
class TermStruct; // used for parsing

enum WinType {
    WIN_DEF_FORM,
    WIN_DEF_LABEL,
    WIN_DEF_BUTTON,
    WIN_DEF_COMBO,
    WIN_DEF_EDIT,
    WIN_DEF_IMAGE,
    WIN_DEF_SLIDER,
    WIN_DEF_LIST,
    WIN_DEF_RICH
};

// +--------------------------------------------------------------------+

class ColumnDef
{
public:
    static const char* TYPENAME() { return "ColumnDef"; }

    ColumnDef();
    ColumnDef(const char* title, int width, int align, int sort);

    Text  title;
    int   width;
    int   align;
    int   sort;
    Color color;
    bool  use_color;
};

// +--------------------------------------------------------------------+

class LayoutDef
{
public:
    static const char* TYPENAME() { return "LayoutDef"; }

    std::vector<DWORD>   x_mins;
    std::vector<DWORD>   y_mins;
    std::vector<float>   x_weights;
    std::vector<float>   y_weights;
};

// +--------------------------------------------------------------------+

class WinDef
{
    friend class FormDef;

public:
    static const char* TYPENAME() { return "WinDef"; }

    WinDef(DWORD id, DWORD type, const char* text=0, DWORD style=0);
    virtual ~WinDef() { }

    int operator == (const WinDef& w) const { return id == w.id; }

    DWORD          GetID() const           { return id;   }
    void           SetID(DWORD id);
    DWORD          GetParentID() const     { return pid;  }
    void           SetParentID(DWORD id);
    DWORD          GetType() const         { return type; }
    void           SetType(DWORD type);

    void           SetRect(const Rect& r);
    Rect           GetRect() const         { return rect;   }
    int            GetX()    const         { return rect.x; }
    int            GetY()    const         { return rect.y; }
    int            GetW()    const         { return rect.w; }
    int            GetH()    const         { return rect.h; }

    void           SetEnabled(bool enable=true);
    bool           IsEnabled() const       { return enabled; }

    void           SetStyle(DWORD s);
    DWORD          GetStyle() const        { return style; }

    void           SetFont(const char* t);
    const Text&    GetFont() const         { return font; }
    void           SetText(const char* t);
    const Text&    GetText() const         { return text; }
    void           SetAltText(const char* t);
    const Text&    GetAltText() const      { return alt_text; }
    void           SetTexture(const char* t);
    const Text&    GetTexture()   const    { return texture;    }

    void           SetBackColor(Color c);
    Color          GetBackColor()    const { return back_color; }
    void           SetBaseColor(Color c);
    Color          GetBaseColor()    const { return base_color; }
    void           SetForeColor(Color c);
    Color          GetForeColor()    const { return fore_color; }
    void           SetSingleLine(bool a);
    bool           GetSingleLine()   const { return single_line; }
    void           SetTextAlign(DWORD a);
    DWORD          GetTextAlign()    const { return text_align;  }
    void           SetTransparent(bool t);
    bool           GetTransparent()  const { return transparent; }
    void           SetHidePartial(bool a);
    bool           GetHidePartial()  const { return hide_partial;}

    void           SetMargins(const Insets& m);
    const Insets&  GetMargins()      const { return margins;     }
    void           SetTextInsets(const Insets& t);
    const Insets&  GetTextInsets()   const { return text_insets; }
    void           SetCellInsets(const Insets& t);
    const Insets&  GetCellInsets()   const { return cell_insets; }
    void           SetCells(const Rect& r);
    const Rect&    GetCells()        const { return cells;       }

    void           SetFixedWidth(int w)    { fixed_width = w;    }
    int            GetFixedWidth()   const { return fixed_width; }
    void           SetFixedHeight(int h)   { fixed_height = h;   }
    int            GetFixedHeight()  const { return fixed_height;}

    const LayoutDef&  GetLayout()    const { return layout;      }

protected:
    DWORD          id;
    DWORD          pid;
    DWORD          type;
    Rect           rect;
    Text           font;
    Text           text;
    Text           alt_text;
    Text           texture;
    Text           picture;
    DWORD          style;
    DWORD          text_align;
    bool           single_line;
    bool           enabled;
    bool           transparent;
    bool           hide_partial;
    Color          back_color;
    Color          base_color;
    Color          fore_color;

    Insets         margins;
    Insets         text_insets;
    Insets         cell_insets;
    Rect           cells;
    int            fixed_width;
    int            fixed_height;

    LayoutDef      layout;
};

// +--------------------------------------------------------------------+

class CtrlDef : public WinDef
{
public:
    static const char* TYPENAME() { return "CtrlDef"; }

    CtrlDef(DWORD id=0, DWORD type=WIN_DEF_LABEL, const char* text=0, DWORD style=0);
    virtual ~CtrlDef();

    virtual CtrlDef& operator=(const CtrlDef& ctrl);

    bool     GetActive() const;
    void     SetActive(bool c);
    Color    GetActiveColor() const;
    void     SetActiveColor(Color c);
    bool     GetAnimated() const;
    void     SetAnimated(bool bNewValue);
    short    GetBevelWidth() const;
    void     SetBevelWidth(short nNewValue);
    bool     GetBorder() const;
    void     SetBorder(bool bNewValue);
    Color    GetBorderColor() const;
    void     SetBorderColor(Color c);
    bool     GetDropShadow() const;
    void     SetDropShadow(bool bNewValue);
    bool     GetIndent() const;
    void     SetIndent(bool bNewValue);
    bool     GetInvertLabel() const;
    void     SetInvertLabel(bool bNewValue);
    int      GetOrientation() const;
    void     SetOrientation(int o);
    Text     GetPicture() const;
    void     SetPicture(const Text& img_name);
    short    GetPictureLocation() const;
    void     SetPictureLocation(short nNewValue);
    short    GetPictureType() const;
    void     SetPictureType(short nNewValue);
    bool     GetSticky() const;
    void     SetSticky(bool bNewValue);
    int      GetNumLeds() const;
    void     SetNumLeds(int nNewValue);

    int      NumItems() const;
    Text     GetItem(int i) const;
    void     AddItem(const char* t);

    int      NumColumns() const;
    ColumnDef* GetColumn(int i) const;
    void     AddColumn(const char* t, int w, int a, int s);

    int      NumTabs() const;
    int      GetTab(int i) const;
    void     SetTab(int i, int t);
    void     AddTab(int i);

    bool     GetShowHeadings() const;
    void     SetShowHeadings(bool bNewValue);
    int      GetLeading() const;
    void     SetLeading(int nNewValue);
    int      GetLineHeight() const;
    void     SetLineHeight(int nNewValue);
    int      GetMultiSelect() const;
    void     SetMultiSelect(int nNewValue);
    int      GetDragDrop() const;
    void     SetDragDrop(int nNewValue);
    int      GetScrollBarVisible() const;
    void     SetScrollBarVisible(int nNewValue);
    bool     GetSmoothScroll() const;
    void     SetSmoothScroll(bool bNewValue);

    short    GetItemStyle() const;
    void     SetItemStyle(short nNewValue);
    short    GetSelectedStyle() const;
    void     SetSelectedStyle(short nNewValue);

    char     GetPasswordChar() const;
    void     SetPasswordChar(char c);

    Text     GetStandardImage() const;
    void     SetStandardImage(const Text& img_name);
    Text     GetActivatedImage() const;
    void     SetActivatedImage(const Text& img_name);
    Text     GetTransitionImage() const;
    void     SetTransitionImage(const Text& img_name);

protected:
    WORD              ctrl_flags;
    short             bevel_width;

    Color             active_color;
    Color             border_color;

    Text              picture;
    short             picture_loc;
    short             picture_type;

    Text              standard_image;
    Text              activated_image;
    Text              transition_image;

    bool              active;
    bool              show_headings;
    int               leading;
    int               line_height;
    int               multiselect;
    int               dragdrop;
    int               scroll_bar;
    int               orientation;
    int               num_leds;

    short             item_style;
    short             selected_style;

    bool              smooth_scroll;

    List<Text>        items;
    List<ColumnDef>   columns;

    int               ntabs;
    int               tabs[10];
    char              pass_char;
};

// +--------------------------------------------------------------------+

class FormDef : public WinDef
{
public:
    static const char* TYPENAME() { return "FormDef"; }

    FormDef(const char* text=0, DWORD style=0);
    virtual ~FormDef();

    void     Load(const char* filename);

    void     AddCtrl(CtrlDef* def);
    CtrlDef* FindCtrl(BYTE ctrl_id);

    ListIter<CtrlDef> GetControls()  const;

protected:
    void     ParseCtrlDef(CtrlDef* ctrl, TermStruct* val);
    void     ParseColumnDef(CtrlDef* ctrl, TermStruct* val);
    void     ParseLayoutDef(LayoutDef* def, TermStruct* val);

    CtrlDef        defctrl;
    List<CtrlDef>  controls;
};

// +--------------------------------------------------------------------+

#endif FormDef_h

