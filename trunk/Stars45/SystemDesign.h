/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         SystemDesign.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Generic ship System Design class
*/

#ifndef SystemDesign_h
#define SystemDesign_h

#include "Types.h"
#include "List.h"
#include "Text.h"

// +--------------------------------------------------------------------+

class ComponentDesign;

// +--------------------------------------------------------------------+

class SystemDesign
{
public:
   static const char* TYPENAME() { return "SystemDesign"; }

   SystemDesign();
   ~SystemDesign();
   int operator == (const SystemDesign& rhs) const { return name == rhs.name; }

   static void          Initialize(const char* filename);
   static void          Close();
   static SystemDesign* Find(const char* name);

   // Unique ID:
   Text              name;

   // Sub-components:
   List<ComponentDesign> components;

   static List<SystemDesign>  catalog;
};

// +--------------------------------------------------------------------+


#endif SystemDesign_h

