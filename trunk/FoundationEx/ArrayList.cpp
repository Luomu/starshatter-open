/*  Project FoundationEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    FoundationEx
    FILE:         ArrayList.cpp
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Implementation of the untyped ArrayList class
*/

#include "MemDebug.h"
#include "ArrayList.h"

// +-------------------------------------------------------------------+

void Print(const char* fmt, ...);

// +-------------------------------------------------------------------+

ArrayList::ArrayList(const ArrayList& l)
  : items(l.items), extent(l.extent)
{
#ifdef MEM_DEBUG
   array = new(__FILE__,__LINE__) DWORD[extent];
#else
   array = new DWORD[extent];
#endif

   memcpy(array, l.array, extent*sizeof(DWORD));
}

void ArrayList::clear()
{
   delete [] array;
   items    = 0;
   extent   = 0;
   array    = 0;
}

// +-------------------------------------------------------------------+

bool ArrayList::check(int& index) const
{
   if (index < 0) {
      Print("Bounds error in ArrayList(%08x) index=%d min=0\n", (int)this, index);
      index = 0;
   }

   else if (index >= items) {
      Print("Bounds error in ArrayList(%08x) index=%d max=%d\n", (int)this,index, items-1);
      index = items-1;
   }

   return (index >= 0 && index < items);
}

// +-------------------------------------------------------------------+

DWORD ArrayList::operator[](int index) const
{
   if (check(index))
      return array[index];
   return 0;
}

DWORD& ArrayList::operator[](int index)
{
   if (check(index))
      return array[index];

   return array[0];
}

DWORD ArrayList::at(int index) const
{
   if (check(index))
      return array[index];
   return 0;
}

DWORD& ArrayList::at(int index)
{
   if (check(index))
      return array[index];

   return array[0];
}

// +-------------------------------------------------------------------+

void ArrayList::resize(int newsize)
{
   if (newsize > extent) {
      extent = 16 * (newsize/16 + 1);

#ifdef MEM_DEBUG
      DWORD* v = new(__FILE__,__LINE__) DWORD[extent];
#else
      DWORD* v = new DWORD[extent];
#endif

      for (int i = 0; i < items; i++)
         v[i] = array[i];

      for (; i < extent; i++)
         v[i] = 0;

      delete [] array;   
      array = v;
   }
}

// +-------------------------------------------------------------------+

void ArrayList::append(DWORD item)
{
   if (items+1 > extent)
      resize(items+1);

   array[items++] = item;
}

void ArrayList::append(const ArrayList& list)
{
   if (&list != this && list.items > 0) {
      int need = items + list.items;
      if (need > extent)
         resize(need);

      for (int i = 0; i < list.items; i++)
         array[items++] = list.array[i];
   }
}

// +-------------------------------------------------------------------+

void ArrayList::insert(DWORD item, int index)
{
   if (index >= 0 && index <= items) {
      if (items+1 > extent)
         resize(items+1);

      // slide right:
      for (int i = items; i > index; i--)
         array[i] = array[i-1];

      array[index] = item;
      items++;
   }
}

// +-------------------------------------------------------------------+

void ArrayList::insertSort(DWORD item)
{
   for (int i = 0; i < items; i++) {
      if (item < array[i])
         break;
   }

   insert(item, i);
}

// +-------------------------------------------------------------------+

void ArrayList::remove(DWORD item)
{
   if (items < 1)
      return;

   for (int i = 0; i < items; i++) {
      if (array[i] == item) {
         removeIndex(i);
         return;
      }
   }
}

// +-------------------------------------------------------------------+

void ArrayList::removeIndex(int index)
{
   if (items < 1 || !check(index))
      return;

   // slide left:
   for (int i = index; i < items-1; i++)
      array[i] = array[i+1];

   // blank out the hole we just created:
   array[items-1] = 0;

   items--;
}

// +-------------------------------------------------------------------+

bool ArrayList::contains(DWORD val) const
{
   for (int i = 0; i < items; i++) {
      if (array[i] == val)
         return true;
   }

   return false;
}

// +-------------------------------------------------------------------+

int ArrayList::count(DWORD val) const
{
   int c = 0;

   for (int i = 0; i < items; i++) {
      if (array[i] == val)
         c++;
   }

   return c;
}

// +-------------------------------------------------------------------+

int ArrayList::index(DWORD val) const
{
   for (int i = 0; i < items; i++) {
      if (array[i] == val)
         return i;
   }

   return -1;
}

// +-------------------------------------------------------------------+

void ArrayList::swap(DWORD* a, int i, int j)
{
   if (i >= 0 && i < items && j >= 0 && j < items && i != j) {
      DWORD t = a[i];
      a[i] = a[j];
      a[j] = t;
   }
}

void ArrayList::qsort(DWORD* a, int lo0, int hi0)
{
   int lo = lo0;
   int hi = hi0;

   // zero or one element list, nothing to do:
   if (lo >= hi) {
      return;
   }

   // two element list, swap if needed:
   else if (lo == hi-1) {
      if (a[hi] < a[lo]) {
         swap(a, lo, hi);
      }
      return;
   }

   // pick a pivot, and move it out of the way:
   int   mid   = (lo+hi)/2;
   DWORD pivot = a[mid];
   a[mid]      = a[hi];
   a[hi]       = pivot;

   while (lo < hi) {
      while ((a[lo] <= pivot) && lo < hi) lo++;
      while ((pivot <= a[hi]) && lo < hi) hi--;

      if (lo < hi) {
         swap(a, lo, hi);
      }
   }

   // Put the pivot into its final location:
   a[hi0] = a[hi];
   a[hi] = pivot;

	qsort(a, lo0, lo-1);
	qsort(a, hi+1, hi0);
}

void ArrayList::sort()
{
   if (items < 2)
      return;

   qsort(array, 0, items-1);
}

void ArrayList::shuffle()
{
   if (items < 3)
      return;

   for (int s = 0; s < 5; s++) {
      for (int i = 0; i < items; i++) {
         int j = (rand()>>4) % items;
         swap(array, i, j);
      }
   }
}


// +===================================================================+

DWORD ArrayListIter::value()
{
   if (list && step >= 0 && step < list->items)
      return list->array[step];

   return 0;
}

// +-------------------------------------------------------------------+

void ArrayListIter::removeItem()
{
   if (list && step >= 0 && step < list->items)
      list->removeIndex(step--);
}

// +-------------------------------------------------------------------+

DWORD ArrayListIter::next()
{
   if (list && step >= -1 && step < list->items-1)
      return list->array[++step];

   return 0;
}

DWORD ArrayListIter::prev()
{
   if (list && step > 0 && step < list->items)
      return list->array[--step];

   return 0;
}

// +-------------------------------------------------------------------+

void ArrayListIter::attach(ArrayList& l)
{
   list = &l;
   step = -1;
}

// +-------------------------------------------------------------------+

int ArrayListIter::size()
{
   if (!list) return 0;
   return list->items;
}

// +-------------------------------------------------------------------+

ArrayList& ArrayListIter::container()
{
   return *list;
}




// +-------------------------------------------------------------------+
// +-------------------------------------------------------------------+
// +-------------------------------------------------------------------+

FloatList::FloatList(const FloatList& l)
  : items(l.items), extent(l.extent)
{
#ifdef MEM_DEBUG
   array = new(__FILE__,__LINE__) float[extent];
#else
   array = new float[extent];
#endif

   memcpy(array, l.array, extent*sizeof(float));
}

void FloatList::clear()
{
   delete [] array;
   items    = 0;
   extent   = 0;
   array    = 0;
}

// +-------------------------------------------------------------------+

bool FloatList::check(int& index) const
{
   if (index < 0) {
      Print("Bounds error in FloatList(%08x) index=%d min=0\n", (int)this, index);
      index = 0;
   }

   else if (index >= items) {
      Print("Bounds error in FloatList(%08x) index=%d max=%d\n", (int)this,index, items-1);
      index = items-1;
   }

   return (index >= 0 && index < items);
}

// +-------------------------------------------------------------------+

float FloatList::operator[](int index) const
{
   if (check(index))
      return array[index];
   return 0;
}

float& FloatList::operator[](int index)
{
   if (check(index))
      return array[index];

   return array[0];
}

float FloatList::at(int index) const
{
   if (check(index))
      return array[index];
   return 0;
}

float& FloatList::at(int index)
{
   if (check(index))
      return array[index];

   return array[0];
}

// +-------------------------------------------------------------------+

void FloatList::resize(int newsize)
{
   if (newsize > extent) {
      extent = 16 * (newsize/16 + 1);

#ifdef MEM_DEBUG
      float* v = new(__FILE__,__LINE__) float[extent];
#else
      float* v = new float[extent];
#endif

      for (int i = 0; i < items; i++)
         v[i] = array[i];

      for (; i < extent; i++)
         v[i] = 0;

      delete [] array;   
      array = v;
   }
}

// +-------------------------------------------------------------------+

void FloatList::append(float item)
{
   if (items+1 > extent)
      resize(items+1);

   array[items++] = item;
}

void FloatList::append(const FloatList& list)
{
   if (&list != this && list.items > 0) {
      int need = items + list.items;
      if (need > extent)
         resize(need);

      for (int i = 0; i < list.items; i++)
         array[items++] = list.array[i];
   }
}

// +-------------------------------------------------------------------+

void FloatList::insert(float item, int index)
{
   if (index >= 0 && index <= items) {
      if (items+1 > extent)
         resize(items+1);

      // slide right:
      for (int i = items; i > index; i--)
         array[i] = array[i-1];

      array[index] = item;
      items++;
   }
}

// +-------------------------------------------------------------------+

void FloatList::insertSort(float item)
{
   for (int i = 0; i < items; i++) {
      if (item < array[i])
         break;
   }

   insert(item, i);
}

// +-------------------------------------------------------------------+

void FloatList::remove(float item)
{
   if (items < 1)
      return;

   for (int i = 0; i < items; i++) {
      if (array[i] == item) {
         removeIndex(i);
         return;
      }
   }
}

// +-------------------------------------------------------------------+

void FloatList::removeIndex(int index)
{
   if (items < 1 || !check(index))
      return;

   // slide left:
   for (int i = index; i < items-1; i++)
      array[i] = array[i+1];

   // blank out the hole we just created:
   array[items-1] = 0;

   items--;
}

// +-------------------------------------------------------------------+

bool FloatList::contains(float val) const
{
   for (int i = 0; i < items; i++) {
      if (array[i] == val)
         return true;
   }

   return false;
}

// +-------------------------------------------------------------------+

int FloatList::count(float val) const
{
   int c = 0;

   for (int i = 0; i < items; i++) {
      if (array[i] == val)
         c++;
   }

   return c;
}

// +-------------------------------------------------------------------+

int FloatList::index(float val) const
{
   for (int i = 0; i < items; i++) {
      if (array[i] == val)
         return i;
   }

   return -1;
}

// +-------------------------------------------------------------------+

void FloatList::swap(float* a, int i, int j)
{
   if (i >= 0 && i < items && j >= 0 && j < items && i != j) {
      float t = a[i];
      a[i] = a[j];
      a[j] = t;
   }
}

void FloatList::qsort(float* a, int lo0, int hi0)
{
   int lo = lo0;
   int hi = hi0;

   // zero or one element list, nothing to do:
   if (lo >= hi) {
      return;
   }

   // two element list, swap if needed:
   else if (lo == hi-1) {
      if (a[hi] < a[lo]) {
         swap(a, lo, hi);
      }
      return;
   }

   // pick a pivot, and move it out of the way:
   int   mid   = (lo+hi)/2;
   float pivot = a[mid];
   a[mid]      = a[hi];
   a[hi]       = pivot;

   while (lo < hi) {
      while ((a[lo] <= pivot) && lo < hi) lo++;
      while ((pivot <= a[hi]) && lo < hi) hi--;

      if (lo < hi) {
         swap(a, lo, hi);
      }
   }

   // Put the pivot into its final location:
   a[hi0] = a[hi];
   a[hi] = pivot;

	qsort(a, lo0, lo-1);
	qsort(a, hi+1, hi0);
}

void FloatList::sort()
{
   if (items < 2)
      return;

   qsort(array, 0, items-1);
}

void FloatList::shuffle()
{
   if (items < 3)
      return;

   for (int s = 0; s < 5; s++) {
      for (int i = 0; i < items; i++) {
         int j = (rand()>>4) % items;
         swap(array, i, j);
      }
   }
}


// +===================================================================+

float FloatListIter::value()
{
   if (list && step >= 0 && step < list->items)
      return list->array[step];

   return 0;
}

// +-------------------------------------------------------------------+

void FloatListIter::removeItem()
{
   if (list && step >= 0 && step < list->items)
      list->removeIndex(step--);
}

// +-------------------------------------------------------------------+

float FloatListIter::next()
{
   if (list && step >= -1 && step < list->items-1)
      return list->array[++step];

   return 0;
}

float FloatListIter::prev()
{
   if (list && step > 0 && step < list->items)
      return list->array[--step];

   return 0;
}

// +-------------------------------------------------------------------+

void FloatListIter::attach(FloatList& l)
{
   list = &l;
   step = -1;
}

// +-------------------------------------------------------------------+

int FloatListIter::size()
{
   if (!list) return 0;
   return list->items;
}

// +-------------------------------------------------------------------+

FloatList& FloatListIter::container()
{
   return *list;
}

