/*  Project nGen
    John DiCamillo Software Consulting
    Copyright © 1997. All Rights Reserved.

    SUBSYSTEM:    DataFile.exe
    FILE:         Archive.cpp
    AUTHOR:       John DiCamillo

*/

#include <string>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <io.h>
#include <stdio.h>
#include <math.h>
#include <time.h>
#include <windows.h>
#include <windowsx.h>

#include "zlib.h"
#include <mmsystem.h>
#include "Archive.h"

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
}

// +--------------------------------------------------------------------+

void DataArchive::WriteEntry(int index, BYTE* buf)
{
   int f = _open(datafile, _O_RDWR|_O_CREAT|_O_BINARY, _S_IREAD|_S_IWRITE);

   if (f != -1) {
      header.dir_size_comp = DirBlocks() * BLOCK_SIZE;
      dirbuf = new BYTE[header.dir_size_comp];

      err = compress(dirbuf, &header.dir_size_comp, 
                     (BYTE*) directory, header.nfiles * sizeof(DataEntry));
      CHECK_ERR(err, "compress");

      header.dir_blocks = Blocks(header.dir_size_comp) * BLOCK_SIZE;

      _lseek(f, 0, SEEK_SET);
      _write(f, &header, sizeof(DataHeader));
      _lseek(f, sizeof(DataHeader) + header.dir_offset, SEEK_SET);
      _write(f, dirbuf, header.dir_blocks);
      
      delete [] dirbuf;
      
      if (buf && directory[index].size_comp) {
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
   return Blocks(directory[index].size_comp);
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
   block_map = new DWORD[nblocks];
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
   strncpy(datafile, name, NAMELEN-1);
   header.nfiles = 0;

   FILE* f = fopen(datafile, "rb");
   if (f) {
      fread(&header, sizeof(DataHeader), 1, f);
      
      if (header.version != VERSION) {
         printf("ERROR: datafile '%s' invalid version '%d'\n", //-V576
                  datafile, header.version);
         fclose(f);
         exit(-2);
      }

      DWORD len = DirBlocks() * BLOCK_SIZE;
      
      dirbuf = new BYTE[len];
      fseek(f, sizeof(DataHeader) + header.dir_offset, SEEK_SET);
      fread(dirbuf, header.dir_size_comp, 1, f);

      int err = uncompress((BYTE*) directory, &len,
                           dirbuf, header.dir_size_comp);
      if (err != Z_OK)
         ZeroMemory(directory, sizeof(directory));
      
      delete [] dirbuf;
      CreateBlockMap();
   }
   else {
      printf("Creating Archive '%s'...\n", datafile);
      
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

   for (DWORD i = 0; i < header.nfiles; i++)
      if (!_stricmp(directory[i].name, req_name))
         return i;
   
   return entry;
}

// +--------------------------------------------------------------------+

BYTE* DataArchive::CompressEntry(int i)
{
   char* name = directory[i].name;

   FILE* f = fopen(name, "rb");

   if (f) {
      fseek(f, 0, SEEK_END);
      DWORD len = ftell(f);
      fseek(f, 0, SEEK_SET);
      
      BYTE* buf = new BYTE[len];
      
      fread(buf, len, 1, f);
      fclose(f);

      directory[i].size_orig = len;
      
      directory[i].size_comp = (int) (len * 1.1);
      BYTE* cbuf = new BYTE[directory[i].size_comp];

      err = compress(cbuf, &directory[i].size_comp, buf, len);
      CHECK_ERR(err, "compress");
      
      delete [] buf;
      return cbuf;
   }
   
   return 0;
}

// +--------------------------------------------------------------------+

int DataArchive::ExpandEntry(int i, BYTE*& buf)
{
   DWORD len = 0;

   FILE* f = fopen(datafile, "rb");

   if (f) {
      DWORD clen = directory[i].size_comp;
      BYTE* cbuf = new BYTE[clen];
      
      fseek(f, sizeof(DataHeader) + directory[i].offset, SEEK_SET);
      fread(cbuf, clen, 1, f);

      len = directory[i].size_orig;
      buf = new BYTE[len];

      int err = uncompress(buf, &len, cbuf, clen);
      if (err != Z_OK) {
         delete [] buf;
         buf = 0;
      }

      delete [] cbuf;
      fclose(f);
   }

   return len;
}

// +--------------------------------------------------------------------+

int DataArchive::InsertEntry(const char* name)
{
   if (!name) return -1;

   DWORD len = strlen(name);

   for (int i = 0; i < MAX_FILES; i++) {
      if (directory[i].size_orig == 0) {
         strncpy(directory[i].name, name, NAMELEN);
         directory[i].name[NAMELEN-1] = '\0';
         directory[i].size_orig = 1;
         
         return i;
      }
   }

   return -1;
}

// +--------------------------------------------------------------------+

void DataArchive::RemoveEntry(int index)
{
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
      
      if (verbose) printf("   Inserting: %-48s ", e.name);

      BYTE* buf = CompressEntry(index);
      
      if (!buf) {
         // this is (almost) unrecoverable,
         // so we quit before screwing things up:
         printf("ERROR: Could not compress %d:%s\n", index, directory[index].name);
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
         printf("%9d => %9d (%2d%%)\n", e.size_orig, e.size_comp, ratio); //-V576
      }
   }
   else if (added)
      header.nfiles--;
}

// +--------------------------------------------------------------------+

std::wstring ToWideString(const std::string& str)
{
	int stringLength = MultiByteToWideChar(CP_ACP, 0, str.data(), str.length(), 0, 0);
	std::wstring wstr(stringLength, 0);
	MultiByteToWideChar(CP_ACP, 0,  str.data(), str.length(), &wstr[0], stringLength);
	return wstr;
}

void DataArchive::Extract(const char* name)
{
   int index = FindEntry(name);

   if (index < 0) {
      printf("Could not extract '%s', not found\n", name);
      return;
   }

   BYTE* buf;
   ExpandEntry(index, buf);
   
   std::string dirname = directory[index].name;
   bool create_subdir = (dirname.find_first_of('/', 0) != std::string::npos);
   std::wstring wdirname = ToWideString(dirname.substr(0, dirname.find_first_of('/')));
   if (create_subdir)
	  CreateDirectory(wdirname.c_str(), NULL);
   size_t offset = wdirname.length();
   while (dirname.find_first_of('/', offset + 1) != std::string::npos) {
	   wdirname.push_back('/');
	   wdirname += ToWideString(dirname.substr(offset + 1, dirname.find_first_of('/', offset + 1) - offset - 1));
	   CreateDirectory(wdirname.c_str(), NULL);
	   offset = wdirname.length();
   }

   FILE* f = fopen(directory[index].name, "w+b");
   if (f) {
      fwrite(buf, directory[index].size_orig, 1, f);
      fclose(f);
   }
   else
      printf("Could not extract '%s', could not open file for writing\n", name);

   delete [] buf;

   if (verbose) printf("   Extracted: %s\n", name);
}

// +--------------------------------------------------------------------+

void DataArchive::Remove(const char* name)
{
   int index = FindEntry(name);

   if (index < 0) {
      printf("Could not remove '%s', not found\n", name);
      return;
   }

   RemoveEntry(index);
   WriteEntry(index, 0);

   if (verbose) printf("   Removed: %s\n", name);
}

// +--------------------------------------------------------------------+

void DataArchive::List()
{
   int total_orig = 0;
   int total_comp = 0;

   printf("DATAFILE: %s\n", datafile);
   printf("Files:    %d\n", header.nfiles); //-V576
   printf("\n");
   printf("Index  Orig Size  Comp Size  Ratio  Name\n");
   printf("-----  ---------  ---------  -----  ----------------\n");

   for (DWORD i = 0; i < header.nfiles; i++) {
      DataEntry& e = directory[i];
      int    ratio = (int) (100.0 * (double) e.size_comp / (double) e.size_orig);
      
      printf("%5d  %9d  %9d   %2d%%   %s\n", i+1, e.size_orig, e.size_comp, ratio, e.name); //-V576
      
      total_orig += e.size_orig;
      total_comp += e.size_comp;
   }

   int total_ratio = (int) (100.0 * (double) total_comp / (double) total_orig);

   printf("-----  ---------  ---------  -----\n");
   printf("TOTAL  %9d  %9d   %2d%%\n\n", total_orig, total_comp, total_ratio);
}


// +--------------------------------------------------------------------+

