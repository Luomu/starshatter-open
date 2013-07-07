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

    SUBSYSTEM:    Magic.exe
    FILE:         Command.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Base class (interface) for command pattern.  A 
    command encapsulates a single editing operation
    that may be performed on a document.  Each command
    may be done or undone multiple times.  The specialized
    implementations for each type of operation are re-
    sponsible for providing a means to return the model
    to the prior state.
*/

#include "stdafx.h"
#include "Command.h"

// +--------------------------------------------------------------------+

void         Print(const char* msg, ...);

// +--------------------------------------------------------------------+

Command::Command(const char* n, MagicDoc* d)
   : name(n), document(d)
{
}

Command::~Command()
{
   document = 0;
}

// +--------------------------------------------------------------------+

void
Command::Do()
{
   Print("WARNING: Command::Do() called for '%s'\n", name.data());
}

void
Command::Undo()
{
   Print("WARNING: Command::Undo() called for '%s'\n", name.data());
}
