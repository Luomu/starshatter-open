/*  Project nGenEx
	Destroyer Studios LLC
	Copyright © 1997-2004. All Rights Reserved.

	SUBSYSTEM:    nGenEx.lib
	FILE:         Archive.h
	AUTHOR:       John DiCamillo

*/

#ifndef Archive_h
#define Archive_h

// +------------------------------------------------------------------+

#define VERSION      0x0010
#define BLOCK_SIZE   1024
#define FILE_BLOCK   1024
#define NAMELEN      64

// +------------------------------------------------------------------+

struct DataHeader
{
	static const char* TYPENAME() { return "DataHeader"; }

	DWORD    version;
	DWORD    nfiles;
	DWORD    dir_blocks;
	DWORD    dir_size_comp;
	DWORD    dir_offset;
};

struct DataEntry
{
	static const char* TYPENAME() { return "DataEntry"; }

	char     name[NAMELEN];
	DWORD    size_orig;
	DWORD    size_comp;
	DWORD    offset; 
};

class DataArchive
{
public:
	static const char* TYPENAME() { return "DataArchive"; }

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
	int      ExpandEntry(int index, BYTE*& buf, bool null_terminate=false);
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

	const char* Name()   const { return datafile; }

private:
	// persistent data members:
	DataHeader  header;
	DataEntry*  directory;
	BYTE*       dirbuf;

	// transient members:
	char        datafile[NAMELEN];

	DWORD*      block_map;
	DWORD       nblocks;
};

#endif Archive_h
