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

