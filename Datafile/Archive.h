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

    SUBSYSTEM:    DataFile.exe
    FILE:         Archive.hpp
    AUTHOR:       John DiCamillo

*/

#ifndef ARCHIVE_HPP
#define ARCHIVE_HPP

// +------------------------------------------------------------------+

#define VERSION      0x0010
#define BLOCK_SIZE   1024
#define MAX_FILES    8192
#define FILE_BLOCK   1024
#define NAMELEN      64

// +------------------------------------------------------------------+

struct DataHeader
{
   DWORD    version;
   DWORD    nfiles;
   DWORD    dir_blocks;
   DWORD    dir_size_comp;
   DWORD    dir_offset;
};

struct DataEntry
{
   char     name[NAMELEN];
   DWORD    size_orig;
   DWORD    size_comp;
   DWORD    offset; 
};

class DataArchive
{
public:
   // ctor:
   DataArchive(const char* name = 0);
   ~DataArchive();

   // operations:
   void     LoadDatafile(const char* name);
   void     Insert(const char* name);
   void     Extract(const char* name);
   void     Remove(const char* name);
   void     List();
   
   void     WriteEntry(int index, BYTE* buf);
   int      FindEntry(const char* req_name);
   int      ExpandEntry(int index, BYTE*& buf);
   BYTE*    CompressEntry(int index);
   int      InsertEntry(const char* name);
   void     RemoveEntry(int index);
   DWORD    Blocks(DWORD raw_size);
   DWORD    DirBlocks();
   DWORD    FileBlocks(int index);
   int      FindDataBlocks(int blocks_needed);
   void     CreateBlockMap();
   
   DWORD    NumFiles() { return header.nfiles; }
   DataEntry* GetFile(int i) { if (i>=0 && i<(int)header.nfiles) return &directory[i]; return 0; }

private:
   // persistent data members:
   DataHeader  header;
   DataEntry   directory[MAX_FILES];
   BYTE*       dirbuf;

   // transient members:
   char        datafile[NAMELEN];
   
   DWORD*      block_map;
   DWORD       nblocks;
};

extern std::wstring ToWideString(const std::string& str);


#endif
