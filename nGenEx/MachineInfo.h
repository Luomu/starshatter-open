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
    FILE:         MachineInfo.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Collect and Display Machine, OS, and Driver Information
*/

#ifndef MachineInfo_h
#define MachineInfo_h

#include "Types.h"

// +--------------------------------------------------------------------+

class MachineInfo
{
public:
    enum { CPU_INVALID, CPU_P5=5, CPU_P6=6, CPU_P7=7, CPU_PLUS };
    enum { OS_INVALID,  OS_WIN95, OS_WIN98, OS_WINNT, OS_WIN2K, OS_WINXP, OS_WINXP64, OS_WINVISTA, OS_WINSEVEN, OS_WINFUTURE };
    enum { DX_NONE,     DX_3=3,   DX_5=5,   DX_6=6,   DX_7=7,   DX_8=8, DX_9=9, DX_PLUS };

    static int     GetCpuClass();
    static int     GetCpuSpeed();
    static int     GetTotalRam();
    static int     GetPlatform();
    static int     GetDirectXVersion();

    static void    DescribeMachine();

    static const char* GetShortDescription();
};

// +--------------------------------------------------------------------+

#endif MachineInfo_h