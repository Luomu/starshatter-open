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
    FILE:         Encrypt.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Simple Encryption / Decryption class
*/


#include "MemDebug.h"
#include "Encrypt.h"

// +--------------------------------------------------------------------+

void Print(const char* fmt, ...);

static long k[4] = {
    0x3B398E26,
    0x40C29501,
    0x614D7630,
    0x7F59409A
};

static void encypher(long* v)
{
    DWORD y=v[0];
    DWORD z=v[1];
    DWORD sum=0;
    DWORD delta=0x9e3779b9;                // a key schedule constant
    DWORD n=32;                            // num iterations

    while (n-->0) {                        // basic cycle start
        sum += delta;
        y += (z<<4)+k[0] ^ z+sum ^ (z>>5)+k[1];
        z += (y<<4)+k[2] ^ y+sum ^ (y>>5)+k[3];
    }

    v[0]=y;
    v[1]=z;
}

static void decypher(long* v)
{
    DWORD y=v[0];
    DWORD z=v[1];
    DWORD sum=0;
    DWORD delta=0x9e3779b9;                // a key schedule constant
    DWORD n=32;                            // num iterations

    sum=delta<<5;

    while (n-->0) {
        z-= (y<<4)+k[2] ^ y+sum ^ (y>>5)+k[3]; 
        y-= (z<<4)+k[0] ^ z+sum ^ (z>>5)+k[1];
        sum-=delta;
    }

    v[0]=y;
    v[1]=z;
}

// +-------------------------------------------------------------------+

Text 
Encryption::Encrypt(Text block)
{
    int len = block.length();

    if (len < 1)
    return Text();

    // pad to eight byte chunks
    if (len & 0x7) {
        len /= 8;
        len *= 8;
        len += 8;
    }

    BYTE* work = new(__FILE__,__LINE__) BYTE[len];
    ZeroMemory(work, len);
    CopyMemory(work, block.data(), block.length());

    long* v = (long*) work;
    for (int i = 0; i < len/8; i++) {
        encypher(v);
        v += 2;
    }

    Text cypher((const char*) work, len);
    delete [] work;
    return cypher;
}

// +-------------------------------------------------------------------+

Text 
Encryption::Decrypt(Text block)
{
    int  len    = block.length();

    if (len & 0x7) {
        Print("WARNING: attempt to decrypt odd length block (len=%d)\n", len);
        return Text();
    }

    BYTE* work = new(__FILE__,__LINE__) BYTE[len];
    CopyMemory(work, block.data(), len);

    long* v = (long*) work;
    for (int i = 0; i < len/8; i++) {
        decypher(v);
        v += 2;
    }

    Text clear((const char*) work, len);
    delete [] work;
    return clear;
}

// +-------------------------------------------------------------------+

static const char* codes = "abcdefghijklmnop";

Text 
Encryption::Encode(Text block)
{
    int   len  = block.length() * 2;
    char* work = new(__FILE__,__LINE__) char[len + 1];

    for (int i = 0; i < block.length(); i++) {
        BYTE b = (BYTE) (block.data()[i]);
        work[2*i]   = codes[b>>4 & 0xf];
        work[2*i+1] = codes[b    & 0xf];
    }

    work[len] = 0;

    Text code(work, len);
    delete [] work;
    return code;
}

// +-------------------------------------------------------------------+

Text 
Encryption::Decode(Text block)
{
    int   len  = block.length() / 2;
    char* work = new(__FILE__,__LINE__) char[len + 1];

    for (int i = 0; i < len; i++) {
        char u = block[2*i];
        char l = block[2*i + 1];

        work[i] = (u - codes[0]) << 4 |
        (l - codes[0]);
    }

    work[len] = 0;

    Text clear(work, len);
    delete [] work;
    return clear;
}

