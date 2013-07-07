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
    FILE:         DataLoader.h
    AUTHOR:       John DiCamillo

*/

#ifndef DataLoader_h
#define DataLoader_h

#include "Types.h"
#include "List.h"
#include "Text.h"

// +--------------------------------------------------------------------+

class Bitmap;
class Sound;
class Video;

// +--------------------------------------------------------------------+

class DataLoader
{
public:
    static const char* TYPENAME() { return "DataLoader"; }

    enum { DATAFILE_OK, DATAFILE_INVALID, DATAFILE_NOTEXIST };

    DataLoader();

    static DataLoader*   GetLoader() { return loader; }
    static void          Initialize();
    static void          Close();

    void        Reset();
    void        UseFileSystem(bool use=true);
    void        UseVideo(Video* v);
    void        EnableMedia(bool enable=true);

    int         EnableDatafile(const char* name);
    int         DisableDatafile(const char* name);

    void        SetDataPath(const char* path);
    const char* GetDataPath() const { return datapath; }

    bool  IsFileSystemEnabled() const { return use_file_system; }
    bool  IsMediaLoadEnabled()  const { return enable_media;   }

    bool  FindFile(const char* fname);
    int   ListFiles(const char* filter, List<Text>& list, bool recurse=false);
    int   ListArchiveFiles(const char* archive, const char* filter, List<Text>& list);
    int   LoadBuffer(const char* name, BYTE*&  buf, bool null_terminate=false, bool optional=false);
    int   LoadBitmap(const char* name, Bitmap& bmp, int type=0, bool optional=false);
    int   CacheBitmap(const char* name, Bitmap*& bmp, int type=0, bool optional=false);
    int   LoadTexture(const char* name, Bitmap*& bmp, int type=0, bool preload_cache=false, bool optional=false);
    int   LoadSound(const char* fname, Sound*& snd, DWORD flags=0, bool optional=false);
    int   LoadStream(const char* fname, Sound*& snd, bool optional=false);

    void  ReleaseBuffer(BYTE*& buf);
    int   fread(void* buffer, size_t size, size_t count, BYTE*& stream);

private:
    int   LoadIndexed(const char* name, Bitmap& bmp, int type);
    int   LoadHiColor(const char* name, Bitmap& bmp, int type);
    int   LoadAlpha(  const char* name, Bitmap& bmp, int type);

    void  ListFileSystem(const char* filter, List<Text>& list, Text base_path, bool recurse);
    int   LoadPartialFile(const char* fname, BYTE*& buf, int max_load, bool optional=false);
    int   LoadOggStream(const char* fname, Sound*& snd);

    Text        datapath;
    Video*      video;
    bool        use_file_system;
    bool        enable_media;

    static DataLoader* loader;
};

// +--------------------------------------------------------------------+

#endif DataLoader_h

