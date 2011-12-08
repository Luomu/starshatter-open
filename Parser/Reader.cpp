/*  Project STARS
    John DiCamillo Software Consulting
    Copyright © 1997-2000. All Rights Reserved.

    SUBSYSTEM:    obelisk
    FILE:         reader.cpp
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
      Print("ERROR(Parse): Could not open file '%s'\n", filename);
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


