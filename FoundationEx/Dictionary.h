/*  Project FoundationEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

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

