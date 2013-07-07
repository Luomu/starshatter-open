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

