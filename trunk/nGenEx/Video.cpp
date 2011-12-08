/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    nGenEx.lib
    FILE:         Video.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Abstract Video Interface (singleton definition)
*/

#include "MemDebug.h"
#include "Video.h"
#include "VideoSettings.h"

// +--------------------------------------------------------------------+

Video* Video::video_instance = 0;

// +--------------------------------------------------------------------+

Video::Video()
{
   status = VIDEO_OK;
   video_instance = this;

   shadow_enabled = true;
   bump_enabled   = true;
   spec_enabled   = true;

   camera         = 0;
}

Video::~Video()
{
   if (video_instance == this)
      video_instance = 0;
}

// +--------------------------------------------------------------------+

bool
Video::IsWindowed() const
{
   const VideoSettings* vs = GetVideoSettings();

   if (vs)
      return vs->IsWindowed();

   return false;
}

bool
Video::IsFullScreen() const
{
   const VideoSettings* vs = GetVideoSettings();

   if (vs)
      return !vs->IsWindowed();

   return true;
}
