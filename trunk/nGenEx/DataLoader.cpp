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
    FILE:         DataLoader.cpp
    AUTHOR:       John DiCamillo

*/

#include "MemDebug.h"
#include "DataLoader.h"
#include "Archive.h"
#include "Color.h"
#include "D3DXImage.h"
#include "Bitmap.h"
#include "Bmp.h"
#include "PCX.h"
#include "Sound.h"
#include "Resource.h"
#include "Video.h"
#include "Wave.h"

// +------------------------------------------------------------------+

static DataLoader*   def_loader = 0;
DataLoader*   DataLoader::loader = 0;

static List<DataArchive>   archives;

// +--------------------------------------------------------------------+

DataLoader::DataLoader()
    : datapath(""), video(0), use_file_system(true), enable_media(true)
{ }

// +--------------------------------------------------------------------+

void
DataLoader::UseVideo(Video* v)
{
    video = v;
}

// +--------------------------------------------------------------------+

void
DataLoader::Initialize()
{
    def_loader = new(__FILE__,__LINE__) DataLoader;
    loader = def_loader;

    archives.destroy();
}

void
DataLoader::Close()
{
    archives.destroy();
    Bitmap::ClearCache();

    delete def_loader;
    def_loader = 0;
    loader = 0;
}

void
DataLoader::Reset()
{
    Close();
}

// +--------------------------------------------------------------------+

void
DataLoader::UseFileSystem(bool use)
{
    use_file_system = use;
}

void
DataLoader::EnableMedia(bool enable)
{
    enable_media = enable;
}

// +--------------------------------------------------------------------+

int
DataLoader::EnableDatafile(const char* name)
{
    int status = DATAFILE_NOTEXIST;

    FILE* f;
    fopen_s(&f, name, "rb");

    if (f) {
        ::fclose(f);

        DataArchive* a = new(__FILE__,__LINE__) DataArchive(name);
        
        if (a && a->NumFiles() >= 1) {
            status = DATAFILE_OK;

            bool found = false;
            ListIter<DataArchive> iter = archives;
            while (++iter && !found) {
                DataArchive* archive = iter.value();
                if (!strcmp(archive->Name(), name)) {
                    found = true;
                }
            }

            if (!found)
            archives.append(a);
        }
        else {
            Print("   WARNING: invalid data file '%s'\n", name);
            status = DATAFILE_INVALID;

            delete a;
        }

        loader   = this;
    }
    else {
        Print("   WARNING: could not open datafile '%s'\n", name);
        status = DATAFILE_NOTEXIST;
    }

    return status;
}

int
DataLoader::DisableDatafile(const char* name)
{
    ListIter<DataArchive> iter = archives;
    while (++iter) {
        DataArchive* a = iter.value();
        if (!strcmp(a->Name(), name)) {
            delete iter.removeItem();
            return DATAFILE_OK;
        }
    }

    return DATAFILE_NOTEXIST;
}

// +--------------------------------------------------------------------+

void
DataLoader::SetDataPath(const char* path)
{
    if (path)
    datapath = path;
    else
    datapath = "";
}

// +--------------------------------------------------------------------+

bool
DataLoader::FindFile(const char* name)
{
    // assemble file name:
    char filename[1024];
    strcpy_s(filename, datapath);
    strcat_s(filename, name);

    // first check current directory:
    if (use_file_system) {
        FILE* f;
        ::fopen_s(&f, filename, "rb");

        if (f) {
            ::fclose(f);
            return true;
        }
    }

    // then check datafiles, from last to first:
    int narchives = archives.size();
    for (int i = 0; i < narchives; i++) {
        DataArchive* a = archives[narchives-1-i];
        if (a->FindEntry(filename) > -1) {
            return true;
        }
    }

    return false;
}

// +--------------------------------------------------------------------+

int
DataLoader::ListFiles(const char* filter, List<Text>& list, bool recurse)
{
    list.destroy();

    ListFileSystem(filter, list, datapath, recurse);

    // then check datafile(s):
    int narchives = archives.size();
    for (int i = 0; i < narchives; i++) {
        DataArchive* a = archives[narchives-1-i];
        ListArchiveFiles(a->Name(), filter, list);
    }

    return list.size();
}

int
DataLoader::ListArchiveFiles(const char* archive_name, const char* filter, List<Text> &list)
{
    int            pathlen  = datapath.length();
    DataArchive*   a        = 0;

    if (archive_name) {
        int narchives = archives.size();
        for (int i = 0; i < narchives && !a; i++) {
            a = archives[narchives-1-i];

            if (_stricmp(a->Name(), archive_name))
            a = 0;
        }
    }

    if (!a) {
        ListFileSystem(filter, list, datapath, true);
        return list.size();
    }

    if (!strcmp(filter, "*.*")) {
        int count = a->NumFiles();
        for (int n = 0; n < count; n++) {
            DataEntry* entry = a->GetFile(n);
            Text entry_name = entry->name;
            entry_name.setSensitive(false);

            if (entry_name.contains(datapath)) {
                Text fname = entry_name(pathlen, 1000);

                if (!list.contains(&fname))
                list.append(new Text(fname));
            }
        }
    }
    else {
        char data_filter[256];
        ZeroMemory(data_filter, 256);

        const char* pf  = filter;
        char*       pdf = data_filter;

        while (*pf) {
            if (*pf != '*')
            *pdf++ = *pf;
            pf++;
        }

        int count = a->NumFiles();
        for (int n = 0; n < count; n++) {
            DataEntry* entry = a->GetFile(n);
            Text entry_name = entry->name;
            entry_name.setSensitive(false);

            if (entry_name.contains(datapath) && entry_name.contains(data_filter)) {
                Text fname = entry_name(pathlen, 1000);

                if (!list.contains(&fname))
                list.append(new Text(fname));
            }
        }
    }

    return list.size();
}

// +--------------------------------------------------------------------+

void
DataLoader::ListFileSystem(const char* filter, List<Text>& list, Text base_path, bool recurse)
{
    if (use_file_system) {
        char data_filter[256];
        ZeroMemory(data_filter, 256);

        const char* pf  = filter;
        char*       pdf = data_filter;

        while (*pf) {
            if (*pf != '*')
            *pdf++ = *pf;
            pf++;
        }

        int pathlen = base_path.length();

        // assemble win32 find filter:
        char win32_filter[1024];
        strcpy_s(win32_filter, datapath);

        if (recurse)
        strcat_s(win32_filter, "*.*");
        else
        strcat_s(win32_filter, filter);

        // first check current directory:
        WIN32_FIND_DATA data;
        HANDLE hFind = FindFirstFile(win32_filter, &data);

        if (hFind != INVALID_HANDLE_VALUE) {
            do {
                if ((data.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) != 0) {
                    if (recurse && data.cFileName[0] != '.') {
                        Text old_datapath = datapath;

                        Text newpath = datapath;
                        newpath += data.cFileName;
                        newpath += "/";
                        datapath = newpath;

                        ListFileSystem(filter, list, base_path, recurse);

                        datapath = old_datapath;
                    }
                }
                else {
                    if (!strcmp(filter, "*.*") || strstr(data.cFileName, data_filter)) {
                        Text full_name = datapath;
                        full_name += data.cFileName;

                        list.append(new Text(full_name(pathlen, 1000)));
                    }
                }
            }
            while (FindNextFile(hFind, &data));
        }
    }
}

// +--------------------------------------------------------------------+

int
DataLoader::LoadBuffer(const char* name, BYTE*& buf, bool null_terminate, bool optional)
{
    buf = 0;

    // assemble file name:
    char filename[1024];
    strcpy_s(filename, datapath);
    strcat_s(filename, name);

    if (use_file_system) {
        // first check current directory:
        FILE* f;
        ::fopen_s(&f, filename, "rb");

        if (f) {
            ::fseek(f, 0, SEEK_END);
            int len = ftell(f);
            ::fseek(f, 0, SEEK_SET);

            if (null_terminate) {
                buf = new(__FILE__,__LINE__) BYTE[len+1];
                if (buf)
                buf[len] = 0;
            }

            else {
                buf = new(__FILE__,__LINE__) BYTE[len];
            }

            if (buf)
            ::fread(buf, len, 1, f);

            ::fclose(f);

            return len;
        }
    }

    // then check datafile(s):
    int narchives = archives.size();

    // vox files are usually in their own archive,
    // so check there first
    if (narchives > 1 && strstr(filename, "Vox")) {
        for (int i = 0; i < narchives; i++) {
            DataArchive* a = archives[narchives-1-i];
            if (strstr(a->Name(), "vox")) {
                int index = a->FindEntry(filename);
                if (index > -1)
                return a->ExpandEntry(index, buf, null_terminate);
            }
        }
    }

    for (int i = 0; i < narchives; i++) {
        DataArchive* a = archives[narchives-1-i];
        int index = a->FindEntry(filename);
        if (index > -1)
        return a->ExpandEntry(index, buf, null_terminate);
    }

    if (!optional)
    Print("WARNING - DataLoader could not load buffer '%s'\n", filename);
    return 0;
}

// +--------------------------------------------------------------------+

int
DataLoader::LoadPartialFile(const char* name, BYTE*& buf, int max_load, bool optional)
{
    buf = 0;

    // assemble file name:
    char filename[1024];
    strcpy_s(filename, datapath);
    strcat_s(filename, name);

    // first check current directory:
    FILE* f; 
    ::fopen_s(&f, filename, "rb");

    if (f) {
        ::fseek(f, 0, SEEK_END);
        int len = ftell(f);
        ::fseek(f, 0, SEEK_SET);

        if (len > max_load) {
            len = max_load;
        }

        buf = new(__FILE__,__LINE__) BYTE[len];

        if (buf)
        ::fread(buf, len, 1, f);

        ::fclose(f);

        return len;
    }

    if (!optional)
    Print("WARNING - DataLoader could not load partial file '%s'\n", filename);
    return 0;
}

int
DataLoader::fread(void* buffer, size_t size, size_t count, BYTE*& stream)
{
    CopyMemory(buffer, stream, size*count);
    stream += size*count;

    return size*count;
}

// +--------------------------------------------------------------------+

void
DataLoader::ReleaseBuffer(BYTE*& buf)
{
    delete [] buf;
    buf = 0;
}

// +--------------------------------------------------------------------+

int
DataLoader::CacheBitmap(const char* name, Bitmap*& bitmap, int type, bool optional)
{
    int      result = 0;

    // search cache:
    bitmap = Bitmap::CheckCache(name);
    if (bitmap) return 1;

    // not in cache yet:
    bitmap = new(__FILE__,__LINE__) Bitmap;

    if (bitmap)
    result = LoadBitmap(name, *bitmap, type, optional);

    if (result && bitmap)
    Bitmap::AddToCache(bitmap);

    return result;
}

// +--------------------------------------------------------------------+

int
DataLoader::LoadBitmap(const char* name, Bitmap& bitmap, int type, bool optional)
{
    if (!enable_media)
    return 0;

    int result = LoadIndexed(name, bitmap, type);

    // check for a matching high color bitmap:
    if (result == 1) {
        int hi_result = LoadHiColor(name, bitmap, type);

        if (hi_result == 2)
        result = 3;
    }

    bitmap.SetFilename(name);

    if (!result && !optional)
    Print("WARNING - DataLoader could not load bitmap '%s%s'\n", datapath.data(), name);

    return result;
}

// +--------------------------------------------------------------------+

int
DataLoader::LoadTexture(const char* name, Bitmap*& bitmap, int type, bool preload_cache, bool optional)
{
    if (!enable_media)
    return 0;

    int      result = 0;

    // assemble file name:
    char filename[256];
    strcpy_s(filename, datapath);
    strcat_s(filename, name);

    // search cache:
    bitmap = Bitmap::CheckCache(filename);
    if (bitmap) return 1;

    // not in cache yet:
    bitmap = new(__FILE__,__LINE__) Bitmap;

    if (bitmap) {
        result = LoadHiColor(name, *bitmap, type);

        if (!result) {
            result = LoadIndexed(name, *bitmap, type);
        }

        bitmap->SetFilename(filename);

        if (result) {
            bitmap->MakeTexture();
            Bitmap::AddToCache(bitmap);
        }
        else {
            delete bitmap;
            bitmap = 0;

            if (!optional)
            Print("WARNING - DataLoader could not load texture '%s%s'\n", datapath.data(), name);
        }
    }
    else if (!optional) {
        Print("WARNING - DataLoader could not allocate texture '%s%s'\n", datapath.data(), name);
    }

    return result;
}

// +--------------------------------------------------------------------+

int
DataLoader::LoadIndexed(const char* name, Bitmap& bitmap, int type)
{
    if (!enable_media)
    return 0;

    int         result = 0;
    PcxImage    pcx;
    D3DXImage   d3dx;
    bool        pcx_file = strstr(name, ".pcx") || strstr(name, ".PCX");

    // assemble file name:
    char filename[256];
    strcpy_s(filename, datapath);
    strcat_s(filename, name);

    // first try to load from current directory:
    bool loaded = false;

    if (use_file_system) {
        if (pcx_file)
        loaded = pcx.Load(filename) == PCX_OK;

        else
        loaded = d3dx.Load(filename);
    }

    if (!loaded) {
        // then check datafile:
        int   len     = 0;
        BYTE* tmp_buf = 0;

        int narchives = archives.size();
        for (int i = 0; i < narchives; i++) {
            DataArchive* a = archives[narchives-1-i];
            int index = a->FindEntry(filename);
            if (index > -1) {
                len = a->ExpandEntry(index, tmp_buf);

                if (pcx_file)
                pcx.LoadBuffer(tmp_buf, len);

                else
                d3dx.LoadBuffer(tmp_buf, len);

                ReleaseBuffer(tmp_buf);
                break;
            }
        }
    }

    // now copy the image into the bitmap:
    if (pcx_file) {
        if (pcx.bitmap) {
            bitmap.CopyImage(pcx.width, pcx.height, pcx.bitmap, type);
            result = 1;
        }

        else if (pcx.himap) {
            bitmap.CopyHighColorImage(pcx.width, pcx.height, pcx.himap, type);
            result = 2;
        }

        if (result == 2)
        LoadAlpha(name, bitmap, type);
    }

    else {
        if (d3dx.image) {
            bitmap.CopyHighColorImage(d3dx.width, d3dx.height, d3dx.image, type);
            result = 2;
        }

        if (result == 2) {
            LoadAlpha(name, bitmap, type);
        }
    }

    return result;
}

int
DataLoader::LoadHiColor(const char* name, Bitmap& bitmap, int type)
{
    if (!enable_media)
    return 0;

    int         result = 0;
    PcxImage    pcx;
    D3DXImage   d3dx;
    bool        pcx_file = strstr(name, ".pcx") || strstr(name, ".PCX");
    bool        bmp_file = strstr(name, ".bmp") || strstr(name, ".BMP");
    bool        png_file = strstr(name, ".png") || strstr(name, ".PNG");

    // check for a matching high color bitmap:
    char filename[256];
    char name2[256];
    strcpy_s(name2, name);

    char* dot = strrchr(name2, '.');
    if (dot && pcx_file)
    strcpy(dot, "+.pcx");
    else if (dot && bmp_file)
    strcpy(dot, "+.bmp");
    else if (dot && png_file)
    strcpy(dot, "+.png");
    else
    return result;

    strcpy_s(filename, datapath);
    strcat_s(filename, name2);

    // first try to load from current directory:
    bool loaded = false;

    if (use_file_system) {
        if (pcx_file)
        loaded = pcx.Load(filename) == PCX_OK;

        else
        loaded = d3dx.Load(filename);
    }

    if (!loaded) {
        // then check datafile:
        int   len     = 0;
        BYTE* tmp_buf = 0;

        int narchives = archives.size();
        for (int i = 0; i < narchives; i++) {
            DataArchive* a = archives[narchives-1-i];
            int index = a->FindEntry(filename);
            if (index > -1) {
                len = a->ExpandEntry(index, tmp_buf);

                if (pcx_file)
                pcx.LoadBuffer(tmp_buf, len);
                else
                d3dx.LoadBuffer(tmp_buf, len);

                ReleaseBuffer(tmp_buf);
                break;
            }
        }
    }

    // now copy the image into the bitmap:
    if (pcx_file && pcx.himap) {
        bitmap.CopyHighColorImage(pcx.width, pcx.height, pcx.himap, type);
        result = 2;
    }

    else if (d3dx.image) {
        bitmap.CopyHighColorImage(d3dx.width, d3dx.height, d3dx.image, type);
        result = 2;
    }

    if (result == 2)
    LoadAlpha(name, bitmap, type);

    return result;
}

int
DataLoader::LoadAlpha(const char* name, Bitmap& bitmap, int type)
{
    if (!enable_media)
    return 0;

    PcxImage    pcx;
    D3DXImage   d3dx;
    bool        pcx_file = strstr(name, ".pcx") || strstr(name, ".PCX");
    bool        bmp_file = strstr(name, ".bmp") || strstr(name, ".BMP");
    bool        png_file = strstr(name, ".png") || strstr(name, ".PNG");
    bool        tga_file = strstr(name, ".tga") || strstr(name, ".TGA");

    // check for an associated alpha-only (grayscale) bitmap:
    char filename[256];
    char name2[256];
    strcpy_s(name2, name);
    char* dot = strrchr(name2, '.');
    if (dot && pcx_file)
    strcpy(dot, "@.pcx");
    else if (dot && bmp_file)
    strcpy(dot, "@.bmp");
    else if (dot && png_file)
    strcpy(dot, "@.png");
    else if (dot && tga_file)
    strcpy(dot, "@.tga");
    else
    return 0;

    strcpy_s(filename, datapath);
    strcat_s(filename, name2);

    // first try to load from current directory:
    bool loaded = false;

    if (use_file_system) {
        if (pcx_file)
        loaded = pcx.Load(filename) == PCX_OK;

        else
        loaded = d3dx.Load(filename);
    }

    if (!loaded) {
        // then check datafile:
        int   len     = 0;
        BYTE* tmp_buf = 0;

        int narchives = archives.size();
        for (int i = 0; i < narchives; i++) {
            DataArchive* a = archives[narchives-1-i];
            int index = a->FindEntry(filename);
            if (index > -1) {
                len = a->ExpandEntry(index, tmp_buf);

                if (pcx_file)
                pcx.LoadBuffer(tmp_buf, len);
                else
                d3dx.LoadBuffer(tmp_buf, len);

                ReleaseBuffer(tmp_buf);
                break;
            }
        }
    }

    // now copy the alpha values into the bitmap:
    if (pcx_file && pcx.bitmap) {
        bitmap.CopyAlphaImage(pcx.width, pcx.height, pcx.bitmap);
    }
    else if (pcx_file && pcx.himap) {
        bitmap.CopyAlphaRedChannel(pcx.width, pcx.height, pcx.himap);
    }
    else if (d3dx.image) {
        bitmap.CopyAlphaRedChannel(d3dx.width, d3dx.height, d3dx.image);
    }

    return 0;
}

// +--------------------------------------------------------------------+

int
DataLoader::LoadSound(const char* name, Sound*& snd, DWORD flags, bool optional)
{
    if (!enable_media)
    return 0;

    if (strstr(name, ".ogg"))
    return LoadStream(name, snd, optional);

    int result = 0;

    WAVE_HEADER    head;
    WAVE_FMT       fmt;
    WAVE_FACT      fact;
    WAVE_DATA      data;
    WAVEFORMATEX   wfex;
    LPBYTE         wave;

    LPBYTE         buf;
    LPBYTE         p;
    int            len;

    ZeroMemory(&head, sizeof(head));
    ZeroMemory(&fmt,  sizeof(fmt));
    ZeroMemory(&fact, sizeof(fact));
    ZeroMemory(&data, sizeof(data));

    len = LoadBuffer(name, buf, false, optional);

    if (len > sizeof(head)) {
        CopyMemory(&head, buf, sizeof(head));

        if (head.RIFF == MAKEFOURCC('R', 'I', 'F', 'F') &&
                head.WAVE == MAKEFOURCC('W', 'A', 'V', 'E')) {

            p = buf + sizeof(WAVE_HEADER);

            do {
                DWORD chunk_id = *((LPDWORD) p);

                switch (chunk_id) {
                case MAKEFOURCC('f', 'm', 't', ' '):
                    CopyMemory(&fmt, p, sizeof(fmt));
                    p += fmt.chunk_size + 8;
                    break;

                case MAKEFOURCC('f', 'a', 'c', 't'):
                    CopyMemory(&fact, p, sizeof(fact));
                    p += fact.chunk_size + 8;
                    break;

                case MAKEFOURCC('s', 'm', 'p', 'l'):
                    CopyMemory(&fact, p, sizeof(fact));
                    p += fact.chunk_size + 8;
                    break;

                case MAKEFOURCC('d', 'a', 't', 'a'):
                    CopyMemory(&data, p, sizeof(data));
                    p += 8;
                    break;

                default:
                    ReleaseBuffer(buf);
                    return result;
                }
            }
            while (data.chunk_size == 0);

            wfex.wFormatTag      = fmt.wFormatTag;
            wfex.nChannels       = fmt.nChannels;
            wfex.nSamplesPerSec  = fmt.nSamplesPerSec;
            wfex.nAvgBytesPerSec = fmt.nAvgBytesPerSec;
            wfex.nBlockAlign     = fmt.nBlockAlign;
            wfex.wBitsPerSample  = fmt.wBitsPerSample;
            wfex.cbSize          = 0;
            wave                 = p;

            snd = Sound::Create(flags, &wfex, data.chunk_size, wave);

            if (snd)
            result = data.chunk_size;
        }
    }

    ReleaseBuffer(buf);
    return result;
}

int
DataLoader::LoadStream(const char* name, Sound*& snd, bool optional)
{
    if (!enable_media)
    return 0;

    if (!name)
    return 0;

    int namelen = strlen(name);

    if (namelen < 5)
    return 0;

    if ((name[namelen-3] == 'o' || name[namelen-3] == 'O') &&
            (name[namelen-2] == 'g' || name[namelen-2] == 'G') &&
            (name[namelen-1] == 'g' || name[namelen-1] == 'G')) {

        return LoadOggStream(name, snd);
    }

    int result = 0;

    WAVE_HEADER    head;
    WAVE_FMT       fmt;
    WAVE_FACT      fact;
    WAVE_DATA      data;
    WAVEFORMATEX   wfex;

    LPBYTE         buf;
    LPBYTE         p;
    int            len;

    ZeroMemory(&head, sizeof(head));
    ZeroMemory(&fmt,  sizeof(fmt));
    ZeroMemory(&fact, sizeof(fact));
    ZeroMemory(&data, sizeof(data));

    len = LoadPartialFile(name, buf, 4096, optional);

    if (len > sizeof(head)) {
        CopyMemory(&head, buf, sizeof(head));

        if (head.RIFF == MAKEFOURCC('R', 'I', 'F', 'F') &&
                head.WAVE == MAKEFOURCC('W', 'A', 'V', 'E')) {

            p = buf + sizeof(WAVE_HEADER);

            do {
                DWORD chunk_id = *((LPDWORD) p);

                switch (chunk_id) {
                case MAKEFOURCC('f', 'm', 't', ' '):
                    CopyMemory(&fmt, p, sizeof(fmt));
                    p += fmt.chunk_size + 8;
                    break;

                case MAKEFOURCC('f', 'a', 'c', 't'):
                    CopyMemory(&fact, p, sizeof(fact));
                    p += fact.chunk_size + 8;
                    break;

                case MAKEFOURCC('s', 'm', 'p', 'l'):
                    CopyMemory(&fact, p, sizeof(fact));
                    p += fact.chunk_size + 8;
                    break;

                case MAKEFOURCC('d', 'a', 't', 'a'):
                    CopyMemory(&data, p, sizeof(data));
                    p += 8;
                    break;

                default:
                    ReleaseBuffer(buf);
                    return result;
                }
            }
            while (data.chunk_size == 0);

            wfex.wFormatTag      = fmt.wFormatTag;
            wfex.nChannels       = fmt.nChannels;
            wfex.nSamplesPerSec  = fmt.nSamplesPerSec;
            wfex.nAvgBytesPerSec = fmt.nAvgBytesPerSec;
            wfex.nBlockAlign     = fmt.nBlockAlign;
            wfex.wBitsPerSample  = fmt.wBitsPerSample;
            wfex.cbSize          = 0;

            snd = Sound::Create(Sound::STREAMED, &wfex);

            if (snd) {
                // assemble file name:
                char filename[1024];
                strcpy_s(filename, datapath);
                strcat_s(filename, name);

                snd->StreamFile(filename, p - buf);

                result = data.chunk_size;
            }
        }
    }

    ReleaseBuffer(buf);
    return result;
}

int
DataLoader::LoadOggStream(const char* name, Sound*& snd)
{
    if (!enable_media)
    return 0;

    snd = Sound::CreateOggStream(name);

    return snd != 0;
}
