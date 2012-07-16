/*  Project nGen
    John DiCamillo
    Copyright © 1997-2001. All Rights Reserved.

    SUBSYSTEM:    foundation
    FILE:         MemDebug.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Memory Debugging class
*/

#include "MemDebug.h"

#include <stdio.h>
#include <string.h>
#include <crtdbg.h>
#include <malloc.h>

// +--------------------------------------------------------------------+

static Memory::LEVEL mem_chk_level = Memory::PERIODIC;

#ifdef _DEBUG
static _CrtMemState  mem_chk_p1,
                     mem_chk_p2;
#endif

static HANDLE        mem_log_file = 0;

// +--------------------------------------------------------------------+

#ifdef _DEBUG
#define CrtSetDebugField(a) _CrtSetDbgFlag((a)  | _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG))
#define CrtClrDebugField(a) _CrtSetDbgFlag(~(a) & _CrtSetDbgFlag(_CRTDBG_REPORT_FLAG))
#endif

static void heapdump()
{
   _HEAPINFO   hinfo;
   int         heapstatus;
   DWORD       used  = 0;
   DWORD       avail = 0;
   char        report[256];

   hinfo._pentry = NULL;
   while ((heapstatus = _heapwalk( &hinfo )) == _HEAPOK) { 
      sprintf_s(report, "%6s block at %Fp of size %4.4X\n",
               ( hinfo._useflag == _USEDENTRY ? "USED" : "FREE" ),
                 hinfo._pentry, hinfo._size);

      _RPT0(_CRT_WARN, report);

      if (hinfo._useflag == _USEDENTRY)
         used  += hinfo._size;
      else
         avail += hinfo._size;
   }

   sprintf_s(report, "------\nUsed Blocks:  %d\nAvail Blocks: %d\nTotal Blocks: %d\n", used, avail, used+avail); //-V576
   _RPT0(_CRT_WARN, report);

   switch (heapstatus) {
   case _HEAPEMPTY:
      _RPT0(_CRT_WARN,  "OK - empty heap\n" );
      break;
   case _HEAPEND:
      _RPT0(_CRT_WARN,  "OK - end of heap\n" );
      break;
   case _HEAPBADPTR:
      _RPT0(_CRT_WARN,  "ERROR - bad pointer to heap\n" );
      break;
   case _HEAPBADBEGIN:
      _RPT0(_CRT_WARN,  "ERROR - bad start of heap\n" );
      break;
   case _HEAPBADNODE:
      _RPT0(_CRT_WARN,  "ERROR - bad node in heap\n" );
      break;
   }
}

// +--------------------------------------------------------------------+

void
Memory::OpenLog(const char* filename)
{
#ifdef _DEBUG
   if (!filename || !strlen(filename))
      filename = "memdbg.txt";

   mem_log_file = CreateFile(filename, GENERIC_WRITE, 0, 0, CREATE_ALWAYS, 0, 0);
   
   if (mem_log_file) {
      _CrtSetReportMode(_CRT_WARN,   _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);
      _CrtSetReportFile(_CRT_WARN,   mem_log_file);
      _CrtSetReportMode(_CRT_ERROR,  _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);
      _CrtSetReportFile(_CRT_ERROR,  mem_log_file);
      _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE | _CRTDBG_MODE_DEBUG);
      _CrtSetReportFile(_CRT_ASSERT, mem_log_file);
   }

   _CrtMemCheckpoint(&mem_chk_p1);
#endif
}

// +--------------------------------------------------------------------+

void
Memory::CloseLog()
{
#ifdef _DEBUG
   if (mem_log_file) {
      CloseHandle(mem_log_file);
      mem_log_file = 0;

      _CrtSetReportMode(_CRT_WARN,   _CRTDBG_MODE_FILE);
      _CrtSetReportFile(_CRT_WARN,   _CRTDBG_FILE_STDOUT);
      _CrtSetReportMode(_CRT_ERROR,  _CRTDBG_MODE_FILE);
      _CrtSetReportFile(_CRT_ERROR,  _CRTDBG_FILE_STDOUT);
      _CrtSetReportMode(_CRT_ASSERT, _CRTDBG_MODE_FILE);
      _CrtSetReportFile(_CRT_ASSERT, _CRTDBG_FILE_STDOUT);
   }
#endif   
}

// +--------------------------------------------------------------------+

void
Memory::Check()
{
#ifdef _DEBUG
   if (! _CrtCheckMemory()) {
      _RPT0(_CRT_ERROR, "\n\nMemory Check Failed.\n");
      heapdump();
      Checkpoint();
      _asm { int 3 }
      exit(1111);
   }
#endif   
}

// +--------------------------------------------------------------------+

void
Memory::Checkpoint()
{
#ifdef _DEBUG
   if (mem_chk_level < PERIODIC) return;

   _RPT0(_CRT_WARN, "\n\nMemory Checkpoint:\n"
         "--------------------------------------------------\n");

   _CrtMemState s;
   _CrtMemCheckpoint(&mem_chk_p2);
   _CrtMemDifference(&s, &mem_chk_p1, &mem_chk_p2);
   _CrtMemDumpStatistics(&s);
   
   memcpy(&mem_chk_p1, &mem_chk_p2, sizeof(mem_chk_p1));
#endif   
}

// +--------------------------------------------------------------------+

void
Memory::Stats()
{
#ifdef _DEBUG
   if (mem_chk_level < PERIODIC) return;

   _RPT0(_CRT_WARN, "\n\nMemory Stats:\n"
         "--------------------------------------------------\n");

   _CrtMemState s;
   _CrtMemCheckpoint(&s);
   _CrtMemDumpStatistics(&s);
#endif   
}

// +--------------------------------------------------------------------+

void
Memory::DumpLeaks()
{
#ifdef _DEBUG
   _RPT0(_CRT_WARN, "\n\nMemory Dump Leaks:\n"
         "--------------------------------------------------\n");
   _CrtDumpMemoryLeaks();
#endif   
}

// +--------------------------------------------------------------------+

void
Memory::SetLevel(LEVEL l)
{
#ifdef _DEBUG
   mem_chk_level = l;
   
   _CrtSetDbgFlag(0);
   
   switch (mem_chk_level) {
   case MAXIMAL:  CrtSetDebugField(_CRTDBG_CHECK_ALWAYS_DF);
   case PERIODIC: CrtSetDebugField(_CRTDBG_DELAY_FREE_MEM_DF);
   case LEAKS:    CrtSetDebugField(_CRTDBG_LEAK_CHECK_DF);
   case OFF:      
   default:       break;
   }
#endif   
}

