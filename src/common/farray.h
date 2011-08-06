#ifndef __COMMON_FARRAY_INCLUDED__
#define __COMMON_FARRAY_INCLUDED__

#include "exception.h"

namespace Ant {
  namespace Common {

    template<class T>
    class FixedArray {
    public: 
      typedef T &reference;
      typedef const T &const_reference;
      typedef unsigned int size_type;

      FixedArray(T *data, size_type size) : dat(data), sz(size) {}

      size_type size() const { return sz; }

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
