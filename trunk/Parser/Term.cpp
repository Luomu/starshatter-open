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
     FILE:         Term.cpp
     AUTHOR:       John DiCamillo


     OVERVIEW
     ========
     Implementation of the Term class
*/


#include "MemDebug.h"
#include "Term.h"

void Print(const char* fmt, ...);

// +-------------------------------------------------------------------+

Term*
error(char* s1, char* s2)
{
    Print("ERROR: ");
    if (s1) Print(s1);
    if (s2) Print(s2);
    Print("\n\n");
    return 0;
}

// +-------------------------------------------------------------------+

void TermBool::print(int level)  { if (level > 0) Print(val? "true" : "false"); else Print("..."); }
void TermNumber::print(int level){ if (level > 0) Print("%g", val);     else Print("..."); }
void TermText::print(int level)  { if (level > 0) Print("\"%s\"", val.data()); else Print("..."); }

// +-------------------------------------------------------------------+

TermArray::TermArray(TermList* elist)
{
    elems = elist;
}

TermArray::~TermArray()
{
    if (elems) elems->destroy();
    delete elems;
}

void
TermArray::print(int level)
{
    if (level > 1) {
        Print("(");

        if (elems) {
            for (int i = 0; i < elems->size(); i++) {
                elems->at(i)->print(level-1);
                if (i < elems->size() -1)
                    Print(", ");
            }
        }

        Print(") ");
    }
    else Print("(...) ");
}

// +-------------------------------------------------------------------+

TermStruct::TermStruct(TermList* elist)
{
    elems = elist;
}

TermStruct::~TermStruct()
{
    if (elems) elems->destroy();
    delete elems;
}

void
TermStruct::print(int level)
{
    if (level > 1) {
        Print("{");

        if (elems) {
            for (int i = 0; i < elems->size(); i++) {
                elems->at(i)->print(level-1);
                if (i < elems->size() -1)
                    Print(", ");
            }
        }

        Print("} ");
    }
    else Print("{...} ");
}

// +-------------------------------------------------------------------+

TermDef::~TermDef()
{
    delete mname;
    delete mval;
}

void
TermDef::print(int level)
{
    if (level >= 0) {
        mname->print(level);
        Print(": ");
        mval->print(level-1);
    }
    else Print("...");
}

// +-------------------------------------------------------------------+
