/*  Starshatter OpenSource Distribution
    Copyright (c) 1997-2004, Destroyer Studios LLC.
    All Rights Reserved.

    Redistribution and use in source and binary forms, with or without
    modification, are permitted provided that the following conditions are met:

    * Redistributions of source code must retain the above copyright notice,
      this list of conditions and the following disclaimer.
    * Redistributions in binary form must reproduce the above copyright notice,
      this list of conditions and the following disclaimer in the documentation
      and/or other materials provided with the distribution.
    * Neither the name "Destroyer Studios" nor the names of its contributors
      may be used to endorse or promote products derived from this software
      without specific prior written permission.

    THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
    AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
    IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
    ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE
    LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
    CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF
    SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS
    INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN
    CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE)
    ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
    POSSIBILITY OF SUCH DAMAGE.

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

