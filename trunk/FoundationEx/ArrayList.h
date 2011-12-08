/*  Project FoundationEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    FoundationEx
    FILE:         ArrayList.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Simple untyped array list
*/

#ifndef ArrayList_h
#define ArrayList_h

#ifdef WIN32
#include <windows.h>
#include <windowsx.h>
#endif

// +-------------------------------------------------------------------+

class ArrayList
{
public:
   ArrayList() : items(0), extent(0), array(0) { }
   ArrayList(const ArrayList& l);
   ~ArrayList()  { delete [] array; }

   DWORD    operator[](int i) const;
   DWORD&   operator[](int i);
   DWORD    at(int i) const;
   DWORD&   at(int i);

   void     append(const ArrayList& list);
   void     append(const DWORD val);
   void     insert(const DWORD val, int index=0);
   void     insertSort(DWORD val);

   DWORD    first()   const      { return operator[](0);       }
   DWORD    last()    const      { return operator[](items-1); }
   void     remove(DWORD val);
   void     removeIndex(int index);

   void     clear();

   int      size()    const      { return items;  }
   bool     isEmpty() const      { return !items; }

   bool     contains(DWORD val) const;
   int      count(DWORD val)    const;
   int      index(DWORD val)    const;

   void     sort();
   void     shuffle();

private:
   void     qsort(DWORD* a, int lo, int hi);
   void     swap(DWORD* a, int i, int j);
   void     resize(int newsize);
   bool     check(int& index) const;

   int      items;
   int      extent;
   DWORD*   array;

   friend class ArrayListIter;
};

// +-------------------------------------------------------------------+

class ArrayListIter
{
public:
   ArrayListIter()                       : list(0),      step(-1)      { }
   ArrayListIter(const ArrayListIter& i) : list(i.list), step(i.step)  { }
   ArrayListIter(ArrayList& l)           : list(&l),     step(-1)      { }

   int   operator++() { return next() != 0; }
   int   operator--() { return prev() != 0; }

   void        reset() { step = -1;          }
   DWORD       next();
   DWORD       prev();
   DWORD       value();
   void        removeItem();

   void        attach(ArrayList& l);
   ArrayList&  container();
   int         size();
   int         index() { return step; }

private:
   ArrayList*  list;
   int         step;
};


// +-------------------------------------------------------------------+
// +-------------------------------------------------------------------+
// +-------------------------------------------------------------------+

class FloatList
{
public:
   FloatList() : items(0), extent(0), array(0) { }
   FloatList(const FloatList& l);
   ~FloatList()  { delete [] array; }

   float    operator[](int i) const;
   float&   operator[](int i);
   float    at(int i) const;
   float&   at(int i);

   void     append(const FloatList& list);
   void     append(const float val);
   void     insert(const float val, int index=0);
   void     insertSort(float val);

   float    first()   const      { return operator[](0);       }
   float    last()    const      { return operator[](items-1); }
   void     remove(float val);
   void     removeIndex(int index);

   void     clear();

   int      size()    const      { return items;  }
   bool     isEmpty() const      { return !items; }

   bool     contains(float val) const;
   int      count(float val)    const;
   int      index(float val)    const;

   void     sort();
   void     shuffle();

private:
   void     qsort(float* a, int lo, int hi);
   void     swap(float* a, int i, int j);
   void     resize(int newsize);
   bool     check(int& index) const;

   int      items;
   int      extent;
   float*   array;

   friend class FloatListIter;
};

// +-------------------------------------------------------------------+

class FloatListIter
{
public:
   FloatListIter()                       : list(0),      step(-1)      { }
   FloatListIter(const FloatListIter& i) : list(i.list), step(i.step)  { }
   FloatListIter(FloatList& l)           : list(&l),     step(-1)      { }

   int   operator++() { return next() != 0; }
   int   operator--() { return prev() != 0; }

   void        reset() { step = -1;          }
   float       next();
   float       prev();
   float       value();
   void        removeItem();

   void        attach(FloatList& l);
   FloatList&  container();
   int         size();
   int         index() { return step; }

private:
   FloatList*  list;
   int         step;
};

#endif ArrayList_h

