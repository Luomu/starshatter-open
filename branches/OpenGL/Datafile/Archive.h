/*  Project nGen
    John DiCamillo Software Consulting
    Copyright © 1997. All Rights Reserved.

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
