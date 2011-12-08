/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

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

