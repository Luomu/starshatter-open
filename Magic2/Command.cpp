/*  Project Magic 2.0
    Destroyer Studios LLC
    Copyright © 1997-2005. All Rights Reserved.

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
