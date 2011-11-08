#ifndef __UTIL_INCLUDED__
#define __UTIL_INCLUDED__

#include <iostream>
#include <stdint.h>

#include "exception.h"

#ifdef CONFIG_DEBUG
#define ASSERT(val) \
  if(!(val)) { \
    std::cerr << std::endl << "Assertion failed at location: "; \
    std::cerr << __FILE__ << ":" << __LINE__ << std::endl << std::endl; \
    throw Ant::BugException(); \
  }
#else
#define ASSERT(val)
#endif

#define STRZ(arg) #arg

namespace Ant {

  // for old compilers (e.g. GCC 3.4)
  template <typename FPTR> inline void *funcPtrToVoidPtr(FPTR fptr) {
    void(*nfptr)() = reinterpret_cast<void(*)()>(fptr);
    intptr_t iptr = *reinterpret_cast<intptr_t*>(&nfptr);
    return reinterpret_cast<void*>(iptr);
  }

}

#endif // __UTIL_INCLUDED__
