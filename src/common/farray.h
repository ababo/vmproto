#ifndef __COMMON_FARRAY_INCLUDED__
#define __COMMON_FARRAY_INCLUDED__

#include <cstddef>

#include "exception.h"

namespace Ant {
  namespace Common {

    template<class T>
    class FixedArray {
    public:
      typedef const T &const_reference;
      typedef const T *const_iterator;
      typedef T &reference;
      typedef T *iterator;
      typedef size_t size_type;

      FixedArray() : dat(NULL), sz(0) {}
      FixedArray(T *data, size_type size) : dat(data), sz(size) {}

      void set(T *data, size_type size) { dat = data, sz = size; }

      size_type size() const { return sz; }

      const_iterator begin() const { return dat; }
      iterator begin() { return dat; }
      const_iterator end() const { return dat + sz; }
      iterator end() { return dat + sz; }

      reference at(size_type n) {
        if(n >= size)
          throw RangeException();
        return dat[n];
      }
      const_reference at(size_type n) const {
        if(n >= size)
          throw RangeException();
        return dat[n];
      }
      reference operator[](size_type n) { return dat[n]; }
      const_reference operator[](size_type n) const { return dat[n]; }

    protected:
      T *dat;
      size_type sz;
    };

  }
}

#endif // __COMMON_FARRAY_INCLUDED__
