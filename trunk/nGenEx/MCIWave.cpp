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

    SUBSYSTEM:    nGenEx.lib
    FILE:         MCIWave.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    MCI Wave Output stuff
*/

#include "MemDebug.h"
#include "Types.h"

// +----------------------------------------------------------------------+

void  Print(const char* fmt, ...);

// +----------------------------------------------------------------------+

const int MCI_MAX_STR = 128;
static char       ret_str[MCI_MAX_STR];
static char       err_str[MCI_MAX_STR];
static MCIERROR   mci_err;
static MMRESULT   wav_err;
extern HWND       hwndApp;

static int mci_send_string(const char* cmd_str)
{
    mci_err = mciSendString(cmd_str, ret_str, sizeof(ret_str), hwndApp);
    if (mci_err) {
        if (mciGetErrorString(mci_err, err_str, sizeof(err_str)))
        Print("Error (%s): '%s'\n", cmd_str, err_str);
        else
        Print("Error (%s): %d - UNKNOWN\n", cmd_str, mci_err);
        return 0;   
    }

    return 1;
}

// +--------------------------------------------------------------------+

static void print_wav_error()
{
    waveOutGetErrorText(wav_err, err_str, MCI_MAX_STR);
    Print(err_str);
}

// +--------------------------------------------------------------------+

int load_wave_file(const char* fname, LPWAVEHDR hdr, LPWAVEFORMATEX format)
{
    HMMIO       hmmio;            /* file handle for open file */ 
    MMCKINFO    mmckinfoParent;   /* parent chunk information structure */ 
    MMCKINFO    mmckinfoSubchunk; /* subchunk information structure    */ 
    DWORD       dwFmtSize;        /* size of "fmt" chunk               */ 
    DWORD       dwDataSize;       /* size of "data" chunk              */ 

    /* 
    * Open the given file for reading with buffered I/O 
    * using the default internal buffer. 
    */ 
    hmmio = mmioOpen((LPSTR) fname, NULL, MMIO_READ | MMIO_ALLOCBUF); 

    if (hmmio == NULL) {
        Print("load_wave_file(): '%s' - Failed to open file.\n", fname); 
        return 0;
    }

    /* 
    * Locate a "RIFF" chunk with a "WAVE" form type 
    * to make sure the file is a WAVE file. 
    */ 
    mmckinfoParent.fccType = mmioFOURCC('W', 'A', 'V', 'E'); 
    if (mmioDescend(hmmio, (LPMMCKINFO) &mmckinfoParent, NULL, MMIO_FINDRIFF)) { 
        Print("load_wave_file(): '%s' - This is not a WAVE file.\n", fname);
        mmioClose(hmmio, 0);
        return 0;
    }

    /* 
    * Find the "fmt " chunk (form type "fmt "); it must be 
    * a subchunk of the "RIFF" parent chunk. 
    */ 
    mmckinfoSubchunk.ckid = mmioFOURCC('f', 'm', 't', ' '); 
    if (mmioDescend(hmmio, &mmckinfoSubchunk, &mmckinfoParent, MMIO_FINDCHUNK)) { 
        Print("load_wave_file(): '%s' - WAVE file has no \"fmt\" chunk\n", fname);
        mmioClose(hmmio, 0);
        return 0;
    }

    /* 
    * Get the size of the "fmt " chunk--allocate and lock memory for it. 
    */ 
    dwFmtSize = mmckinfoSubchunk.cksize; 

    /* Read the "fmt " chunk. */ 
    if (mmioRead(hmmio, (HPSTR) format, dwFmtSize) != (LRESULT)dwFmtSize) {
        Print("load_wave_file(): '%s' - Failed to read format chunk.\n", fname);
        mmioClose(hmmio, 0);
        return 0;
    }

    /* Ascend out of the "fmt " subchunk. */ 
    mmioAscend(hmmio, &mmckinfoSubchunk, 0); 

    /* 
    * Find the data subchunk. The current file position 
    * should be at the beginning of the data chunk. 
    */ 
    mmckinfoSubchunk.ckid = mmioFOURCC('d', 'a', 't', 'a'); 
    if (mmioDescend(hmmio, &mmckinfoSubchunk, &mmckinfoParent, MMIO_FINDCHUNK)) {
        Print("load_wave_file(): '%s' - WAVE file has no data chunk.\n", fname);
        mmioClose(hmmio, 0); 
        return 0; 
    } 

    /* Get the size of the data subchunk. */ 
    dwDataSize = mmckinfoSubchunk.cksize;
    if (dwDataSize == 0L) {
        Print("load_wave_file(): '%s' - The data chunk contains no data.\n", fname);
        mmioClose(hmmio, 0);
        return 0;
    }

    // allocate the data block:
    hdr->lpData = (LPSTR) new(__FILE__,__LINE__) BYTE[dwDataSize];
    hdr->dwBufferLength = dwDataSize;

    /* Read the waveform data subchunk. */
    if (mmioRead(hmmio, (HPSTR) hdr->lpData, dwDataSize) != (LRESULT)dwDataSize) {
        Print("load_wave_file(): '%s' - Failed to read data chunk.\n", fname);
        mmioClose(hmmio, 0);
        return 0;
    }

    /* Close the file. */
    mmioClose(hmmio, 0);

    return 1;
}

// +--------------------------------------------------------------------+

void delete_wave_file(LPWAVEHDR hdr, LPWAVEFORMATEX format)
{
    if (hdr) {
        delete hdr->lpData;
        hdr->lpData = 0;
    }
}
