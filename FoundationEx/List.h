/*  Project FoundationEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    FoundationEx
    FILE:         List.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Declaration of the List class template
*/

#ifndef List_h
#define List_h

// +-------------------------------------------------------------------+

template <class T> class List;
template <class T> class ListIter;

// +-------------------------------------------------------------------+

template <class T> class List
{
public:
   List()   : items(0), extent(0), array(0) { }
   List(const List<T>& l);
   ~List()  { delete [] array; }

   T*&      operator[](int i);
   T*       operator[](int i) const;
   T*&      at(int i);
   T*       at(int i) const;

   void     append(List<T>& list);
   void     append(const T* val);
   void     insert(const T* val, int index=0);
   void     insertSort(const T* val);

   T*       first()   const      { return operator[](0);       }
   T*       last()    const      { return operator[](items-1); }
   T*       remove(const T* val);
   T*       removeIndex(int index);

   void     clear();
   void     destroy();

   int      size()    const      { return items;  }
   bool     isEmpty() const      { return !items; }

   bool     contains(const T* val) const;
   int      count(const T* val)    const;
   int      index(const T* val)    const;
   T*       find(const T* val)     const;

   void     sort();
   void     shuffle();

private:
   typedef  T* PTR;
   void     qsort(T** a, int lo, int hi);
   void     resize(int newsize);
   bool     check(int& index) const;
   void     swap(T** a, int i, int j);

   int      items;
   int      extent;
   PTR*     array;

   friend class ListIter<T>;
};

// +-------------------------------------------------------------------+

template <class T> class ListIter
{
public:
   ListIter()                       : list(0),      step(-1)      { }
   ListIter(const ListIter<T>& i)   : list(i.list), step(i.step)  { }
   ListIter(List<T>& l)             : list(&l),     step(-1)      { }

   int operator++() { return next() != 0; }
   int operator--() { return prev() != 0; }
   T*  operator->() { return value();     }
   T&  operator* () { return *value();    }

   void     reset() { step = -1;          }
   T*       next();
   T*       prev();
   T*       value();
   T*       removeItem();

   void     attach(List<T>& l);
   List<T>& container();
   int      size();
   int      index() { return step; }

private:
   List<T>* list;
   int      step;
};

#include "List.inl"
#endif List_h

