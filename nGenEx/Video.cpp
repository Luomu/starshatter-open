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
