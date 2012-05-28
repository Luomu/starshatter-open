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
#include <algorithm>

// +-------------------------------------------------------------------+

void Print(const char* fmt, ...);

// +-------------------------------------------------------------------+

ArrayList::ArrayList(const ArrayList& l)
{
	for (auto ali = l.array.begin(); ali != l.array.end(); ++ali)
		array.push_back(*ali);

}

void ArrayList::clear()
{
   array.clear();
}

// +-------------------------------------------------------------------+

bool ArrayList::check(int& index) const
{
	if (index > array.size()) {
      Print("Bounds error in ArrayList(%08x) index=%d min=0\n", (int)this, index);
      index = 0;
   }

   return (index >= 0 && index < array.size());
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
	array.resize(newsize);
}

// +-------------------------------------------------------------------+

void ArrayList::append(DWORD item)
{
	array.push_back(item);
}

void ArrayList::append(const ArrayList& list)
{
	for (auto li = list.array.begin(); li != list.array.end(); ++li)
		array.push_back(*li);
}

// +-------------------------------------------------------------------+

void ArrayList::insert(DWORD item, int index)
{
	auto it = array.begin();
	array.insert(it + index, item);
}

// +-------------------------------------------------------------------+

void ArrayList::insertSort(DWORD item)
{
	for (auto arrit = array.begin(); arrit != array.end(); arrit++) {
		if (*arrit < item) {
			array.insert(arrit, item);
			return;
		}
	}
}

// +-------------------------------------------------------------------+

void ArrayList::remove(DWORD item)
{
   if (array.size() < 1)
      return;

   for (auto it = array.begin(); it != array.end(); ++it) {
	   if (*it == item)
		   array.erase(it);
   }
}

// +-------------------------------------------------------------------+

void ArrayList::removeIndex(int index)
{
   if (array.size() < 1 || !check(index))
      return;

	array.erase(array.begin()+index);
}

// +-------------------------------------------------------------------+

bool ArrayList::contains(DWORD val) const
{
   for (auto it = array.begin(); it != array.end(); ++it) {
      if (*it == val)
         return true;
   }

   return false;
}

// +-------------------------------------------------------------------+

int ArrayList::count(DWORD val) const
{
   int c = 0;

   for (auto it = array.begin(); it != array.end(); ++it) {
      if (*it == val)
         c++;
   }

   return c;
}

// +-------------------------------------------------------------------+

int ArrayList::index(DWORD val) const
{
   for (size_t i = 0; i < array.size(); i++) {
      if (array[i] == val)
         return i;
   }

   return -1;
}

// +-------------------------------------------------------------------+

void ArrayList::swap(int i, int j)
{
   if (i >= 0 && i < array.size() && j >= 0 && j < array.size() && i != j) {
      DWORD t = array[i];
      array[i] = array[j];
      array[j] = t;
   }
}

void ArrayList::sort()
{
   if (array.size() < 2)
      return;

   std::sort(array.begin(), array.end());
}

void ArrayList::shuffle()
{
   if (array.size() < 3)
      return;

   for (int s = 0; s < 5; s++) {
      for (int i = 0; i < array.size(); i++) {
         int j = (rand()>>4) % array.size();
         swap(i, j);
      }
   }
}


// +===================================================================+

DWORD ArrayListIter::value()
{
   if (list && step >= 0)
      return list->array[step];

   return 0;
}

// +-------------------------------------------------------------------+

void ArrayListIter::removeItem()
{
   if (list && step >= 0)
      list->removeIndex(step--);
}

// +-------------------------------------------------------------------+

DWORD ArrayListIter::next()
{
   if (list && step >= -1)
      return list->array[++step];

   return 0;
}

DWORD ArrayListIter::prev()
{
   if (list && step > 0)
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
   return list->size();
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
{
	for (auto lit = l.array.begin(); lit != l.array.end(); lit++)
		array.push_back(*lit);
}

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

void FloatList::append(float value) {
	array.push_back(value);
}

void FloatList::append(const FloatList& list)
{
	for (auto li = list.array.begin(); li != list.array.end(); ++li)
		array.push_back(*li);
}


void FloatList::insert(float item, int index)
{
	auto it = array.begin();
	array.insert(it + index, item);
}

// +-------------------------------------------------------------------+

void FloatList::insertSort(float item)
{
	for (auto arrit = array.begin(); arrit != array.end(); arrit++) {
		if (*arrit < item) {
			array.insert(arrit, item);
			return;
		}
	}
}

// +-------------------------------------------------------------------+

void FloatList::remove(float item)
{
   if (array.size() < 1)
      return;

   for (auto it = array.begin(); it != array.end(); ++it) {
	   if (*it == item)
		   array.erase(it);
   }
}

// +===================================================================+

float FloatListIter::value()
{
   if (list && step >= 0)
      return list->array[step];

   return 0;
}

// +-------------------------------------------------------------------+

void FloatListIter::removeItem()
{
   if (list && step >= 0)
      list->removeIndex(step--);
}

// +-------------------------------------------------------------------+

float FloatListIter::next()
{
   if (list && step >= -1)
      return list->array[++step];

   return 0;
}

float FloatListIter::prev()
{
   if (list && step > 0)
      return list->array[--step];

   return 0;
}

bool FloatList::contains(float val) const
{
   for (auto it = array.begin(); it != array.end(); ++it) {
      if (*it == val)
         return true;
   }

   return false;
}

// +-------------------------------------------------------------------+

int FloatList::count(float val) const
{
   int c = 0;

   for (auto it = array.begin(); it != array.end(); ++it) {
      if (*it == val)
         c++;
   }

   return c;
}

// +-------------------------------------------------------------------+

int FloatList::index(float val) const
{
   for (size_t i = 0; i < array.size(); i++) {
      if (array[i] == val)
         return i;
   }

   return -1;
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
   return list->size();
}

// +-------------------------------------------------------------------+

FloatList& FloatListIter::container()
{
   return *list;
}

