/*  Project Magic 2.0
    Destroyer Studios LLC
    Copyright © 1997-2005. All Rights Reserved.

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
