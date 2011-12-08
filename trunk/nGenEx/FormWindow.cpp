/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    nGenEx.lib
    FILE:         Form.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Form Window class
*/

#include "MemDebug.h"
#include "FormWindow.h"
#include "Screen.h"
#include "DataLoader.h"
#include "Font.h"
#include "FontMgr.h"

#include "Button.h"
#include "ComboBox.h"
#include "EditBox.h"
#include "ImageBox.h"
#include "ListBox.h"
#include "RichTextBox.h"
#include "Slider.h"

// +--------------------------------------------------------------------+

FormWindow::FormWindow(Screen* screen, int ax, int ay, int aw, int ah,
                       DWORD aid, DWORD s, ActiveWindow* pParent)
   : ActiveWindow(screen, ax, ay, aw, ah, aid, s, pParent)
{
   char buf[32];
   sprintf(buf, "Form %d", id);
   desc = buf;
}

// +--------------------------------------------------------------------+

FormWindow::~FormWindow()
{
}

// +--------------------------------------------------------------------+

void
FormWindow::Init()
{
}

// +--------------------------------------------------------------------+

void
FormWindow::Destroy()
{
   Hide();
   children.destroy();
}

// +--------------------------------------------------------------------+

void
FormWindow::AddControl(ActiveWindow* ctrl)
{
   if (ctrl) {
      if (!children.contains(ctrl))
         children.append(ctrl);

      ctrl->SetForm(this);
      
      if (!shown)
         ctrl->Hide();
   }
}

// +--------------------------------------------------------------------+

Button*
FormWindow::CreateButton(const char* btn_text, int ax, int ay, int aw, int ah, DWORD aid, DWORD pid)
{
   Button*        button = 0;
   ActiveWindow*  parent = this;

   if (pid)
      parent = FindControl(pid);

   button = new(__FILE__,__LINE__) Button(parent, ax, ay, aw, ah, aid);

   if (button) {
      button->SetForm(this);
      button->SetText(btn_text);

      if (!shown)
         button->Hide();
   }

   return button;
}

// +--------------------------------------------------------------------+

ImageBox*
FormWindow::CreateImageBox(const char* lbl_text, int ax, int ay, int aw, int ah, DWORD aid, DWORD pid)
{
   ImageBox*      image  = 0;
   ActiveWindow*  parent = this;

   if (pid)
      parent = FindControl(pid);

   image = new(__FILE__,__LINE__) ImageBox(parent, ax, ay, aw, ah, aid);

   if (image) {
      image->SetForm(this);
      image->SetText(lbl_text);

      if (!shown)
         image->Hide();
   }

   return image;
}

// +--------------------------------------------------------------------+

ActiveWindow*
FormWindow::CreateLabel(const char* label_text, int ax, int ay, int aw, int ah, DWORD aid, DWORD pid, DWORD astyle)
{
   ActiveWindow*  label  = 0;
   ActiveWindow*  parent = this;

   if (pid)
      parent = FindControl(pid);

   label = new(__FILE__,__LINE__) ActiveWindow(screen, ax, ay, aw, ah, aid, astyle, parent);

   if (label) {
      label->SetForm(this);
      label->SetText(label_text);

      if (!shown)
         label->Hide();
   }

   return label;
}

// +--------------------------------------------------------------------+

ListBox*
FormWindow::CreateListBox(const char* lbl_text, int ax, int ay, int aw, int ah, DWORD aid, DWORD pid)
{
   ListBox*       list   = 0;
   ActiveWindow*  parent = this;

   if (pid)
      parent = FindControl(pid);

   list = new(__FILE__,__LINE__) ListBox(parent, ax, ay, aw, ah, aid);

   if (list) {
      list->SetForm(this);

      if (!shown)
         list->Hide();
   }

   return list;
}

// +--------------------------------------------------------------------+

ComboBox*
FormWindow::CreateComboBox(const char* lbl_text, int ax, int ay, int aw, int ah, DWORD aid, DWORD pid)
{
   ComboBox*      combo  = 0;
   ActiveWindow*  parent = this;

   if (pid)
      parent = FindControl(pid);

   combo = new(__FILE__,__LINE__) ComboBox(parent, ax, ay, aw, ah, aid);

   if (combo) {
      combo->SetForm(this);
      combo->SetLabel(lbl_text);

      if (!shown)
         combo->Hide();
   }

   return combo;
}

// +--------------------------------------------------------------------+

EditBox*
FormWindow::CreateEditBox(const char* lbl_text, int ax, int ay, int aw, int ah, DWORD aid, DWORD pid)
{
   EditBox*       edit   = 0;
   ActiveWindow*  parent = this;

   if (pid)
      parent = FindControl(pid);

   edit = new(__FILE__,__LINE__) EditBox(parent, ax, ay, aw, ah, aid);

   if (edit) {
      edit->SetForm(this);
      edit->SetText(lbl_text);

      if (!shown)
         edit->Hide();
   }

   return edit;
}

// +--------------------------------------------------------------------+

RichTextBox*
FormWindow::CreateRichTextBox(const char* label_text, int ax, int ay, int aw, int ah, DWORD aid, DWORD pid, DWORD astyle)
{
   RichTextBox*   rtb    = 0;
   ActiveWindow*  parent = this;

   if (pid)
      parent = FindControl(pid);

   rtb = new(__FILE__,__LINE__) RichTextBox(parent, ax, ay, aw, ah, aid, astyle);

   if (rtb) {
      rtb->SetForm(this);
      rtb->SetText(label_text);

      if (!shown)
         rtb->Hide();
   }

   return rtb;
}

// +--------------------------------------------------------------------+

Slider*
FormWindow::CreateSlider(const char* label_text, int ax, int ay, int aw, int ah, DWORD aid, DWORD pid, DWORD astyle)
{
   Slider*        slider = 0;
   ActiveWindow*  parent = this;

   if (pid)
      parent = FindControl(pid);

   slider = new(__FILE__,__LINE__) Slider(parent, ax, ay, aw, ah, aid);

   if (slider) {
      slider->SetForm(this);

      if (!shown)
         slider->Hide();
   }

   return slider;
}

// +--------------------------------------------------------------------+

void
FormWindow::Init(const FormDef& def)
{
   if (def.GetRect().w > 0 && def.GetRect().h > 0) {
      // if form size is specified in def, and it is 
      // smaller than the current screen size,
      // center the form on the display:

      Rect r = def.GetRect();

      if (r.w < screen->Width()) {
         r.x = (screen->Width()  - r.w) / 2;
      }
      else {
         r.x = 0;
         r.w = screen->Width();
      }

      if (r.h < screen->Height()) {
         r.y = (screen->Height() - r.h) / 2;
      }
      else {
         r.y = 0;
         r.h = screen->Height();
      }

      MoveTo(r);
   }

   SetMargins(def.GetMargins());
   SetTextInsets(def.GetTextInsets());
   SetCellInsets(def.GetCellInsets());
   SetCells(def.GetCells());
   SetFixedWidth(def.GetFixedWidth());
   SetFixedHeight(def.GetFixedHeight());

   UseLayout(def.GetLayout().x_mins,
             def.GetLayout().y_mins,
             def.GetLayout().x_weights,
             def.GetLayout().y_weights);

   if (def.GetTexture().length() > 0) {
      DataLoader* loader = DataLoader::GetLoader();
      loader->SetDataPath("Screens/");
      loader->LoadTexture(def.GetTexture(), texture);
      loader->SetDataPath("");
   }

   SetBackColor(def.GetBackColor());
   SetForeColor(def.GetForeColor());

   Font* f = FontMgr::Find(def.GetFont());
   if (f) SetFont(f);

   SetTransparent(def.GetTransparent());

   ListIter<CtrlDef> ctrl = def.GetControls();
   while (++ctrl) {
      switch (ctrl->GetType()) {
      case WIN_DEF_FORM:
      case WIN_DEF_LABEL:
      default:
         CreateDefLabel(*ctrl);
         break;

      case WIN_DEF_BUTTON:
         CreateDefButton(*ctrl);
         break;

      case WIN_DEF_COMBO:
         CreateDefCombo(*ctrl);
         break;

      case WIN_DEF_IMAGE:
         CreateDefImage(*ctrl);
         break;

      case WIN_DEF_EDIT:
         CreateDefEdit(*ctrl);
         break;

      case WIN_DEF_LIST:
         CreateDefList(*ctrl);
         break;

      case WIN_DEF_SLIDER:
         CreateDefSlider(*ctrl);
         break;

      case WIN_DEF_RICH:
         CreateDefRichText(*ctrl);
         break;
      }
   }

   RegisterControls();
   DoLayout();
   CalcGrid();
}

// +--------------------------------------------------------------------+

void
FormWindow::CreateDefLabel(CtrlDef& def)
{
   ActiveWindow* ctrl = CreateLabel(def.GetText(),
                                    def.GetX(),
                                    def.GetY(),
                                    def.GetW(),
                                    def.GetH(),
                                    def.GetID(),
                                    def.GetParentID(),
                                    def.GetStyle());

   ctrl->SetAltText(def.GetAltText());
   ctrl->SetBackColor(def.GetBackColor());
   ctrl->SetForeColor(def.GetForeColor());
   ctrl->SetTextAlign(def.GetTextAlign());
   ctrl->SetSingleLine(def.GetSingleLine());
   ctrl->SetTransparent(def.GetTransparent());
   ctrl->SetHidePartial(def.GetHidePartial());

   ctrl->SetMargins(def.GetMargins());
   ctrl->SetTextInsets(def.GetTextInsets());
   ctrl->SetCellInsets(def.GetCellInsets());
   ctrl->SetCells(def.GetCells());
   ctrl->SetFixedWidth(def.GetFixedWidth());
   ctrl->SetFixedHeight(def.GetFixedHeight());

   ctrl->UseLayout(def.GetLayout().x_mins,
                   def.GetLayout().y_mins,
                   def.GetLayout().x_weights,
                   def.GetLayout().y_weights);

   if (def.GetTexture().length() > 0) {
      Bitmap*     ctrl_tex = 0;
      DataLoader* loader   = DataLoader::GetLoader();
      loader->SetDataPath("Screens/");
      loader->LoadTexture(def.GetTexture(), ctrl_tex);
      loader->SetDataPath("");

      ctrl->SetTexture(ctrl_tex);
   }

   Font* f = FontMgr::Find(def.GetFont());
   if (f) ctrl->SetFont(f);
}

void
FormWindow::CreateDefButton(CtrlDef& def)
{
   Button* ctrl = CreateButton(def.GetText(),
                               def.GetX(),
                               def.GetY(),
                               def.GetW(),
                               def.GetH(),
                               def.GetID(),
                               def.GetParentID());

   if (def.GetStandardImage().length()) {
      DataLoader* loader = DataLoader::GetLoader();
      loader->SetDataPath("Screens/");

      Bitmap* bmp = 0;
      loader->LoadTexture(def.GetStandardImage(), bmp);
      ctrl->SetStandardImage(bmp);

      if (def.GetActivatedImage().length()) {
         loader->LoadTexture(def.GetActivatedImage(), bmp);
         ctrl->SetActivatedImage(bmp);
      }

      if (def.GetTransitionImage().length()) {
         loader->LoadTexture(def.GetTransitionImage(), bmp);
         ctrl->SetTransitionImage(bmp);
      }

      loader->SetDataPath("");
   }

   ctrl->SetAltText(def.GetAltText());
   ctrl->SetEnabled(def.IsEnabled());
   ctrl->SetBackColor(def.GetBackColor());
   ctrl->SetForeColor(def.GetForeColor());
   ctrl->SetTextAlign(def.GetTextAlign());
   ctrl->SetSingleLine(def.GetSingleLine());

   ctrl->SetBevelWidth(def.GetBevelWidth());
   ctrl->SetDropShadow(def.GetDropShadow());
   ctrl->SetSticky(def.GetSticky());
   ctrl->SetTransparent(def.GetTransparent());
   ctrl->SetHidePartial(def.GetHidePartial());
   ctrl->SetPictureLocation(def.GetPictureLocation());

   ctrl->SetMargins(def.GetMargins());
   ctrl->SetTextInsets(def.GetTextInsets());
   ctrl->SetCellInsets(def.GetCellInsets());
   ctrl->SetCells(def.GetCells());
   ctrl->SetFixedWidth(def.GetFixedWidth());
   ctrl->SetFixedHeight(def.GetFixedHeight());

   if (def.GetPicture().length() > 0) {
      Bitmap pict;
      int    type = def.GetPictureType();

      DataLoader* loader = DataLoader::GetLoader();
      loader->SetDataPath("Screens/");
      loader->LoadBitmap(def.GetPicture(), pict, type);
      loader->SetDataPath("");

      ctrl->SetPicture(pict);
   }

   Font* f = FontMgr::Find(def.GetFont());
   if (f) ctrl->SetFont(f);
}

void
FormWindow::CreateDefImage(CtrlDef& def)
{
   ImageBox* ctrl = CreateImageBox(def.GetText(),
                                   def.GetX(),
                                   def.GetY(),
                                   def.GetW(),
                                   def.GetH(),
                                   def.GetID(),
                                   def.GetParentID());

   ctrl->SetAltText(def.GetAltText());
   ctrl->SetBackColor(def.GetBackColor());
   ctrl->SetForeColor(def.GetForeColor());
   ctrl->SetStyle(def.GetStyle());
   ctrl->SetTextAlign(def.GetTextAlign());
   ctrl->SetSingleLine(def.GetSingleLine());
   ctrl->SetTransparent(def.GetTransparent());
   ctrl->SetHidePartial(def.GetHidePartial());

   ctrl->SetMargins(def.GetMargins());
   ctrl->SetTextInsets(def.GetTextInsets());
   ctrl->SetCellInsets(def.GetCellInsets());
   ctrl->SetCells(def.GetCells());
   ctrl->SetFixedWidth(def.GetFixedWidth());
   ctrl->SetFixedHeight(def.GetFixedHeight());

   if (def.GetPicture().length() > 0) {
      Bitmap picture;

      DataLoader* loader = DataLoader::GetLoader();
      loader->SetDataPath("Screens/");
      loader->LoadBitmap(def.GetPicture(), picture);
      loader->SetDataPath("");

      ctrl->SetPicture(picture);
   }

   Font* f = FontMgr::Find(def.GetFont());
   if (f) ctrl->SetFont(f);
}

void
FormWindow::CreateDefList(CtrlDef& def)
{
   ListBox* ctrl = CreateListBox(def.GetText(),
                                 def.GetX(),
                                 def.GetY(),
                                 def.GetW(),
                                 def.GetH(),
                                 def.GetID(),
                                 def.GetParentID());

   ctrl->SetAltText(def.GetAltText());
   ctrl->SetEnabled(def.IsEnabled());
   ctrl->SetBackColor(def.GetBackColor());
   ctrl->SetForeColor(def.GetForeColor());
   ctrl->SetStyle(def.GetStyle());
   ctrl->SetTextAlign(def.GetTextAlign());
   ctrl->SetTransparent(def.GetTransparent());
   ctrl->SetHidePartial(def.GetHidePartial());

   ctrl->SetLineHeight(def.GetLineHeight());
   ctrl->SetShowHeadings(def.GetShowHeadings());
   ctrl->SetLeading(def.GetLeading());
   ctrl->SetMultiSelect(def.GetMultiSelect());
   ctrl->SetDragDrop(def.GetDragDrop());
   ctrl->SetScrollBarVisible(def.GetScrollBarVisible());
   ctrl->SetSmoothScroll(def.GetSmoothScroll());
   ctrl->SetItemStyle(def.GetItemStyle());
   ctrl->SetSelectedStyle(def.GetSelectedStyle());

   ctrl->SetMargins(def.GetMargins());
   ctrl->SetTextInsets(def.GetTextInsets());
   ctrl->SetCellInsets(def.GetCellInsets());
   ctrl->SetCells(def.GetCells());
   ctrl->SetFixedWidth(def.GetFixedWidth());
   ctrl->SetFixedHeight(def.GetFixedHeight());

   if (def.GetTexture().length() > 0) {
      Bitmap*     ctrl_tex = 0;
      DataLoader* loader   = DataLoader::GetLoader();
      loader->SetDataPath("Screens/");
      loader->LoadTexture(def.GetTexture(), ctrl_tex);
      loader->SetDataPath("");

      ctrl->SetTexture(ctrl_tex);
   }

   int ncols = def.NumColumns();
   for (int i = 0; i < ncols; i++) {
      ColumnDef* col = def.GetColumn(i);
      ctrl->AddColumn(col->title, col->width, col->align, col->sort);

      if (col->use_color)
         ctrl->SetColumnColor(i, col->color);
   }

   int nitems = def.NumItems();
   for (i = 0; i < nitems; i++)
      ctrl->AddItem(def.GetItem(i));

   Font* f = FontMgr::Find(def.GetFont());
   if (f) ctrl->SetFont(f);
}

void
FormWindow::CreateDefCombo(CtrlDef& def)
{
   ComboBox* ctrl = CreateComboBox(def.GetText(),
                                   def.GetX(),
                                   def.GetY(),
                                   def.GetW(),
                                   def.GetH(),
                                   def.GetID(),
                                   def.GetParentID());

   ctrl->SetAltText(def.GetAltText());
   ctrl->SetEnabled(def.IsEnabled());
   ctrl->SetBackColor(def.GetBackColor());
   ctrl->SetForeColor(def.GetForeColor());
   ctrl->SetTextAlign(def.GetTextAlign());

   ctrl->SetActiveColor(def.GetActiveColor());
   ctrl->SetBorderColor(def.GetBorderColor());
   ctrl->SetBorder(def.GetBorder());
   ctrl->SetBevelWidth(def.GetBevelWidth());
   ctrl->SetTransparent(def.GetTransparent());
   ctrl->SetHidePartial(def.GetHidePartial());

   ctrl->SetMargins(def.GetMargins());
   ctrl->SetTextInsets(def.GetTextInsets());
   ctrl->SetCellInsets(def.GetCellInsets());
   ctrl->SetCells(def.GetCells());
   ctrl->SetFixedWidth(def.GetFixedWidth());
   ctrl->SetFixedHeight(def.GetFixedHeight());

   int nitems = def.NumItems();
   for (int i = 0; i < nitems; i++)
      ctrl->AddItem(def.GetItem(i));

   Font* f = FontMgr::Find(def.GetFont());
   if (f) ctrl->SetFont(f);
}

void
FormWindow::CreateDefEdit(CtrlDef& def)
{
   EditBox* ctrl = CreateEditBox(def.GetText(),
                                 def.GetX(),
                                 def.GetY(),
                                 def.GetW(),
                                 def.GetH(),
                                 def.GetID(),
                                 def.GetParentID());

   ctrl->SetAltText(def.GetAltText());
   ctrl->SetEnabled(def.IsEnabled());
   ctrl->SetBackColor(def.GetBackColor());
   ctrl->SetForeColor(def.GetForeColor());
   ctrl->SetStyle(def.GetStyle());
   ctrl->SetSingleLine(def.GetSingleLine());
   ctrl->SetTextAlign(def.GetTextAlign());
   ctrl->SetTransparent(def.GetTransparent());
   ctrl->SetHidePartial(def.GetHidePartial());
   ctrl->SetPasswordChar(def.GetPasswordChar());

   ctrl->SetMargins(def.GetMargins());
   ctrl->SetTextInsets(def.GetTextInsets());
   ctrl->SetCellInsets(def.GetCellInsets());
   ctrl->SetCells(def.GetCells());
   ctrl->SetFixedWidth(def.GetFixedWidth());
   ctrl->SetFixedHeight(def.GetFixedHeight());

   ctrl->SetLineHeight(def.GetLineHeight());
   ctrl->SetScrollBarVisible(def.GetScrollBarVisible());
   ctrl->SetSmoothScroll(def.GetSmoothScroll());

   if (def.GetTexture().length() > 0) {
      Bitmap*     ctrl_tex = 0;
      DataLoader* loader   = DataLoader::GetLoader();
      loader->SetDataPath("Screens/");
      loader->LoadTexture(def.GetTexture(), ctrl_tex);
      loader->SetDataPath("");

      ctrl->SetTexture(ctrl_tex);
   }

   Font* f = FontMgr::Find(def.GetFont());
   if (f) ctrl->SetFont(f);
}

void
FormWindow::CreateDefSlider(CtrlDef& def)
{
   Slider* ctrl = CreateSlider(def.GetText(),
                               def.GetX(),
                               def.GetY(),
                               def.GetW(),
                               def.GetH(),
                               def.GetID(),
                               def.GetParentID());


   ctrl->SetAltText(def.GetAltText());
   ctrl->SetEnabled(def.IsEnabled());
   ctrl->SetBackColor(def.GetBackColor());
   ctrl->SetForeColor(def.GetForeColor());

   ctrl->SetActive(def.GetActive());
   ctrl->SetOrientation(def.GetOrientation());
   ctrl->SetFillColor(def.GetActiveColor());
   ctrl->SetBorderColor(def.GetBorderColor());
   ctrl->SetBorder(def.GetBorder());
   ctrl->SetStyle(def.GetStyle());
   ctrl->SetTransparent(def.GetTransparent());
   ctrl->SetHidePartial(def.GetHidePartial());
   ctrl->SetNumLeds(def.GetNumLeds());

   ctrl->SetMargins(def.GetMargins());
   ctrl->SetTextInsets(def.GetTextInsets());
   ctrl->SetCellInsets(def.GetCellInsets());
   ctrl->SetCells(def.GetCells());
   ctrl->SetFixedWidth(def.GetFixedWidth());
   ctrl->SetFixedHeight(def.GetFixedHeight());

   Font* f = FontMgr::Find(def.GetFont());
   if (f) ctrl->SetFont(f);
}

void
FormWindow::CreateDefRichText(CtrlDef& def)
{
   RichTextBox* ctrl = CreateRichTextBox(def.GetText(),
                                    def.GetX(),
                                    def.GetY(),
                                    def.GetW(),
                                    def.GetH(),
                                    def.GetID(),
                                    def.GetParentID(),
                                    def.GetStyle());

   ctrl->SetAltText(def.GetAltText());
   ctrl->SetBackColor(def.GetBackColor());
   ctrl->SetForeColor(def.GetForeColor());
   ctrl->SetLineHeight(def.GetLineHeight());
   ctrl->SetLeading(def.GetLeading());
   ctrl->SetScrollBarVisible(def.GetScrollBarVisible());
   ctrl->SetSmoothScroll(def.GetSmoothScroll());
   ctrl->SetTextAlign(def.GetTextAlign());
   ctrl->SetTransparent(def.GetTransparent());
   ctrl->SetHidePartial(def.GetHidePartial());

   ctrl->SetMargins(def.GetMargins());
   ctrl->SetTextInsets(def.GetTextInsets());
   ctrl->SetCellInsets(def.GetCellInsets());
   ctrl->SetCells(def.GetCells());
   ctrl->SetFixedWidth(def.GetFixedWidth());
   ctrl->SetFixedHeight(def.GetFixedHeight());

   if (def.GetTexture().length() > 0) {
      Bitmap*     ctrl_tex = 0;
      DataLoader* loader   = DataLoader::GetLoader();
      loader->SetDataPath("Screens/");
      loader->LoadTexture(def.GetTexture(), ctrl_tex);
      loader->SetDataPath("");

      ctrl->SetTexture(ctrl_tex);
   }

   Font* f = FontMgr::Find(def.GetFont());
   if (f) ctrl->SetFont(f);

   for (int i = 0; i < def.NumTabs(); i++)
      ctrl->SetTabStop(i, def.GetTab(i));
}

// +--------------------------------------------------------------------+

void
FormWindow::AdoptFormDef(const FormDef& def)
{
   Destroy();
   Init(def);
}

// +--------------------------------------------------------------------+

ActiveWindow*
FormWindow::FindControl(DWORD id)
{
   return FindChild(id);
}


// +--------------------------------------------------------------------+

ActiveWindow*
FormWindow::FindControl(int x, int y)
{
   ActiveWindow* mouse_tgt = 0;

   ListIter<ActiveWindow> iter = children;
   while (++iter) {
      ActiveWindow* test = iter.value();
      if (test->TargetRect().Contains(x,y)) {
         mouse_tgt = test;

         while (test) {
            test = test->FindChild(x,y);

            if (test)
               mouse_tgt = test;
         }
      }
   }

   return mouse_tgt;
}



