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
    FILE:         Sprite.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Sprite Object
*/

#ifndef Sprite_h
#define Sprite_h

#include "Types.h"
#include "Graphic.h"
#include "Polygon.h"

// +--------------------------------------------------------------------+

class Bitmap;

class Sprite : public Graphic
{
public:
    static const char* TYPENAME() { return "Sprite"; }

    Sprite();
    Sprite(Bitmap* animation, int length=1, int repeat=1, int share=1);
    virtual ~Sprite();

    // operations
    virtual void   Render(Video* video, DWORD flags);
    virtual void   Render2D(Video* video);
    virtual void   Update();
    virtual void   Scale(double scale);
    virtual void   Rescale(double scale);
    virtual void   Reshape(int w1, int h1);

    // accessors / mutators
    int            Width()     const    { return w;       }
    int            Height()    const    { return h;       }
    int            Looping()   const    { return loop;    }
    int            NumFrames() const    { return nframes; }
    double         FrameRate() const;
    void           SetFrameRate(double rate);

    double         Shade()     const    { return shade;      }
    void           SetShade(double s)   { shade = s;         }
    double         Angle()     const    { return angle;      }
    void           SetAngle(double a)   { angle = a;         }
    int            BlendMode() const    { return blend_mode; }
    void           SetBlendMode(int a)  { blend_mode = a;    }
    int            Filter()    const    { return filter;     }
    void           SetFilter(int f)     { filter = f;        }
    virtual void   SetAnimation(Bitmap* animation, int length=1, int repeat=1, int share=1);
    virtual void   SetTexCoords(const double* uv_interleaved);

    Bitmap*        Frame()     const;
    void           SetFrameIndex(int n);

    virtual bool   IsSprite()  const    { return true;    }

protected:
    int            w, h;
    int            loop;

    int            nframes;
    int            own_frames;
    Bitmap*        frames;
    int            frame_index;
    DWORD          frame_time;
    DWORD          last_time;
    double         shade;
    double         angle;
    int            blend_mode;
    int            filter;

    Poly           poly;
    Material       mtl;
    VertexSet      vset;
};

// +--------------------------------------------------------------------+

#endif Sprite_h

