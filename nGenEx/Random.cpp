/*  Project nGenEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    nGenEx.lib
    FILE:         Random.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Utility functions for generating random numbers and locations.
*/

#include "MemDebug.h"
#include "Random.h"

// +----------------------------------------------------------------------+

void RandomInit()
{
   srand(timeGetTime());
}

// +----------------------------------------------------------------------+

Point RandomDirection()
{
   Point p = Point(rand() - 16384, rand() - 16384, 0);
   p.Normalize();
   return p;
}

// +----------------------------------------------------------------------+

Point RandomPoint()
{
   Point p = Point(rand() - 16384, rand() - 16384, 0);
   p.Normalize();
   p *= 15e3 + rand()/3;
   return p;
}

// +----------------------------------------------------------------------+

Vec3 RandomVector(double radius)
{
   Vec3 v = Vec3(rand() - 16384, rand() - 16384, rand() - 16384);
   v.Normalize();

   if (radius > 0)
      v *= (float) radius;
   else
      v *= (float) Random(radius/3, radius);

   return v;
}

// +----------------------------------------------------------------------+

double Random(double min, double max)
{
   double delta = max - min;
   double r     = delta * rand() / 32768.0;

   return min + r;
}

// +----------------------------------------------------------------------+

int RandomIndex()
{
   static int index     = 0;
   static int table[16] = { 0, 9, 4, 7, 14, 11, 2, 12, 1, 5, 13, 8, 6, 10, 3, 15 };

   int r = 1 + ((rand() & 0x0700) >> 8);
   index += r;
   if (index > 1e7) index = 0;
   return table[index % 16];
}

// +----------------------------------------------------------------------+

bool RandomChance(int wins, int tries)
{
   double fraction = 256.0 * wins / tries;
   double r        = (rand() >> 4) & 0xFF;

   return r < fraction;
}

// +----------------------------------------------------------------------+

int RandomSequence(int current, int range)
{
   if (range > 1) {
      int step = (int) Random(1, range-1);
      return (current + step) % range;
   }

   return current;
}

// +----------------------------------------------------------------------+

int RandomShuffle(int count)
{
   static int  set_size = -1;
   static BYTE set[256];
   static int  index    = -1;

   if (count < 0 || count > 250)
      return 0;

   if (set_size != count) {
      set_size = count;
      index    = -1;
   }

   // need to reshuffle
   if (index < 0 || index > set_size-1) {
      // set up the deck
      int tmp[256];
      for (int i = 0; i < 256; i++)
         tmp[i] = i;

      // shuffle the cards
      for (i = 0; i < set_size; i++) {
         int n     = (int) Random(0, set_size);
         int tries = set_size;
         while (tmp[n] < 0 && tries--) {
            n = (n+1) % set_size;
         }

         if (tmp[n] >= 0) {
            set[i] = tmp[n];
            tmp[n] = -1;
         }
         else {
            set[i] = 0;
         }
      }

      index = 0;
   }

   return set[index++];
}
