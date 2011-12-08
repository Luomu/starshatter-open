/*  Project nGen
    John DiCamillo Software Consulting
    Copyright © 1997-2000. All Rights Reserved.

    SUBSYSTEM:    obelisk
    FILE:         reader.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Declaration of the Reader class
*/

#ifndef READER_H
#define READER_H

#include "text.h"

// +-------------------------------------------------------------------+

class Reader
{
public:
   Reader()          { }
   virtual ~Reader() { }

   virtual Text more() = 0;
};

class ConsoleReader : public Reader
{
public:
   virtual Text more();
   
   void printPrimaryPrompt();
   void fillInputBuffer();
   
private:
   char  buffer[1000];
   char* p;
};

class FileReader : public Reader
{
public:
   FileReader(const char* fname);
   virtual Text more();
   
private:
   Text filename;
   int  done;
};

class BlockReader : public Reader
{
public:
   BlockReader(const char* block);
   BlockReader(const char* block, int len);
   virtual Text more();

private:
   char* data;
   int   done;
   int   length;
};

#endif
