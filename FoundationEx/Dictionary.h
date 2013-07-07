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
    FILE:         Dictionary.h
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Declaration of the Dictionary class
*/

#ifndef Dictionary_h
#define Dictionary_h

#include "Text.h"

// +-------------------------------------------------------------------+

template <class T> class Dictionary;
template <class T> class DictionaryIter;
template <class T> class DictionaryCell;

// +-------------------------------------------------------------------+

template <class T> class Dictionary
{
public:
   Dictionary();
   ~Dictionary();
   
   T&   operator[](const Text& key);

   void insert(const Text& key, const T& val);
   void remove(const Text& key);

   void clear();

   int  size() const         { return items; }
   int  isEmpty() const      { return !items; }
   
   int  contains(const Text& key)         const;
   T    find(const Text& key, T defval)   const;
   
private:
   void           init();

   int            items;
   
   typedef DictionaryCell<T>* PTR;
   PTR            table[256];
   
   friend class DictionaryIter<T>;
};

// +-------------------------------------------------------------------+

template <class T> class DictionaryIter
{
public:
   DictionaryIter(Dictionary<T>& l);
   ~DictionaryIter();

   int operator++();       // prefix

   void     reset();
   void     forth();

   Text     key()    const;
   T        value()  const;

   void           attach(Dictionary<T>& l);
   Dictionary<T>& container();

private:
   Dictionary<T>*       dict;
   DictionaryCell<T>*   here;
   int                  chain;
};

// +-------------------------------------------------------------------+

template <class T> class DictionaryCell
{
public:
   DictionaryCell(const Text& k)             : key(k), value( ), next(0) { }
   DictionaryCell(const Text& k, const T& v) : key(k), value(v), next(0) { }
   ~DictionaryCell() { }

   Text                 key;
   T                    value;
   DictionaryCell<T>*   next;
};

// +-------------------------------------------------------------------+

#include "Dictionary.inl"
#endif Dictionary_h

