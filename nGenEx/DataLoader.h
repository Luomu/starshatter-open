/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

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

