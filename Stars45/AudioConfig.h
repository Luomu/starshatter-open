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

    SUBSYSTEM:    Stars.exe
    FILE:         AudioConfig.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Audio Configuration class
*/

#ifndef AudioConfig_h
#define AudioConfig_h

#include "Types.h"

// +--------------------------------------------------------------------+

class AudioConfig
{
public:
    AudioConfig();
    ~AudioConfig();

    static void          Initialize();
    static void          Close();
    static AudioConfig*  GetInstance();

    void     Load();
    void     Save();

    static int  MenuMusic();
    static int  GameMusic();
    static int  EfxVolume();
    static int  GuiVolume();
    static int  WrnVolume();
    static int  VoxVolume();
    static int  Silence();
    static void SetTraining(bool t);

    int      GetMenuMusic() const { return menu_music; }
    int      GetGameMusic() const { return game_music; }
    int      GetEfxVolume() const { return efx_volume; }
    int      GetGuiVolume() const { return gui_volume; }
    int      GetWrnVolume() const { return wrn_volume; }
    int      GetVoxVolume() const { return vox_volume; }

    void     SetMenuMusic(int v);
    void     SetGameMusic(int v);
    void     SetEfxVolume(int v);
    void     SetGuiVolume(int v);
    void     SetWrnVolume(int v);
    void     SetVoxVolume(int v);

protected:
    int      menu_music;
    int      game_music;

    int      efx_volume;
    int      gui_volume;
    int      wrn_volume;
    int      vox_volume;

    bool     training;
};

#endif AudioConfig_h

