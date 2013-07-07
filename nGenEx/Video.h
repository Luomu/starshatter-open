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

    SUBSYSTEM:    nGen.lib
    FILE:         Video.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Abstract Video Interface
*/

#ifndef Video_h
#define Video_h

#include "Geometry.h"
#include "Color.h"
#include "List.h"

// +--------------------------------------------------------------------+

class  Video;
struct VideoMode;
class  VideoSettings;
class  VideoPrivateData;

class  Bitmap;
class  Camera;
struct Rect;
struct Poly;
struct Material;
struct VertexSet;
class  Light;
class  Solid;
class  Surface;

// +--------------------------------------------------------------------+

struct RenderStats
{
    int   nframe;
    int   nverts;
    int   npolys;
    int   nlines;

    int   ncalls;

    int   total_verts;
    int   total_polys;
    int   total_lines;

    void  Clear() { nverts = npolys = nlines = ncalls = 
        total_verts = total_polys = total_lines = 0; }
};

// +--------------------------------------------------------------------+

class Video
{
public:
    enum STATUS { VIDEO_OK, VIDEO_ERR, VIDEO_BAD_PARM };

    enum RENDER_STATE {
        FILL_MODE,
        SHADE_MODE,
        LIGHTING_ENABLE,
        Z_ENABLE,
        Z_WRITE_ENABLE,
        Z_BIAS,
        TEXTURE_FILTER,
        DITHER_ENABLE,
        SPECULAR_ENABLE,
        FOG_ENABLE,
        FOG_COLOR,
        FOG_DENSITY,
        STENCIL_ENABLE,
        TEXTURE_WRAP,
        LIGHTING_PASS,

        RENDER_STATE_MAX
    };

    enum BLEND_TYPE {
        BLEND_SOLID             = 1,
        BLEND_ALPHA             = 2,
        BLEND_ADDITIVE          = 4,
        BLEND_FORCE_DWORD       = 0x7fffffff,
    };

    enum SHADE_TYPE {
        SHADE_FLAT              = 1,
        SHADE_GOURAUD           = 2,
        SHADE_PHONG             = 3,
        SHADE_FORCE_DWORD       = 0x7fffffff,
    };

    enum FILL_TYPE {
        FILL_POINT              = 1,
        FILL_WIREFRAME          = 2,
        FILL_SOLID              = 3,
        FILL_FORCE_DWORD        = 0x7fffffff,
    };

    enum FILTER_TYPE {
        FILTER_NONE             = 1,
        FILTER_LINEAR           = 2,
        FILTER_MIPMAP           = 3,
        FILTER_MIPLINEAR        = 4,
        FILTER_TRILINEAR        = 6,
        FILTER_FORCE_DWORD      = 0x7fffffff,
    };

    enum PROJECTION_TYPE {
        PROJECTION_PERSPECTIVE  = 1,
        PROJECTION_ORTHOGONAL   = 2,
        PROJECTION_FORCE_DWORD  = 0x7fffffff,
    };

    Video();
    virtual ~Video();

    STATUS         Status()    const                               { return status; }
    virtual const VideoSettings* 
    GetVideoSettings() const                        { return 0;     }
    virtual bool   SetVideoSettings(const VideoSettings* vs)       { return false; }
    virtual bool   Reset(const VideoSettings* vs)                  { return false; }

    virtual bool   SetBackgroundColor(Color c)                     { return false; }
    virtual bool   SetGammaLevel(int g)                            { return true;  }
    virtual bool   SetObjTransform(const Matrix& o, const Point& l){ return false; }

    virtual int    Width()     const                               { return 0;     }
    virtual int    Height()    const                               { return 0;     }
    virtual int    Depth()     const                               { return 0;     }

    virtual void   RecoverSurfaces()                               { }

    virtual bool   ClearAll()                                      { return false; }
    virtual bool   ClearDepthBuffer()                              { return false; }
    virtual bool   Present()                                       { return false; }
    virtual bool   Pause()                                         { return false; }
    virtual bool   Resume()                                        { return false; }

    virtual bool   IsWindowed()         const;
    virtual bool   IsFullScreen()       const;
    virtual bool   IsModeSupported(int width, int height, int bpp)
    const { return true;  }
    virtual bool   IsHardware()         const { return false; }
    virtual bool   IsHardwareTL()       const { return false; }
    virtual int    ZDepth()             const { return 0;     }
    virtual DWORD  VidMemFree()         const { return 0;     }
    virtual int    D3DLevel()           const { return 0;     }
    virtual int    MaxTexSize()         const { return 256;   }
    virtual int    MaxTexAspect()       const { return 0;     }
    virtual int    GammaLevel()         const { return 190;   }

    virtual bool   IsShadowEnabled()    const { return shadow_enabled;   }
    virtual bool   IsBumpMapEnabled()   const { return bump_enabled;     }
    virtual bool   IsSpecMapEnabled()   const { return spec_enabled;     }

    virtual void   SetShadowEnabled(bool e)   { shadow_enabled = e;      }
    virtual void   SetBumpMapEnabled(bool e)  { bump_enabled   = e;      }
    virtual void   SetSpecMapEnabled(bool e)  { spec_enabled   = e;      }

    virtual bool   Capture(Bitmap& bmp)                                  { return false;   }
    virtual bool   GetWindowRect(Rect& r)                                { return false;   }
    virtual bool   SetWindowRect(const Rect& r)                          { return false;   }
    virtual bool   SetViewport(int x, int y, int w, int h)               { return false;   }
    virtual bool   SetCamera(const Camera* cam)                          { camera = cam;
        return false;   }
    virtual bool   SetProjection(float fov, 
    float znear=1.0f,
    float zfar=1.0e6f, 
    DWORD type=PROJECTION_PERSPECTIVE)      { return false;   }
    virtual bool   SetEnvironment(Bitmap** faces)                        { return false;   }
    virtual bool   SetAmbient(Color c)                                   { return false;   }
    virtual bool   SetLights(const List<Light>& lights)                  { return false;   }
    virtual bool   SetRenderState(RENDER_STATE state, DWORD value)       { return false;   }
    virtual bool   SetBlendType(int blend_type)                          { return false;   }
    virtual bool   StartFrame()                                          { return false;   }
    virtual bool   EndFrame()                                            { return false;   }

    virtual bool   DrawPolys(int npolys, Poly* p)                        { return false;   }
    virtual bool   DrawScreenPolys(int npolys, Poly* p, int blend=0)     { return false;   }
    virtual bool   DrawSolid(Solid* s, DWORD blend_modes=0xf)            { return false;   }
    virtual bool   DrawShadow(Solid* s, int nverts, Vec3* verts, bool vis=false)
    { return false;   }
    virtual bool   DrawLines(int nlines, Vec3* v, Color c, int blend=0)  { return false;   }
    virtual bool   DrawScreenLines(int nlines, float* v, Color c, int blend=0) 
    { return false;   }
    virtual bool   DrawPoints(VertexSet* v)                              { return false;   }
    virtual bool   DrawPolyOutline(Poly* p)                              { return false;   }
    virtual bool   UseMaterial(Material* m)                              { return false;   }

    virtual bool   UseXFont(const char* name, int size, bool b, bool i)  { return false;   }
    virtual bool   DrawText(const char* text, int count, const Rect& rect,
    DWORD format, Color c)                       { return false;   }

    virtual void   PreloadTexture(Bitmap* bmp)   { }
    virtual void   PreloadSurface(Surface* s)    { }
    virtual void   InvalidateCache()             { }

    const Camera*        GetCamera()       const { return camera;  }
    const RenderStats&   GetStats()        const { return stats; }
    static Video*        GetInstance()           { return video_instance; }

protected:
    STATUS            status;
    RenderStats       stats;
    const Camera*     camera;

    bool              shadow_enabled;
    bool              bump_enabled;
    bool              spec_enabled;

    static Video*     video_instance;
};

// +--------------------------------------------------------------------+

class VideoPrivateData
{
public:
    VideoPrivateData() : valid(false)   { }
    virtual ~VideoPrivateData()         { }

    virtual int    GetType()      const { return 0;       }

    virtual bool   IsValid()      const { return valid;   }
    virtual void   Invalidate()         { valid = false;  }
    virtual void   Validate()           { valid = true;   }

protected:
    bool valid;
};

// +--------------------------------------------------------------------+

#endif Video_h

