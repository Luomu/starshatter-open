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
    FILE:         VideoSettings.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Video Settings class
*/

#ifndef VideoSettings_h
#define VideoSettings_h

#include "Types.h"
#include "Video.h"

// +--------------------------------------------------------------------+

struct VideoMode
{
    enum Format {
        FMT_NONE       =  0,
        FMT_R5G5B5     = 24,
        FMT_R5G6B5     = 23,
        FMT_R8G8B8     = 20,
        FMT_X8R8G8B8   = 22
    };

    VideoMode()                                : width(0), height(0), refresh(0), format(0) { }
    VideoMode(int w, int h, Format f, int r=0) : width(w), height(h), refresh(r), format(f) { }

    int operator == (const VideoMode& m)  const { return m.width  == width  &&
        m.height == height &&
        m.format == format;   }
    int operator != (const VideoMode& m)  const { return m.width  != width  ||
        m.height != height ||
        m.format != format;   }

    const char* GetDescription() const;

    int               width;
    int               height;
    int               refresh;
    int               format;
};

// +--------------------------------------------------------------------+

struct VideoDeviceInfo
{
    VideoDeviceInfo();
    ~VideoDeviceInfo();

    int               vertex_processing;
    int               depth_buffer_bits;
    int               adapter_index;
    int               device_index;
    DWORD             device_type;
    DWORD             depth_stencil_format;
    DWORD             back_buffer_format;
    DWORD             multisample_type;
    DWORD             multisample_qual;
    char              adapter_desc[128];
    char              device_desc[128];
};

// +--------------------------------------------------------------------+

class VideoSettings
{
public:
    enum VertexProcessing {
        VTX_SOFTWARE,
        VTX_MIXED,
        VTX_HARDWARE,
        VTX_PURE
    };

    VideoSettings();
    ~VideoSettings();

    // accessor methods

    bool              IsWindowed()            const;
    bool              UseEffects()            const;
    int               GetWidth()              const;
    int               GetHeight()             const;
    int               GetDepth()              const;
    int               GetPixSize()            const;
    int               GetRefreshRate()        const;

    const char*       GetModeDescription()    const;

    int               GetVertexProcessing()   const;
    int               GetDepthBufferBits()    const;
    int               GetAdapterIndex()       const;
    int               GetDeviceIndex()        const;
    DWORD             GetDeviceType()         const;
    DWORD             GetDepthStencilFormat() const;
    DWORD             GetBackBufferFormat()   const;
    const char*       GetAdapterDesc()        const;
    const char*       GetDeviceDesc()         const;

    // properties

    bool              is_windowed;
    bool              use_effects;
    VideoMode         fullscreen_mode;
    VideoMode         windowed_mode;
    int               window_width;
    int               window_height;
    VideoDeviceInfo   fullscreen_device;
    VideoDeviceInfo   windowed_device;

    // feature set

    bool              shadows;
    bool              bumpmaps;
    bool              specmaps;
    int               max_detail;
    DWORD             enable_vs;
    DWORD             enable_ps;
    float             depth_bias;
};

#endif VideoSettings_h

