/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    nGenEx.lib
    FILE:         ImageBox.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    ImageBox class
*/

#ifndef ImageBox_h
#define ImageBox_h

#include "Types.h"
#include "ActiveWindow.h"
#include "Bitmap.h"

// +--------------------------------------------------------------------+

class ImageBox : public ActiveWindow
{
public:
   ImageBox(ActiveWindow* p, int ax, int ay, int aw, int ah, DWORD id=0);
   ImageBox(Screen* s,       int ax, int ay, int aw, int ah, DWORD id=0);
   virtual ~ImageBox();

   // Operations:
   virtual void      Draw();

   // Event Target Interface:
   virtual int       OnMouseMove(int x, int y);
   virtual int       OnLButtonDown(int x, int y);
   virtual int       OnLButtonUp(int x, int y);
   virtual int       OnClick();
   virtual int       OnMouseEnter(int x, int y);
   virtual int       OnMouseExit(int x, int y);

   // Property accessors:
   int      GetBlendMode()             const { return blend_mode;    }
   void     SetBlendMode(int blend)          { blend_mode = blend;   }
   bool     GetBorder()                const { return border;        }
   void     SetBorder(bool bNewValue)        { border = bNewValue;   }
   Color    GetBorderColor()           const { return border_color;  }
   void     SetBorderColor(Color c)          { border_color = c;     }
	void     GetPicture(Bitmap& img)    const;
	void     SetPicture(const Bitmap& img);
	int      GetPictureLocation()       const;
	void     SetPictureLocation(int nNewValue);
   Rect     GetTargetRect()            const { return target_rect;   }
   void     SetTargetRect(const Rect& r)     { target_rect = r;      }  

protected:
   virtual void      DrawTabbedText();

   Rect  CalcLabelRect(int img_w, int img_h);
   Rect  CalcPictureRect();

   bool           border;
   Color          border_color;
   Bitmap         picture;
   int            picture_loc;
   int            blend_mode;
   Rect           target_rect;
};

#endif ImageBox_h

