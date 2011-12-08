/*  Project nGen
    John DiCamillo Software Consulting
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    parser
    FILE:         parser.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Declaration of the generic Parser class
*/

#ifndef PARSER_H
#define PARSER_H

#include "text.h"
#include "term.h"

// +-------------------------------------------------------------------+

class Reader;
class Scanner;

// +-------------------------------------------------------------------+

class Parser
{
public:
   Parser(Reader* r = 0);
   ~Parser();

   Term*          ParseTerm();
   Term*          ParseTermBase();
   Term*          ParseTermRest(Term* base);
   TermList*      ParseTermList(int for_struct);
   TermArray*     ParseArray();
   TermStruct*    ParseStruct();

private:
   Reader*        reader;
   Scanner*       lexer;
};

#endif
