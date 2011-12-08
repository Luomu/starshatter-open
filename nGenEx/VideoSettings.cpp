/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    nGenEx.lib
    FILE:         VideoSettings.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Video Settings class implementation
*/

#include "MemDebug.h"
#include "VideoSettings.h"

// +--------------------------------------------------------------------+

VideoSettings::VideoSettings()
{
   fullscreen_mode.width   = 1024;
   fullscreen_mode.height  = 768;
   fullscreen_mode.refresh = 75;
   fullscreen_mode.format  = VideoMode::FMT_X8R8G8B8;

   windowed_mode.width     = 800;
   windowed_mode.height    = 600;
   windowed_mode.refresh   = 0;
   windowed_mode.format    = VideoMode::FMT_NONE;

   window_width            = 800;
   window_height           = 600;

   is_windowed             = false;
   use_effects             = true;
   shadows                 = true;
   bumpmaps                = true;
   specmaps                = true;
   max_detail              = 4;
   enable_vs               = true;
   enable_ps               = true;
   depth_bias              = 0;
}

VideoSettings::~VideoSettings()
{ }

// +--------------------------------------------------------------------+

bool
VideoSettings::IsWindowed() const
{
   return is_windowed;
}

bool
VideoSettings::UseEffects() const
{
   return use_effects;
}

int
VideoSettings::GetWidth() const
{
   if (is_windowed)
      return window_width;

   return fullscreen_mode.width;
}

int
VideoSettings::GetHeight() const
{
   if (is_windowed)
      return window_height;

   return fullscreen_mode.height;
}

int
VideoSettings::GetDepth() const
{
   int fmt = 0;
   int bpp = 0;

   if (is_windowed)
      fmt = windowed_mode.format;
   else
      fmt = fullscreen_mode.format;

   switch (fmt) {
   default:
   case VideoMode::FMT_NONE:       bpp =  0; break;
   case VideoMode::FMT_R5G5B5:     bpp = 15; break;
   case VideoMode::FMT_R5G6B5:     bpp = 16; break;
   case VideoMode::FMT_R8G8B8:     bpp = 24; break;
   case VideoMode::FMT_X8R8G8B8:   bpp = 32; break;
   }

   return bpp;
}

int
VideoSettings::GetPixSize() const
{
   int fmt = 0;
   int pix = 0;

   if (is_windowed)
      fmt = windowed_mode.format;
   else
      fmt = fullscreen_mode.format;

   switch (fmt) {
   default:
   case VideoMode::FMT_NONE:       pix =  0; break;
   case VideoMode::FMT_R5G5B5:     pix =  2; break;
   case VideoMode::FMT_R5G6B5:     pix =  2; break;
   case VideoMode::FMT_R8G8B8:     pix =  3; break;
   case VideoMode::FMT_X8R8G8B8:   pix =  4; break;
   }

   return pix;
}

int
VideoSettings::GetRefreshRate() const
{
   if (is_windowed)
      return windowed_mode.refresh;

   return fullscreen_mode.refresh;
}

// +--------------------------------------------------------------------+

const char*
VideoSettings::GetModeDescription() const
{
   if (is_windowed)
      return windowed_mode.GetDescription();

   return fullscreen_mode.GetDescription();
}

// +--------------------------------------------------------------------+

int
VideoSettings::GetVertexProcessing() const
{
   if (is_windowed)
      return windowed_device.vertex_processing;

   return fullscreen_device.vertex_processing;
}

int
VideoSettings::GetDepthBufferBits() const
{
   if (is_windowed)
      return windowed_device.depth_buffer_bits;

   return fullscreen_device.depth_buffer_bits;
}

int
VideoSettings::GetAdapterIndex() const
{
   if (is_windowed)
      return windowed_device.adapter_index;

   return fullscreen_device.adapter_index;
}

int
VideoSettings::GetDeviceIndex() const
{
   if (is_windowed)
      return windowed_device.device_index;

   return fullscreen_device.device_index;
}

DWORD
VideoSettings::GetDeviceType() const
{
   if (is_windowed)
      return windowed_device.device_type;

   return fullscreen_device.device_type;
}

DWORD
VideoSettings::GetDepthStencilFormat() const
{
   if (is_windowed)
      return windowed_device.depth_stencil_format;

   return fullscreen_device.depth_stencil_format;
}

DWORD
VideoSettings::GetBackBufferFormat() const
{
   if (is_windowed)
      return windowed_device.back_buffer_format;

   return fullscreen_device.back_buffer_format;
}

const char*
VideoSettings::GetAdapterDesc() const
{
   if (is_windowed)
      return windowed_device.adapter_desc;

   return fullscreen_device.adapter_desc;
}

const char*
VideoSettings::GetDeviceDesc() const
{
   if (is_windowed)
      return windowed_device.device_desc;

   return fullscreen_device.device_desc;
}


// +--------------------------------------------------------------------+
// +--------------------------------------------------------------------+
// +--------------------------------------------------------------------+

const char*
VideoMode::GetDescription() const
{
   static char desc[32];

   int bpp = 0;
   switch (format) {
   default:
   case VideoMode::FMT_NONE:       bpp =  0; break;
   case VideoMode::FMT_R5G5B5:     bpp = 15; break;
   case VideoMode::FMT_R5G6B5:     bpp = 16; break;
   case VideoMode::FMT_R8G8B8:     bpp = 24; break;
   case VideoMode::FMT_X8R8G8B8:   bpp = 32; break;
   }

   sprintf_s(desc, "%d x %d x %d", width, height, bpp);
   return desc;
}

// +--------------------------------------------------------------------+
// +--------------------------------------------------------------------+
// +--------------------------------------------------------------------+

VideoDeviceInfo::VideoDeviceInfo()
{
   ZeroMemory(this, sizeof(VideoDeviceInfo));

   vertex_processing = VideoSettings::VTX_HARDWARE;
   depth_buffer_bits = 32;
}

VideoDeviceInfo::~VideoDeviceInfo()
{
}
