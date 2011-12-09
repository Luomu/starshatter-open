/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    nGenEx.lib
    FILE:         Archive.cpp
    AUTHOR:       John DiCamillo

*/

#include "MemDebug.h"
#include "Types.h"
#include "Archive.h"

#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#include <stdio.h>
#include <time.h>

#include "zlib.h"

// +--------------------------------------------------------------------+

void Print(const char* fmt, ...);

int verbose = 1;
int err;

#define CHECK_ERR(err, msg) { \
    if (err != Z_OK) { \
        fprintf(stderr, "%s error: %d\n", msg, err); \
        exit(1); \
    } \
}

// +--------------------------------------------------------------------+

DataArchive::DataArchive(const char* name)
{
   ZeroMemory(this, sizeof(DataArchive));
   
   if (name)
      LoadDatafile(name);
}

DataArchive::~DataArchive()
{
   delete [] block_map;
   delete [] directory;
}

// +--------------------------------------------------------------------+

void DataArchive::WriteEntry(int index, BYTE* buf)
{
   int f = _open(datafile, _O_RDWR|_O_CREAT|_O_BINARY, _S_IREAD|_S_IWRITE);

   if (f != -1) {
      header.dir_size_comp = DirBlocks() * BLOCK_SIZE;
      dirbuf = new(__FILE__,__LINE__) BYTE[header.dir_size_comp];

      if (!dirbuf) {
         err = Z_MEM_ERROR;
      }

      else {
         err = compress(dirbuf, &header.dir_size_comp, 
                        (BYTE*) directory, header.nfiles * sizeof(DataEntry));
         CHECK_ERR(err, "compress");

         header.dir_blocks = Blocks(header.dir_size_comp) * BLOCK_SIZE;

         _lseek(f, 0, SEEK_SET);
         _write(f, &header, sizeof(DataHeader));
         _lseek(f, sizeof(DataHeader) + header.dir_offset, SEEK_SET);
         _write(f, dirbuf, header.dir_blocks);
      
         delete [] dirbuf;
         dirbuf = 0;
      }
      
      if (buf && directory && directory[index].size_comp) {
         _lseek(f, sizeof(DataHeader) + directory[index].offset, SEEK_SET);
         _write(f, buf, directory[index].size_comp);
      }
      _close(f);
   }
   else
      perror("WriteEntry");
}

// +--------------------------------------------------------------------+

DWORD DataArchive::Blocks(DWORD raw_size)
{
   int full_blocks =  raw_size / BLOCK_SIZE;
   int part_blocks = (raw_size % BLOCK_SIZE) > 0;
   
   return full_blocks + part_blocks;
}

DWORD DataArchive::DirBlocks()
{
   DWORD result = Blocks(header.nfiles * sizeof(DataEntry));
   if (result == 0) result = 1;
   return result;
}

DWORD DataArchive::FileBlocks(int index)
{
   if (index >= 0 && index < (int) header.nfiles && directory)
      return Blocks(directory[index].size_comp);

   return 0;
}

// +--------------------------------------------------------------------+

void DataArchive::CreateBlockMap()
{
   delete [] block_map;
   block_map = 0;
   
   if (header.nfiles == 0) return;

   DWORD i,j;
   DWORD dir_usage = header.dir_offset + DirBlocks() * BLOCK_SIZE;
   DWORD max_usage = dir_usage;

   for (i = 0; i < header.nfiles; i++) {
      DWORD last_block = directory[i].offset + FileBlocks(i) * BLOCK_SIZE;
      if (last_block > max_usage)
         max_usage = last_block;
   }

   nblocks   = max_usage/BLOCK_SIZE;
   block_map = new(__FILE__,__LINE__) DWORD[nblocks];

   if (!block_map) {
      nblocks = 0;
   }

   else {
      ZeroMemory(block_map, nblocks*sizeof(DWORD));

      DWORD first_block = header.dir_offset/BLOCK_SIZE + 
                         (header.dir_offset%BLOCK_SIZE > 0);
      
      for (j = 0; j < DirBlocks(); j++)
         block_map[first_block+j] = 1;

      for (i = 0; i < header.nfiles; i++) {
         DWORD first_block = directory[i].offset/BLOCK_SIZE + 
                            (directory[i].offset%BLOCK_SIZE > 0);
      
         for (j = 0; j < FileBlocks(i); j++)
            block_map[first_block+j] = i+2;
      }
   }
}

// +--------------------------------------------------------------------+

int DataArchive::FindDataBlocks(int need)
{
   if ((int) (nblocks)-need > 0) {
      DWORD start;
      int   i;

      for (start = 0; start < nblocks-need; start++) {
         for (i = 0; block_map[start+i] == 0 && i < need; i++);
         
         if (i == need) return start*BLOCK_SIZE;
         
         start += i;
      }
   }

   return nblocks*BLOCK_SIZE;
}

// +--------------------------------------------------------------------+

void DataArchive::LoadDatafile(const char* name)
{
   if (!name) return;

   delete [] directory;
   delete [] block_map;

   ZeroMemory(this, sizeof(DataArchive));
   strncpy_s(datafile, name, NAMELEN-1);

   FILE* f; 
   fopen_s(&f, datafile, "rb");
   if (f) {
      fread(&header, sizeof(DataHeader), 1, f);
      
      if (header.version != VERSION) {
         Print("ERROR: datafile '%s' invalid version '%d'\n",
                  datafile, header.version);
         fclose(f);
         ZeroMemory(&header, sizeof(header));
         return;
      }

      DWORD len      = DirBlocks() * BLOCK_SIZE;
      DWORD dirsize  = header.nfiles + 64;

      dirbuf    = new(__FILE__,__LINE__) BYTE[len];
      directory = new(__FILE__,__LINE__) DataEntry[dirsize];

      if (!dirbuf || !directory) {
         err = Z_MEM_ERROR;
      }

      else {
         ZeroMemory(directory, sizeof(DataEntry) * dirsize);

         fseek(f, sizeof(DataHeader) + header.dir_offset, SEEK_SET);
         fread(dirbuf, header.dir_size_comp, 1, f);

         int err = uncompress((BYTE*) directory, &len,
                              dirbuf, header.dir_size_comp);
         if (err != Z_OK)
            ZeroMemory(directory, sizeof(DataEntry) * dirsize);
      
         delete [] dirbuf;
         dirbuf = 0;

         CreateBlockMap();
      }
   }
   else {
      Print("Creating Archive '%s'...\n", datafile);
      
      header.version       = VERSION;
      header.nfiles        = 0;
      header.dir_blocks    = 0;
      header.dir_size_comp = 0;
      header.dir_offset    = 0;
      
      nblocks = DirBlocks();
      
      delete [] block_map;
      block_map = 0;
   }
}

// +--------------------------------------------------------------------+

int DataArchive::FindEntry(const char* req_name)
{
   int entry = -1;

   if (req_name && *req_name && directory) {
      char path[256];
      int  len = strlen(req_name);

      ZeroMemory(path, sizeof(path));

      for (int c = 0; c < len; c++) {
         if (req_name[c] == '\\')
            path[c] = '/';
         else
            path[c] = req_name[c];
      }

      for (DWORD i = 0; i < header.nfiles; i++) {
         if (!_stricmp(directory[i].name, path))
            return i;
      }
   }
   
   return entry;
}

// +--------------------------------------------------------------------+

BYTE* DataArchive::CompressEntry(int i)
{
   if (directory && i >= 0 && i < (int) header.nfiles) {
      char* name = directory[i].name;

      FILE* f;
	  fopen_s(&f, name, "rb");

      if (f) {
         fseek(f, 0, SEEK_END);
         DWORD len = ftell(f);
         fseek(f, 0, SEEK_SET);

         BYTE* buf = new(__FILE__,__LINE__) BYTE[len];

         if (!buf) {
            err = Z_MEM_ERROR;
         }

         else {
            fread(buf, len, 1, f);
            fclose(f);

            directory[i].size_orig = len;
      
            directory[i].size_comp = (int) (len * 1.1);
            BYTE* cbuf = new(__FILE__,__LINE__) BYTE[directory[i].size_comp];

            if (!cbuf) {
               err = Z_MEM_ERROR;
            }
            else {
               err = compress(cbuf, &directory[i].size_comp, buf, len);
               CHECK_ERR(err, "compress");
            }

            delete [] buf;
            return cbuf;
         }
      }
   }
   
   return 0;
}

// +--------------------------------------------------------------------+

int DataArchive::ExpandEntry(int i, BYTE*& buf, bool null_terminate)
{
   DWORD len = 0;

   if (directory && i >= 0 && i < (int) header.nfiles) {
      FILE* f;
	  fopen_s(&f, datafile, "rb");

      if (f) {
         DWORD clen = directory[i].size_comp;
         BYTE* cbuf = new(__FILE__,__LINE__) BYTE[clen];

         if (!cbuf) {
            err = Z_MEM_ERROR;
         }

         else {
            fseek(f, sizeof(DataHeader) + directory[i].offset, SEEK_SET);
            fread(cbuf, clen, 1, f);

            len = directory[i].size_orig;

            if (null_terminate) {
               buf = new(__FILE__,__LINE__) BYTE[len+1];
               if (buf) buf[len] = 0;
            }

            else {
               buf = new(__FILE__,__LINE__) BYTE[len];
            }

            if (!buf) {
               err = Z_MEM_ERROR;
            }

            else {
               err = uncompress(buf, &len, cbuf, clen);
               if (err != Z_OK) {
                  delete [] buf;
                  buf = 0;
               }
            }

            delete [] cbuf;
            fclose(f);
         }
      }
   }

   return len;
}

// +--------------------------------------------------------------------+

int DataArchive::InsertEntry(const char* name)
{
   if (name && *name) {
      char  path[256];
      DWORD len = strlen(name);

      ZeroMemory(path, sizeof(path));

      for (DWORD c = 0; c < len; c++) {
         if (name[c] == '\\')
            path[c] = '/';
         else
            path[c] = name[c];
      }

      int dirsize = header.nfiles + 64;

      if (directory && dirsize) {
         for (int i = 0; i < dirsize; i++) {
            if (directory[i].size_orig == 0) {
               ZeroMemory(directory[i].name, NAMELEN);
               strncpy_s(directory[i].name, path, NAMELEN);
               directory[i].name[NAMELEN-1] = '\0';
               directory[i].size_orig = 1;
         
               return i;
            }
         }
      }

      DataEntry* dir = new(__FILE__,__LINE__) DataEntry[dirsize + 64];

      if (directory && dirsize) {
         ZeroMemory(dir, (dirsize + 64) * sizeof(DataEntry));
         CopyMemory(dir, directory, dirsize * sizeof(DataEntry));
      }

      delete [] directory;

      header.nfiles = dirsize + 64;
      directory     = dir;

      ZeroMemory(directory[dirsize].name, NAMELEN);
      strncpy_s(directory[dirsize].name, path, NAMELEN);
      directory[dirsize].name[NAMELEN-1] = '\0';
      directory[dirsize].size_orig = 1;

      return dirsize;
   }

   return -1;
}

// +--------------------------------------------------------------------+

void DataArchive::RemoveEntry(int index)
{
   if (directory && index >= 0 && index < (int) header.nfiles)
      ZeroMemory(&directory[index], sizeof(DataEntry));
}

// +--------------------------------------------------------------------+

void DataArchive::Insert(const char* name)
{
   DWORD old_blocks = 0,     old_offset = 0,     new_blocks = 0;
   DWORD old_dir_blocks = 0, old_dir_offset = 0, new_dir_blocks = 0;
   int   added = 0;

   int index = FindEntry(name);
   
   if (index < 0) {
      old_dir_blocks = DirBlocks();
      old_dir_offset = header.dir_offset;
      
      index = InsertEntry(name);
      
      if (index >= (int) header.nfiles) {
         header.nfiles = index+1;
         added = 1;
      }

      new_dir_blocks = DirBlocks();
      
      if (new_dir_blocks > old_dir_blocks) {
         header.dir_offset = FindDataBlocks(new_dir_blocks);
         CreateBlockMap();
      }
   }
   else {
      old_blocks = FileBlocks(index);
      old_offset = directory[index].offset;
   }
   
   if (index >= 0) {
      DataEntry& e = directory[index];
      
      if (verbose) Print("   Inserting: %-16s ", e.name);

      BYTE* buf = CompressEntry(index);
      
      if (!buf) {
         // this is (almost) unrecoverable,
         // so we quit before screwing things up:
         Print("ERROR: Could not compress %d:%s\n", index, directory[index].name);
         exit(1);
      }

      new_blocks = FileBlocks(index);
      
      // the file is new, or got bigger,
      // need to find room for the data:
      if (new_blocks > old_blocks) {
         directory[index].offset = FindDataBlocks(new_blocks);
         CreateBlockMap();
      }

      WriteEntry(index, buf);
      delete [] buf;
      
      if (verbose) {
         int    ratio = (int) (100.0 * (double) e.size_comp / (double) e.size_orig);
         Print("%9d => %9d (%2d%%)\n", e.size_orig, e.size_comp, ratio);
      }
   }
   else if (added)
      header.nfiles--;
}

// +--------------------------------------------------------------------+

void DataArchive::Extract(const char* name)
{
   int index = FindEntry(name);

   if (!directory || index < 0 || index >= (int) header.nfiles) {
      Print("Could not extract '%s', not found\n", name);
      return;
   }

   BYTE* buf;
   ExpandEntry(index, buf);
   
   FILE* f;
   fopen_s(&f, directory[index].name, "wb");
   if (f) {
      fwrite(buf, directory[index].size_orig, 1, f);
      fclose(f);
   }
   else
      Print("Could not extract '%s', could not open file for writing\n", name);

   delete [] buf;

   if (verbose) Print("   Extracted: %s\n", name);
}

// +--------------------------------------------------------------------+

void DataArchive::Remove(const char* name)
{
   int index = FindEntry(name);

   if (!directory || index < 0 || index >= (int) header.nfiles) {
      Print("Could not remove '%s', not found\n", name);
      return;
   }

   RemoveEntry(index);
   WriteEntry(index, 0);

   if (verbose) Print("   Removed: %s\n", name);
}

// +--------------------------------------------------------------------+

void DataArchive::List()
{
   int total_orig = 0;
   int total_comp = 0;

   printf("DATAFILE: %s\n", datafile);
   printf("Files:    %d\n", header.nfiles);
   printf("\n");

   if (directory && header.nfiles) {
      printf("Index  Orig Size  Comp Size  Ratio  Name\n");
      printf("-----  ---------  ---------  -----  ----------------\n");

      for (DWORD i = 0; i < header.nfiles; i++) {
         DataEntry& e = directory[i];
         int    ratio = (int) (100.0 * (double) e.size_comp / (double) e.size_orig);
      
         printf("%5d  %9d  %9d   %2d%%   %s\n", i+1, e.size_orig, e.size_comp, ratio, e.name);
      
         total_orig += e.size_orig;
         total_comp += e.size_comp;
      }

      int total_ratio = (int) (100.0 * (double) total_comp / (double) total_orig);

      printf("-----  ---------  ---------  -----\n");
      printf("TOTAL  %9d  %9d   %2d%%\n\n", total_orig, total_comp, total_ratio);
   }
}


// +--------------------------------------------------------------------+

