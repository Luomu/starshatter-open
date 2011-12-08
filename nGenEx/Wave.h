/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    nGenEx.lib
    FILE:         Wave.h
    AUTHOR:       John DiCamillo

*/

#ifndef Wave_h
#define Wave_h

#include "Types.h"

// +--------------------------------------------------------------------+

struct WAVE_HEADER
{
   DWORD RIFF;
   DWORD file_len;
   DWORD WAVE;
};

struct WAVE_FMT
{
   DWORD FMT;
   DWORD chunk_size;
   WORD  wFormatTag; 
   WORD  nChannels; 
   DWORD nSamplesPerSec; 
   DWORD nAvgBytesPerSec; 
   WORD  nBlockAlign; 
   WORD  wBitsPerSample;
};

struct WAVE_FACT
{
   DWORD FACT;
   DWORD chunk_size;
};

struct WAVE_DATA
{
   DWORD DATA;
   DWORD chunk_size;
};

// +--------------------------------------------------------------------+

#endif Wave_h

