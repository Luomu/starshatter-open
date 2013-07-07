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
    FILE:         VideoDX9Enum.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Direct3D Video class for DirectX 9
*/

#include "MemDebug.h"
#include "VideoDX9Enum.h"
#include "VideoSettings.h"
#include "Color.h"

// +--------------------------------------------------------------------+

void           Print(const char* msg, ...);
char*          DDErrStr(HRESULT dderr);
void           VideoDX9Error(const char* msg, HRESULT dderr);
int            VD3D_describe_things;

#ifndef RELEASE
#define RELEASE(x) if (x) { x->Release(); x=NULL; }
#endif


// +--------------------------------------------------------------------+

static UINT GetBitsPerPixel(D3DFORMAT fmt)
{
    switch (fmt) {
    case D3DFMT_A8R8G8B8:      return 32;
    case D3DFMT_X8R8G8B8:      return 32;
    case D3DFMT_A2B10G10R10:   return 32;
    case D3DFMT_A2R10G10B10:   return 32;
    case D3DFMT_R8G8B8:        return 24;
    case D3DFMT_R5G6B5:        return 16;
    case D3DFMT_X1R5G5B5:      return 16;
    case D3DFMT_A1R5G5B5:      return 16;
    case D3DFMT_A4R4G4B4:      return 16;
    case D3DFMT_A8R3G3B2:      return 16;
    case D3DFMT_X4R4G4B4:      return 16;
    case D3DFMT_R3G3B2:        return 8;
    default:                   return 0;
    }
}

// +--------------------------------------------------------------------+

static UINT GetColorChannelBits(D3DFORMAT fmt)
{
    switch (fmt) {
    case D3DFMT_A2B10G10R10:   return 10;
    case D3DFMT_A2R10G10B10:   return 10;
    case D3DFMT_R8G8B8:        return 8;
    case D3DFMT_A8R8G8B8:      return 8;
    case D3DFMT_X8R8G8B8:      return 8;
    case D3DFMT_R5G6B5:        return 6;
    case D3DFMT_X1R5G5B5:      return 5;
    case D3DFMT_A1R5G5B5:      return 5;
    case D3DFMT_A4R4G4B4:      return 4;
    case D3DFMT_R3G3B2:        return 2;
    case D3DFMT_A8R3G3B2:      return 2;
    case D3DFMT_X4R4G4B4:      return 4;
    default:                   return 0;
    }
}

// +--------------------------------------------------------------------+

static UINT GetAlphaChannelBits(D3DFORMAT fmt)
{
    switch (fmt) {
    case D3DFMT_R8G8B8:        return 0;
    case D3DFMT_A8R8G8B8:      return 8;
    case D3DFMT_X8R8G8B8:      return 0;
    case D3DFMT_R5G6B5:        return 0;
    case D3DFMT_X1R5G5B5:      return 0;
    case D3DFMT_A1R5G5B5:      return 1;
    case D3DFMT_A4R4G4B4:      return 4;
    case D3DFMT_R3G3B2:        return 0;
    case D3DFMT_A8R3G3B2:      return 8;
    case D3DFMT_X4R4G4B4:      return 0;
    case D3DFMT_A2B10G10R10:   return 2;
    case D3DFMT_A2R10G10B10:   return 2;
    default:                   return 0;
    }
}

// +--------------------------------------------------------------------+

static UINT GetDepthBits(D3DFORMAT fmt)
{
    switch (fmt) {
    case D3DFMT_D16:           return 16;
    case D3DFMT_D15S1:         return 15;
    case D3DFMT_D24X8:         return 24;
    case D3DFMT_D24S8:         return 24;
    case D3DFMT_D24X4S4:       return 24;
    case D3DFMT_D32:           return 32;
    default:                   return 0;
    }
}

// +--------------------------------------------------------------------+

static UINT GetStencilBits(D3DFORMAT fmt)
{
    switch (fmt) {
    case D3DFMT_D16:           return 0;
    case D3DFMT_D15S1:         return 1;
    case D3DFMT_D24X8:         return 0;
    case D3DFMT_D24S8:         return 8;
    case D3DFMT_D24X4S4:       return 4;
    case D3DFMT_D32:           return 0;
    default:                   return 0;
    }
}

// +--------------------------------------------------------------------+
//
// This routine prints a text description of the indicated driver
// into the error log file.

static void DescribeGUID(GUID* lpGUID)
{
    if (lpGUID)
    Print("   GUID: %08x-%04x-%04x-%02x%02x-%02x%02x%02x%02x%02x%02x\n",
    lpGUID->Data1,    lpGUID->Data2,   lpGUID->Data3,
    lpGUID->Data4[0], lpGUID->Data4[1],
    lpGUID->Data4[2], lpGUID->Data4[3],
    lpGUID->Data4[4], lpGUID->Data4[5],
    lpGUID->Data4[6], lpGUID->Data4[7]);
    else
    Print("   GUID IS NULL!\n");
}


// +--------------------------------------------------------------------+

VideoDX9Enum::VideoDX9Enum(IDirect3D9* d3d9)
{
    if (d3d9) {
        d3d = d3d9;
        d3d->AddRef();
    }

    min_width         = 640;
    min_height        = 480;
    min_color_bits    =   5;
    min_alpha_bits    =   0;
    min_depth_bits    =  16;
    min_stencil_bits  =   0;

    uses_depth_buffer = false;
    uses_mixed_vp     = false;
    req_windowed      = false;
    req_fullscreen    = false;

    adapter_index     = 0;
}

VideoDX9Enum::~VideoDX9Enum()
{
    adapter_info_list.destroy();
    RELEASE(d3d);
}

void
VideoDX9Enum::SetDirect3D9(IDirect3D9* d3d9)
{
    RELEASE(d3d);

    if (d3d9) {
        d3d = d3d9;
        d3d->AddRef();
    }
}


// +--------------------------------------------------------------------+

void
VideoDX9Enum::SelectAdapter(int index)
{
    if (index >= 0 && index < adapter_info_list.size())
    adapter_index = index;
}

VideoDX9AdapterInfo*
VideoDX9Enum::GetAdapterInfo()
{
    if (adapter_index >= 0 && adapter_index < adapter_info_list.size())
    return adapter_info_list[adapter_index];

    return 0;
}

VideoDX9DeviceInfo*
VideoDX9Enum::GetDeviceInfo(DWORD devtype)
{
    if (adapter_index >= 0 && adapter_index < adapter_info_list.size()) {
        VideoDX9AdapterInfo* adapter = adapter_info_list[adapter_index];

        if (adapter) {
            ListIter<VideoDX9DeviceInfo> iter = adapter->device_info_list;
            while (++iter) {
                VideoDX9DeviceInfo* dev_info = iter.value();

                if (dev_info->device_type == (D3DDEVTYPE) devtype)
                return dev_info;
            }
        }
    }

    return 0;
}

bool
VideoDX9Enum::IsModeSupported(int w, int h, int b) const
{
    if (adapter_index >= 0 && adapter_index < adapter_info_list.size()) {
        VideoDX9AdapterInfo* adapter = adapter_info_list[adapter_index];

        ListIter<VideoDX9DisplayMode>  mode_iter = adapter->display_mode_list;
        while (++mode_iter) {
            VideoDX9DisplayMode* mode = mode_iter.value();

            if (mode->width                   == (UINT) w &&
                    mode->height                  == (UINT) h &&
                    GetBitsPerPixel(mode->format) == (UINT) b) {

                return true;
            }
        }
    }

    return false;
}

// +--------------------------------------------------------------------+

HRESULT  
VideoDX9Enum::Enumerate()
{
    HRESULT hr = E_FAIL;

    if (!d3d)
    return hr;

    if (VD3D_describe_things > 0) {
        Print("Video DX9 Enumerate Adapters and Devices\n");
        Print("----------------------------------------\n\n");
    }

    allowed_adapter_format_list.push_back(D3DFMT_X8R8G8B8);
    allowed_adapter_format_list.push_back(D3DFMT_X1R5G5B5);
    allowed_adapter_format_list.push_back(D3DFMT_R5G6B5);
    allowed_adapter_format_list.push_back(D3DFMT_A2R10G10B10);

    VideoDX9AdapterInfo* adapter_info = 0;
    std::vector<D3DFORMAT> adapter_format_list;
    UINT                 num_adapters = d3d->GetAdapterCount();

    for (UINT ordinal = 0; ordinal < num_adapters; ordinal++) {
        adapter_info = new(__FILE__,__LINE__) VideoDX9AdapterInfo;
        if (!adapter_info)
        return E_OUTOFMEMORY;

        adapter_info->adapter_ordinal = ordinal;
        d3d->GetAdapterIdentifier(ordinal, 0, &adapter_info->adapter_identifier);

        // Get list of all display modes on this adapter.  
        // Also build a temporary list of all display adapter formats.
        adapter_format_list.clear();

        for (size_t iaaf = 0; iaaf < allowed_adapter_format_list.size(); iaaf++) {
            D3DFORMAT allowed_adapter_format = allowed_adapter_format_list[iaaf];
            UINT      num_adapter_modes      = d3d->GetAdapterModeCount(ordinal, allowed_adapter_format);
            
            for (UINT mode = 0; mode < num_adapter_modes; mode++) {
                D3DDISPLAYMODE display_mode;
                d3d->EnumAdapterModes(ordinal, allowed_adapter_format, mode, &display_mode);
                
                if (display_mode.Width  < min_width  ||
                        display_mode.Height < min_height ||
                        GetColorChannelBits(display_mode.Format) < min_color_bits) {
                    continue;
                }

                VideoDX9DisplayMode* dx9_display_mode = new(__FILE__,__LINE__) VideoDX9DisplayMode(display_mode);

                if (!dx9_display_mode) {
                    delete adapter_info;
                    return E_OUTOFMEMORY;
                }

                adapter_info->display_mode_list.append(dx9_display_mode);

                bool contains_display_mode = false;
                for (auto afli = adapter_format_list.begin(); afli != adapter_format_list.end(); ++afli) {
                    if (*afli == display_mode.Format) {
                        contains_display_mode = true;
                        break;
                    }
                }
                if (!contains_display_mode)
                    adapter_format_list.push_back(display_mode.Format);
                
            }
        }

        // Sort displaymode list
        adapter_info->display_mode_list.sort();

        if (VD3D_describe_things > 0) {
            Print("Adapter %d. %s\n", ordinal, adapter_info->adapter_identifier.Description);
            DescribeGUID(&adapter_info->adapter_identifier.DeviceIdentifier);

            if (VD3D_describe_things > 4) {
                ListIter<VideoDX9DisplayMode> m_iter = adapter_info->display_mode_list;
                while (++m_iter) {
                    VideoDX9DisplayMode* m = m_iter.value();

                    Print("   Mode %3d %s\n", m_iter.index(), m->GetDescription());
                }

                Print("\n");
            }
        }

        // Get info for each device on this adapter
        if (FAILED(hr = EnumerateDevices(adapter_info, adapter_format_list))) {
            delete adapter_info;
            return hr;
        }

        // If at least one device on this adapter is available and compatible
        // with the app, add the adapterInfo to the list
        if (adapter_info->device_info_list.size() == 0)
        delete adapter_info;
        else
        adapter_info_list.append(adapter_info);

        if (VD3D_describe_things > 0) {
            Print("\n");
        }
    }

    return S_OK;
}

// +--------------------------------------------------------------------+

HRESULT  
VideoDX9Enum::EnumerateDevices(VideoDX9AdapterInfo* adapter_info, std::vector<D3DFORMAT>& adapter_format_list)
{
    HRESULT              hr          = E_FAIL;
    const D3DDEVTYPE     dtypes[3]   = { D3DDEVTYPE_HAL, D3DDEVTYPE_SW, D3DDEVTYPE_REF };
    const char*          dtypestr[3] = { "D3DDEVTYPE_HAL", "D3DDEVTYPE_SW", "D3DDEVTYPE_REF" };
    VideoDX9DeviceInfo*  device_info = 0;

    for (int i = 0; i < 3; i++ ) {
        device_info = new(__FILE__,__LINE__) VideoDX9DeviceInfo;
        if (!device_info)
        return E_OUTOFMEMORY;

        device_info->adapter_ordinal = adapter_info->adapter_ordinal;
        device_info->device_type     = dtypes[i];
        
        if (FAILED(d3d->GetDeviceCaps(adapter_info->adapter_ordinal, 
                        device_info->device_type, 
                        &device_info->caps))) {
            delete device_info;
            continue;
        }

        if (VD3D_describe_things > 1) {
            Print("   Device %d - %s\n", i, dtypestr[i]);
            Print("      Max Texture Width:    %d\n", device_info->caps.MaxTextureWidth);
            Print("      Max Texture Height:   %d\n", device_info->caps.MaxTextureHeight);
        }

        // Get info for each device combo on this device
        if (FAILED(hr = EnumerateDeviceCombos(device_info, adapter_format_list))) {
            delete device_info;
            return hr;
        }

        // make sure at least one devicecombo for this device was found
        if (device_info->device_combo_list.size() < 1) {
            delete device_info;
            continue;
        }

        adapter_info->device_info_list.append(device_info);
    }

    return S_OK;
}

// +--------------------------------------------------------------------+

HRESULT  
VideoDX9Enum::EnumerateDeviceCombos(VideoDX9DeviceInfo* device_info, std::vector<D3DFORMAT>& adapter_format_list)
{
    const D3DFORMAT back_buffer_formats[] = {
        D3DFMT_A8R8G8B8,
        D3DFMT_X8R8G8B8,
        D3DFMT_R8G8B8,
        D3DFMT_R5G6B5,
        D3DFMT_A1R5G5B5,
        D3DFMT_X1R5G5B5
    };

    bool is_windowed[] = { false, true };

    // See which adapter formats are supported by this device
    D3DFORMAT a_fmt;
    for (size_t i = 0; i < adapter_format_list.size(); i++) {
        a_fmt = adapter_format_list[i];
        
        D3DFORMAT b_fmt;
        for (int n = 0; n < 6; n++) {
            b_fmt = back_buffer_formats[n];

            if (GetAlphaChannelBits(b_fmt) < min_alpha_bits)
            continue;

            bool win;
            for (int w = 0; w < 2; w++) {
                win = is_windowed[w];

                if (!win && req_windowed)
                continue;

                if (win && req_fullscreen)
                continue;

                if (FAILED(d3d->CheckDeviceType(device_info->adapter_ordinal,
                                device_info->device_type, 
                                a_fmt,
                                b_fmt,
                                win))) {
                    continue;
                }

                // At this point, we have an adapter/device/adapterformat/backbufferformat/iswindowed
                // DeviceCombo that is supported by the system.  We still need to confirm that it's 
                // compatible with the app, and find one or more suitable depth/stencil buffer format,
                // multisample type, vertex processing type, and present interval.

                VideoDX9DeviceCombo* device_combo = 0;
                device_combo = new(__FILE__,__LINE__) VideoDX9DeviceCombo;
                if (!device_combo)
                return E_OUTOFMEMORY;

                device_combo->adapter_ordinal    = device_info->adapter_ordinal;
                device_combo->device_type        = device_info->device_type;
                device_combo->adapter_format     = a_fmt;
                device_combo->back_buffer_format = b_fmt;
                device_combo->is_windowed        = win;

                if (uses_depth_buffer) {
                    BuildDepthStencilFormatList(device_combo);
                    if (device_combo->depth_stencil_fmt_list.size() < 1) {
                        delete device_combo;
                        continue;
                    }
                }

                BuildMultiSampleTypeList(device_combo);
                if (device_combo->multisample_type_list.size() < 1) {
                    delete device_combo;
                    continue;
                }
                
                BuildDSMSConflictList(device_combo);

                BuildVertexProcessingTypeList(device_info, device_combo);
                if (device_combo->vertex_processing_list.size() < 1) {
                    delete device_combo;
                    continue;
                }

                BuildPresentIntervalList(device_info, device_combo);

                device_info->device_combo_list.append(device_combo);
            }
        }
    }

    return S_OK;
}

void  
VideoDX9Enum::BuildDepthStencilFormatList(VideoDX9DeviceCombo* device_combo)
{
    const D3DFORMAT depth_stencil_formats[] = {
        D3DFMT_D32,
        D3DFMT_D24S8,
        D3DFMT_D24X4S4,
        D3DFMT_D24X8,
        D3DFMT_D16,
        D3DFMT_D15S1,
    };

    for (int i = 0; i < 6; i++) {
        D3DFORMAT fmt = depth_stencil_formats[i];

        if (GetDepthBits(fmt) < min_depth_bits)
        continue;

        if (GetStencilBits(fmt) < min_stencil_bits)
        continue;

        if (SUCCEEDED(d3d->CheckDeviceFormat(device_combo->adapter_ordinal, 
                        device_combo->device_type,
                        device_combo->adapter_format, 
                        D3DUSAGE_DEPTHSTENCIL,
                        D3DRTYPE_SURFACE,
                        fmt))) {

            if (SUCCEEDED(d3d->CheckDepthStencilMatch(device_combo->adapter_ordinal, 
                            device_combo->device_type,
                            device_combo->adapter_format,
                            device_combo->back_buffer_format,
                            fmt))) {

                device_combo->depth_stencil_fmt_list.push_back(fmt);
            }
        }
    }
}

void  
VideoDX9Enum::BuildMultiSampleTypeList(VideoDX9DeviceCombo* device_combo)
{
    const D3DMULTISAMPLE_TYPE multisample_type_array[] = { 
        D3DMULTISAMPLE_NONE,
        D3DMULTISAMPLE_NONMASKABLE,
        D3DMULTISAMPLE_2_SAMPLES,
        D3DMULTISAMPLE_3_SAMPLES,
        D3DMULTISAMPLE_4_SAMPLES,
        D3DMULTISAMPLE_5_SAMPLES,
        D3DMULTISAMPLE_6_SAMPLES,
        D3DMULTISAMPLE_7_SAMPLES,
        D3DMULTISAMPLE_8_SAMPLES,
        D3DMULTISAMPLE_9_SAMPLES,
        D3DMULTISAMPLE_10_SAMPLES,
        D3DMULTISAMPLE_11_SAMPLES,
        D3DMULTISAMPLE_12_SAMPLES,
        D3DMULTISAMPLE_13_SAMPLES,
        D3DMULTISAMPLE_14_SAMPLES,
        D3DMULTISAMPLE_15_SAMPLES,
        D3DMULTISAMPLE_16_SAMPLES,
    };

    for (int i = 0; i < 17; i++) {
        D3DMULTISAMPLE_TYPE  multisample_type = multisample_type_array[i];
        DWORD                multisample_qual = 0;

        if (SUCCEEDED(d3d->CheckDeviceMultiSampleType(device_combo->adapter_ordinal,
                        device_combo->device_type, 
                        device_combo->back_buffer_format,
                        device_combo->is_windowed,
                        multisample_type,
                        &multisample_qual))) {

            device_combo->multisample_type_list.push_back(multisample_type);
            device_combo->multisample_qual_list.push_back(multisample_qual);
        }
    }
}

void  
VideoDX9Enum::BuildDSMSConflictList(VideoDX9DeviceCombo* device_combo)
{
    for (size_t i = 0; i < device_combo->depth_stencil_fmt_list.size(); i++) {
        D3DFORMAT depth_format = (D3DFORMAT) device_combo->depth_stencil_fmt_list[i];

        for (size_t n = 0; n < device_combo->multisample_type_list.size(); n++) {
            D3DMULTISAMPLE_TYPE multisample_type = (D3DMULTISAMPLE_TYPE) device_combo->multisample_type_list[n];

            if (FAILED(d3d->CheckDeviceMultiSampleType(device_combo->adapter_ordinal,
                            device_combo->device_type,
                            depth_format,
                            device_combo->is_windowed,
                            multisample_type,
                            NULL))) {

                VideoDX9FormatConflict* conflict = new(__FILE__,__LINE__) VideoDX9FormatConflict;

                conflict->ds_format        = depth_format;
                conflict->multisample_type = multisample_type;

                device_combo->conflict_list.append(conflict);
            }
        }
    }
}

void  
VideoDX9Enum::BuildVertexProcessingTypeList(VideoDX9DeviceInfo* device_info, VideoDX9DeviceCombo* device_combo)
{
    if ((device_info->caps.DevCaps & D3DDEVCAPS_HWTRANSFORMANDLIGHT) != 0) {
        if ((device_info->caps.DevCaps & D3DDEVCAPS_PUREDEVICE) != 0) {
            device_combo->vertex_processing_list.push_back(PURE_HARDWARE_VP);
        }

        device_combo->vertex_processing_list.push_back(HARDWARE_VP);

        if (uses_mixed_vp) {
            device_combo->vertex_processing_list.push_back(MIXED_VP);
        }
    }

    device_combo->vertex_processing_list.push_back(SOFTWARE_VP);
}

void  
VideoDX9Enum::BuildPresentIntervalList(VideoDX9DeviceInfo* device_info, VideoDX9DeviceCombo* device_combo)
{
    const DWORD present_interval_array[] = { 
        D3DPRESENT_INTERVAL_IMMEDIATE,
        D3DPRESENT_INTERVAL_DEFAULT,
        D3DPRESENT_INTERVAL_ONE,
        D3DPRESENT_INTERVAL_TWO,
        D3DPRESENT_INTERVAL_THREE,
        D3DPRESENT_INTERVAL_FOUR,
    };

    for (int i = 0; i < 6; i++) {
        DWORD interval = present_interval_array[i];
        
        if (device_combo->is_windowed && i > 2) {
            // none of the remaining intervals are supported in windowed mode.
            break;
        }

        // Note that D3DPRESENT_INTERVAL_DEFAULT is zero, so you
        // can't do a caps check for it -- it is always available.

        if (interval == D3DPRESENT_INTERVAL_DEFAULT ||
                (device_info->caps.PresentationIntervals & interval)) {

            device_combo->present_interval_list.push_back(interval);
        }
    }
}

bool  
VideoDX9Enum::SuggestWindowSettings(VideoSettings* vs)
{
    if (!vs)
    return false;

    // Get display mode of primary adapter (which is assumed to be where the window 
    // will appear)
    D3DDISPLAYMODE desktop_display_mode;
    d3d->GetAdapterDisplayMode(0, &desktop_display_mode);

    VideoDX9AdapterInfo* best_adapter_info    = 0;
    VideoDX9DeviceInfo*  best_device_info     = 0;
    VideoDX9DeviceCombo* best_device_combo    = 0;
    int                  best_adapter_index   = 0;
    int                  best_device_index    = 0;

    ListIter<VideoDX9AdapterInfo> a_iter = adapter_info_list;
    while (++a_iter) {
        VideoDX9AdapterInfo* adapter_info = a_iter.value();

        ListIter<VideoDX9DeviceInfo> d_iter = adapter_info->device_info_list;
        while (++d_iter) {
            VideoDX9DeviceInfo* device_info = d_iter.value();

            ListIter<VideoDX9DeviceCombo> c_iter = device_info->device_combo_list;
            while (++c_iter) {
                VideoDX9DeviceCombo* device_combo = c_iter.value();

                bool formats_match = (device_combo->back_buffer_format == device_combo->adapter_format);
                
                if (!device_combo->is_windowed)
                continue;
                
                if (device_combo->adapter_format != desktop_display_mode.Format)
                continue;

                // If we haven't found a compatible DeviceCombo yet, or if this set
                // is better (because it's a HAL, and/or because formats match better),
                // save it
                if (best_device_combo == NULL || 
                        best_device_combo->device_type != D3DDEVTYPE_HAL && device_combo->device_type == D3DDEVTYPE_HAL ||
                        device_combo->device_type == D3DDEVTYPE_HAL && formats_match) {

                    best_adapter_info    = adapter_info;
                    best_adapter_index   = a_iter.index();
                    best_device_info     = device_info;
                    best_device_index    = d_iter.index();
                    best_device_combo    = device_combo;

                    if (device_combo->device_type == D3DDEVTYPE_HAL && formats_match) {
                        // This windowed device combo looks great -- take it
                        goto EndWindowedDeviceComboSearch;
                    }

                    // Otherwise keep looking for a better windowed device combo
                }
            }
        }
    }

EndWindowedDeviceComboSearch:
    if (best_device_combo == NULL)
    return false;

    VideoDeviceInfo* win_device = &vs->windowed_device;

    vs->is_windowed = true;
    vs->windowed_mode.width          = desktop_display_mode.Width;
    vs->windowed_mode.height         = desktop_display_mode.Height;
    vs->windowed_mode.refresh        = desktop_display_mode.RefreshRate;
    vs->windowed_mode.format         = desktop_display_mode.Format;

    win_device->adapter_index        = best_adapter_index;
    win_device->device_index         = best_device_index;
    win_device->device_type          = best_device_info->device_type;
    win_device->back_buffer_format   = best_device_combo->back_buffer_format;
    win_device->depth_buffer_bits    = GetDepthBits((D3DFORMAT) best_device_combo->depth_stencil_fmt_list[0]);
    win_device->depth_stencil_format = best_device_combo->depth_stencil_fmt_list[0];
    win_device->multisample_type     = best_device_combo->multisample_type_list[0];
    win_device->multisample_qual     = best_device_combo->multisample_qual_list[0];
    win_device->vertex_processing    = best_device_combo->vertex_processing_list[0];

    return true;
}

bool
VideoDX9Enum::SuggestFullscreenSettings(VideoSettings* vs)
{
    if (!vs)
    return false;

    WORD desired_width  = vs->fullscreen_mode.width;
    WORD desired_height = vs->fullscreen_mode.height;

    // For fullscreen, default to first HAL DeviceCombo that supports the current desktop 
    // display mode, or any display mode if HAL is not compatible with the desktop mode, or 
    // non-HAL if no HAL is available
    D3DDISPLAYMODE desktop_display_mode;
    D3DDISPLAYMODE best_desktop_display_mode;

    best_desktop_display_mode.Width   = 0;
    best_desktop_display_mode.Height  = 0;
    best_desktop_display_mode.Format  = D3DFMT_UNKNOWN;
    best_desktop_display_mode.RefreshRate = 0;

    VideoDX9AdapterInfo* best_adapter_info    = 0;
    VideoDX9DeviceInfo*  best_device_info     = 0;
    VideoDX9DeviceCombo* best_device_combo    = 0;
    int                  best_adapter_index   = 0;
    int                  best_device_index    = 0;

    ListIter<VideoDX9AdapterInfo> a_iter = adapter_info_list;
    while (++a_iter) {
        VideoDX9AdapterInfo* adapter_info = a_iter.value();
        d3d->GetAdapterDisplayMode(adapter_info->adapter_ordinal, &desktop_display_mode);

        ListIter<VideoDX9DeviceInfo> d_iter = adapter_info->device_info_list;
        while (++d_iter) {
            VideoDX9DeviceInfo* device_info = d_iter.value();

            ListIter<VideoDX9DeviceCombo> c_iter = device_info->device_combo_list;
            while (++c_iter) {
                VideoDX9DeviceCombo* device_combo = c_iter.value();

                bool bAdapterMatchesBB      = (device_combo->back_buffer_format == device_combo->adapter_format);
                bool bAdapterMatchesDesktop = (device_combo->adapter_format     == desktop_display_mode.Format);

                if (device_combo->is_windowed)
                continue;

                // If we haven't found a compatible set yet, or if this set
                // is better (because it's a HAL, and/or because formats match better),
                // save it
                if (best_device_combo == NULL ||
                        best_device_combo->device_type != D3DDEVTYPE_HAL && device_info->device_type == D3DDEVTYPE_HAL ||
                        device_combo->device_type == D3DDEVTYPE_HAL && best_device_combo->adapter_format != desktop_display_mode.Format && bAdapterMatchesDesktop ||
                        device_combo->device_type == D3DDEVTYPE_HAL && bAdapterMatchesDesktop && bAdapterMatchesBB ) {

                    best_desktop_display_mode  = desktop_display_mode;
                    best_adapter_info          = adapter_info;
                    best_device_info           = device_info;
                    best_device_combo          = device_combo;

                    if (device_info->device_type == D3DDEVTYPE_HAL && bAdapterMatchesDesktop && bAdapterMatchesBB) {
                        // This fullscreen device combo looks great -- take it
                        goto EndFullscreenDeviceComboSearch;
                    }

                    // Otherwise keep looking for a better fullscreen device combo
                }
            }
        }
    }

EndFullscreenDeviceComboSearch:
    if (best_device_combo == NULL)
    return false;

    // Need to find a display mode on the best adapter that uses best_device_combo->adapter_format
    // and is as close to best_desktop_display_mode's res as possible
    VideoDX9DisplayMode best_display_mode;

    ListIter<VideoDX9DisplayMode> m_iter = best_adapter_info->display_mode_list;
    while (++m_iter) {
        VideoDX9DisplayMode* display_mode = m_iter.value();

        if (display_mode->format != best_device_combo->adapter_format)
        continue;

        if (display_mode->width   == desired_width  && //best_desktop_display_mode.Width &&
                display_mode->height  == desired_height && //best_desktop_display_mode.Height && 
                display_mode->refresh == best_desktop_display_mode.RefreshRate) {

            // found a perfect match, so stop
            best_display_mode = *display_mode;
            break;
        }
        else if (display_mode->width   == desired_width  && //best_desktop_display_mode.Width &&
                display_mode->height  == desired_height && //best_desktop_display_mode.Height && 
                display_mode->refresh >  best_desktop_display_mode.RefreshRate) {
            // refresh rate doesn't match, but width/height match, so keep this
            // and keep looking
            best_display_mode = *display_mode;
        }
        else if (display_mode->width   == desired_width) { //best_desktop_display_mode.Width) {
            // width matches, so keep this and keep looking
            best_display_mode = *display_mode;
        }
        else if (best_display_mode.width == 0)
        {
            // we don't have anything better yet, so keep this and keep looking
            best_display_mode = *display_mode;
        }
    }

    VideoDeviceInfo* fs_device = &vs->fullscreen_device;

    vs->is_windowed = false;
    vs->fullscreen_mode.width        = best_display_mode.width;
    vs->fullscreen_mode.height       = best_display_mode.height;
    vs->fullscreen_mode.refresh      = best_display_mode.refresh;
    vs->fullscreen_mode.format       = best_display_mode.format;

    fs_device->adapter_index         = best_adapter_index;
    fs_device->device_index          = best_device_index;
    fs_device->device_type           = best_device_info->device_type;
    fs_device->back_buffer_format    = best_device_combo->back_buffer_format;
    fs_device->depth_buffer_bits     = GetDepthBits((D3DFORMAT) best_device_combo->depth_stencil_fmt_list[0]);
    fs_device->depth_stencil_format  = best_device_combo->depth_stencil_fmt_list[0];
    fs_device->multisample_type      = best_device_combo->multisample_type_list[0];
    fs_device->multisample_qual      = best_device_combo->multisample_qual_list[0];
    fs_device->vertex_processing     = best_device_combo->vertex_processing_list[0];

    return true;
}


// +--------------------------------------------------------------------+
// +--------------------------------------------------------------------+
// +--------------------------------------------------------------------+

VideoDX9DisplayMode::VideoDX9DisplayMode()
: width(0), height(0), refresh(0), format(D3DFMT_UNKNOWN)
{ }

VideoDX9DisplayMode::VideoDX9DisplayMode(const VideoDX9DisplayMode& m)
: width(m.width), height(m.height), refresh(m.refresh), format(m.format)
{ }

VideoDX9DisplayMode::VideoDX9DisplayMode(const D3DDISPLAYMODE& m)
: width(m.Width), height(m.Height), refresh(m.RefreshRate), format(m.Format)
{ }

int
VideoDX9DisplayMode::operator<(const VideoDX9DisplayMode& m) const
{
    if (width < m.width)
    return 1;

    if (width > m.width)
    return 0;

    if (height < m.height)
    return 1;

    if (height > m.height)
    return 0;

    if (format < m.format)
    return 1;

    if (format > m.format)
    return 0;

    if (refresh < m.refresh)
    return 1;

    return 0;
}

int
VideoDX9DisplayMode::operator<=(const VideoDX9DisplayMode& m) const
{
    // if less than ...
    if (*this < m)
    return 1;

    // ... or equal to ...
    if (width   == m.width  &&
            height  == m.height &&
            format  == m.format &&
            refresh == m.refresh)
    return 1;

    // must be greater than
    return 0;
}

const char*
VideoDX9DisplayMode::GetDescription() const
{
    static char desc[32];

    sprintf_s(desc, "%4d x %4d %-12s %d Hz", //-V576
    width,
    height,
    D3DFormatToString(format),
    refresh);

    return desc;
}

const char*
VideoDX9DisplayMode::D3DFormatToString(D3DFORMAT format)
{
    const char* str = "Unknown Format";

    switch (format) {
    case D3DFMT_UNKNOWN:         str = "UNKNOWN";         break;
    case D3DFMT_R8G8B8:          str = "R8G8B8";          break;
    case D3DFMT_A8R8G8B8:        str = "A8R8G8B8";        break;
    case D3DFMT_X8R8G8B8:        str = "X8R8G8B8";        break;
    case D3DFMT_R5G6B5:          str = "R5G6B5";          break;
    case D3DFMT_X1R5G5B5:        str = "X1R5G5B5";        break;
    case D3DFMT_A1R5G5B5:        str = "A1R5G5B5";        break;
    case D3DFMT_A4R4G4B4:        str = "A4R4G4B4";        break;
    case D3DFMT_R3G3B2:          str = "R3G3B2";          break;
    case D3DFMT_A8:              str = "A8";              break;
    case D3DFMT_A8R3G3B2:        str = "A8R3G3B2";        break;
    case D3DFMT_X4R4G4B4:        str = "X4R4G4B4";        break;
    case D3DFMT_A2B10G10R10:     str = "A2B10G10R10";     break;
    case D3DFMT_A8B8G8R8:        str = "A8B8G8R8";        break;
    case D3DFMT_X8B8G8R8:        str = "X8B8G8R8";        break;
    case D3DFMT_G16R16:          str = "G16R16";          break;
    case D3DFMT_A2R10G10B10:     str = "A2R10G10B10";     break;
    case D3DFMT_A16B16G16R16:    str = "A16B16G16R16";    break;
    case D3DFMT_A8P8:            str = "A8P8";            break;
    case D3DFMT_P8:              str = "P8";              break;
    case D3DFMT_L8:              str = "L8";              break;
    case D3DFMT_A8L8:            str = "A8L8";            break;
    case D3DFMT_A4L4:            str = "A4L4";            break;
    case D3DFMT_V8U8:            str = "V8U8";            break;
    case D3DFMT_L6V5U5:          str = "L6V5U5";          break;
    case D3DFMT_X8L8V8U8:        str = "X8L8V8U8";        break;
    case D3DFMT_Q8W8V8U8:        str = "Q8W8V8U8";        break;
    case D3DFMT_V16U16:          str = "V16U16";          break;
    case D3DFMT_A2W10V10U10:     str = "A2W10V10U10";     break;
    case D3DFMT_UYVY:            str = "UYVY";            break;
    case D3DFMT_YUY2:            str = "YUY2";            break;
    case D3DFMT_DXT1:            str = "DXT1";            break;
    case D3DFMT_DXT2:            str = "DXT2";            break;
    case D3DFMT_DXT3:            str = "DXT3";            break;
    case D3DFMT_DXT4:            str = "DXT4";            break;
    case D3DFMT_DXT5:            str = "DXT5";            break;
    case D3DFMT_D16_LOCKABLE:    str = "D16_LOCKABLE";    break;
    case D3DFMT_D32:             str = "D32";             break;
    case D3DFMT_D15S1:           str = "D15S1";           break;
    case D3DFMT_D24S8:           str = "D24S8";           break;
    case D3DFMT_D24X8:           str = "D24X8";           break;
    case D3DFMT_D24X4S4:         str = "D24X4S4";         break;
    case D3DFMT_D16:             str = "D16";             break;
    case D3DFMT_L16:             str = "L16";             break;
    case D3DFMT_VERTEXDATA:      str = "VERTEXDATA";      break;
    case D3DFMT_INDEX16:         str = "INDEX16";         break;
    case D3DFMT_INDEX32:         str = "INDEX32";         break;
    case D3DFMT_Q16W16V16U16:    str = "Q16W16V16U16";    break;
    case D3DFMT_MULTI2_ARGB8:    str = "MULTI2_ARGB8";    break;
    case D3DFMT_R16F:            str = "R16F";            break;
    case D3DFMT_G16R16F:         str = "G16R16F";         break;
    case D3DFMT_A16B16G16R16F:   str = "A16B16G16R16F";   break;
    case D3DFMT_R32F:            str = "R32F";            break;
    case D3DFMT_G32R32F:         str = "G32R32F";         break;
    case D3DFMT_A32B32G32R32F:   str = "A32B32G32R32F";   break;
    case D3DFMT_CxV8U8:          str = "CxV8U8";          break;
    default:                     str = "Unknown format";  break;
    }

    return str;
}


// +--------------------------------------------------------------------+

VideoDX9AdapterInfo::VideoDX9AdapterInfo()
: adapter_ordinal(0)
{
    ZeroMemory(&adapter_identifier, sizeof(adapter_identifier));
}

VideoDX9AdapterInfo::~VideoDX9AdapterInfo()
{
    display_mode_list.destroy();
    device_info_list.destroy();
}

const char*
VideoDX9AdapterInfo::GetDescription() const
{
    return adapter_identifier.Description;
}

// +--------------------------------------------------------------------+

VideoDX9DeviceInfo::VideoDX9DeviceInfo()
: adapter_ordinal(0), device_type(D3DDEVTYPE_HAL)
{
    ZeroMemory(&caps, sizeof(caps));
}

VideoDX9DeviceInfo::~VideoDX9DeviceInfo()
{
    device_combo_list.destroy();
}

// +--------------------------------------------------------------------+

VideoDX9DeviceCombo::VideoDX9DeviceCombo()
: adapter_ordinal(0), device_type(D3DDEVTYPE_HAL),
adapter_format((D3DFORMAT) 0),
back_buffer_format((D3DFORMAT) 0),
is_windowed(false)
{
}

VideoDX9DeviceCombo::~VideoDX9DeviceCombo()
{
    conflict_list.destroy();
}
