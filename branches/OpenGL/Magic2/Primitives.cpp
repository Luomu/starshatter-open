/*  Project MAGIC
    John DiCamillo Software Consulting
    Copyright © 1994-1997. All Rights Reserved.

    SUBSYSTEM:    Magic.exe Application
    FILE:         Primitives.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Commands for adding basic geometric primitives to a mesh
*/


#include "stdafx.h"
#include "Primitives.h"
#include "MagicDoc.h"
#include "ModelView.h"
#include "Selection.h"

// +----------------------------------------------------------------------+
// +----------------------------------------------------------------------+
// +----------------------------------------------------------------------+

CreatePolyCommand::CreatePolyCommand(MagicDoc* d)
   : Command(n, d)
{
}

CreatePolyCommand::~CreatePolyCommand()
{
}

// +----------------------------------------------------------------------+

void
CreatePolyCommand::Do()
{
   if (document) {
   }
}

// +----------------------------------------------------------------------+

void
EditCommand::Undo()
{
   if (document) {
   }
}

