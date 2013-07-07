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
    FILE:         VideoDX9Enum.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Direct3D and Direct3D Video classes for DirectX 7 
*/

#ifndef VideoDX9Enum_h
#define VideoDX9Enum_h

#include <d3d9.h>

#include "Video.h"
#include "List.h"
#include <vector>

// +--------------------------------------------------------------------+

class  VideoDX9Enum;
struct VideoDX9DisplayMode;
struct VideoDX9AdapterInfo;
struct VideoDX9DeviceInfo;
struct VideoDX9DSMSConflict;
struct VideoDX9DeviceCombo;
class  VideoSettings;

// +--------------------------------------------------------------------+

class VideoDX9Enum
{
public:
    enum VP_TYPE {
        SOFTWARE_VP,
        MIXED_VP,
        HARDWARE_VP,
        PURE_HARDWARE_VP
    };

    VideoDX9Enum(IDirect3D9* d3d9);
    ~VideoDX9Enum();

    void     SetDirect3D9(IDirect3D9* d3d9);

    HRESULT  Enumerate();
    bool     SuggestWindowSettings(VideoSettings* vs);
    bool     SuggestFullscreenSettings(VideoSettings* vs);

    int                  NumAdapters()  const { return adapter_info_list.size(); }
    void                 SelectAdapter(int index);
    VideoDX9AdapterInfo* GetAdapterInfo();
    VideoDX9DeviceInfo*  GetDeviceInfo(DWORD devtype);

    bool     IsModeSupported(int width, int height, int bpp) const;

    UINT     min_width;
    UINT     min_height;
    UINT     min_color_bits;
    UINT     min_alpha_bits;
    UINT     min_depth_bits;
    UINT     min_stencil_bits;

    bool     uses_depth_buffer;
    bool     uses_mixed_vp;
    bool     req_windowed;
    bool     req_fullscreen;

private:
    HRESULT  EnumerateDevices(VideoDX9AdapterInfo* adapter_info, std::vector<D3DFORMAT>& adapter_format_list);
    HRESULT  EnumerateDeviceCombos(VideoDX9DeviceInfo* device_info, std::vector<D3DFORMAT>& adapter_format_list);

    void     BuildDepthStencilFormatList(VideoDX9DeviceCombo* device_combo);
    void     BuildMultiSampleTypeList(VideoDX9DeviceCombo* device_combo);
    void     BuildDSMSConflictList(VideoDX9DeviceCombo* device_combo);
    void     BuildVertexProcessingTypeList(VideoDX9DeviceInfo* device_info, VideoDX9DeviceCombo* device_combo);
    void     BuildPresentIntervalList(VideoDX9DeviceInfo* device_info, VideoDX9DeviceCombo* device_combo);

    IDirect3D9*                d3d;
    std::vector<D3DFORMAT>     allowed_adapter_format_list;

    List<VideoDX9AdapterInfo>  adapter_info_list;
    int                        adapter_index;
};


// +--------------------------------------------------------------------+

struct VideoDX9AdapterInfo
{
    static const char* TYPENAME() { return "VideoDX9AdapterInfo"; }

    VideoDX9AdapterInfo();
    ~VideoDX9AdapterInfo();

    const char* GetDescription() const;

    int                        adapter_ordinal;
    D3DADAPTER_IDENTIFIER9     adapter_identifier;
    List<VideoDX9DisplayMode>  display_mode_list;
    List<VideoDX9DeviceInfo>   device_info_list;
};


// +--------------------------------------------------------------------+

struct VideoDX9DisplayMode
{
    static const char* TYPENAME() { return "VideoDX9DisplayMode"; }

    VideoDX9DisplayMode();
    VideoDX9DisplayMode(const VideoDX9DisplayMode& m);
    VideoDX9DisplayMode(const D3DDISPLAYMODE& m);

    int operator<(const VideoDX9DisplayMode& m) const;
    int operator<=(const VideoDX9DisplayMode& m) const;

    const char*          GetDescription() const;
    static const char*   D3DFormatToString(D3DFORMAT format);

    UINT        width;
    UINT        height;
    UINT        refresh;
    D3DFORMAT   format;
};


// +--------------------------------------------------------------------+

struct VideoDX9DeviceInfo
{
    static const char* TYPENAME() { return "VideoDX9DeviceInfo"; }

    VideoDX9DeviceInfo();
    ~VideoDX9DeviceInfo();

    int                        adapter_ordinal;
    D3DDEVTYPE                 device_type;
    D3DCAPS9                   caps;
    List<VideoDX9DeviceCombo>  device_combo_list;
};


// +--------------------------------------------------------------------+

struct VideoDX9FormatConflict
{
    static const char* TYPENAME() { return "VideoDX9FormatConflict"; }

    D3DFORMAT                  ds_format;
    D3DMULTISAMPLE_TYPE        multisample_type;
};


// +--------------------------------------------------------------------+

struct VideoDX9DeviceCombo
{
    VideoDX9DeviceCombo();
    ~VideoDX9DeviceCombo();

    int                  adapter_ordinal;
    D3DDEVTYPE           device_type;
    D3DFORMAT            adapter_format;
    D3DFORMAT            back_buffer_format;
    bool                 is_windowed;

    std::vector<int>                    vertex_processing_list;
    std::vector<D3DFORMAT>                depth_stencil_fmt_list;
    std::vector<D3DMULTISAMPLE_TYPE>    multisample_type_list;
    std::vector<DWORD>                    multisample_qual_list;
    std::vector<DWORD>                    present_interval_list;

    List<VideoDX9FormatConflict> conflict_list;
};

// +--------------------------------------------------------------------+

#endif VideoDX9Enum_h

