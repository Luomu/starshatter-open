/*  Project FoundationEx
    Destroyer Studios LLC
    Copyright © 1997-2004. All Rights Reserved.

    SUBSYSTEM:    FoundationEx
    FILE:         List.inl
    AUTHOR:       John DiCamillo


    OVERVIEW
    ========
    Implementation of the List class template
*/

// +-------------------------------------------------------------------+

void Print(const char* fmt, ...);

// +-------------------------------------------------------------------+

template <class T>
List<T>::List(const List<T>& l)
  : items(l.items), extent(l.extent)
{
#ifdef MEM_DEBUG
   array = new(__FILE__,__LINE__) PTR[extent];
#else
   array = new PTR[extent];
#endif

   for (int i = 0; i < extent; i++)
      array[i] = l.array[i];
}

template <class T>
void List<T>::clear()
{
   delete [] array;
   items       = 0;
   extent  = 0;
   array       = 0;
}

template <class T>
void List<T>::destroy()
{
   if (items) {
      items = 0;  // prevent dangerous re-entrancy

      for (int i = 0; i < extent; i++)
         delete array[i];

      delete [] array;
      items       = 0;
      extent  = 0;
      array       = 0;
   }
}

// +-------------------------------------------------------------------+

template <class T>
bool List<T>::check(int& index) const
{
   if (index < 0) {
      Print("Bounds error in List(%08x) T=%s index=%d min=0\n", (int)this, T::TYPENAME(), index);
      index = 0;
   }

   else if (index >= items) {
      Print("Bounds error in List(%08x) T=%s index=%d max=%d\n", (int)this, T::TYPENAME(), index, items-1);
      index = items-1;
   }

   return (index >= 0 && index < items);
}

// +-------------------------------------------------------------------+

template <class T>
T*& List<T>::operator[](int index)
{
   if (check(index))
      return array[index];

   if (!array || !extent)
      resize(1);

   return array[0];
}

template <class T>
T* List<T>::operator[](int index) const
{
   if (check(index))
      return array[index];
   return 0;
}

template <class T>
T*& List<T>::at(int index)
{
   if (check(index))
      return array[index];

   if (!array || !extent)
      resize(1);

   return array[0];
}

template <class T>
T* List<T>::at(int index) const
{
   if (check(index))
      return array[index];
   return 0;
}

// +-------------------------------------------------------------------+

template <class T>
void List<T>::resize(int newsize)
{
   if (newsize > extent) {
      extent = 16 * (newsize/16 + 1);

#ifdef MEM_DEBUG
      T** v = new(__FILE__,__LINE__) PTR[extent];
#else
      T** v = new PTR[extent];
#endif
	  int i;
      for (i = 0; i < items; i++)
         v[i] = array[i];

      for (; i < extent; i++)
         v[i] = 0;

      delete [] array;   
      array = v;
   }
}

// +-------------------------------------------------------------------+

template <class T>
void List<T>::append(const T* item)
{
   if (item) {
      if (items+1 > extent) resize(items+1);
      array[items++] = (T*)item;
   }
}

template <class T>
void List<T>::append(List<T>& list)
{
   if (&list != this && list.items > 0) {
      int need = items + list.items;
      if (need > extent) resize(need);

      for (int i = 0; i < list.items; i++)
         array[items++] = list.array[i];
   }
}

// +-------------------------------------------------------------------+

template <class T>
void List<T>::insert(const T* item, int index)
{
   if (item && index >= 0 && index <= items) {
      if (items+1 > extent) resize(items+1);
      
      // slide right:
      for (int i = items; i > index; i--)
         array[i] = array[i-1];

      array[index] = (T*)item;
      items++;
   }
}

// +-------------------------------------------------------------------+

template <class T>
void List<T>::insertSort(const T* item)
{
	if (item) {
		int i;
		for (i = 0; i < items; i++) {
			if (*item < *array[i])
				break;
		}

		insert(item, i);
	}
}

// +-------------------------------------------------------------------+

template <class T>
T* List<T>::remove(const T* val)
{
   if (items == 0 || val == 0)
      return 0;

   for (int i = 0; i < items; i++) {
      if (array[i] == val) {
         return removeIndex(i);
      }
   }
   
   return 0;
}

// +-------------------------------------------------------------------+

template <class T>
T* List<T>::removeIndex(int index)
{
   if (!check(index))
      return 0;

   T* tmp = array[index];
   array[index] = 0;

   // slide left:
   for (int i = index; i < items-1; i++)
      array[i] = array[i+1];

   // blank out the hole we just created:
   array[items-1] = 0;

   items--;
   return tmp;
}

// +-------------------------------------------------------------------+

template <class T>
bool List<T>::contains(const T* val) const
{
   if (val) {
      if (index(val) != -1)
		  return true;
   }

   return false;
}

// +-------------------------------------------------------------------+

template <class T>
int List<T>::count(const T* val) const
{
   int c = 0;

   if (val) {
      for (int i = 0; i < items; i++) {
         if (array[i] && ((*array[i])==(*val)))
            c++;
      }
   }

   return c;
}

// +-------------------------------------------------------------------+

template <class T>
int List<T>::index(const T* val) const
{
   if (val) {
      for (int i = 0; i < items; i++) {
         if (array[i] && ((*array[i])==(*val)))
            return i;
      }
   }

   return -1;
}

// +-------------------------------------------------------------------+

template <class T>
T* List<T>::find(const T* val) const
{
   if (val) {
      for (int i = 0; i < items; i++) {
         if (array[i] && ((*array[i])==(*val)))
            return array[i];
      }
   }

   return 0;
}

// +-------------------------------------------------------------------+

template <class T>
void List<T>::swap(T** a, int i, int j)
{
   if (i >= 0 && i < items && j >= 0 && j < items && i != j) {
      T* t = a[i];
      a[i] = a[j];
      a[j] = t;
   }
}

template <class T>
void List<T>::qsort(T** a, int lo0, int hi0)
{
   int lo = lo0;
   int hi = hi0;

   // zero or one element list, nothing to do:
   if (lo >= hi) {
      return;
   }

   // two element list, swap if needed:
   else if (lo == hi-1) {
      if (*a[hi] < *a[lo]) {
         swap(a, lo, hi);
      }
      return;
   }

   // pick a pivot, and move it out of the way:
   int mid   = (lo+hi)/2;
   T*  pivot = a[mid];
   a[mid]    = a[hi];
   a[hi]     = pivot;

   while (lo < hi) {
      while ((*a[lo] <= *pivot) && lo < hi) lo++;
      while ((*pivot <= *a[hi]) && lo < hi) hi--;

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

template <class T>
void List<T>::sort()
{
   if (items < 2)
      return;

   qsort(array, 0, items-1);
}

template <class T>
void List<T>::shuffle()
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

template <class T>
T* ListIter<T>::value()
{
   if (list && step >= 0 && step < list->items)
      return list->array[step];

   return 0;
}

// +-------------------------------------------------------------------+

template <class T>
T* ListIter<T>::removeItem()
{
   if (list && step >= 0 && step < list->items)
      return list->removeIndex(step--);

   return 0;
}

// +-------------------------------------------------------------------+

template <class T>
T* ListIter<T>::next()
{
   if (list && step >= -1 && step < list->items-1)
      return list->array[++step];

   return 0;
}

template <class T>
T* ListIter<T>::prev()
{
   if (list && step > 0 && step < list->items)
      return list->array[--step];

   return 0;
}

// +-------------------------------------------------------------------+

template <class T>
void ListIter<T>::attach(List<T>& l)
{
   list = &l;
   step = -1;
}

// +-------------------------------------------------------------------+

template <class T>
int ListIter<T>::size()
{
   if (!list) return 0;
   return list->items;
}

// +-------------------------------------------------------------------+

template <class T>
List<T>& ListIter<T>::container()
{
   return *list;
}

