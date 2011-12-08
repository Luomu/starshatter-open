/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    nGenEx.lib
    FILE:         ImageBox.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    ImageBox class
*/

#include "MemDebug.h"
#include "ImageBox.h"
#include "Video.h"
#include "Bitmap.h"
#include "Font.h"
#include "DataLoader.h"

// +--------------------------------------------------------------------+

ImageBox::ImageBox(ActiveWindow* p, int ax, int ay, int aw, int ah, DWORD aid)
   : ActiveWindow(p->GetScreen(), ax, ay, aw, ah, aid, 0, p), blend_mode(Video::BLEND_ALPHA)
{
   picture_loc    = 1;
   text_align     = DT_CENTER;

   char buf[32];
   sprintf(buf, "ImageBox %d", id);
   desc = buf;
}

ImageBox::ImageBox(Screen* s, int ax, int ay, int aw, int ah, DWORD aid)
   : ActiveWindow(s, ax, ay, aw, ah, aid, 0, 0), blend_mode(Video::BLEND_ALPHA)
{
   picture_loc    = 1;
   text_align     = DT_CENTER;

   char buf[32];
   sprintf(buf, "ImageBox %d", id);
   desc = buf;
}

// +--------------------------------------------------------------------+

ImageBox::~ImageBox()
{ }

// +--------------------------------------------------------------------+

void
ImageBox::Draw()
{
   if (!shown)
      return;

   int x = 0;
   int y = 0;
   int w = rect.w;
   int h = rect.h;
   int img_w = picture.Width();
   int img_h = picture.Height();

   Rect box_rect(x,y,w,h);

   // draw the picture (if any)
   if (picture.Width()) {
      Rect irect = CalcPictureRect();
      DrawBitmap(irect.x,
                 irect.y,
                 irect.x + irect.w,
                 irect.y + irect.h,
                 &picture,
                 blend_mode);
   }

   // draw the border:
   DrawStyleRect(0, 0, w, h, style);

   // draw text here:
   if (font && text.length()) {
      int border_size = 4;

      if (style & WIN_RAISED_FRAME && style & WIN_SUNK_FRAME)
         border_size = 8;

      Rect label_rect = CalcLabelRect(img_w,img_h);
      int  vert_space = label_rect.h;
      int  horz_space = label_rect.w;

      DrawText(text.data(), 0, label_rect, DT_CALCRECT | DT_WORDBREAK | DT_CENTER);
      vert_space = (vert_space - label_rect.h)/2;
      
      label_rect.w = horz_space;

      if (vert_space > 0)
         label_rect.y += vert_space;

      Color fore = ShadeColor(fore_color, 1);
      font->SetColor(fore);
      DrawText(text.data(), 0, label_rect, DT_WORDBREAK | DT_CENTER);
   }
}

void
ImageBox::DrawTabbedText()
{
   if (!shown)
      return;

   int x = 0;
   int y = 0;
   int w = rect.w;
   int h = rect.h;
   int img_w = picture.Width();
   int img_h = picture.Height();

   Rect box_rect(x,y,w,h);

   // draw the picture (if any)
   if (picture.Width()) {
      Rect irect = CalcPictureRect();
      DrawBitmap(irect.x,
                 irect.y,
                 irect.x + irect.w,
                 irect.y + irect.h,
                 &picture,
                 Video::BLEND_ALPHA);
   }

   ActiveWindow::DrawTabbedText();
}

Rect ImageBox::CalcLabelRect(int img_w, int img_h)
{
   // fit the text in the bevel:
   Rect label_rect;
   label_rect.x = 0;
   label_rect.y = 0;
   label_rect.w = rect.w;
   label_rect.h = rect.h;

   label_rect.Deflate(2,2);

   // and around the picture, if any:
   if (img_h != 0) {
      switch (picture_loc) {
      default:
      case 0:  // the four corner positions
      case 2:  // and the center position
      case 4:  // don't affect the text position
      case 6:
      case 8:
               break;

      case 1:  // north
               label_rect.y += img_h;
               label_rect.h -= img_h;
               break;

      case 3:  // west
               label_rect.x += img_w;
               label_rect.w -= img_w;
               break;

      case 5:  // east
               label_rect.w -= img_w;
               break;

      case 7:  // south
               label_rect.h -= img_h;
               break;
      }
   }

   return label_rect;
}

// +--------------------------------------------------------------------+

Rect
ImageBox::CalcPictureRect()
{
   if (target_rect.w > 0 && target_rect.h > 0)
      return target_rect;

   int w     = rect.w;
   int h     = rect.h;
   int img_w = picture.Width();
   int img_h = picture.Height();
            
   if (img_h > h) img_h = h-2;
   if (img_w > w) img_w = w-2;
         
   int img_x_offset = 0;
   int img_y_offset = 0;
         
   switch (picture_loc) {
   default:
   // TOP ROW:
   case  0: break;

   case  1: img_x_offset = (w/2-img_w/2);
            break;

   case  2: img_x_offset = w - img_w;
            break;

   // MIDDLE ROW:
   case  3: img_y_offset = (h/2-img_h/2);
            break;
   case  4: img_x_offset = (w/2-img_w/2);
            img_y_offset = (h/2-img_h/2);
            break;
   case  5: img_x_offset = w - img_w;
            img_y_offset = (h/2-img_h/2);
            break;

   // BOTTOM ROW:
   case  6:
            img_y_offset = h - img_h;
            break;
   case  7: img_x_offset = (w/2-img_w/2);
            img_y_offset = h - img_h;
            break;
   case  8: img_x_offset = w - img_w;
            img_y_offset = h - img_h;
            break;
   }
         
   Rect img_rect;
   img_rect.x = img_x_offset;
   img_rect.y = img_y_offset;
   img_rect.w = img_w;
   img_rect.h = img_h;
   
   return img_rect;
}

// +--------------------------------------------------------------------+

int ImageBox::OnMouseMove(int x, int y)
{
   return ActiveWindow::OnMouseMove(x,y);
}

int ImageBox::OnLButtonDown(int x, int y)
{
   return ActiveWindow::OnLButtonDown(x,y);
}

int ImageBox::OnLButtonUp(int x, int y)
{
   return ActiveWindow::OnLButtonUp(x,y);
}

int ImageBox::OnClick()
{
   return ActiveWindow::OnClick();
}

int ImageBox::OnMouseEnter(int mx, int my)
{ 
   return ActiveWindow::OnMouseEnter(mx, my);
}

int ImageBox::OnMouseExit(int mx, int my)
{ 
   return ActiveWindow::OnMouseExit(mx, my);
}

// +--------------------------------------------------------------------+

void ImageBox::GetPicture(Bitmap& img) const
{
   img.CopyBitmap(picture);
}

void  ImageBox::SetPicture(const Bitmap& img)
{
   picture.CopyBitmap(img);
   picture.AutoMask();
   picture.MakeTexture();
}

int ImageBox::GetPictureLocation() const
{
   return picture_loc;
}

void  ImageBox::SetPictureLocation(int n)
{
   if (picture_loc != n && n >= 0 && n <= 8) {
      picture_loc = n;
   }
}
