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
    FILE:         TexDX9.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Direct 3D Texture Cache
*/

#ifndef TexDX9_h
#define TexDX9_h

#include "Bitmap.h"

// +--------------------------------------------------------------------+

class  Video;
class  VideoDX9;
class  Bitmap;
struct VD3D_texture_format;

// +--------------------------------------------------------------------+

struct TexCacheDX9Entry
{
    TexCacheDX9Entry() : bitmap_id(0), texture(0), used_last(0), 
    last_modified(0), normal(false) { }

    void Release();
    void Unload();

    HANDLE               bitmap_id;
    IDirect3DTexture9*   texture;
    DWORD                used_last;
    DWORD                last_modified;
    bool                 normal;
};

// +--------------------------------------------------------------------+

class TexCacheDX9
{
public:
    TexCacheDX9(VideoDX9* video);
    virtual ~TexCacheDX9();

    void                 FrameNumber(int n) { frame_number = n; }
    IDirect3DTexture9*   FindTexture(Bitmap* bmp);
    IDirect3DTexture9*   FindNormalMap(Bitmap* bmp, float amp=1);
    bool                 LoadTexture(Bitmap* bmp, TexCacheDX9Entry* entry);
    void                 InvalidateCache();

    int                  count;

private:
    int                  FreeLRU(int tries=4);
    int                  FreeUpCache();
    void                 CreateNormalMap(int index, float amp=1);

    VideoDX9*            video;
    IDirect3D9*          d3d;
    IDirect3DDevice9*    d3ddevice;

    DWORD                vidmem;

    int                  bad_frame;
    DWORD                frame_number;

    int                  mru;
    TexCacheDX9Entry*    cache;
};

#endif // TexDX9_h

