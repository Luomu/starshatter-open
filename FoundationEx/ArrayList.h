/*  Project FoundationEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    FoundationEx
    FILE:         ArrayList.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Simple untyped array list

	The E: That is going to be removed very very soon. For now, it has been aliased to std::vector<DWORD>
	(or std::vector<float> for FloatList). Full conversion to std::vector is still needed.
*/

#ifndef ArrayList_h
#define ArrayList_h

#ifdef WIN32
#include <windows.h>
#include <windowsx.h>
#endif

#include <vector>

// +-------------------------------------------------------------------+

class ArrayList
{
public:
   ArrayList() { array.clear(); }
   ArrayList(const ArrayList& l);
   ~ArrayList()  { }

   DWORD    operator[](int i) const;
   DWORD&   operator[](int i);
   DWORD    at(int i) const;
   DWORD&   at(int i);

   void     append(const ArrayList& list);
   void     append(const DWORD val);
   void     insert(const DWORD val, int index=0);
   void     insertSort(DWORD val);

   DWORD    first()   const      { return *array.begin();       }
   DWORD    last()    const      { return array.back(); }
   void     remove(DWORD val);
   void     removeIndex(int index);

   void     clear();

   int      size()    const      { return array.size();  }
   bool     isEmpty() const      { return array.size() == 0; }

   bool     contains(DWORD val) const;
   int      count(DWORD val)    const;
   int      index(DWORD val)    const;

   void     sort();
   void     shuffle();

   bool     check(int& index) const;

private:
   void     swap(int i, int j);
   void     resize(int newsize);
   

   std::vector<DWORD>   array;

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

class FloatList : public ArrayList
{
public:
   FloatList() { array.clear(); }
   FloatList(const FloatList& l);
   ~FloatList()  {}

   float    operator[](int i) const;
   float&   operator[](int i);
   float    at(int i) const;
   float&   at(int i);

   void     append(const float val);
   void		append(const FloatList& list);
   void     insert(const float val, int index=0);
   void     insertSort(float val);

   float    first()   const      { return *array.begin();       }
   float    last()    const      { return array.back(); }
   void     remove(float val);

    bool     contains(float val) const;
   int      count(float val)    const;
   int      index(float val)    const;
  
private:

   std::vector<float>   array;

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

