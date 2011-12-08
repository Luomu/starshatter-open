/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    nGenEx.lib
    FILE:         Res.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Abstract Resource class
*/

#ifndef Res_h
#define Res_h

#include "Types.h"

// +--------------------------------------------------------------------+

class Resource
{
public:
   Resource();
   virtual ~Resource();
   
   int operator == (const Resource& r) const { return id == r.id; }
   
   HANDLE   Handle() const { return id; }

protected:
   HANDLE   id;
};

#endif Res_h

