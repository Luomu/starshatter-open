/*  Project FoundationEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    FoundationEx
    FILE:         MemDebug.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Memory Debugging class
*/


#ifndef MemDebug_h
#define MemDebug_h

// +--------------------------------------------------------------------+

#ifdef WIN32
#include <windows.h>
#include <windowsx.h>
#endif

#ifdef FOUNDATION_USE_MFC

#ifndef _DEBUG

inline void* __cdecl operator new(unsigned int s, const char*, int) { return ::operator new(s);    }
inline void  __cdecl operator delete(void* p, const char*, int)     {        ::operator delete(p); }

#else

void* __cdecl operator new(unsigned int s, const char*, int);
void  __cdecl operator delete(void* p, const char*, int);

#endif

#else

//
// MEMORY DEBUGGING NOT SUPPORTED UNDER MFC
//

// +--------------------------------------------------------------------+

class Memory
{
public:
   enum LEVEL { OFF, LEAKS, PERIODIC, MAXIMAL };

   static void OpenLog(const char* filename=0);
   static void CloseLog();

   static void Check();
   static void Checkpoint();
   static void Stats();
   static void DumpLeaks();

   static void SetLevel(LEVEL l);
};

// +--------------------------------------------------------------------+

#ifndef _DEBUG

inline void* __cdecl operator new(unsigned int s, const char*, int) { return ::operator new(s);    }
inline void  __cdecl operator delete(void* p, const char*, int)     {        ::operator delete(p); }

#else
/*_CRTIMP*/
void* __cdecl operator new(unsigned int, int, const char*, int);

inline void* __cdecl operator new(unsigned int s, const char* f, int l)
        { return ::operator new(s, 1, f, l); }

inline void* __cdecl operator new(unsigned int s)
        { return ::operator new(s, 1, __FILE__, __LINE__); }

inline void  __cdecl operator delete(void* p, const char*, int)
        {        ::operator delete(p); }

#endif _DEBUG

// +--------------------------------------------------------------------+

#endif FOUNDATION_USE_MFC

#endif MemDebug_h

