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
    FILE:         Shadow.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Dynamic Stencil Shadow Volumes
*/

#ifndef Shadow_h
#define Shadow_h

#include "Geometry.h"
#include "Color.h"

// +--------------------------------------------------------------------+

#define Shadow_DESTROY(x) if (x) { x->Destroy(); x = 0; }

// +--------------------------------------------------------------------+

class Light;
class Scene;
class Solid;
class Video;

// +--------------------------------------------------------------------+

class Shadow
{
public:
    static const char* TYPENAME() { return "Shadow"; }

    Shadow(Solid* solid);
    virtual ~Shadow();

    int operator == (const Shadow& s) const { return this == &s; }

    // operations
    void     Render(Video* video);
    void     Update(Light* light);
    void     AddEdge(WORD v1, WORD v2);
    void     Reset();

    bool     IsEnabled()          const { return enabled; }
    void     SetEnabled(bool e)         { enabled = e;    }

    static void SetVisibleShadowVolumes(bool vis);
    static bool GetVisibleShadowVolumes();

protected:
    Solid*   solid;
    Vec3*    verts;
    int      nverts;
    int      max_verts;
    bool     enabled;

    WORD*    edges;
    DWORD    num_edges;
};

// +--------------------------------------------------------------------+

#endif Shadow_h

