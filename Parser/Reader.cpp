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

     SUBSYSTEM:    Parser
     FILE:         Reader.cpp
     AUTHOR:       John DiCamillo


     OVERVIEW
     ========
     Implementation of the Reader class
*/

#include "MemDebug.h"
#include "reader.h"
#include <stdio.h>
#include <fstream>
#include <ctype.h>


void              Print(const char* fmt, ...);

// +-------------------------------------------------------------------+

Text
ConsoleReader::more()
{
    // loop until the user types something
    do {
        printPrimaryPrompt();
        fillInputBuffer();
    } while (! *p);

    return Text(p);
}

void
ConsoleReader::printPrimaryPrompt()
{
    printf("- ");
}

void
ConsoleReader::fillInputBuffer()
{
    fgets(buffer, 980, stdin);
    p = buffer;
    while (isspace(*p)) p++;
}

// +-------------------------------------------------------------------+

FileReader::FileReader(const char* fname)
    : filename(fname), done(0)
{ }

Text
FileReader::more()
{
    if (done) return Text();

    std::fstream fin(filename, std::fstream::in);

    if (!fin) {
        Print("ERROR(Parse): Could not open file '%s'\n", filename.data());
        return Text();
    }

    Text result;
    char buf[1000], newline;
    
    while (fin.get(buf, 1000)) {
        result.append(buf);
        fin.get(newline);
        result.append(newline);
    }

    done = 1;
    return result;   
}

// +-------------------------------------------------------------------+

BlockReader::BlockReader(const char* block)
    : data((char*) block), done(0), length(0)
{ }

BlockReader::BlockReader(const char* block, int len)
    : data((char*) block), done(0), length(len)
{ }

Text
BlockReader::more()
{
    if (done) return Text();

    if (length) {
        Text result(data, length);
        done = 1;
        return result;   
    }
    else if (data) {
        Text result(data);
        done = 1;
        return result;   
    }

    done = 1;
    return Text();
}


