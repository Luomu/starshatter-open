/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    nGenEx.lib
    FILE:         FormDef.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Form and Control Definition Resources
*/

#include "MemDebug.h"
#include "FormDef.h"
#include "ParseUtil.h"
#include "DataLoader.h"
#include "Bitmap.h"
#include "Game.h"

// +----------------------------------------------------------------------+

ColumnDef::ColumnDef()
   : width(10), align(0), sort(0), color(Color::White), use_color(false)
{ }

ColumnDef::ColumnDef(const char* t, int w, int a, int s)
   : title(t), width(w), align(a), sort(s),
     color(Color::White), use_color(false)
{ }

// +----------------------------------------------------------------------+

WinDef::WinDef(DWORD a_id, DWORD a_type, const char* a_text, DWORD a_style)
   : id(a_id), pid(0), type(a_type), text(a_text), style(a_style) 
{
   rect           = Rect(0,0,0,0);
   text_align     = 0;
   single_line    = false;
   enabled        = true;
   transparent    = false;
   hide_partial   = true;
   back_color     = Color::Gray;
   base_color     = Color::Gray;
   fore_color     = Color::Black;
   fixed_width    = 0;
   fixed_height   = 0;
}

void WinDef::SetID(DWORD i)                  { id           = i; }
void WinDef::SetParentID(DWORD i)            { pid          = i; }
void WinDef::SetType(DWORD t)                { type         = t; }
void WinDef::SetRect(const Rect& r)          { rect         = r; }
void WinDef::SetEnabled(bool e)              { enabled      = e; }
void WinDef::SetStyle(DWORD s)               { style        = s; }
void WinDef::SetFont(const char* t)          { font         = t; }
void WinDef::SetText(const char* t)          { text         = t; }
void WinDef::SetAltText(const char* t)       { alt_text     = t; }
void WinDef::SetTexture(const char* t)       { texture      = t; }
void WinDef::SetBackColor(Color c)           { back_color   = c; }
void WinDef::SetBaseColor(Color c)           { base_color   = c; }
void WinDef::SetForeColor(Color c)           { fore_color   = c; }
void WinDef::SetTextAlign(DWORD a)           { text_align   = a; }
void WinDef::SetSingleLine(bool a)           { single_line  = a; }
void WinDef::SetTransparent(bool t)          { transparent  = t; }
void WinDef::SetHidePartial(bool a)          { hide_partial = a; }

void WinDef::SetMargins(const Insets& m)     { margins      = m; }
void WinDef::SetTextInsets(const Insets& t)  { text_insets  = t; }
void WinDef::SetCellInsets(const Insets& c)  { cell_insets  = c; }
void WinDef::SetCells(const Rect& r)         { cells        = r; }

// +----------------------------------------------------------------------+

#define CTRL_DEF_ANIMATED        0x0001
#define CTRL_DEF_BORDER          0x0002
#define CTRL_DEF_DROP_SHADOW     0x0004
#define CTRL_DEF_INDENT          0x0008
#define CTRL_DEF_INVERT_LABEL    0x0010
#define CTRL_DEF_GLOW            0x0020
#define CTRL_DEF_SIMPLE          0x0040
#define CTRL_DEF_STICKY          0x0080

CtrlDef::CtrlDef(DWORD a_id, DWORD a_type, const char* a_text, DWORD a_style)
   : WinDef(a_id, a_type, a_text, a_style)
{
   ctrl_flags     = CTRL_DEF_ANIMATED | CTRL_DEF_BORDER | CTRL_DEF_INDENT;
   bevel_width    = 5;
   picture_loc    = 1; // North
   picture_type   = Bitmap::BMP_SOLID;

   active          = false;
   show_headings   = false;

   leading        = 0;
   line_height    = 0;
   multiselect    = 0;
   dragdrop       = 0;
   orientation    = 0;
   scroll_bar     = 1;
   num_leds       = 1;

   smooth_scroll  = false;

   item_style     = 0;
   selected_style = 0;
   pass_char      = 0;

   items.destroy();

   ZeroMemory(tabs, sizeof(tabs));
   ntabs          = 0;
}

CtrlDef::~CtrlDef()
{
   items.destroy();
   columns.destroy();
}

CtrlDef& CtrlDef::operator=(const CtrlDef& ctrl)
{
   WinDef::operator=(ctrl);

   ctrl_flags        = ctrl.ctrl_flags;
   bevel_width       = ctrl.bevel_width;
   picture_loc       = ctrl.picture_loc;
   picture_type      = ctrl.picture_type;

   active            = ctrl.active;
   show_headings     = ctrl.show_headings;

   leading           = ctrl.leading;
   line_height       = ctrl.line_height;
   multiselect       = ctrl.multiselect;
   dragdrop          = ctrl.dragdrop;
   orientation       = ctrl.orientation;
   scroll_bar        = ctrl.scroll_bar;
   pass_char         = ctrl.pass_char;
   active_color      = ctrl.active_color;
   border_color      = ctrl.border_color;

   smooth_scroll     = ctrl.smooth_scroll;

   item_style        = ctrl.item_style;
   selected_style    = ctrl.selected_style;
   pass_char         = ctrl.pass_char;

   standard_image    = ctrl.standard_image;
   activated_image   = ctrl.activated_image;
   transition_image  = ctrl.transition_image;

   return *this;
}

int CtrlDef::GetOrientation() const
{
   return orientation;
}

void  CtrlDef::SetOrientation(int o)
{
   orientation = o;
}

bool CtrlDef::GetActive() const
{
   return active;
}

void  CtrlDef::SetActive(bool c)
{
   active = c;
}

Color CtrlDef::GetActiveColor() const
{
   return active_color;
}

void  CtrlDef::SetActiveColor(Color c)
{
   active_color = c;
}

bool  CtrlDef::GetAnimated() const
{
   return ctrl_flags & CTRL_DEF_ANIMATED;
}

void  CtrlDef::SetAnimated(bool bNewValue)
{
   if (bNewValue)
      ctrl_flags |= CTRL_DEF_ANIMATED;
   else
      ctrl_flags &= ~CTRL_DEF_ANIMATED;
}

short CtrlDef::GetBevelWidth() const
{
   return bevel_width;
}

void  CtrlDef::SetBevelWidth(short nNewValue)
{
   bevel_width = nNewValue;
}

bool  CtrlDef::GetBorder() const
{
   return (ctrl_flags & CTRL_DEF_BORDER)?true:false;
}

void  CtrlDef::SetBorder(bool bNewValue)
{
   if (bNewValue)
      ctrl_flags |= CTRL_DEF_BORDER;
   else
      ctrl_flags &= ~CTRL_DEF_BORDER;
}

Color CtrlDef::GetBorderColor() const
{
   return border_color;
}

void  CtrlDef::SetBorderColor(Color c)
{
   border_color = c;
}

bool  CtrlDef::GetDropShadow() const
{
   return (ctrl_flags & CTRL_DEF_DROP_SHADOW)?true:false;
}

void  CtrlDef::SetDropShadow(bool bNewValue)
{
   if (bNewValue)
      ctrl_flags |= CTRL_DEF_DROP_SHADOW;
   else
      ctrl_flags &= ~CTRL_DEF_DROP_SHADOW;
}

bool  CtrlDef::GetIndent() const
{
   return (ctrl_flags & CTRL_DEF_INDENT)?true:false;
}

void  CtrlDef::SetIndent(bool bNewValue)
{
   if (bNewValue)
      ctrl_flags |= CTRL_DEF_INDENT;
   else
      ctrl_flags &= ~CTRL_DEF_INDENT;
}

bool  CtrlDef::GetInvertLabel() const
{
   return (ctrl_flags & CTRL_DEF_INVERT_LABEL)?true:false;
}

void  CtrlDef::SetInvertLabel(bool bNewValue)
{
   if (bNewValue)
      ctrl_flags |= CTRL_DEF_INVERT_LABEL;
   else
      ctrl_flags &= ~CTRL_DEF_INVERT_LABEL;
}

Text  CtrlDef::GetPicture() const
{
   return picture;
}

void  CtrlDef::SetPicture(const Text& img_name)
{
   picture = img_name;
}

short CtrlDef::GetPictureLocation() const
{
   return picture_loc;
}

void  CtrlDef::SetPictureLocation(short nNewValue)
{
   picture_loc = nNewValue;
}

short CtrlDef::GetPictureType() const
{
   return picture_type;
}

void  CtrlDef::SetPictureType(short nNewValue)
{
   picture_type = nNewValue;
}

bool  CtrlDef::GetSticky() const
{
   return (ctrl_flags & CTRL_DEF_STICKY)?true:false;
}

void  CtrlDef::SetSticky(bool bNewValue)
{
   if (bNewValue)
      ctrl_flags |= CTRL_DEF_STICKY;
   else
      ctrl_flags &= ~CTRL_DEF_STICKY;
}

int   CtrlDef::GetNumLeds() const
{
   return num_leds;
}

void  CtrlDef::SetNumLeds(int n)
{
   if (n > 0)
      num_leds = n;
}

int   CtrlDef::NumItems() const
{
   return items.size();
}

Text  CtrlDef::GetItem(int i) const
{
   Text result;

   if (i >= 0 && i < items.size())
      result = *(items[i]);

   return result;
}

void  CtrlDef::AddItem(const char* t)
{
   items.append(new(__FILE__,__LINE__) Text(t));
}

int   CtrlDef::NumColumns() const
{
   return columns.size();
}

ColumnDef*  CtrlDef::GetColumn(int i) const
{
   ColumnDef* result = 0;

   if (i >= 0 && i < columns.size())
      result = columns[i];

   return result;
}

void  CtrlDef::AddColumn(const char* t, int w, int a, int s)
{
   columns.append(new(__FILE__,__LINE__) ColumnDef(t,w,a,s));
}

int   CtrlDef::NumTabs() const
{
   return ntabs;
}

int   CtrlDef::GetTab(int i) const
{
   if (i >= 0 && i < ntabs)
      return tabs[i];
   return 0;
}

void  CtrlDef::SetTab(int i, int t)
{
   if (i >= 0 && i < 10) {
      tabs[i] = t;
      if (i >= ntabs)
         ntabs = i+1;
   }
}

void  CtrlDef::AddTab(int i)
{
   if (ntabs < 10)
      tabs[ntabs++] = i;
}

bool  CtrlDef::GetShowHeadings() const
{
   return show_headings;
}

void  CtrlDef::SetShowHeadings(bool bNewValue)
{
   show_headings = bNewValue;
}

int   CtrlDef::GetLeading() const
{
   return leading;
}

void  CtrlDef::SetLeading(int nNewValue)
{
   leading = nNewValue;
}

int   CtrlDef::GetLineHeight() const
{
   return line_height;
}

void  CtrlDef::SetLineHeight(int nNewValue)
{
   line_height = nNewValue;
}

int   CtrlDef::GetMultiSelect() const
{
   return multiselect;
}

void  CtrlDef::SetMultiSelect(int nNewValue)
{
   multiselect = nNewValue;
}

int   CtrlDef::GetDragDrop() const
{
   return dragdrop;
}

void  CtrlDef::SetDragDrop(int nNewValue)
{
   dragdrop = nNewValue;
}

int   CtrlDef::GetScrollBarVisible() const
{
   return scroll_bar;
}

void  CtrlDef::SetScrollBarVisible(int nNewValue)
{
   scroll_bar = nNewValue;
}

bool  CtrlDef::GetSmoothScroll() const
{
   return smooth_scroll;
}

void  CtrlDef::SetSmoothScroll(bool bNewValue)
{
   smooth_scroll = bNewValue;
}

short CtrlDef::GetItemStyle() const
{
   return item_style;
}

void  CtrlDef::SetItemStyle(short nNewValue)
{
   item_style = nNewValue;
}

short CtrlDef::GetSelectedStyle() const
{
   return selected_style;
}

void  CtrlDef::SetSelectedStyle(short nNewValue)
{
   selected_style = nNewValue;
}

char CtrlDef::GetPasswordChar() const
{
   return pass_char;
}

void  CtrlDef::SetPasswordChar(char nNewValue)
{
   pass_char = nNewValue;
}

Text  CtrlDef::GetStandardImage() const
{
   return standard_image;
}

void  CtrlDef::SetStandardImage(const Text& img_name)
{
   standard_image = img_name;
}

Text  CtrlDef::GetActivatedImage() const
{
   return activated_image;
}

void  CtrlDef::SetActivatedImage(const Text& img_name)
{
   activated_image = img_name;
}

Text  CtrlDef::GetTransitionImage() const
{
   return transition_image;
}

void  CtrlDef::SetTransitionImage(const Text& img_name)
{
   transition_image = img_name;
}


// +----------------------------------------------------------------------+

FormDef::FormDef(const char* a_text, DWORD a_style)
   : WinDef(0, WIN_DEF_FORM, a_text, a_style)
{
}

FormDef::~FormDef()
{
   controls.destroy();
}

void FormDef::AddCtrl(CtrlDef* def)
{
   if (def)
      controls.append(def);
}

CtrlDef* FormDef::FindCtrl(BYTE ctrl_id)
{
   if (ctrl_id > 0) {
      CtrlDef test(ctrl_id, 0);
      return controls.find(&test);
   }

   return 0;
}

ListIter<CtrlDef>
FormDef::GetControls() const
{
   // cast away const
   FormDef* f = (FormDef*) this;
   return f->controls;
}

// +----------------------------------------------------------------------+

static char filename[64];
static char path_name[64];

void
FormDef::Load(const char* fname)
{
   sprintf_s(filename, "%s.frm", fname);

   Print("Loading Form '%s'\n", fname);

   sprintf_s(path_name, "Screens/");

   // Load Design File:
   DataLoader* loader = DataLoader::GetLoader();
   loader->SetDataPath(path_name);

   BYTE* block;
   int blocklen = loader->LoadBuffer(filename, block, true);

   if (!block || blocklen < 4)
      return;

   Parser parser(new(__FILE__,__LINE__) BlockReader((const char*) block, blocklen));
   Term*  term = parser.ParseTerm();

   if (!term) {
      Print("ERROR: could not parse '%s'\n", filename);
      return;
   }
   else {
      TermText* file_type = term->isText();
      if (!file_type || file_type->value() != "FORM") {
         Print("ERROR: invalid form file '%s'\n", filename);
         return;
      }
   }

   do {
      delete term;

      term = parser.ParseTerm();
      
      if (term) {
         TermDef* def = term->isDef();
         if (def) {
            if (def->name()->value() == "form") {

               if (!def->term() || !def->term()->isStruct()) {
                  Print("WARNING: form structure missing in '%s'\n", filename);
               }
               else {
                  FormDef*    form = this;
                  TermStruct* val  = def->term()->isStruct();

                  for (int i = 0; i < val->elements()->size(); i++) {
                     char buf[256];

                     TermDef* pdef = val->elements()->at(i)->isDef();
                     if (pdef) {
                        if (pdef->name()->value() == "text" ||
                            pdef->name()->value() == "caption") {

                           GetDefText(buf, pdef, filename);
                           form->SetText(Game::GetText(buf));
                        }

                        else if (pdef->name()->value() == "id") {
                           DWORD id;
                           GetDefNumber(id, pdef, filename);
                           form->SetID(id);
                        }

                        else if (pdef->name()->value() == "pid") {
                           DWORD id;
                           GetDefNumber(id, pdef, filename);
                           form->SetParentID(id);
                        }

                        else if (pdef->name()->value() == "rect") {
                           Rect r;
                           GetDefRect(r, pdef, filename);
                           form->SetRect(r);
                        }

                        else if (pdef->name()->value() == "font") {
                           GetDefText(buf, pdef, filename);
                           form->SetFont(buf);
                        }

                        else if (pdef->name()->value() == "back_color") {
                           Color c;
                           GetDefColor(c, pdef, filename);
                           form->SetBackColor(c);
                        }

                        else if (pdef->name()->value() == "base_color") {
                           Color c;
                           GetDefColor(c, pdef, filename);
                           form->SetBaseColor(c);
                        }

                        else if (pdef->name()->value() == "fore_color") {
                           Color c;
                           GetDefColor(c, pdef, filename);
                           form->SetForeColor(c);
                        }

                        else if (pdef->name()->value() == "margins") {
                           GetDefInsets(form->margins, pdef, filename);
                        }

                        else if (pdef->name()->value() == "text_insets") {
                           GetDefInsets(form->text_insets, pdef, filename);
                        }

                        else if (pdef->name()->value() == "cell_insets") {
                           GetDefInsets(form->cell_insets, pdef, filename);
                        }

                        else if (pdef->name()->value() == "cells") {
                           GetDefRect(form->cells, pdef, filename);
                        }

                        else if (pdef->name()->value() == "texture") {
                           GetDefText(buf, pdef, filename);

                           if (*buf && !strchr(buf, '.'))
                              strcat_s(buf, ".pcx");

                           form->SetTexture(buf);
                        }

                        else if (pdef->name()->value() == "transparent") {
                           bool b;
                           GetDefBool(b, pdef, filename);
                           form->SetTransparent(b);
                        }

                        else if (pdef->name()->value() == "style") {
                           DWORD s;
                           GetDefNumber(s, pdef, filename);
                           form->SetStyle(s);
                        }

                        else if (pdef->name()->value() == "align" || 
                                 pdef->name()->value() == "text_align") {
                           DWORD a = DT_LEFT;

                           if (GetDefText(buf, pdef, filename)) {
                              if (!_stricmp(buf, "left"))
                                 a = DT_LEFT;
                              else if (!_stricmp(buf, "right"))
                                 a = DT_RIGHT;
                              else if (!_stricmp(buf, "center"))
                                 a = DT_CENTER;
                           }
                           
                           else {
                              GetDefNumber(a, pdef, filename);
                           }

                           form->SetTextAlign(a);
                        }

                        // layout constraints:

                        else if (pdef->name()->value() == "layout") {

                           if (!pdef->term() || !pdef->term()->isStruct()) {
                              Print("WARNING: layout structure missing in '%s'\n", filename);
                           }
                           else {
                              TermStruct* val  = pdef->term()->isStruct();
                              ParseLayoutDef(&form->layout, val);
                           }
                        }

                        // controls:

                        else if (pdef->name()->value() == "defctrl") {

                           if (!pdef->term() || !pdef->term()->isStruct()) {
                              Print("WARNING: defctrl structure missing in '%s'\n", filename);
                           }
                           else {
                              TermStruct* val  = pdef->term()->isStruct();
                              ParseCtrlDef(&form->defctrl, val);
                           }
                        }

                        else if (pdef->name()->value() == "ctrl") {

                           if (!pdef->term() || !pdef->term()->isStruct()) {
                              Print("WARNING: ctrl structure missing in '%s'\n", filename);
                           }
                           else {
                              CtrlDef*    ctrl = new(__FILE__,__LINE__) CtrlDef;
                              TermStruct* val  = pdef->term()->isStruct();

                              form->AddCtrl(ctrl);
                              *ctrl = form->defctrl;  // copy default params

                              ParseCtrlDef(ctrl, val);
                           }
                        }

                        // end of controls.
                     }
                  }     // end form params
               }        // end form struct
            }           // end form
            
            else
               Print("WARNING: unknown object '%s' in '%s'\n",
                        def->name()->value().data(), filename);
         }
         else {
            Print("WARNING: term ignored in '%s'\n", filename);
            term->print();
         }
      }
   }
   while (term);

   loader->ReleaseBuffer(block);
   loader->SetDataPath(0);
}

void FormDef::ParseCtrlDef(CtrlDef* ctrl, TermStruct* val)
{
   Text buf;

   ctrl->SetText("");

   for (int i = 0; i < val->elements()->size(); i++) {
      TermDef* pdef = val->elements()->at(i)->isDef();
      if (pdef) {
         if (pdef->name()->value() == "text" ||
             pdef->name()->value() == "caption") {
            GetDefText(buf, pdef, filename);
            ctrl->SetText(Game::GetText(buf));
         }

         else if (pdef->name()->value() == "id") {
            DWORD id;
            GetDefNumber(id, pdef, filename);
            ctrl->SetID(id);
         }

         else if (pdef->name()->value() == "pid") {
            DWORD id;
            GetDefNumber(id, pdef, filename);
            ctrl->SetParentID(id);
         }

         else if (pdef->name()->value() == "alt") {
            GetDefText(buf, pdef, filename);
            ctrl->SetAltText(Game::GetText(buf));
         }

         else if (pdef->name()->value() == "type") {
            DWORD type = WIN_DEF_LABEL;

            GetDefText(buf, pdef, filename);
            Text type_name(buf);

            if (type_name == "button")
               type = WIN_DEF_BUTTON;

            else if (type_name == "combo")
               type = WIN_DEF_COMBO;

            else if (type_name == "edit")
               type = WIN_DEF_EDIT;

            else if (type_name == "image")
               type = WIN_DEF_IMAGE;

            else if (type_name == "slider")
               type = WIN_DEF_SLIDER;

            else if (type_name == "list")
               type = WIN_DEF_LIST;

            else if (type_name == "rich" || type_name == "text" || type_name == "rich_text")
               type = WIN_DEF_RICH;

            ctrl->SetType(type);
         }

         else if (pdef->name()->value() == "rect") {
            Rect r;
            GetDefRect(r, pdef, filename);
            ctrl->SetRect(r);
         }

         else if (pdef->name()->value() == "font") {
            GetDefText(buf, pdef, filename);
            ctrl->SetFont(buf);
         }

         else if (pdef->name()->value() == "active_color") {
            Color c;
            GetDefColor(c, pdef, filename);
            ctrl->SetActiveColor(c);
         }

         else if (pdef->name()->value() == "back_color") {
            Color c;
            GetDefColor(c, pdef, filename);
            ctrl->SetBackColor(c);
         }

         else if (pdef->name()->value() == "base_color") {
            Color c;
            GetDefColor(c, pdef, filename);
            ctrl->SetBaseColor(c);
         }

         else if (pdef->name()->value() == "border_color") {
            Color c;
            GetDefColor(c, pdef, filename);
            ctrl->SetBorderColor(c);
         }

         else if (pdef->name()->value() == "fore_color") {
            Color c;
            GetDefColor(c, pdef, filename);
            ctrl->SetForeColor(c);
         }

         else if (pdef->name()->value() == "texture") {
            GetDefText(buf, pdef, filename);

            if (buf.length() > 0 && !buf.contains('.'))
               buf.append(".pcx");

            ctrl->SetTexture(buf);
         }

         else if (pdef->name()->value() == "margins") {
            GetDefInsets(ctrl->margins, pdef, filename);
         }

         else if (pdef->name()->value() == "text_insets") {
            GetDefInsets(ctrl->text_insets, pdef, filename);
         }

         else if (pdef->name()->value() == "cell_insets") {
            GetDefInsets(ctrl->cell_insets, pdef, filename);
         }

         else if (pdef->name()->value() == "cells") {
            GetDefRect(ctrl->cells, pdef, filename);
         }

         else if (pdef->name()->value() == "fixed_width") {
            GetDefNumber(ctrl->fixed_width, pdef, filename);
         }

         else if (pdef->name()->value() == "fixed_height") {
            GetDefNumber(ctrl->fixed_height, pdef, filename);
         }

         else if (pdef->name()->value() == "standard_image") {
            GetDefText(buf, pdef, filename);

            if (buf.length() > 0 && !buf.contains('.'))
               buf.append(".pcx");

            ctrl->SetStandardImage(buf);
         }

         else if (pdef->name()->value() == "activated_image") {
            GetDefText(buf, pdef, filename);

            if (buf.length() > 0 && !buf.contains('.'))
               buf.append(".pcx");

            ctrl->SetActivatedImage(buf);
         }

         else if (pdef->name()->value() == "transition_image") {
            GetDefText(buf, pdef, filename);

            if (buf.length() > 0 && !buf.contains('.'))
               buf.append(".pcx");

            ctrl->SetTransitionImage(buf);
         }

         else if (pdef->name()->value() == "picture") {
            GetDefText(buf, pdef, filename);

            if (buf.length() > 0 && !buf.contains('.'))
               buf.append(".pcx");

            ctrl->SetPicture(buf);
         }

         else if (pdef->name()->value() == "enabled") {
            bool e;
            GetDefBool(e, pdef, filename);
            ctrl->SetEnabled(e);
         }

         else if (pdef->name()->value() == "item") {
            GetDefText(buf, pdef, filename);
            ctrl->AddItem(Game::GetText(buf));
         }

         else if (pdef->name()->value() == "tab") {
            int tab = 0;
            GetDefNumber(tab, pdef, filename);
            ctrl->AddTab(tab);
         }

         else if (pdef->name()->value() == "column") {

            if (!pdef->term() || !pdef->term()->isStruct()) {
               Print("WARNING: column structure missing in '%s'\n", filename);
            }
            else {
               TermStruct* val  = pdef->term()->isStruct();
               ParseColumnDef(ctrl, val);
            }
         }

         else if (pdef->name()->value() == "orientation") {
            int n;
            GetDefNumber(n, pdef, filename);
            ctrl->SetOrientation(n);
         }

         else if (pdef->name()->value() == "leading") {
            int n;
            GetDefNumber(n, pdef, filename);
            ctrl->SetLeading(n);
         }

         else if (pdef->name()->value() == "line_height") {
            int n;
            GetDefNumber(n, pdef, filename);
            ctrl->SetLineHeight(n);
         }

         else if (pdef->name()->value() == "multiselect") {
            int n;
            GetDefNumber(n, pdef, filename);
            ctrl->SetMultiSelect(n);
         }

         else if (pdef->name()->value() == "dragdrop") {
            int n;
            GetDefNumber(n, pdef, filename);
            ctrl->SetDragDrop(n);
         }

         else if (pdef->name()->value() == "scroll_bar") {
            int n;
            GetDefNumber(n, pdef, filename);
            ctrl->SetScrollBarVisible(n);
         }

         else if (pdef->name()->value() == "smooth_scroll") {
            bool b;
            GetDefBool(b, pdef, filename);
            ctrl->SetSmoothScroll(b);
         }

         else if (pdef->name()->value() == "picture_loc") {
            int n;
            GetDefNumber(n, pdef, filename);
            ctrl->SetPictureLocation((short) n);
         }

         else if (pdef->name()->value() == "picture_type") {
            int n;
            GetDefNumber(n, pdef, filename);
            ctrl->SetPictureType((short) n);
         }

         else if (pdef->name()->value() == "style") {
            DWORD s;
            GetDefNumber(s, pdef, filename);
            ctrl->SetStyle(s);
         }

         else if (pdef->name()->value() == "align" || 
                  pdef->name()->value() == "text_align") {
            DWORD a = DT_LEFT;

            if (GetDefText(buf, pdef, filename)) {
               if (!_stricmp(buf, "left"))
                  a = DT_LEFT;
               else if (!_stricmp(buf, "right"))
                  a = DT_RIGHT;
               else if (!_stricmp(buf, "center"))
                  a = DT_CENTER;
            }
         
            else {
               GetDefNumber(a, pdef, filename);
            }

            ctrl->SetTextAlign(a);
         }

         else if (pdef->name()->value() == "single_line") {
            bool single = false;
            GetDefBool(single, pdef, filename);
            ctrl->SetSingleLine(single);
         }

         else if (pdef->name()->value() == "bevel_width") {
            DWORD s;
            GetDefNumber(s, pdef, filename);
            ctrl->SetBevelWidth((short) s);
         }

         else if (pdef->name()->value() == "active") {
            bool b;
            GetDefBool(b, pdef, filename);
            ctrl->SetActive(b);
         }

         else if (pdef->name()->value() == "animated") {
            bool b;
            GetDefBool(b, pdef, filename);
            ctrl->SetAnimated(b);
         }

         else if (pdef->name()->value() == "border") {
            bool b;
            GetDefBool(b, pdef, filename);
            ctrl->SetBorder(b);
         }

         else if (pdef->name()->value() == "drop_shadow") {
            bool b;
            GetDefBool(b, pdef, filename);
            ctrl->SetDropShadow(b);
         }

         else if (pdef->name()->value() == "show_headings") {
            bool b;
            GetDefBool(b, pdef, filename);
            ctrl->SetShowHeadings(b);
         }

         else if (pdef->name()->value() == "sticky") {
            bool b;
            GetDefBool(b, pdef, filename);
            ctrl->SetSticky(b);
         }

         else if (pdef->name()->value() == "transparent") {
            bool b;
            GetDefBool(b, pdef, filename);
            ctrl->SetTransparent(b);
         }

         else if (pdef->name()->value() == "hide_partial") {
            bool b;
            GetDefBool(b, pdef, filename);
            ctrl->SetHidePartial(b);
         }

         else if (pdef->name()->value() == "num_leds") {
            int n;
            GetDefNumber(n, pdef, filename);
            ctrl->SetNumLeds(n);
         }

         else if (pdef->name()->value() == "item_style") {
            int n;
            GetDefNumber(n, pdef, filename);
            ctrl->SetItemStyle((short) n);
         }

         else if (pdef->name()->value() == "selected_style") {
            int n;
            GetDefNumber(n, pdef, filename);
            ctrl->SetSelectedStyle((short) n);
         }

         else if (pdef->name()->value() == "password") {
            Text password;
            GetDefText(password, pdef, filename);
            ctrl->SetPasswordChar((char) password[0]);
         }

         // layout constraints:

         else if (pdef->name()->value() == "layout") {

            if (!pdef->term() || !pdef->term()->isStruct()) {
               Print("WARNING: layout structure missing in '%s'\n", filename);
            }
            else {
               TermStruct* val  = pdef->term()->isStruct();
               ParseLayoutDef(&ctrl->layout, val);
            }
         }
      }
   }
}

void FormDef::ParseColumnDef(CtrlDef* ctrl, TermStruct* val)
{
   Text  text;
   char  buf[256];
   int   width  = 0;
   int   align  = 0;
   int   sort   = 0;
   Color c;
   bool  use_color = false;

   for (int i = 0; i < val->elements()->size(); i++) {
      TermDef* pdef = val->elements()->at(i)->isDef();
      if (pdef) {
         if (pdef->name()->value() == "text" ||
             pdef->name()->value() == "title") {
            GetDefText(buf, pdef, filename);
            text = Game::GetText(buf);
         }

         else if (pdef->name()->value() == "width") {
            GetDefNumber(width, pdef, filename);
         }

         else if (pdef->name()->value() == "align") {
            align = DT_LEFT;

            if (GetDefText(buf, pdef, filename)) {
               if (!_stricmp(buf, "left"))
                  align = DT_LEFT;
               else if (!_stricmp(buf, "right"))
                  align = DT_RIGHT;
               else if (!_stricmp(buf, "center"))
                  align = DT_CENTER;
            }
            
            else {
               GetDefNumber(align, pdef, filename);
            }
         }

         else if (pdef->name()->value() == "sort") {
            GetDefNumber(sort, pdef, filename);
         }

         else if (pdef->name()->value() == "color") {
            GetDefColor(c, pdef, filename);
            use_color = true;
         }
      }
   }

   ctrl->AddColumn(text, width, align, sort);

   if (use_color) {
      int         index  = ctrl->NumColumns()-1;
      ColumnDef*  column = ctrl->GetColumn(index);

      if (column) {
         column->color     = c;
         column->use_color = true;
      }
   }
}

void FormDef::ParseLayoutDef(LayoutDef* def, TermStruct* val)
{
   if (!def || !val)
      return;

   for (int i = 0; i < val->elements()->size(); i++) {
      TermDef* pdef = val->elements()->at(i)->isDef();
      if (pdef) {
         if (pdef->name()->value() == "x_mins" ||
             pdef->name()->value() == "cols") {
            GetDefArray(def->x_mins, pdef, filename);
         }

         else
         if (pdef->name()->value() == "y_mins" ||
             pdef->name()->value() == "rows") {
            GetDefArray(def->y_mins, pdef, filename);
         }

         else 
         if (pdef->name()->value() == "x_weights" ||
             pdef->name()->value() == "col_wts") {
            GetDefArray(def->x_weights, pdef, filename);
         }

         else
         if (pdef->name()->value() == "y_weights" ||
             pdef->name()->value() == "row_wts") {
            GetDefArray(def->y_weights, pdef, filename);
         }
      }
   }

}


