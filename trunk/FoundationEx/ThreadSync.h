/*  Project nGen
    John DiCamillo
    Copyright © 1997-2001. All Rights Reserved.

    SUBSYSTEM:    foundation
    FILE:         ThreadSync.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Declaration of the ThreadSync class
*/

#ifndef ThreadSync_h
#define ThreadSync_h

#include <windows.h>

// +-------------------------------------------------------------------+

class ThreadSync
{
#if defined(_MT)        // MULTITHREADED: WITH SYNC ------------
   CRITICAL_SECTION sync;

public:
   ThreadSync()   { ::InitializeCriticalSection(&sync); }
   ~ThreadSync()  { ::DeleteCriticalSection(&sync);     }
   
   void acquire() { ::EnterCriticalSection(&sync);      }
   void release() { ::LeaveCriticalSection(&sync);      }

#else                   // SINGLE THREADED: NO SYNC ------------

public:
   ThreadSync()   { }
   ~ThreadSync()  { }
   
   void acquire() { }
   void release() { }

#endif
};

// +-------------------------------------------------------------------+

class AutoThreadSync
{
public:
   AutoThreadSync(ThreadSync& s) : sync(s)   { sync.acquire(); }
   ~AutoThreadSync()                         { sync.release(); }
private:
   ThreadSync& sync;
};

#endif ThreadSync_h
