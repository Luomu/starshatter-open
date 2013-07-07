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
    FILE:         Particles.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Particle burst class
*/

#ifndef Particles_h
#define Particles_h

#include "Types.h"
#include "Geometry.h"
#include "Graphic.h"
#include "Sprite.h"

// +--------------------------------------------------------------------+

class Particles : public Graphic
{
public:
    Particles(Bitmap* bitmap, 
    int     np, 
    const   Vec3& base_loc, 
    const   Vec3& vel,
    float   base_speed  = 500.0f, 
    float   drag        = 1.0f,
    float   scale       = 1.0f, 
    float   bloom       = 0.0f, 
    float   decay       = 100.0f, 
    float   release     = 1.0f, 
    bool    cont        = false, 
    bool    trail       = true,
    bool    rise        = false,
    int     blend       = 3,
    int     nframes     = 1);

    virtual ~Particles();

    virtual void   Render(Video* video, DWORD flags);
    virtual void   ExecFrame(double seconds);
    virtual void   TranslateBy(const Point& ref) { offset = ref; loc = loc - ref; }
    virtual bool   CheckVisibility(Projector& projector);

    virtual bool   IsEmitting()   const { return emitting;  }
    virtual void   StopEmitting()       { emitting = false; }

protected:
    int         nparts;
    int         nverts;
    int         blend;
    bool        continuous;
    bool        trailing;
    bool        rising;
    bool        emitting;

    float       base_speed;
    float       max_speed;
    float       drag;
    float       release_rate;
    float       decay;
    float       min_scale;
    float       max_scale;
    float       extra;

    Point       ref_loc;
    Point       offset;
    Point*      velocity;
    Point*      part_loc;
    Point*      release;
    float*      timestamp;
    float*      intensity;
    float*      scale;
    float*      angle;
    BYTE*       frame;

    Sprite*     point_sprite;
};

#endif Particles_h
