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
    FILE:         Command.h
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

#ifndef Command_h
#define Command_h

#include "Text.h"

// +--------------------------------------------------------------------+

class MagicDoc;
class Solid;
class Model;

// +--------------------------------------------------------------------+

class Command
{
public:
   static const char* TYPENAME() { return "Command"; }

   Command(const char* name, MagicDoc* document);
   virtual ~Command();

   // operations
   virtual void   Do();
   virtual void   Undo();

   const char*    Name()   const { return name; }

protected:
   Text           name;
   MagicDoc*      document;
};

// +--------------------------------------------------------------------+

#endif Command_h
