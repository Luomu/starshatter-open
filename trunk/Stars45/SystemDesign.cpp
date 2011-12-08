/*  Project Starshatter 4.5
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    Stars.exe
    FILE:         SystemDesign.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Weapon Design parameters class
*/

#include "MemDebug.h"
#include "SystemDesign.h"
#include "Component.h"

#include "Game.h"
#include "Bitmap.h"
#include "DataLoader.h"
#include "ParseUtil.h"

// +--------------------------------------------------------------------+

List<SystemDesign> SystemDesign::catalog;

#define GET_DEF_TEXT(p,d,x) if(p->name()->value()==(#x))GetDefText(d->x,p,filename)
#define GET_DEF_NUM(p,d,x)  if(p->name()->value()==(#x))GetDefNumber(d->x,p,filename)

// +--------------------------------------------------------------------+

SystemDesign::SystemDesign()
{ }

SystemDesign::~SystemDesign()
{
   components.destroy();
}

// +--------------------------------------------------------------------+

void
SystemDesign::Initialize(const char* filename)
{
   Print("Loading System Designs '%s'\n", filename);

   // Load Design File:
   DataLoader* loader = DataLoader::GetLoader();
   BYTE* block;
   
   int blocklen = loader->LoadBuffer(filename, block, true);
   Parser parser(new(__FILE__,__LINE__) BlockReader((const char*) block, blocklen));
   Term*  term = parser.ParseTerm();
   
   if (!term) {
      Print("ERROR: could not parse '%s'\n", filename);
      exit(-3);
   }
   else {
      TermText* file_type = term->isText();
      if (!file_type || file_type->value() != "SYSTEM") {
         Print("ERROR: invalid system design file '%s'\n", filename);
         exit(-4);
      }
   }
   
   int type = 1;

   do {
      delete term;

      term = parser.ParseTerm();
      
      if (term) {
         TermDef* def = term->isDef();
         if (def) {
            if (def->name()->value() == "system") {

               if (!def->term() || !def->term()->isStruct()) {
                  Print("WARNING: system structure missing in '%s'\n", filename);
               }
               else {
                  TermStruct*   val    = def->term()->isStruct();
                  SystemDesign* design = new(__FILE__,__LINE__) SystemDesign;
                  
                  for (int i = 0; i < val->elements()->size(); i++) {
                     TermDef* pdef = val->elements()->at(i)->isDef();
                     if (pdef) {
                        GET_DEF_TEXT(pdef, design, name);

                        else if (pdef->name()->value()==("component")) {
                           if (!pdef->term() || !pdef->term()->isStruct()) {
                              Print("WARNING: component structure missing in system '%s' in '%s'\n", (const char*) design->name, filename);
                           }
                           else {
                              TermStruct*   val2    = pdef->term()->isStruct();
                              ComponentDesign* comp_design = new(__FILE__,__LINE__) ComponentDesign;
                  
                              for (int i = 0; i < val2->elements()->size(); i++) {
                                 TermDef* pdef2 = val2->elements()->at(i)->isDef();
                                 if (pdef2) {
                                         GET_DEF_TEXT(pdef2, comp_design, name);
                                    else GET_DEF_TEXT(pdef2, comp_design, abrv);
                                    else GET_DEF_NUM (pdef2, comp_design, repair_time);
                                    else GET_DEF_NUM (pdef2, comp_design, replace_time);
                                    else GET_DEF_NUM (pdef2, comp_design, spares);
                                    else GET_DEF_NUM (pdef2, comp_design, affects);

                                    else {
                                       Print("WARNING: parameter '%s' ignored in '%s'\n",
                                             pdef2->name()->value().data(), filename);
                                    }
                                 }
                              }

                              design->components.append(comp_design);
                           }
                        }

                        else {
                           Print("WARNING: parameter '%s' ignored in '%s'\n",
                                 pdef->name()->value().data(), filename);
                        }
                     }
                     else {
                        Print("WARNING: term ignored in '%s'\n", filename);
                        val->elements()->at(i)->print();
                     }
                  }

                  catalog.append(design);
               }
            }
            
            else
               Print("WARNING: unknown definition '%s' in '%s'\n",
                        def->name()->value().data(), filename);
         }
         else {
            Print("WARNING: term ignored in '%s'\n", filename);
            term->print();
         }
      }
   }
   while (term);
   
   loader->ReleaseBuffer(block);
}

// +--------------------------------------------------------------------+

void
SystemDesign::Close()
{
   catalog.destroy();
}

// +--------------------------------------------------------------------+

SystemDesign*
SystemDesign::Find(const char* name)
{
   SystemDesign  test;
   test.name = name;
   return catalog.find(&test);
}

