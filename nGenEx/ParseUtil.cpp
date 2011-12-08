/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    nGenEx.lib
    FILE:         ParseUtil.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Parser utility functions
*/

#include "MemDebug.h"
#include "ParseUtil.h"
#include "DataLoader.h"

#include "stdio.h"

// +--------------------------------------------------------------------+

bool GetDefBool(bool& dst, TermDef* def, const char* file)
{
   if (!def || !def->term()) {
      Print("WARNING: missing BOOL TermDef in '%s'\n", file);
      return false;
   }

   TermBool* tn = def->term()->isBool();
   if (tn) {
      dst = tn->value();
      return true;
   }
   else {
      Print("WARNING: invalid bool %s in '%s'.  value = ", def->name()->value().data(), file);
      def->term()->print(10);
      Print("\n");
   }

   return false;
}

bool GetDefText(Text& dst, TermDef* def, const char* file)
{
   if (!def || !def->term()) {
      Print("WARNING: missing TEXT TermDef in '%s'\n", file);
      return false;
   }

   TermText* tn = def->term()->isText();
   if (tn) {
      dst = tn->value();
      return true;
   }
   else
      Print("WARNING: invalid TEXT %s in '%s'\n", def->name()->value().data(), file);

   return false;
}

bool GetDefText(char* dst, TermDef* def, const char* file)
{
   if (!def || !def->term()) {
      Print("WARNING: missing TEXT TermDef in '%s'\n", file);
      return false;
   }

   TermText* tn = def->term()->isText();
   if (tn) {
      strcpy(dst, tn->value());
      return true;
   }
   else
      Print("WARNING: invalid TEXT %s in '%s'\n", def->name()->value().data(), file);

   return false;
}

bool GetDefNumber(int& dst, TermDef* def, const char* file)
{
   if (!def || !def->term()) {
      Print("WARNING: missing NUMBER TermDef in '%s'\n", file);
      return false;
   }

   TermNumber* tr = def->term()->isNumber();
   if (tr) {
      dst = (int) tr->value();
      return true;
   }
   else
      Print("WARNING: invalid NUMBER %s in '%s'\n", def->name()->value().data(), file);

   return false;
}

bool GetDefNumber(DWORD& dst, TermDef* def, const char* file)
{
   if (!def || !def->term()) {
      Print("WARNING: missing NUMBER TermDef in '%s'\n", file);
      return false;
   }

   TermNumber* tr = def->term()->isNumber();
   if (tr) {
      dst = (DWORD) tr->value();
      return true;
   }
   else
      Print("WARNING: invalid NUMBER %s in '%s'\n", def->name()->value().data(), file);

   return false;
}

bool GetDefNumber(float& dst, TermDef* def, const char* file)
{
   if (!def || !def->term()) {
      Print("WARNING: missing NUMBER TermDef in '%s'\n", file);
      return false;
   }

   TermNumber* tr = def->term()->isNumber();
   if (tr) {
      dst = (float) tr->value();
      return true;
   }
   else
      Print("WARNING: invalid NUMBER %s in '%s'\n", def->name()->value().data(), file);

   return false;
}

bool GetDefNumber(double& dst, TermDef* def, const char* file)
{
   if (!def || !def->term()) {
      Print("WARNING: missing NUMBER TermDef in '%s'\n", file);
      return false;
   }

   TermNumber* tr = def->term()->isNumber();
   if (tr) {
      dst = (double) tr->value();
      return true;
   }
   else
      Print("WARNING: invalid NUMBER %s in '%s'\n", def->name()->value().data(), file);

   return false;
}

bool GetDefVec(Vec3& dst, TermDef* def, const char* file)
{
   if (!def || !def->term()) {
      Print("WARNING: missing VEC3 TermDef in '%s'\n", file);
      return false;
   }

   TermArray* val = def->term()->isArray();
   if (val) {
      if (val->elements()->size() != 3) {
         Print("WARNING: malformed vector in '%s'\n", file);
      }
      else {
         dst.x = (float) (val->elements()->at(0)->isNumber()->value());
         dst.y = (float) (val->elements()->at(1)->isNumber()->value());
         dst.z = (float) (val->elements()->at(2)->isNumber()->value());

         return true;
      }
   }
   else {
      Print("WARNING: vector expected in '%s'\n", file);
   }

   return false;
}

bool GetDefRect(Rect& dst, TermDef* def, const char* file)
{
   if (!def || !def->term()) {
      Print("WARNING: missing RECT TermDef in '%s'\n", file);
      return false;
   }

   TermArray* val = def->term()->isArray();
   if (val) {
      if (val->elements()->size() != 4) {
         Print("WARNING: malformed rect in '%s'\n", file);
      }
      else {
         dst.x = (int) (val->elements()->at(0)->isNumber()->value());
         dst.y = (int) (val->elements()->at(1)->isNumber()->value());
         dst.w = (int) (val->elements()->at(2)->isNumber()->value());
         dst.h = (int) (val->elements()->at(3)->isNumber()->value());

         return true;
      }
   }
   else {
      Print("WARNING: rect expected in '%s'\n", file);
   }

   return false;
}

bool GetDefInsets(Insets& dst, TermDef* def, const char* file)
{
   if (!def || !def->term()) {
      Print("WARNING: missing Insets TermDef in '%s'\n", file);
      return false;
   }

   TermArray* val = def->term()->isArray();
   if (val) {
      if (val->elements()->size() != 4) {
         Print("WARNING: malformed Insets in '%s'\n", file);
      }
      else {
         dst.left   = (WORD) (val->elements()->at(0)->isNumber()->value());
         dst.right  = (WORD) (val->elements()->at(1)->isNumber()->value());
         dst.top    = (WORD) (val->elements()->at(2)->isNumber()->value());
         dst.bottom = (WORD) (val->elements()->at(3)->isNumber()->value());

         return true;
      }
   }
   else {
      Print("WARNING: Insets expected in '%s'\n", file);
   }

   return false;
}

bool GetDefColor(Color& dst, TermDef* def, const char* file)
{
   if (!def || !def->term()) {
      Print("WARNING: missing COLOR TermDef in '%s'\n", file);
      return false;
   }

   TermArray* val = def->term()->isArray();
   if (val) {
      if (val->elements()->size() != 3) {
         Print("WARNING: malformed color in '%s'\n", file);
      }
      else {
         BYTE r, g, b;
         double v0 = (val->elements()->at(0)->isNumber()->value());
         double v1 = (val->elements()->at(1)->isNumber()->value());
         double v2 = (val->elements()->at(2)->isNumber()->value());

         if (v0 >= 0 && v0 <= 1 &&
             v1 >= 0 && v1 <= 1 &&
             v2 >= 0 && v2 <= 1) {

            r = (BYTE) (v0 * 255);
            g = (BYTE) (v1 * 255);
            b = (BYTE) (v2 * 255);

         }
         else {
            r = (BYTE) v0;
            g = (BYTE) v1;
            b = (BYTE) v2;
         }

         dst = Color(r,g,b);
         return true;
      }
   }
   else {
      Print("WARNING: color expected in '%s'\n", file);
   }

   return false;
}

bool GetDefColor(ColorValue& dst, TermDef* def, const char* file)
{
   if (!def || !def->term()) {
      Print("WARNING: missing COLOR TermDef in '%s'\n", file);
      return false;
   }

   TermArray* val = def->term()->isArray();
   if (val) {
      if (val->elements()->size() < 3 || val->elements()->size() > 4) {
         Print("WARNING: malformed color in '%s'\n", file);
      }
      else {
         double r = (val->elements()->at(0)->isNumber()->value());
         double g = (val->elements()->at(1)->isNumber()->value());
         double b = (val->elements()->at(2)->isNumber()->value());
         double a = 1;

         if (val->elements()->size() == 4)
            a = (val->elements()->at(3)->isNumber()->value());

         dst.Set((float) r, (float) g, (float) b, (float) a);
         return true;
      }
   }
   else {
      Print("WARNING: color expected in '%s'\n", file);
   }

   return false;
}

// +--------------------------------------------------------------------+

bool GetDefArray(int* dst, int size, TermDef* def, const char* file)
{
   if (!def || !def->term()) {
      Print("WARNING: missing ARRAY TermDef in '%s'\n", file);
      return false;
   }

   TermArray* val = def->term()->isArray();
   if (val) {
      int nelem = val->elements()->size();

      if (nelem > size)
         nelem = size;

      for (int i = 0; i < nelem; i++)
         *dst++ = (int) (val->elements()->at(i)->isNumber()->value());

      return true;
   }
   else {
      Print("WARNING: array expected in '%s'\n", file);
   }

   return false;
}

bool GetDefArray(float* dst, int size, TermDef* def, const char* file)
{
   if (!def || !def->term()) {
      Print("WARNING: missing ARRAY TermDef in '%s'\n", file);
      return false;
   }

   TermArray* val = def->term()->isArray();
   if (val) {
      int nelem = val->elements()->size();

      if (nelem > size)
         nelem = size;

      for (int i = 0; i < nelem; i++)
         *dst++ = (float) (val->elements()->at(i)->isNumber()->value());

      return true;
   }
   else {
      Print("WARNING: array expected in '%s'\n", file);
   }

   return false;
}

bool GetDefArray(double* dst, int size, TermDef* def, const char* file)
{
   if (!def || !def->term()) {
      Print("WARNING: missing ARRAY TermDef in '%s'\n", file);
      return false;
   }

   TermArray* val = def->term()->isArray();
   if (val) {
      int nelem = val->elements()->size();

      if (nelem > size)
         nelem = size;

      for (int i = 0; i < nelem; i++)
         *dst++ = (double) (val->elements()->at(i)->isNumber()->value());

      return true;
   }
   else {
      Print("WARNING: array expected in '%s'\n", file);
   }

   return false;
}

// +--------------------------------------------------------------------+

bool GetDefArray(ArrayList& array, TermDef* def, const char* file)
{
   if (!def || !def->term()) {
      Print("WARNING: missing ARRAY TermDef in '%s'\n", file);
      return false;
   }

   TermArray* val = def->term()->isArray();
   if (val) {
      int nelem = val->elements()->size();

      array.clear();

      for (int i = 0; i < nelem; i++)
         array.append((DWORD) (val->elements()->at(i)->isNumber()->value()));

      return true;
   }
   else {
      Print("WARNING: integer array expected in '%s'\n", file);
   }

   return false;
}

bool GetDefArray(FloatList& array, TermDef* def, const char* file)
{
   if (!def || !def->term()) {
      Print("WARNING: missing ARRAY TermDef in '%s'\n", file);
      return false;
   }

   TermArray* val = def->term()->isArray();
   if (val) {
      int nelem = val->elements()->size();

      array.clear();

      for (int i = 0; i < nelem; i++)
         array.append((float) (val->elements()->at(i)->isNumber()->value()));

      return true;
   }
   else {
      Print("WARNING: float array expected in '%s'\n", file);
   }

   return false;
}

// +--------------------------------------------------------------------+

bool GetDefTime(int& dst, TermDef* def, const char* file)
{
   if (!def || !def->term()) {
      Print("WARNING: missing TIME TermDef in '%s'\n", file);
      return false;
   }

   TermText* tn = def->term()->isText();

   if (tn) {
      int d = 0;
      int h = 0;
      int m = 0;
      int s = 0;

      char buf[64];
      strcpy(buf, tn->value());

      if (strchr(buf, '/'))
         sscanf(buf, "%d/%d:%d:%d", &d, &h, &m, &s);
      else
         sscanf(buf, "%d:%d:%d", &h, &m, &s);

      dst = d * 24 * 60 * 60 +
            h      * 60 * 60 +
            m           * 60 +
            s;

      return true;
   }
   else
      Print("WARNING: invalid TIME %s in '%s'\n", def->name()->value().data(), file);

   return false;
}


